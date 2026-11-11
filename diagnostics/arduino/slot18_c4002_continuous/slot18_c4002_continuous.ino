/*
 Slot-18 — C4002 CONTINUOUS LIVE MONITOR (READ-ONLY, candidate)
 Uses Waveshare ESP32-S3-Touch-LCD-7 + CJMCU-752 SC16IS752 at 0x4D,
 C4002 on UART A. User-confirmed crystal marking 1.8432 MHz,
 115200 baud = oscillator / (16 * 1), 8N1.

 Continuous monitoring until power is removed or firmware is replaced. No radar commands, reset, settings
 writes, persistent configuration writes, or display graphics.
 Print only checksum-valid notifications (type=0x04, cmd=0x60)
 using wire payload layout from DFRobot C4002 sensor library.
 C4002 is a presence / distance sensor, NOT an XY position sensor.

 Do not connect the Waveshare I2C connector red VCC wire to the
 external breadboard supply. Keep red VCC insulated, screen on USB-C,
 SDA GPIO8, SCL GPIO9, shared GND, verified 5V radar/bridge supply.
 Power OFF before checking/changing wiring. Verify rail polarity.
*/
#include <Wire.h>

static const uint8_t BRIDGE=0x4D;
static const uint8_t REG_RHR=0, REG_IER=1, REG_FCR=2;
static const uint8_t REG_LCR=3, REG_MCR=4, REG_LSR=5;
static const uint32_t NO_DATA_WARNING_MS=5000;
static const uint32_t SUMMARY_MS=30000;
static const uint32_t I2C_WARN_INTERVAL_MS=5000;
static bool ready=false;
static bool noDataWarned=false, bridgeErrorActive=false;
static uint32_t lastDetectionMs=0, lastSummaryMs=0, lastI2cWarningMs=0;
static uint8_t frame[128];
static size_t pos=0;
static uint16_t expected=0;
static uint32_t valid=0, invalid=0, results=0, rxCount=0;
static uint32_t overrun=0, parity=0, framing=0, breaks=0, i2cErrors=0;

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
  uint16_t checksum=0;
  for(size_t i=0;i<(size_t)expected-2;i++) checksum=(uint16_t)(checksum+frame[i]);
  if(checksum!=u16(&frame[expected-2])) {
    ++invalid;
    Serial.println("Frame checksum FAIL");
    return;
  }
  ++valid;
  // frame header(8), dataHeader(4), report data(18), checksum(2) => 32
  if(expected!=32 || frame[7]!=0x04 || frame[8]!=0x60 || frame[9]!=0x01 ||
     u16(&frame[10])!=22) {
    Serial.printf("Valid non-detection frame: type=%02X cmd=%02X bytes=%u\n",
                  frame[7],frame[8],(unsigned)expected);
    return;
  }
  const uint8_t *p=&frame[12];
  ++results;
  lastDetectionMs=millis();
  if(noDataWarned) Serial.println("DATA RESUMED: valid detection packets are arriving again.");
  noDataWarned=false;
  const uint8_t status=p[0];
  const float light=(float)u16(&p[1])/10.0f;
  const uint32_t gates=u32(&p[3]);
  const unsigned countdown=u16(&p[7]);
  const float presenceMeters=(float)u16(&p[9])/100.0f;
  const unsigned presenceEnergy=p[11];
  const float motionMeters=(float)u16(&p[12])/100.0f;
  const float motionSpeed=(float)s16(&p[14])/100.0f;
  const unsigned motionEnergy=p[16];
  const unsigned direction=p[17];
  if(status==0) {
    // Detection range fields may contain old values after a target leaves.
    Serial.printf("t=%lus NO_TARGET light=%.1f lux; distances omitted (may be stale)\n",
        (unsigned long)(millis()/1000),light);
  } else if(status==255) {
    Serial.printf("t=%lus SENSOR_PIN_ERROR reported\n",
        (unsigned long)(millis()/1000));
  } else {
    const char *dir=(direction==0)?"AWAY":(direction==1)?"NONE":
                    (direction==2)?"APPROACHING":"UNKNOWN";
    Serial.printf("t=%lus %-22s presence=%.2fm energy=%u motion=%.2fm speed=%.2fm/s energy=%u direction=%s light=%.1f lux countdown=%u gates=%lu\n",
        (unsigned long)(millis()/1000),targetName(status),presenceMeters,
        presenceEnergy,motionMeters,motionSpeed,motionEnergy,dir,light,
        countdown,(unsigned long)gates);
  }
  // Presence / motion numeric fields can retain stale values even in
  // NO_TARGET state. Interpret target status first; don't claim XY/person ID.
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
  delay(1800);
  Serial.println("\nSlot-18 C4002 CONTINUOUS LIVE MONITOR (READ-ONLY)");
  Wire.begin(8,9,100000);
  Wire.beginTransmission(BRIDGE);
  if(Wire.endTransmission()!=0) {
    Serial.println("STOP: bridge 0x4D not found"); return;
  }
  if(!writeReg(REG_IER,0) || !writeReg(REG_MCR,0) ||
     !writeReg(REG_LCR,0x80) ||
     !writeReg(0,1) || !writeReg(1,0) ||
     !writeReg(REG_LCR,0x03) ||
     !writeReg(REG_FCR,0x07)) {
    Serial.println("STOP: bridge UART initialization failed");return;
  }
  Serial.println("Bridge ACK; UART A 115200 8N1, divisor 1");
  Serial.println("Live radar packets print continuously until power is removed or new firmware is uploaded.");
  Serial.println("Suggested test: still, move hand toward/away, then leave detection area.");
  Serial.println("Expect a NO_TARGET only after the radar\x27s configured hold/disappear delay.");
  Serial.println("If no valid sensor packets arrive for 5 seconds, a data-link warning will appear.");
  Serial.println("Status summary every 30 seconds. Begin capture...");
  ready=true;
  lastDetectionMs=millis();
  lastSummaryMs=millis();
}

