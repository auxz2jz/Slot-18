/*
 Slot-18 C4002 SENSITIVITY CONTROL — OPERATOR CONFIRMATION REQUIRED
 Candidate S18-C4002-CTRL-20261110-02
 UNTESTED on hardware. No automatic sensor configuration writes.
 
 Previously user verified the independent READ-ONLY audit with:
  motion group 1/MID, presence group 1/MID,
  detection range 0..1100 cm, disappear hold 15 sec, 80cm gates.
 See docs/C4002_VERIFIED_CONFIGURATION_2026-11-10.md.

 Wiring unchanged from 17-minute verified Slot-18 continuous sketch:
 Waveshare ESP32-S3-Touch-LCD-7; GPIO8 SDA, GPIO9 SCL;
 CJMCU-752 SC16IS752 bridge I2C=0x4D, UART A 115200 8N1
 (1.8432MHz crystal, divisor 1). Sensor C4002 SEN0691 UART.
 Leave the Waveshare red I2C VCC wire disconnected/insulated.

 Commands in Arduino Serial Monitor 115200 (Newline enabled):
   STATUS
   PLAN MOTION LOW
   PLAN MOTION MID
   PLAN MOTION HIGH
   PLAN PRESENCE LOW
   PLAN PRESENCE MID
   PLAN PRESENCE HIGH
   CONFIRM                (required within 60s of PLAN)
   CANCEL
   HELP

 Every PLAN reads the sensor's CURRENT value, checks proposed change,
 then waits for CONFIRM. CONFIRM checks the current value again, writes
 ONE threshold group only after direct operator confirmation, and then
 reads back to verify. No automatic change on power-up/reset.
 No calibration, factory reset, baud changes, or range/time changes.
 Manual return to original: PLAN MOTION MID / PLAN PRESENCE MID,
 then CONFIRM separately for each. Do not assume sensor reset restores
 factory/original settings.

 This sketch is NOT a continuous radar display or range calibration.
 Keep verified slot18_c4002_continuous.ino and settings_audit.ino intact.
 Manufacturer protocol: https://github.com/DFRobot/DFRobot_C4002
*/
#include <Wire.h>
#include <string.h>

static const uint8_t BRIDGE=0x4D;
static const uint8_t REG_RHR=0, REG_THR=0, REG_IER=1, REG_FCR=2;
static const uint8_t REG_LCR=3, REG_MCR=4, REG_LSR=5;
static const uint8_t CMD_THRESHOLD_GROUP=0x87;
static const uint32_t REPLY_TIMEOUT_MS=2500;
static const uint32_t TX_TIMEOUT_MS=250;
static const uint32_t PLAN_EXPIRY_MS=60000;

static bool ready=false, armed=false;
static uint8_t pendingGate=0, pendingFrom=0, pendingTo=0;
static uint32_t planMs=0, validFrames=0, corruptFrames=0;
static uint32_t notifications=0, uartErrors=0, i2cErrors=0;
static uint8_t fr[128];
static size_t pos=0;
static uint16_t expectedLen=0;
static bool expecting=false, answered=false, replyGood=false;
static uint8_t expectedType=0, expectedGate=0, replyValue=0;
static char input[64];
static uint8_t inputPos=0;

