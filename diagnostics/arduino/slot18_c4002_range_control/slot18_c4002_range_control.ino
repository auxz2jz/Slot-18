/*
 Slot-18 C4002 DETECTION RANGE CONTROL — EXPLICIT CONFIRM REQUIRED
 Candidate S18-C4002-RANGE-20261110-01
 Hardware: Waveshare ESP32-S3-Touch-LCD-7 + CJMCU-752 (SC16IS752),
 I2C 0x4D, GPIO8 SDA, GPIO9 SCL, UART A 115200 8N1,
 1.8432 MHz oscillator/divisor=1. USB Serial Monitor through CH343 UART port.

 THIS SKETCH IS A SEPARATE EXPERIMENTAL CANDIDATE. Never edit, erase
 or overwrite the user's known-good continuous monitor, settings audit,
 or sensitivity control sketch in GitHub.

 Original USER-VERIFIED baseline (READ ONLY audit):
   Min distance=0 cm, max distance=1100 cm.
   Motion sensitivity=MID (1); Presence sensitivity=MID (1).
   Target disappear delay=15 sec; resolution=80cm (code 0).
 Original config recorded in:
   docs/C4002_VERIFIED_CONFIGURATION_2026-11-10.md

 Arduino Serial Monitor: 115200, Newline line-ending.
 Commands:
   STATUS                (READ ONLY)
   PLAN RANGE 0 500      (propose max range 5m, NO WRITE)
   PLAN RANGE 100 500    (propose min 1m, max 5m, NO WRITE)
   PLAN RANGE 0 1100     (propose restoring original range)
   CONFIRM               (only within 60 sec after PLAN)
   CANCEL
   HELP

 NOTE: C4002 range setting is 0..1100cm; min must be less than max.
 On boot and STATUS, only READ requests are sent to the C4002.
 On PLAN: READ original and arm. On CONFIRM: re-READ and guard that the
 value is unchanged, write ONE paired min/max range command and then
 READ back for verification. No automatic radar writes, calibration,
 factory reset, baud changes, or sensitivity modifications.
 If ACK is missed, actual state is UNKNOWN until validated READBACK.
 Restore to baseline only with explicit PLAN RANGE 0 1100 / CONFIRM.
 Range controls are only hardware-verified AFTER user executes them;
 this candidate is source-inspected but not yet Arduino-compiled/ran.
 Manufacturer: https://github.com/DFRobot/DFRobot_C4002
*/
#include <Wire.h>
#include <string.h>
#include <stdio.h>

static const uint8_t BRIDGE=0x4D;
static const uint8_t REG_RHR=0,REG_THR=0,REG_IER=1,REG_FCR=2;
static const uint8_t REG_LCR=3,REG_MCR=4,REG_LSR=5;
static const uint8_t CMD_DETECT_RANGE=0x86;
static const uint32_t REPLY_TIMEOUT_MS=2500;
static const uint32_t TX_TIMEOUT_MS=250;
static const uint32_t PLAN_EXPIRY_MS=60000;
static bool ready=false, armed=false;
static uint16_t pendingOldMin=0,pendingOldMax=0,pendingNewMin=0,pendingNewMax=0;
static uint32_t planMs=0,validFrames=0,corruptFrames=0;
static uint32_t notifications=0,uartErrors=0,i2cErrors=0;
static uint8_t fr[128];
static size_t pos=0;
static uint16_t expectedLen=0;
static bool expecting=false,answered=false,replyGood=false;
static uint8_t expectedType=0;
static uint16_t replyMin=0,replyMax=0;
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
  const uint16_t datalen=le16(&fr[10]);
  if(checksum!=le16(&fr[expectedLen-2]) ||
     datalen<4 || (uint32_t)datalen+10!=expectedLen) {
    ++corruptFrames;
    return;
  }
  ++validFrames;
  if(fr[7]==0x04) { ++notifications; return; } // Radar notifications
  if(!expecting || fr[7]!=expectedType || fr[8]!=CMD_DETECT_RANGE)
    return;

  if(fr[9]!=0x01) {
    answered=true;
    Serial.printf("Sensor rejected RANGE request: status=%u\n",fr[9]);
    return;
  }
  const size_t n=(size_t)datalen-4;
  if(expectedType==0x03) {
    if(n!=4) {
      Serial.printf("Unexpected RANGE read response size: %u (expected 4)\n",
                    (unsigned)n);
      answered=true;
      return;
    }
    const uint16_t lo=le16(&fr[12]);
    const uint16_t hi=le16(&fr[14]);
    if(lo>=hi || hi>1100) {
      Serial.printf("INVALID RANGE reply: min=%u max=%u cm; ignored.\n",lo,hi);
      answered=true;
      return;
    }
    replyMin=lo;
    replyMax=hi;
  } else if(expectedType==0x02) {
    // Successful WRITE ACK. Data is only verified by another READ.
  } else return;
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