void loop() {
  if(!ready) { delay(1000); return; }

  uint8_t lsr=0;
  if(!readReg(REG_LSR,lsr)) {
    ++i2cErrors;
    if(!bridgeErrorActive ||
        (uint32_t)(millis()-lastI2cWarningMs)>=I2C_WARN_INTERVAL_MS) {
      Serial.println("I2C ERROR: unable to read CJMCU-752 line status (check bridge power and wiring).");
      bridgeErrorActive=true;
      lastI2cWarningMs=millis();
    }
    delay(50);
    return;
  }
  if(bridgeErrorActive) {
    Serial.println("I2C LINK RESTORED: bridge responding.");
    bridgeErrorActive=false;
  }

  if(lsr&0x02) ++overrun;
  if(lsr&0x04) ++parity;
  if(lsr&0x08) ++framing;
  if(lsr&0x10) ++breaks;
  if(lsr&0x01) {
    uint8_t b=0;
    if(readReg(REG_RHR,b)) {
      ++rxCount;
      appendByte(b);
    } else {
      ++i2cErrors;
      Serial.println("I2C ERROR: unable to read UART byte from CJMCU-752.");
      delay(50);
    }
  } else {
    delay(1);
  }

  const uint32_t now=millis();
  if(!noDataWarned && (uint32_t)(now-lastDetectionMs)>=NO_DATA_WARNING_MS) {
    Serial.println("NO NEW DATA for 5 seconds. This is NOT the same as NO_TARGET.");
    noDataWarned=true;
  }
  if((uint32_t)(now-lastSummaryMs)>=SUMMARY_MS) {
    Serial.printf("SUMMARY up=%lus rx=%lu valid=%lu bad=%lu detection=%lu overrun=%lu parity=%lu framing=%lu breaks=%lu I2C_errors=%lu last_detection_age=%lus\n",
        (unsigned long)(now/1000),(unsigned long)rxCount,
        (unsigned long)valid,(unsigned long)invalid,(unsigned long)results,
        (unsigned long)overrun,(unsigned long)parity,(unsigned long)framing,
        (unsigned long)breaks,(unsigned long)i2cErrors,
        (unsigned long)((uint32_t)(now-lastDetectionMs)/1000));
    lastSummaryMs=now;
  }
}