const char *gateName(uint8_t g) { return g==0 ? "MOTION" : "PRESENCE"; }
const char *groupName(uint8_t n) {
  switch(n) {
    case 0: return "LOW";
    case 1: return "MID";
    case 2: return "HIGH";
    case 3: return "CUSTOM";
    default: return "UNKNOWN";
  }
}
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
  uint16_t check=0;
  for(size_t i=0;i<expectedLen-2;i++)
    check=(uint16_t)(check+fr[i]);
  const uint16_t dataLen=le16(&fr[10]);
  if(check!=le16(&fr[expectedLen-2]) || dataLen<4 ||
     (uint32_t)dataLen+10!=expectedLen) {
    ++corruptFrames; return;
  }
  ++validFrames;
  if(fr[7]==0x04) { ++notifications; return; } // unsolicited live report
  if(!expecting || fr[7]!=expectedType ||
     fr[8]!=CMD_THRESHOLD_GROUP) return;
  if(fr[9]!=0x01) {
    answered=true;
    Serial.printf("Sensor rejected request: status=%u\n",fr[9]);
    return;
  }
  const size_t n=(size_t)dataLen-4;
  if(expectedType==0x03) {
    // DFRobot READ reply contains [gate, thresholdGroup].
    if(n<2 || fr[12]!=expectedGate || fr[13]>3) {
      Serial.println("Unexpected READ reply payload/gate, ignored.");
      return;
    }
    replyValue=fr[13];
  } else if(expectedType==0x02) {
    // DFRobot WRITE ACK can have zero or more payload bytes.
    // Never rely on ACK alone: a separate READ follows.
    if(n>0 && fr[12]!=expectedGate) {
      Serial.println("WRITE ACK gate mismatch, ignored.");
      return;
    }
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

bool sendSensitivityFrame(uint8_t type,uint8_t gate,uint8_t group) {
  if(gate>1 || group>2) return false;
  // DFRobot: magic, length=16, sequence=0, frameType, cmd=0x87,
  // marker=0, payload length=6, gateType, thresholdGroup, sum16 LE.
  uint8_t bytes[16]={0xFA,0xF5,0xAA,0xA5,0x10,0x00,0x00,
                     type,0x87,0x00,0x06,0x00,gate,group,0,0};
  uint16_t sum=0;
  for(size_t i=0;i<14;i++) sum=(uint16_t)(sum+bytes[i]);
  bytes[14]=(uint8_t)(sum&0xFF);
  bytes[15]=(uint8_t)(sum>>8);
  for(uint8_t b:bytes) if(!txByte(b)) return false;
  return true;
}

bool query(uint8_t frameType,uint8_t gate,uint8_t group,uint8_t &readValue) {
  if(frameType!=0x01 && frameType!=0x00) return false;
  expecting=true;answered=false;replyGood=false;
  expectedType=frameType==0x01 ? 0x03 : 0x02;
  expectedGate=gate;
  // Read group is ignored by sensor; vendor read request uses byte 0.
  const bool sent=sendSensitivityFrame(frameType,gate,
                                      frameType==0x01 ? 0 : group);
  if(!sent) {
    expecting=false;
    Serial.println("ERROR: unable to transmit radar request.");
    return false;
  }
  const uint32_t start=millis();
  while(!answered && (uint32_t)(millis()-start)<REPLY_TIMEOUT_MS) {
    if(!pump()) {
      expecting=false;
      Serial.println("ERROR: bridge I2C read failed.");
      return false;
    }
  }
  expecting=false;
  if(!answered) {
    Serial.println("ERROR: radar reply timeout (2.5 seconds).");
    return false;
  }
  if(!replyGood) return false;
  if(frameType==0x01) readValue=replyValue;
  return true;
}
bool readSensitivity(uint8_t gate,uint8_t &val) {
  return query(0x01,gate,0,val);
}
bool setSensitivity(uint8_t gate,uint8_t value) {
  uint8_t unused=0;
  return query(0x00,gate,value,unused);
}

void reportStatus() {
  Serial.println("\nSTATUS: current radar sensitivity groups:");
  uint8_t m=255,p=255;
  const bool mok=readSensitivity(0,m);
  const bool pok=readSensitivity(1,p);
  if(mok) Serial.printf("  Motion  = %s (group %u)\n",groupName(m),m);
  else Serial.println("  Motion  = UNAVAILABLE (no confirmed response)");
  if(pok) Serial.printf("  Presence= %s (group %u)\n",groupName(p),p);
  else Serial.println("  Presence= UNAVAILABLE (no confirmed response)");
  Serial.printf("  Link diagnostics: valid_frames=%lu bad=%lu notices=%lu UART_errors=%lu I2C_errors=%lu\n",
    (unsigned long)validFrames,(unsigned long)corruptFrames,
    (unsigned long)notifications,(unsigned long)uartErrors,
    (unsigned long)i2cErrors);
  Serial.println("  Original hardware snapshot: MOTION MID, PRESENCE MID.");
  Serial.println("  NO sensor settings changed by STATUS.");
}

void help() {
  Serial.println("\nCOMMANDS (Serial Monitor 115200, Newline):");
  Serial.println("STATUS; PLAN MOTION LOW|MID|HIGH;");
  Serial.println("PLAN PRESENCE LOW|MID|HIGH; CONFIRM; CANCEL; HELP");
  Serial.println("PLAN does NOT change radar; CONFIRM performs ONE write after re-reading.");
  Serial.println("Restore original group with PLAN MOTION MID or PLAN PRESENCE MID.");
}

void plan(uint8_t gate,uint8_t next) {
  armed=false;
  uint8_t actual=255;
  if(!readSensitivity(gate,actual)) {
    Serial.println("PLAN ABORTED: current setting could not be verified.");
    return;
  }
  Serial.printf("CURRENT %s: %s (%u)\n",
                gateName(gate),groupName(actual),actual);
  if(actual==next) {
    Serial.println("NO CHANGE REQUIRED; settings not written.");
    return;
  }
  pendingGate=gate;
  pendingFrom=actual;
  pendingTo=next;
  planMs=millis();
  armed=true;
  Serial.printf("PROPOSED %s: %s -> %s\n",
                gateName(gate),groupName(actual),groupName(next));
  Serial.println("Type CONFIRM within 60s to change this ONE setting.");
  Serial.println("Or CANCEL to leave everything unchanged.");
}

void confirm() {
  if(!armed) {
    Serial.println("No pending change; enter PLAN first.");
    return;
  }
  const uint8_t gate=pendingGate;
  const uint8_t original=pendingFrom;
  const uint8_t intended=pendingTo;
  const uint32_t age=(uint32_t)(millis()-planMs);
  armed=false;
  if(age>PLAN_EXPIRY_MS) {
    Serial.println("PLAN EXPIRED. No settings changed.");
    return;
  }
  uint8_t current=255;
  if(!readSensitivity(gate,current)) {
    Serial.println("CANCELLED: current value could not be re-read.");
    return;
  }
  if(current!=original) {
    Serial.println("CANCELLED: value changed since PLAN; re-run PLAN.");
    return;
  }
  Serial.printf("APPLYING %s %s -> %s (single radar WRITE)...\n",
                gateName(gate),groupName(original),groupName(intended));
  const bool ack=setSensitivity(gate,intended);
  Serial.println(ack?"WRITE ACK: radar reported success.":
                     "WRITE ACK FAILED/NOT RECEIVED. Result is UNKNOWN.");
  uint8_t after=255;
  const bool readOK=readSensitivity(gate,after);
  if(readOK) {
    Serial.printf("READBACK %s: %s (%u)\n",
                  gateName(gate),groupName(after),after);
    if(after==intended) Serial.println("VERIFIED: intended setting is active.");
    else Serial.println("WARNING: intended setting NOT verified. Do not assume unchanged.");
  } else {
    Serial.println("WARNING: no verified readback. Actual setting UNKNOWN.");
  }
  Serial.println("Original hardware baseline is MID for each group.");
  Serial.println("To restore, PLAN this group MID, then CONFIRM separately.");
}

void command(char *line) {
  if(armed && (uint32_t)(millis()-planMs)>PLAN_EXPIRY_MS) {
    armed=false;
    Serial.println("Pending PLAN timed out; no write performed.");
  }
  if(strcmp(line,"HELP")==0) { help(); return; }
  if(strcmp(line,"CANCEL")==0) {
    armed=false;Serial.println("PLAN cancelled. No changes made.");return;
  }
  if(strcmp(line,"STATUS")==0) {
    armed=false;reportStatus();return;
  }
  if(strcmp(line,"CONFIRM")==0) { confirm(); return; }
  if(strncmp(line,"PLAN ",5)==0) {
    const char *p=line+5;
    uint8_t gate;
    if(strncmp(p,"MOTION ",7)==0) { gate=0;p+=7; }
    else if(strncmp(p,"PRESENCE ",9)==0) { gate=1;p+=9; }
    else { armed=false;Serial.println("Invalid gate. HELP for commands.");return; }
    uint8_t val;
    if(strcmp(p,"LOW")==0) val=0;
    else if(strcmp(p,"MID")==0) val=1;
    else if(strcmp(p,"HIGH")==0) val=2;
    else { armed=false;Serial.println("Invalid preset. HELP for commands.");return; }
    plan(gate,val);
    return;
  }
  armed=false;
  Serial.println("Unrecognized command; no changes made. Type HELP.");
}

void setup() {
  Serial.begin(115200);
  delay(1600);
  Serial.println("\nSLOT-18 C4002 SENSITIVITY CONTROL — CONFIRM REQUIRED");
  Serial.println("No automatic C4002 writes. STATUS is read-only.");
  Wire.begin(8,9,100000);
  Wire.beginTransmission(BRIDGE);
  if(Wire.endTransmission()!=0 || !initBridge()) {
    Serial.println("STOP: CJMCU-752 0x4D/UART A not available.");
    return;
  }
  ready=true;
  Serial.println("Bridge OK; UART A 115200, 8N1.");
  // Allow UART activity to settle before first config query.
  const uint32_t start=millis();
  while((uint32_t)(millis()-start)<500) if(!pump()) break;
  reportStatus();
  help();
}
void loop() {
  if(!ready) { delay(1000); return; }
  // Continuously drain unsolicited notifications, but do not print every one.
  if(!pump()) {
    Serial.println("WARNING: CJMCU-752 I2C read failed; commands will recheck.");
    delay(500);
  }
  while(Serial.available()>0) {
    const int c=Serial.read();
    if(c=='\r' || c=='\n') {
      if(inputPos>0) {
        input[inputPos]=0;
        // Be forgiving of lower case and extra whitespace.
        for(uint8_t i=0;i<inputPos;i++)
          if(input[i]>='a' && input[i]<='z') input[i]-=('a'-'A');
        while(inputPos>0 && input[inputPos-1]==' ')
          input[--inputPos]=0;
        if(inputPos>0) command(input);
        inputPos=0;
      }
    } else if(c>=32 && c<=126) {
      if(inputPos+1<sizeof(input)) input[inputPos++]=(char)c;
      else {
        inputPos=0;
        armed=false;
        Serial.println("Input too long. No change was made.");
      }
    }
  }
  if(armed && (uint32_t)(millis()-planMs)>PLAN_EXPIRY_MS) {
    armed=false;
    Serial.println("PLAN expired. No setting was changed.");
  }
}
