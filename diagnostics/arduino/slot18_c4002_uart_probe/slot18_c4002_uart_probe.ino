/*
 Slot-18 — C4002 UART A test (replacement radar, one-shot, 2026-11-10)
 Evidence: CJMCU-752 oscillator marked 1.8432 MHz (user photo / confirmation).
 SC16IS752 formula: baud = oscillator / (16 * divisor) with prescaler=1.
 1,843,200 / (16 * 1) = 115,200 baud (8N1).
 Manufacturer reference: DFRobot_C4002::getVersioninfo(), sendPack().

 CONNECTIONS (power OFF to inspect/change):
 Waveshare USB-C UART1 -> PC COM3; screen I2C SDA(GPIO8) -> CJMCU SDA,
 SCL(GPIO9) -> CJMCU SCL; common GND; screen red I2C VCC disconnected.
 Regulator-equipped CJMCU module breadboard +5V and GND; C4002 VIN +5V,
 GND common, C4002 TX -> CJMCU RXA, C4002 RX -> CJMCU TXA.

 Do NOT power screen I2C VCC from external +5V. Confirm supply polarity
 with meter BEFORE running test; power off before wiring.

 This test:
 - checks bridge 0x4D ACK
 - sets UART A 8N1, 115200 using verified crystal, FIFO
 - passively captures RX for 1200 ms
 - sends ONE read-only DFRobot software version request (NOT config/reset)
 - captures received bytes and SC16IS752 line status for 3000ms
 - checks FA F5 AA A5 frame header, size, additive checksum
 - prints results ONCE, no loops/repeated commands.
 It does not activate display, change radar settings or prove distance.
*/
#include <Wire.h>

static constexpr uint8_t DEV=0x4D;
static constexpr uint8_t CH=0;
static constexpr uint8_t RHR=0, THR=0, IER=1, FCR=2, LCR=3, MCR=4, LSR=5;
static constexpr size_t MAX_BYTES=256;
static uint8_t captured[MAX_BYTES];
static size_t used=0;
static uint32_t overruns=0, parityErrors=0, framingErrors=0, breaks=0;
static uint8_t lastError=0;

uint8_t subaddr(uint8_t r) { return (uint8_t)((r << 3) | (CH << 1)); }

bool wr(uint8_t r,uint8_t val) {
  Wire.beginTransmission(DEV);
  Wire.write(subaddr(r)); Wire.write(val);
  lastError=Wire.endTransmission();
  return lastError==0;
}

bool rd(uint8_t r,uint8_t &val) {
  Wire.beginTransmission(DEV);
  Wire.write(subaddr(r));
  lastError=Wire.endTransmission(false);
  if(lastError!=0) return false;
  if(Wire.requestFrom((uint8_t)DEV,(uint8_t)1,(uint8_t)true)!=1) {
    lastError=0xFE; return false;
  }
  val=(uint8_t)Wire.read(); return true;
}

bool initPort() {
  // Disable UART interrupts; clear prescaler (MCR[7]=0), disable loopback.
  if(!wr(IER,0x00) || !wr(MCR,0x00)) return false;
  if(!wr(LCR,0x80)) return false; // enable divisor latch (DLAB)
  if(!wr(0,0x01) || !wr(1,0x00)) return false; // divisor = 1
  if(!wr(LCR,0x03)) return false; // 8 data, no parity, 1 stop
  if(!wr(FCR,0x07)) return false; // FIFO on, clear RX/TX FIFO
  uint8_t check=0;
  if(!rd(LCR,check)) return false;
  Serial.printf("UART A LCR=0x%02X (expected 0x03)\n",check);
  return check==0x03;
}

void readSerialFor(uint32_t duration) {
  uint32_t start=millis();
  while((uint32_t)(millis()-start)<duration) {
    uint8_t stat=0;
    if(!rd(LSR,stat)) { Serial.printf("I2C LSR read error %u\n",lastError); break; }
    if(stat&0x02) overruns++;
    if(stat&0x04) parityErrors++;
    if(stat&0x08) framingErrors++;
    if(stat&0x10) breaks++;
    if(stat&0x01) {
      uint8_t b=0;
      if(!rd(RHR,b)) { Serial.printf("I2C RX read error %u\n",lastError); break; }
      if(used<MAX_BYTES) captured[used++]=b;
    } else {
      delay(1);
    }
  }
}

