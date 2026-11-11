/*
 Slot-18: C4002 TARGET DISAPPEARANCE DELAY (hold time) CONTROL
 Candidate S18-C4002-DELAY-20261110-01 — NOT YET HARDWARE TESTED
 Waveshare ESP32-S3 Touch LCD7 + CJMCU-752 SC16IS752 (I2C 0x4D),
 GPIO8 SDA, GPIO9 SCL; UART A 115200 8N1 with 1.8432MHz oscillator.
 USB Serial Monitor through Waveshare CH343 UART USB-C port, 115200.
 Previous audited baseline: 15-second target disappearance delay.
 C4002 manufacturer DFRobot_C4002: cmd 0x84; READ sends dataLen4,
 WRITE sends dataLen6 and 2-byte little-endian delay in seconds.
 NO AUTOMATIC SENSOR WRITES. NO CALIBRATION OR RESET.
 Keep other proven Slot-18 continuous, audit, sensitivity and range
 control sketches untouched. This is a SEPARATE experimental sketch.
 Commands with Serial Monitor line ending Newline:
  STATUS              (only reads)
  PLAN DELAY 5        (propose 5 seconds)
  PLAN DELAY 15       (restore original 15 seconds)
  CONFIRM             (second explicit command, 60s expiry)
  CANCEL
  HELP
 Safe interactive pilot limits: 0..60 seconds. Manufacturer protocol
 nominally supports 0..65535; broader ranges intentionally not offered.
 Every PLAN reads current delay; CONFIRM re-reads, writes one setting,
 checks write ACK and re-reads to verify. If ACK is absent but READ
 succeeds, READ result governs and operator must inspect warnings.
 Don't claim this validates timing behavior until separate stopwatch tests.
 See docs/C4002_VERIFIED_CONFIGURATION_2026-11-10.md
*/
#include <Wire.h>
#include <string.h>
#include <stdio.h>

static const uint8_t BRIDGE=0x4D;
static const uint8_t REG_RHR=0,REG_THR=0,REG_IER=1,REG_FCR=2;
static const uint8_t REG_LCR=3,REG_MCR=4,REG_LSR=5;
static const uint8_t CMD_DISAPPEAR_DELAY=0x84;
static const uint32_t REPLY_TIMEOUT_MS=2500;
static const uint32_t TX_TIMEOUT_MS=250;
static const uint32_t PLAN_EXPIRY_MS=60000;
static bool ready=false,armed=false;
static uint16_t pendingOld=0,pendingNew=0;
static uint32_t planMs=0,validFrames=0,corruptFrames=0;
static uint32_t notifications=0,uartErrors=0,i2cErrors=0;
static uint8_t fr[128];
static size_t pos=0;
static uint16_t expectedLen=0;
static bool expecting=false,answered=false,replyGood=false;
static uint8_t expectedType=0;
static uint16_t replyDelay=0;
static char input[64];
static uint8_t inputPos=0;

uint16_t le16(const uint8_t *p) {
  return (uint16_t)p[0] | ((uint16_t)p[1]<<8);
}
uint8_t addr(uint8_t reg) { return (uint8_t)(reg<<3); }

bool wr(uint8_t reg,uint8_t value) {
  Wire.beginTransmission(BRIDGE);
  Wire.write(addr(reg));
  Wire.write(value);
  const bool ok=(Wire.endTransmission()==0);
  if(!ok) ++i2cErrors;
  return ok;
}
bool rd(uint8_t reg,uint8_t &value) {
  Wire.beginTransmission(BRIDGE);
  Wire.write(addr(reg));
  if(Wire.endTransmission(false)!=0) { ++i2cErrors; return false; }
  if(Wire.requestFrom((uint8_t)BRIDGE,(uint8_t)1,(uint8_t)true)!=1) {
    ++i2cErrors; return false;
  }
  value=(uint8_t)Wire.read();
  return true;
}
bool initBridge() {
  if(!wr(REG_IER,0x00) || !wr(REG_MCR,0x00) ||
     !wr(REG_LCR,0x80) || !wr(0,0x01) || !wr(1,0x00) ||
     !wr(REG_LCR,0x03) || !wr(REG_FCR,0x07)) return false;
  uint8_t v=0;
  return rd(REG_LCR,v) && v==0x03;
}



