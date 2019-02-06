/*
ADAS1000 SPI Test
Arias Research Group
Author: Seiya Ono

*/

#include <SPI.h>

//ADAS1000's memory register addresses:
const char NOP       = 0x00;
const char ECGCTL    = 0x01;   // ECG control // reset value 0x000000
const char FRMCTL    = 0x0A;   // Controls what shows up in FRAME
const char LOFFCTL   = 0x02;   // Lead-off control 
const char PACEAMPTH = 0x07;   // Show PACEAMPTH or Pace amplitude threshold // reset value 0x242424

const char LADATA    = 0x11;   // LA or Lead I data 
const char LLDATA    = 0x12;   // LL or Lead II data
const char RADATA    = 0x13;   // RA or Lead III data
const char V1DATA    = 0x14;   // V1 or V1’ data
const char V2DATA    = 0x15;   // V2 or V2’ data
const char PACEDATA  = 0x1A;   // Read pace detection data/status
const char FRAMES    = 0x40;   // Frame header // reset value 0x800000

// pins used for the connection with the AFE
const int CS = 18;

unsigned char buff[4];

bool DEBUG = false;

void setup() {
  Serial.begin(9600);

  // start the SPI library:
  SPI.begin();
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  // initialize the  data ready and chip select pins:
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);

  Serial.println("Setting up ADAS1000...\n");
  
  delay(50);
  // Control Register
  readRegister(NOP);
  writeRegister(FRMCTL, 0x07FC00);
  readRegister(FRMCTL);
  writeRegister(ECGCTL, 0xF800BE);
  readRegister(ECGCTL);
  writeRegister(LOFFCTL, 0x000000);
  writeRegister(PACEAMPTH, 0x242424);
  Serial.println("Setup Complete!\n");
  // give the sensor time to set up:
  delay(100);
  readRegister(FRAMES);
}

void loop() {
  readRegister(NOP);
  delay(0.5);
}

void readRegister(byte reg) {
  /* R/W:  [31]
     Reg:  [30:24]
     Data: [23:0] */
  unsigned int payload = (0b0 << 31) | (reg << 24) | 0x000000;
  if (DEBUG) {Serial.print("Reading 0x"); Serial.println(payload, HEX);}
  
  // read out
  digitalWrite(CS, LOW);
  for (int i = 0; i < 4; i++) {
    char c = (payload >> (24 - (8 * i))) & 0xFF;
    buff[i] = SPI.transfer(c);
  }
  digitalWrite(CS, HIGH);
}


// Write to ADAS1000
void writeRegister(byte reg, int val) {
  /* R/W:  [31]
     Reg:  [30:24]
     Data: [23:0] */
  unsigned int payload = (0b1 << 31) | (reg << 24) | (val & 0x00FFFFFF);
  if (DEBUG) {Serial.print("Writing 0x"); Serial.println(payload, HEX);}
  // transfer payload
  digitalWrite(CS, LOW);
  for (int i = 0; i < 4; i++) {
    char c = (payload >> (24 - (8 * i))) & 0xFF;
    SPI.transfer(c);
  }
  digitalWrite(CS, HIGH);
}