bool sendRangeFrame(uint8_t type,uint16_t minCm,uint16_t maxCm) {
  if(type!=0x00 && type!=0x01) return false;
  if(type==0x00 && (minCm>=maxCm || maxCm>1100)) return false;
  // Vendor request protocol:
  // read: [cmd=0x86, marker=0, dataLen=4] and no payload
  // write: [cmd=0x86, marker=0, dataLen=8,
  //         min_lo,min_hi,max_lo,max_hi]
  const uint8_t dataLength=(type==0x00)?8:4;
  const uint16_t totalLength=(uint16_t)dataLength+10;
  uint8_t tx[18]={0xFA,0xF5,0xAA,0xA5,0,0,0,type,0x86,0,
                  dataLength,0,0,0,0,0,0,0};
  tx[4]=(uint8_t)(totalLength&0xFF);
  tx[5]=(uint8_t)(totalLength>>8);
  if(type==0x00) {
    tx[12]=(uint8_t)(minCm&0xFF);
    tx[13]=(uint8_t)(minCm>>8);
    tx[14]=(uint8_t)(maxCm&0xFF);
    tx[15]=(uint8_t)(maxCm>>8);
  }
  uint16_t checksum=0;
  for(uint16_t i=0;i<totalLength-2;i++)
    checksum=(uint16_t)(checksum+tx[i]);
  tx[totalLength-2]=(uint8_t)(checksum&0xFF);
  tx[totalLength-1]=(uint8_t)(checksum>>8);
  for(uint16_t i=0;i<totalLength;i++) if(!txByte(tx[i])) return false;
  return true;
}

bool transact(uint8_t requestType,uint16_t lo,uint16_t hi,
              uint16_t &readMin,uint16_t &readMax) {
  if(requestType!=0x00 && requestType!=0x01) return false;
  expecting=true;answered=false;replyGood=false;
  expectedType=requestType==0x00?0x02:0x03;
  if(!sendRangeFrame(requestType,lo,hi)) {
    expecting=false;
    Serial.println("ERROR: unable to transmit radar RANGE frame.");
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
    Serial.println("ERROR: RANGE reply timeout after 2.5 seconds.");
    return false;
  }
  if(!replyGood) return false;
  if(requestType==0x01) {
    readMin=replyMin;
    readMax=replyMax;
  }
  return true;
}