void dumpCapture() {
  Serial.printf("RX bytes stored: %u (maximum %u)\n",(unsigned)used,(unsigned)MAX_BYTES);
  Serial.printf("Line flags: overrun=%lu parity=%lu framing=%lu break=%lu\n",
    (unsigned long)overruns,(unsigned long)parityErrors,
    (unsigned long)framingErrors,(unsigned long)breaks);
  for(size_t i=0;i<used;i++) {
    if(i%16==0) Serial.printf("%03u: ",(unsigned)i);
    Serial.printf("%02X ",captured[i]);
    if(i%16==15 || i+1==used) Serial.println();
  }
}

void checkFrames() {
  unsigned good=0, bad=0, incomplete=0;
  for(size_t i=0;i+7<used;i++) {
    if(captured[i]!=0xFA || captured[i+1]!=0xF5 ||
       captured[i+2]!=0xAA || captured[i+3]!=0xA5) continue;
    const uint16_t len=(uint16_t)captured[i+4] |
      ((uint16_t)captured[i+5]<<8);
    if(len<10 || len>128) {
      Serial.printf("Header at %u with invalid length %u\n",(unsigned)i,(unsigned)len);
      bad++; continue;
    }
    if(i+len>used) {
      Serial.printf("Partial frame at %u length %u\n",(unsigned)i,(unsigned)len);
      incomplete++; continue;
    }
    uint16_t sum=0;
    for(size_t j=i;j<i+len-2;j++) sum=(uint16_t)(sum+captured[j]);
    uint16_t want=(uint16_t)captured[i+len-2] |
       ((uint16_t)captured[i+len-1]<<8);
    Serial.printf("Frame at %u len=%u type=0x%02X checksum %s\n",
      (unsigned)i,(unsigned)len,captured[i+7],sum==want?"PASS":"FAIL");
    if(sum==want) good++; else bad++;
    i+=len-1;
  }
  Serial.printf("Valid framed replies: %u, invalid: %u, incomplete: %u\n",
    good,bad,incomplete);
  if(good) Serial.println("PASS: C4002 serial framing/checksum observed; inspect frame type before interpreting.");
  else Serial.println("NOT VERIFIED: no valid C4002 frame; check error flags/hex output and TX/RX levels.");
}

bool writeByte(uint8_t b) {
  uint32_t start=millis();
  uint8_t stat=0;
  do {
    if(!rd(LSR,stat)) return false;
    if(stat&0x20) return wr(THR,b);
    delay(1);
  } while((uint32_t)(millis()-start)<200);
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(1600);
  Serial.println("\nSlot-18 C4002 UART A probe — ONE SHOT");
  Serial.println("Crystal MARKED 1.8432MHz; divisor 1 -> 115200 8N1");
  Serial.println("Replacement C4002; bridge 0x4D; no radar config change.");
  Wire.begin(8,9,100000);
  Wire.beginTransmission(DEV);
  if(Wire.endTransmission()!=0) {
    Serial.println("STOP: CJMCU bridge 0x4D not responding on I2C.");
    return;
  }
  Serial.println("I2C 0x4D ACK");
  if(!initPort()) {
    Serial.printf("STOP: UART setup failed (I2C error %u).\n",lastError);
    return;
  }
  Serial.println("Stage A: passive RX sample 1200 ms");
  readSerialFor(1200);
  Serial.printf("Passive bytes: %u\n",(unsigned)used);
  // Keep passive sample separate from commanded reply to make results interpretable.
  used=0; overruns=0; parityErrors=0; framingErrors=0; breaks=0;
  // DFRobot software-version query: header, length=15, seq=0, read req=1,
  // command=0x82, request code=0x00, payload length=5, software-version=1,
  // additive 16-bit checksum little endian.
  const uint8_t payload[]={0x82,0x00,0x05,0x00,0x01};
  uint8_t req[15]={0xFA,0xF5,0xAA,0xA5,0x0F,0x00,0x00,0x01,
                   payload[0],payload[1],payload[2],payload[3],payload[4],0,0};
  uint16_t check=0; for(size_t i=0;i<13;i++) check=(uint16_t)(check+req[i]);
  req[13]=(uint8_t)(check&0xFF); req[14]=(uint8_t)(check>>8);
  Serial.printf("Stage B: ONE read-only version query, checksum 0x%04X\n",check);
  bool ok=true;
  for(uint8_t b:req) { if(!writeByte(b)) {ok=false;break;} }
  if(!ok) {Serial.printf("FAIL: writing UART request (I2C error %u).\n",lastError);return;}
  Serial.println("UART request sent; capturing RX 3000 ms.");
  readSerialFor(3000);
  dumpCapture();
  checkFrames();
  Serial.println("TEST COMPLETE: no automatic retry. Copy all output to ChatGPT.");
}

void loop() { delay(60000); }
