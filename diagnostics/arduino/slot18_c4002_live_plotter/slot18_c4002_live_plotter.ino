/*
 Slot-18 C4002 LIVE RADAR PLOTTER — READ ONLY, VISUAL DEMO v1
 Candidate S18-PLOTTER-20261110-01
 User-visible result: Arduino IDE > Tools > Serial Plotter (115200)
 displays LIVE distance traces in meters, reported radar detection state,
 and incoming-data heartbeat. No commands or sensor setting writes.

 Verified hardware interface (from continuous sketch):
 Waveshare ESP32-S3 Touch LCD 7 + CJMCU-752 SC16IS752
 I2C 0x4D, SDA GPIO8, SCL GPIO9, UART A 115200 8N1, 1.8432MHz
 crystal/divisor 1. Use Waveshare USB-C port labeled UART (CH343).
 Keep existing wiring unchanged. Do not connect red Waveshare I2C
 VCC wire to the external breadboard power rail.

 IMPORTANT: This program plots on the COMPUTER via Arduino Serial
 Plotter, not yet on the Waveshare 7-inch integrated LCD.
 DFRobot C4002 outputs presence/motion distances, not XY positions.
 Changes in traces are not proof of detecting a specific person.
 Nearby metal, pets and clutter can trigger sensor returns.
 These traces are NOT a guaranteed physical measuring tape.
 On NO_TARGET, plots distances as ZERO (avoids stale range data).
 If radar notifications are absent for >5 s, Data_OK drops to zero.
 Never sends a C4002 configuration frame; only bridge UART
 initialization writes to SC16IS752 registers.

 Graph curves (single 0..11m-ish graph):
  Presence_m: reported presence distance in meters (0 when not valid)
  Motion_m: reported motion distance in meters (0 when not valid)
  Radar_state: 0=no-target, 5=presence report, 10=motion report
  Data_OK: 11=recent packets, 0=none for >5 sec
 Output once per 500ms. One capture can show moving closer/farther.

 Source derived from user's previously hardware-verified diagnostic:
 diagnostics/arduino/slot18_c4002_continuous/
   slot18_c4002_continuous.ino
 Original firmware and all other verified tests remain untouched.
 This visual demo source has NOT yet been uploaded or user-hardware tested.
*/
#include <Wire.h>

static const uint8_t BRIDGE=0x4D;
static const uint8_t REG_RHR=0, REG_IER=1, REG_FCR=2;
static const uint8_t REG_LCR=3, REG_MCR=4, REG_LSR=5;
static bool ready=false;
static uint8_t frame[128];
static size_t pos=0;
static uint16_t expected=0;
static uint32_t valid=0, invalid=0, results=0, rxCount=0;
static uint32_t overrun=0, parity=0, framing=0, breaks=0, i2cErrors=0;
static uint32_t lastDetectionMs=0, lastGraphMs=0;
static uint8_t reportedState=0;
static float presenceM=0.0f, motionM=0.0f;

uint8_t addr(uint8_t reg) { return (uint8_t)(reg<<3); } // channel A=0

bool writeReg(uint8_t r, uint8_t v) {
  Wire.beginTransmission(BRIDGE);
  Wire.write(addr(r));
  Wire.write(v);
  return Wire.endTransmission()==0;
}
bool readReg(uint8_t r, uint8_t &v) {
  Wire.beginTransmission(BRIDGE);
  Wire.write(addr(r));
  if(Wire.endTransmission(false)!=0) return false;
  if(Wire.requestFrom((uint8_t)BRIDGE,(uint8_t)1,(uint8_t)true)!=1) return false;
  v=(uint8_t)Wire.read();
  return true;
}
uint16_t u16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1]<<8); }
uint32_t u32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24); }
int16_t s16(const uint8_t *p) { return (int16_t)u16(p); }

const char *targetName(uint8_t state) {
  switch (state) {
    case 0: return "NO_TARGET";
    case 1: return "PRESENCE";
    case 2: return "MOTION";
    case 3: return "MOTION_OR_PRESENCE";
    case 4: return "MOTION_OR_NO_TARGET";
    case 5: return "PRESENCE_OR_NO_TARGET";
    default: return "OTHER";
  }
}