void decode() {
  if(expectedLen<14) { ++corruptFrames;return; }
  uint16_t checksum=0;
  for(size_t i=0;i<expectedLen-2;i++)
    checksum=(uint16_t)(checksum+fr[i]);
  const uint16_t dataLen=le16(&fr[10]);
  if(checksum!=le16(&fr[expectedLen-2]) ||
     dataLen<4 || (uint32_t)dataLen+10!=expectedLen) {
    ++corruptFrames;return;
  }
  ++validFrames;
  if(fr[7]==0x04) { ++notifications;return; }
  if(!expecting || fr[7]!=expectedType ||
     fr[8]!=CMD_DISAPPEAR_DELAY) return;
  if(fr[9]!=0x01) {
    Serial.printf("Sensor rejected DELAY request: status=%u\n",fr[9]);
    answered=true;return;
  }
  const size_t n=(size_t)dataLen-4;
  if(expectedType==0x03) {
    if(n!=2) {
      Serial.printf("Unexpected DELAY reply length=%u (expected 2)\n",
                    (unsigned)n);
      answered=true;return;
    }
    replyDelay=le16(&fr[12]);
  } else if(expectedType!=0x02) return;
  answered=true;
  replyGood=true;
}

void take(uint8_t b) {
  static const uint8_t magic[]={0xFA,0xF5,0xAA,0xA5};
  if(pos<4) {
    if(b==magic[pos]) fr[pos++]=b;
    else {
      pos=(b==magic[0])?1:0;
      if(pos) fr[0]=b;
    }
    return;
  }
  if(pos>=sizeof(fr)) { ++corruptFrames;pos=0;expectedLen=0;return; }
  fr[pos++]=b;
  if(pos==6) {
    expectedLen=le16(&fr[4]);
    if(expectedLen<14 || expectedLen>sizeof(fr)) {
      ++corruptFrames;pos=0;expectedLen=0;
    }
  }
  if(expectedLen!=0 && pos==expectedLen) {
    decode();
    pos=0;expectedLen=0;
  }
}

bool pump() {
  uint8_t flags=0;
  if(!rd(REG_LSR,flags)) return false;
  if(flags&0x1E) ++uartErrors;
  if(flags&0x01) {
    uint8_t b=0;
    if(!rd(REG_RHR,b)) return false;
    take(b);
  } else delay(1);
  return true;
}

bool txByte(uint8_t b) {
  const uint32_t start=millis();
  while((uint32_t)(millis()-start)<TX_TIMEOUT_MS) {
    uint8_t flags=0;
    if(!rd(REG_LSR,flags)) return false;
    if(flags&0x20) return wr(REG_THR,b);
    delay(1);
  }
  return false;
}



