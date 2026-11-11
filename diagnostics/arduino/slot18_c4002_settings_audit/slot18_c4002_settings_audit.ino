/*
 Slot-18: C4002 SETTINGS AUDIT, READ-ONLY, ONE SHOT
 Candidate S18-C4002-AUDIT-20261110-01

 Waveshare ESP32-S3 Touch LCD 7 + CJMCU-752 SC16IS752 I2C/UART bridge
 SDA GPIO8, SCL GPIO9, bridge 0x4D, UART A 115200 8N1
 Physical CJMCU crystal marking: 1.8432 MHz, divisor 1.

 This sketch sends ONLY DFRobot FRAME_TYPE_READ_REQUEST (0x01) frames
 to the C4002. It never sends calibration, sensitivity setters, a
 factory reset, baud changes, or any other sensor-setting commands.
 Bridge initialization does write bridge UART registers (as in the
 previously verified continuous monitor); it does not configure radar.

 Queries (one at a time, bounded timeout):
   Software version, motion threshold group, presence threshold group,
   detection range, target disappearance delay, resolution mode.
 Unsolicited detection notifications are validated and counted.

 Keep the Waveshare red I2C connector VCC wire disconnected/insulated;
 use original tested power and ground wiring. Power OFF before rewiring.
 The existing continuous sketch is preserved separately on GitHub.

 Manufacturer: https://github.com/DFRobot/DFRobot_C4002
 Tested UART bridge baseline: slot18_c4002_continuous.ino
 STATUS: new candidate; not physically tested until user runs it.
*/
#include <Wire.h>

static const uint8_t BRIDGE=0x4D;
static const uint8_t REG_RHR=0, REG_THR=0, REG_IER=1, REG_FCR=2;
static const uint8_t REG_LCR=3, REG_MCR=4, REG_LSR=5;
static const uint32_t QUERY_TIMEOUT_MS=2500;
static const uint32_t TX_BYTE_TIMEOUT_MS=250;

enum QueryKind : uint8_t { VERSION, SENSITIVITY, RANGE, DELAY, RESOLUTION };
struct Query {
  const char *name;
  uint8_t cmd;
  QueryKind kind;
  uint8_t argCount;
  uint8_t arg0;
  uint8_t arg1;
};

static const Query QUERIES[] = {
  {"Software version",             0x82,VERSION,    1, 1,0},
  {"Motion threshold group",        0x87,SENSITIVITY,2, 0,0},
  {"Presence threshold group",      0x87,SENSITIVITY,2, 1,0},
  {"Detection range",               0x86,RANGE,      0, 0,0},
  {"Target disappearance delay",    0x84,DELAY,      0, 0,0},
  {"Range gate resolution",         0x66,RESOLUTION, 0, 0,0}
};

static uint8_t frame[128];
static size_t framePos=0;
static uint16_t frameLen=0;

static const Query *active=nullptr;
static bool answered=false, responseOK=false;
static uint32_t rxBytes=0, notifications=0, goodFrames=0, badFrames=0;
static uint32_t timeouts=0, i2cErrors=0, uartErrors=0;
static uint32_t settingsRead=0, settingsFailed=0;

uint8_t subaddr(uint8_t reg) { return (uint8_t)(reg<<3); } // UART A
uint16_t u16(const uint8_t *p) {
  return (uint16_t)p[0] | ((uint16_t)p[1]<<8);
}

bool writeReg(uint8_t reg,uint8_t value) {
  Wire.beginTransmission(BRIDGE);
  Wire.write(subaddr(reg));
  Wire.write(value);
  const bool ok=(Wire.endTransmission()==0);
  if(!ok) ++i2cErrors;
  return ok;
}

bool readReg(uint8_t reg,uint8_t &value) {
  Wire.beginTransmission(BRIDGE);
  Wire.write(subaddr(reg));
  if(Wire.endTransmission(false)!=0) { ++i2cErrors; return false; }
  if(Wire.requestFrom((uint8_t)BRIDGE,(uint8_t)1,(uint8_t)true)!=1) {
    ++i2cErrors; return false;
  }
  value=(uint8_t)Wire.read();
  return true;
}

bool configureBridge() {
  if(!writeReg(REG_IER,0x00) || !writeReg(REG_MCR,0x00) ||
     !writeReg(REG_LCR,0x80) ||
     !writeReg(0,0x01) || !writeReg(1,0x00) ||
     !writeReg(REG_LCR,0x03) ||
     !writeReg(REG_FCR,0x07)) return false;
  uint8_t check=0;
  if(!readReg(REG_LCR,check)) return false;
  return check==0x03;
}