void decodeFrame() {
  if(expected<14) { ++invalid;return; }
  uint16_t checksum=0;
  for(size_t i=0;i<(size_t)expected-2;i++)
    checksum=(uint16_t)(checksum+frame[i]);
  if(checksum!=u16(&frame[expected-2])) {
    ++invalid;return;
  }
  ++valid;
  if(expected!=32 || frame[7]!=0x04 || frame[8]!=0x60 ||
     frame[9]!=0x01 || u16(&frame[10])!=22) return;

  const uint8_t *p=&frame[12];
  ++results;
  lastDetectionMs=millis();
  const uint8_t state=p[0];
  reportedState=state;
  // Ignore distances after NO_TARGET to avoid displaying stale fields.
  if(state==0 || state==255) {
    presenceM=0.0f;
    motionM=0.0f;
    return;
  }
  const uint16_t presenceCm=u16(&p[9]);
  const uint16_t motionCm=u16(&p[12]);
  // Energy is only an indicator, not a percent confidence.
  presenceM=(p[11]!=0 && presenceCm<=1100)
      ?(float)presenceCm/100.0f:0.0f;
  motionM=(p[16]!=0 && motionCm<=1100)
      ?(float)motionCm/100.0f:0.0f;
}

void appendByte(uint8_t b) {
  // Initial synchronization to FA F5 AA A5.
  static const uint8_t magic[]={0xFA,0xF5,0xAA,0xA5};
  if(pos<4) {
    if(b==magic[pos]) frame[pos++]=b;
    else {
      pos=(b==magic[0])?1:0;
      if(pos) frame[0]=b;
    }
    return;
  }
  if(pos>=sizeof(frame)) { pos=0; expected=0; ++invalid; return; }
  frame[pos++]=b;
  if(pos==6) {
    expected=u16(&frame[4]);
    if(expected<10 || expected>sizeof(frame)) {
      ++invalid; pos=0; expected=0;
    }
  }
  if(expected!=0 && pos==expected) {
    decodeFrame();
    pos=0; expected=0;
  }
}


void setup() {
  Serial.begin(115200);
  delay(1600);
  Wire.begin(8,9,100000);
  Wire.beginTransmission(BRIDGE);
  if(Wire.endTransmission()!=0) {
    // Plotter compatible all-zero lines indicate no link.
    return;
  }
  if(!writeReg(REG_IER,0) || !writeReg(REG_MCR,0) ||
     !writeReg(REG_LCR,0x80) || !writeReg(0,1) ||
     !writeReg(1,0) || !writeReg(REG_LCR,0x03) ||
     !writeReg(REG_FCR,0x07)) return;
  ready=true;
  lastDetectionMs=millis();
  lastGraphMs=millis();
}

void loop() {
  if(!ready) {
    Serial.println("Presence_m:0.00\tMotion_m:0.00\tRadar_state:0\tData_OK:0");
    delay(1000);
    return;
  }
  uint8_t lsr=0;
  if(readReg(REG_LSR,lsr)) {
    if(lsr&0x02) ++overrun;
    if(lsr&0x04) ++parity;
    if(lsr&0x08) ++framing;
    if(lsr&0x10) ++breaks;
    if(lsr&0x01) {
      uint8_t b=0;
      if(readReg(REG_RHR,b)) {
        ++rxCount;
        appendByte(b);
      } else ++i2cErrors;
    } else delay(1);
  } else {
    ++i2cErrors;
    delay(5);
  }
  const uint32_t now=millis();
  if((uint32_t)(now-lastGraphMs)>=500) {
    lastGraphMs=now;
    const bool recent=results>0 &&
                      (uint32_t)(now-lastDetectionMs)<=5000;
    // 'Radar_state' is a visual state band, NOT a measured range.
    const int visualState=!recent?0:
      (reportedState==2||reportedState==3||reportedState==4)?10:
      (reportedState==1||reportedState==5)?5:0;
    Serial.printf("Presence_m:%.2f\tMotion_m:%.2f\tRadar_state:%d\tData_OK:%d\n",
      recent?presenceM:0.0f,recent?motionM:0.0f,
      visualState,recent?11:0);
  }
}