bool sendDelayFrame(uint8_t type,uint16_t seconds) {
  if(type!=0x00 && type!=0x01) return false;
  // Standard C4002 frame: 8-byte header, 4-byte data header,
  // optional 2-byte LE delay, and trailing 16-bit checksum.
  const uint8_t dataLen=(type==0x00)?6:4;
  const uint16_t n=(uint16_t)dataLen+10;
  uint8_t tx[16]={0xFA,0xF5,0xAA,0xA5,0,0,0,type,0x84,0,
                  dataLen,0,0,0,0,0};
  tx[4]=(uint8_t)(n&0xFF);
  tx[5]=(uint8_t)(n>>8);
  if(type==0x00) {
    tx[12]=(uint8_t)(seconds&0xFF);
    tx[13]=(uint8_t)(seconds>>8);
  }
  uint16_t sum=0;
  for(uint16_t i=0;i<n-2;i++)
    sum=(uint16_t)(sum+tx[i]);
  tx[n-2]=(uint8_t)(sum&0xFF);
  tx[n-1]=(uint8_t)(sum>>8);
  for(uint16_t i=0;i<n;i++) if(!txByte(tx[i])) return false;
  return true;
}
bool transact(uint8_t reqType,uint16_t seconds,uint16_t &out) {
  expecting=true;answered=false;replyGood=false;
  expectedType=reqType==0x01?0x03:0x02;
  if(!sendDelayFrame(reqType,seconds)) {
    expecting=false;
    Serial.println("ERROR: unable to send DELAY request.");
    return false;
  }
  const uint32_t started=millis();
  while(!answered && (uint32_t)(millis()-started)<REPLY_TIMEOUT_MS) {
    if(!pump()) {
      expecting=false;
      Serial.println("ERROR: bridge I2C read failed.");
      return false;
    }
  }
  expecting=false;
  if(!answered) {
    Serial.println("ERROR: DELAY response timed out (2.5 seconds).");
    return false;
  }
  if(!replyGood) return false;
  if(reqType==0x01) out=replyDelay;
  return true;
}
bool readDelay(uint16_t &out) {
  return transact(0x01,0,out);
}
bool writeDelay(uint16_t secs) {
  uint16_t unused=0;
  return transact(0x00,secs,unused);
}
void status() {
  armed=false;
  Serial.println("\nSTATUS: current C4002 target disappearance delay:");
  uint16_t sec=0;
  if(readDelay(sec)) Serial.printf("  Disappearance delay=%u seconds\n",sec);
  else Serial.println("  Disappearance delay=UNAVAILABLE (no valid sensor reply)");
  Serial.printf("  Link diagnostics: valid=%lu bad=%lu notices=%lu UART_errors=%lu I2C_errors=%lu\n",
    (unsigned long)validFrames,(unsigned long)corruptFrames,
    (unsigned long)notifications,(unsigned long)uartErrors,
    (unsigned long)i2cErrors);
  Serial.println("  Original verified delay=15 seconds.");
  Serial.println("  STATUS is READ ONLY; no settings changed.");
}
void help() {
  Serial.println("\nCOMMANDS (115200, Newline):");
  Serial.println("  STATUS");
  Serial.println("  PLAN DELAY 5          example 5-second hold");
  Serial.println("  PLAN DELAY 15         restore original 15-second hold");
  Serial.println("  CONFIRM               apply ONE pending write, within 60 seconds");
  Serial.println("  CANCEL                discard pending proposal");
  Serial.println("  HELP");
  Serial.println("No automatic radar writes on boot or while idle.");
}
void plan(uint16_t requested) {
  armed=false;
  if(requested>60) {
    Serial.println("REJECTED: this pilot only permits 0..60 seconds. No change.");
    return;
  }
  uint16_t current=0;
  if(!readDelay(current)) {
    Serial.println("PLAN ABORTED: current delay could not be verified.");
    return;
  }
  Serial.printf("CURRENT DELAY: %u seconds\n",current);
  if(current==requested) {
    Serial.println("NO CHANGE REQUIRED; no radar write.");
    return;
  }
  pendingOld=current;
  pendingNew=requested;
  planMs=millis();
  armed=true;
  Serial.printf("PROPOSED DELAY: %u -> %u seconds\n",current,requested);
  Serial.println("Type CONFIRM within 60 seconds for one setting change.");
  Serial.println("Or CANCEL to leave delay unchanged.");
}
void confirm() {
  if(!armed) {
    Serial.println("No pending proposal. Enter PLAN DELAY first.");
    return;
  }
  const uint16_t original=pendingOld,target=pendingNew;
  const uint32_t age=(uint32_t)(millis()-planMs);
  armed=false;
  if(age>PLAN_EXPIRY_MS) {
    Serial.println("PLAN EXPIRED. No radar setting changed.");
    return;
  }
  uint16_t current=0;
  if(!readDelay(current)) {
    Serial.println("CANCELLED: could not re-read current delay.");
    return;
  }
  if(current!=original) {
    Serial.println("CANCELLED: current delay changed since PLAN. Replan.");
    return;
  }
  Serial.printf("APPLYING DELAY %u -> %u seconds (one radar WRITE)...\n",
    original,target);
  const bool ack=writeDelay(target);
  Serial.println(ack?"WRITE ACK: radar reported success.":
                     "WRITE ACK FAILED/NOT RECEIVED. Delay state UNKNOWN.");
  uint16_t after=0;
  if(readDelay(after)) {
    Serial.printf("READBACK DELAY: %u seconds\n",after);
    if(after==target)
      Serial.println("VERIFIED: intended disappearance delay is active.");
    else Serial.println("WARNING: actual delay differs from requested setting.");
  } else Serial.println("WARNING: delay readback unavailable; state UNKNOWN.");
  Serial.println("Original delay: 15 seconds.");
  Serial.println("Restore with PLAN DELAY 15, then CONFIRM.");
}
void command(char *line) {
  if(armed && (uint32_t)(millis()-planMs)>PLAN_EXPIRY_MS) {
    armed=false;
    Serial.println("PLAN expired; no radar write.");
  }
  if(strcmp(line,"STATUS")==0) { status();return; }
  if(strcmp(line,"HELP")==0) { help();return; }
  if(strcmp(line,"CANCEL")==0) {
    armed=false;
    Serial.println("PLAN cancelled; no setting changed.");
    return;
  }
  if(strcmp(line,"CONFIRM")==0) { confirm();return; }
  if(strncmp(line,"PLAN DELAY ",11)==0) {
    unsigned long v=0;
    char extra='\0';
    if(sscanf(line+11,"%lu %c",&v,&extra)!=1 || v>60UL) {
      armed=false;
      Serial.println("REJECTED: use PLAN DELAY <seconds>, 0..60. No change.");
      return;
    }
    plan((uint16_t)v);
    return;
  }
  armed=false;
  Serial.println("Unrecognized command; no changes made. Type HELP.");
}
void setup() {
  Serial.begin(115200);
  delay(1600);
  Serial.println("\nSLOT-18 C4002 DELAY CONTROL — CONFIRM REQUIRED");
  Serial.println("No automatic C4002 writes; STATUS is read-only.");
  Wire.begin(8,9,100000);
  Wire.beginTransmission(BRIDGE);
  if(Wire.endTransmission()!=0 || !initBridge()) {
    Serial.println("STOP: CJMCU-752 bridge 0x4D/UART A not available.");
    return;
  }
  ready=true;
  Serial.println("Bridge OK; UART A 115200, 8N1.");
  const uint32_t begin=millis();
  while((uint32_t)(millis()-begin)<500) if(!pump()) break;
  status();
  help();
}
void loop() {
  if(!ready) { delay(1000);return; }
  if(!pump()) {
    Serial.println("WARNING: CJMCU-752 I2C read failed.");
    delay(500);
  }
  while(Serial.available()>0) {
    const int ch=Serial.read();
    if(ch=='\r'||ch=='\n') {
      if(inputPos>0) {
        input[inputPos]=0;
        for(uint8_t i=0;i<inputPos;i++)
          if(input[i]>='a'&&input[i]<='z') input[i]-=('a'-'A');
        while(inputPos>0 && input[inputPos-1]==' ')
          input[--inputPos]=0;
        if(inputPos>0) command(input);
        inputPos=0;
      }
    } else if(ch>=32&&ch<=126) {
      if(inputPos+1<sizeof(input)) input[inputPos++]=(char)ch;
      else {
        inputPos=0;armed=false;
        Serial.println("Command too long; no settings changed.");
      }
    }
  }
  if(armed && (uint32_t)(millis()-planMs)>PLAN_EXPIRY_MS) {
    armed=false;
    Serial.println("Pending PLAN expired; no settings changed.");
  }
}