const char *thresholdName(uint8_t value) {
  switch(value) {
    case 0: return "LOW threshold group";
    case 1: return "MID threshold group";
    case 2: return "HIGH threshold group";
    case 3: return "CUSTOM threshold group";
    default: return "UNKNOWN group";
  }
}

void printData(const uint8_t *p,size_t n) {
  for(size_t i=0;i<n;i++) Serial.printf("%02X ",p[i]);
  Serial.println();
}

void handleFrame() {
  uint16_t sum=0;
  for(size_t i=0;i<frameLen-2;i++) sum=(uint16_t)(sum+frame[i]);
  if(sum!=u16(&frame[frameLen-2])) {
    ++badFrames;
    Serial.println("RX ERROR: radar packet checksum mismatch.");
    return;
  }
  const uint16_t dataLen=u16(&frame[10]);
  if(dataLen<4 || (uint32_t)dataLen+10 != frameLen) {
    ++badFrames;
    Serial.println("RX ERROR: radar packet data length mismatch.");
    return;
  }
  ++goodFrames;
  if(frame[7]==0x04) { ++notifications; return; } // live reports
  if(frame[7]!=0x03) {
    Serial.printf("Other valid radar packet type=0x%02X cmd=0x%02X\n",
                  frame[7],frame[8]);
    return;
  }
  if(active==nullptr || frame[8]!=active->cmd) {
    Serial.printf("Unmatched READ response cmd=0x%02X, ignoring.\n",frame[8]);
    return;
  }
  const uint8_t *p=&frame[12];
  const size_t n=(size_t)dataLen-4;
  if(active->kind==SENSITIVITY && (n<2 || p[0]!=active->arg0)) {
    Serial.println("Ignored: sensitivity response is for a different gate.");
    return;
  }
  answered=true;
  Serial.printf("READ response command=0x%02X status=%u payload=%u bytes\n",
                frame[8],frame[9],(unsigned)n);
  Serial.print("  Raw payload: ");
  printData(p,n);
  if(frame[9]!=0x01) {
    Serial.println("  FAIL: radar did not report success.");
    return;
  }
  switch(active->kind) {
    case VERSION:
      if(n<1) return;
      Serial.print("  Firmware text: ");
      for(size_t i=0;i<n;i++) {
        if(p[i]>=32 && p[i]<=126) Serial.write(p[i]);
      }
      Serial.println();
      break;
    case SENSITIVITY:
      if(n<2) return;
      Serial.printf("  %s: group=%u (%s)\n",
                    active->arg0==0?"Motion":"Presence",
                    p[1],thresholdName(p[1]));
      break;
    case RANGE:
      if(n<4) return;
      Serial.printf("  Range: min=%u cm (%.2f m), max=%u cm (%.2f m)\n",
                    u16(p),(float)u16(p)/100.0f,
                    u16(p+2),(float)u16(p+2)/100.0f);
      break;
    case DELAY:
      if(n<2) return;
      Serial.printf("  Disappearance delay: %u seconds\n",u16(p));
      break;
    case RESOLUTION:
      if(n<1) return;
      Serial.printf("  Range gate resolution: code=%u (%s)\n",p[0],
                    p[0]==0?"80 cm":p[0]==1?"20 cm":"unknown");
      break;
  }
  responseOK=true;
}

void acceptByte(uint8_t b) {
  static const uint8_t header[4]={0xFA,0xF5,0xAA,0xA5};
  if(framePos<4) {
    if(b==header[framePos]) frame[framePos++]=b;
    else {
      framePos=(b==header[0])?1:0;
      if(framePos) frame[0]=b;
    }
    return;
  }
  if(framePos>=sizeof(frame)) {
    ++badFrames; framePos=0; frameLen=0; return;
  }
  frame[framePos++]=b;
  if(framePos==6) {
    frameLen=u16(&frame[4]);
    if(frameLen<14 || frameLen>sizeof(frame)) {
      ++badFrames; framePos=0; frameLen=0;
    }
  }
  if(frameLen!=0 && framePos==frameLen) {
    handleFrame();
    framePos=0; frameLen=0;
  }
}

bool pumpUart() {
  uint8_t lsr=0;
  if(!readReg(REG_LSR,lsr)) return false;
  if(lsr & 0x1E) ++uartErrors;  // overrun, parity, framing or break
  if(lsr & 0x01) {
    uint8_t b=0;
    if(!readReg(REG_RHR,b)) return false;
    ++rxBytes;
    acceptByte(b);
  } else delay(1);
  return true;
}