bool readRange(uint16_t &lo,uint16_t &hi) {
  return transact(0x01,0,0,lo,hi);
}
bool writeRange(uint16_t lo,uint16_t hi) {
  uint16_t unusedLo=0,unusedHi=0;
  return transact(0x00,lo,hi,unusedLo,unusedHi);
}
void status() {
  armed=false;
  Serial.println("\nSTATUS: current C4002 radar detection range:");
  uint16_t lo=0,hi=0;
  if(readRange(lo,hi)) {
    Serial.printf("  Range min=%u cm (%.2f m), max=%u cm (%.2f m)\n",
                  lo,(float)lo/100.0f,hi,(float)hi/100.0f);
  } else Serial.println("  Range=UNAVAILABLE (no confirmed C4002 reply)");
  Serial.printf("  Link diagnostics: valid=%lu bad=%lu notices=%lu UART_errors=%lu I2C_errors=%lu\n",
                (unsigned long)validFrames,(unsigned long)corruptFrames,
                (unsigned long)notifications,(unsigned long)uartErrors,
                (unsigned long)i2cErrors);
  Serial.println("  Original verified range=0..1100 cm.");
  Serial.println("  STATUS is READ ONLY; no settings changed.");
}
void help() {
  Serial.println("\nCOMMANDS (Serial Monitor 115200, Newline):");
  Serial.println("  STATUS");
  Serial.println("  PLAN RANGE <min_cm> <max_cm>   e.g. PLAN RANGE 0 500");
  Serial.println("  PLAN RANGE 0 1100             restore original after testing");
  Serial.println("  CONFIRM                       apply one pending range write");
  Serial.println("  CANCEL                        discard planned change");
  Serial.println("  HELP");
  Serial.println("PLAN never writes. CONFIRM expires 60s after PLAN.");
}
void plan(uint16_t wantMin,uint16_t wantMax) {
  armed=false;
  if(wantMin>=wantMax || wantMax>1100) {
    Serial.println("REJECTED: require 0<=min<max<=1100 cm. No change.");
    return;
  }
  uint16_t fromMin=0,fromMax=0;
  if(!readRange(fromMin,fromMax)) {
    Serial.println("PLAN ABORTED: cannot read existing C4002 range.");
    return;
  }
  Serial.printf("CURRENT RANGE: %u..%u cm\n",fromMin,fromMax);
  if(fromMin==wantMin && fromMax==wantMax) {
    Serial.println("NO CHANGE REQUIRED; no sensor write.");
    return;
  }
  pendingOldMin=fromMin;
  pendingOldMax=fromMax;
  pendingNewMin=wantMin;
  pendingNewMax=wantMax;
  planMs=millis();
  armed=true;
  Serial.printf("PROPOSED RANGE: %u..%u -> %u..%u cm\n",
    fromMin,fromMax,wantMin,wantMax);
  Serial.println("Type CONFIRM within 60 seconds to change this setting.");
  Serial.println("Or CANCEL to leave current range unchanged.");
}
void confirm() {
  if(!armed) {
    Serial.println("No pending range change; enter PLAN RANGE first.");
    return;
  }
  const uint16_t oldMin=pendingOldMin,oldMax=pendingOldMax;
  const uint16_t newMin=pendingNewMin,newMax=pendingNewMax;
  const uint32_t age=(uint32_t)(millis()-planMs);
  armed=false;
  if(age>PLAN_EXPIRY_MS) {
    Serial.println("PLAN EXPIRED; no change.");
    return;
  }
  uint16_t actualMin=0,actualMax=0;
  if(!readRange(actualMin,actualMax)) {
    Serial.println("CANCELLED: could not re-read current range.");
    return;
  }
  if(actualMin!=oldMin || actualMax!=oldMax) {
    Serial.println("CANCELLED: current range differs from PLAN; replan.");
    return;
  }
  Serial.printf("APPLYING RANGE %u..%u -> %u..%u cm (one radar WRITE)...\n",
    oldMin,oldMax,newMin,newMax);
  const bool ack=writeRange(newMin,newMax);
  Serial.println(ack?"WRITE ACK: radar reported success.":
                     "WRITE ACK FAILED/NOT RECEIVED. Actual range UNKNOWN.");
  uint16_t afterMin=0,afterMax=0;
  const bool checked=readRange(afterMin,afterMax);
  if(checked) {
    Serial.printf("READBACK RANGE: %u..%u cm\n",afterMin,afterMax);
    if(afterMin==newMin && afterMax==newMax)
      Serial.println("VERIFIED: intended detection range is active.");
    else Serial.println("WARNING: range did NOT match intended target.");
  } else Serial.println("WARNING: readback failed; actual range UNKNOWN.");
  Serial.println("Original hardware range is 0..1100 cm.");
  Serial.println("To restore: PLAN RANGE 0 1100, then CONFIRM.");
}
void command(char *line) {
  if(armed && (uint32_t)(millis()-planMs)>PLAN_EXPIRY_MS) {
    armed=false;
    Serial.println("Pending PLAN expired with no write.");
  }
  if(strcmp(line,"STATUS")==0) { status();return; }
  if(strcmp(line,"HELP")==0) { help();return; }
  if(strcmp(line,"CANCEL")==0) {
    armed=false;
    Serial.println("Pending PLAN cancelled; no setting changed.");
    return;
  }
  if(strcmp(line,"CONFIRM")==0) { confirm();return; }
  if(strncmp(line,"PLAN RANGE ",11)==0) {
    unsigned long a=0,b=0;
    char extra='\0';
    if(sscanf(line+11,"%lu %lu %c",&a,&b,&extra)!=2 ||
       a>1100UL || b>1100UL) {
      armed=false;
      Serial.println("REJECTED: use PLAN RANGE <min_cm> <max_cm>; 0..1100.");
      return;
    }
    plan((uint16_t)a,(uint16_t)b);
    return;
  }
  armed=false;
  Serial.println("Unrecognized command. Type HELP; no setting changed.");
}
void setup() {
  Serial.begin(115200);
  delay(1600);
  Serial.println("\nSLOT-18 C4002 RANGE CONTROL — CONFIRM REQUIRED");
  Serial.println("No automatic C4002 writes. STATUS is read-only.");
  Wire.begin(8,9,100000);
  Wire.beginTransmission(BRIDGE);
  if(Wire.endTransmission()!=0 || !initBridge()) {
    Serial.println("STOP: CJMCU-752 0x4D/UART A not available.");
    return;
  }
  ready=true;
  Serial.println("Bridge OK; UART A 115200, 8N1.");
  const uint32_t begin=millis();
  while((uint32_t)(millis()-begin)<500)
    if(!pump()) break;
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
    int ch=Serial.read();
    if(ch=='\r' || ch=='\n') {
      if(inputPos>0) {
        input[inputPos]=0;
        for(uint8_t i=0;i<inputPos;i++)
          if(input[i]>='a' && input[i]<='z') input[i]-=('a'-'A');
        while(inputPos>0 && input[inputPos-1]==' ')
          input[--inputPos]=0;
        if(inputPos>0) command(input);
        inputPos=0;
      }
    } else if(ch>=32 && ch<=126) {
      if(inputPos+1<sizeof(input)) input[inputPos++]=(char)ch;
      else {
        inputPos=0;
        armed=false;
        Serial.println("Input too long; no setting changed.");
      }
    }
  }
  if(armed && (uint32_t)(millis()-planMs)>PLAN_EXPIRY_MS) {
    armed=false;
    Serial.println("Pending PLAN expired with no write.");
  }
}