bool sendByte(uint8_t b) {
  const uint32_t start=millis();
  while((uint32_t)(millis()-start)<TX_BYTE_TIMEOUT_MS) {
    uint8_t lsr=0;
    if(!readReg(REG_LSR,lsr)) return false;
    if(lsr & 0x20) return writeReg(REG_THR,b);
    delay(1);
  }
  return false;
}

bool sendReadRequest(const Query &q) {
  // Header(8) + cmd/responseCode/fieldLength(4) + args + checksum(2).
  // Explicit allowlist: only these six vendor-supported READ operations.
  if(q.cmd!=0x82 && q.cmd!=0x87 && q.cmd!=0x86 &&
     q.cmd!=0x84 && q.cmd!=0x66) return false;
  const uint8_t count=(uint8_t)(4+q.argCount);
  const uint16_t n=(uint16_t)(count+10);
  uint8_t tx[18]={0xFA,0xF5,0xAA,0xA5,0,0,0,0x01};
  tx[4]=(uint8_t)(n&0xFF);
  tx[5]=(uint8_t)(n>>8);
  tx[8]=q.cmd;
  tx[9]=0x00; // read/write request marker, NOT a setting value
  tx[10]=count;
  tx[11]=0;
  if(q.argCount>0) tx[12]=q.arg0;
  if(q.argCount>1) tx[13]=q.arg1;
  uint16_t checksum=0;
  for(size_t i=0;i<n-2;i++) checksum=(uint16_t)(checksum+tx[i]);
  tx[n-2]=(uint8_t)(checksum&0xFF);
  tx[n-1]=(uint8_t)(checksum>>8);
  for(size_t i=0;i<n;i++) if(!sendByte(tx[i])) return false;
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1600);
  Serial.println("\nSLOT-18 C4002 READ-ONLY SETTINGS AUDIT (ONE SHOT)");
  Serial.println("No calibration, sensitivity write, reset or baud change.");
  Wire.begin(8,9,100000);
  Wire.beginTransmission(BRIDGE);
  if(Wire.endTransmission()!=0) {
    Serial.println("STOP: CJMCU-752 I2C 0x4D not found.");
    return;
  }
  if(!configureBridge()) {
    Serial.println("STOP: unable to configure bridge UART A.");
    return;
  }
  Serial.println("Bridge ACK; UART A 115200 8N1 divisor 1.");
  // Drain startup notifications. We must parse, not blindly flush
  // arbitrary half-frames into later response validation.
  const uint32_t settle=millis();
  while((uint32_t)(millis()-settle)<600) {
    if(!pumpUart()) {
      Serial.println("STOP: bridge read failed during initial settle.");
      return;
    }
  }
  const size_t total=sizeof(QUERIES)/sizeof(QUERIES[0]);
  for(size_t i=0;i<total;i++) {
    const Query &q=QUERIES[i];
    active=&q;
    answered=false;
    responseOK=false;
    Serial.printf("\nQUERY %u/%u: %s (READ only)\n",
                  (unsigned)(i+1),(unsigned)total,q.name);
    if(!sendReadRequest(q)) {
      Serial.println("STOP: radar READ request could not be transmitted.");
      ++settingsFailed;
      break;
    }
    const uint32_t start=millis();
    bool linkOK=true;
    while(!answered && (uint32_t)(millis()-start)<QUERY_TIMEOUT_MS) {
      if(!pumpUart()) { linkOK=false; break; }
    }
    if(!linkOK) {
      Serial.println("STOP: I2C/bridge communication failed while waiting.");
      ++settingsFailed;
      break;
    }
    if(!answered) {
      ++timeouts; ++settingsFailed;
      Serial.println("NO RESPONSE: valid matching reply not seen in 2.5s.");
    } else if(responseOK) {
      ++settingsRead;
      Serial.println("QUERY PASS: checksum-valid successful response decoded.");
    } else {
      ++settingsFailed;
      Serial.println("QUERY FAILED: sensor error or unsupported payload.");
    }
    active=nullptr;
    delay(100);
  }
  Serial.printf("\nAUDIT COMPLETE: good_settings=%lu failed_settings=%lu timeouts=%lu valid_frames=%lu checksum_or_length_errors=%lu notifications=%lu rx_bytes=%lu UART_error_flags=%lu I2C_errors=%lu\n",
                (unsigned long)settingsRead,(unsigned long)settingsFailed,
                (unsigned long)timeouts,(unsigned long)goodFrames,
                (unsigned long)badFrames,(unsigned long)notifications,
                (unsigned long)rxBytes,(unsigned long)uartErrors,
                (unsigned long)i2cErrors);
  Serial.println("READ ONLY: no C4002 settings were changed.");
  Serial.println("Copy the full Serial Monitor output. No automatic retry.");
}

void loop() { delay(60000); }
