/*
ADAS1000 SPI Test
Arias Research Group
Author: Seiya Ono

*/

#include <SPI.h>

//ADAS1000's memory register addresses:
const char ECGCTL = 0x01;     // ECG control // reset value 0x000000
const char LOFFCTL = 0x02;    // Lead-off control 
const char PACEAMPTH = 0x07;  // Show PACEAMPTH or Pace amplitude threshold // reset value 0x242424

const char LADATA = 0x11;     // LA or Lead I data 
const char LLDATA = 0x12;     // LL or Lead II data
const char RADATA = 0x13;     // RA or Lead III data
const char V1DATA = 0x14;     // V1 or V1’ data
const char V2DATA = 0x15;     // V2 or V2’ data
const char PACEDATA = 0x1A;   // Read pace detection data/status

const char FRAMES = 0x40;     // Frame header // reset value 0x800000

// pins used for the connection with the AFE
const int CS = 18;

unsigned char values[10];

bool DEBUG = true;

void setup() {
  Serial.begin(9600);

  // start the SPI library:
  SPI.begin();
  SPI.setDataMode(SPI_MODE3);
  // initialize the  data ready and chip select pins:
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);

  Serial.println("Setting up ADAS1000...\n");
  
  delay(50);
  // Control Register
  writeRegister(ECGCTL, 0xF800BE);
  writeRegister(LOFFCTL, 0x000000);
  writeRegister(PACEAMPTH, 0x242424);
  Serial.println("Setup Complete!\n");
  // give the sensor time to set up:
  delay(100);
  readRegister(FRAMES);
}

void loop() {
  digitalWrite(CS, LOW);
  unsigned payload = PACEAMPTH << 24;
//  Serial.print("Sending 0x"); Serial.println(payload, HEX);
  Serial.print("Sending 0x"); Serial.println(0x00000000, HEX);
  int val = SPI.transfer(payload);
  Serial.print("Read 0x"); Serial.println(val, HEX);
  digitalWrite(CS, HIGH);
  delay(1000);
}

//void readRegister(char registerAddress, int numBytes, unsigned char * values) {
//  //Since we're performing a read operation, the most significant bit of the register address should be set.
//  char address = 0x00 | registerAddress; // 0x08-->0x00
//  //If we're doing a multi-byte read, bit 6 needs to be set as well.
//  if (numBytes > 1)address = address | 0x40;
//  
//  //Set the Chip select pin low to start an SPI packet.
//  digitalWrite(CS, LOW);
//  //Transfer the starting register address that needs to be read.
//  SPI.transfer(address);
//  //Continue to read registers until we've read the number specified, storing the results to the input buffer.
//  for (int i = 0; i < numBytes; i++) {
//  values[i] = SPI.transfer(0x00);
//  }
//  //Set the Chips Select pin high to end the SPI packet.
//  digitalWrite(CS, HIGH);
//}

void readRegister(byte reg) {
  // R/W:  [31]
  // Reg:  [30:24]
  // Data: [23:0]
  unsigned int payload = (0b0 << 31) | (reg << 24) | 0x000000;
  if (DEBUG) {Serial.print("Reading 0x"); Serial.println(payload, HEX);}
  // transfer payload
  digitalWrite(CS, LOW);
  SPI.transfer(payload);
  digitalWrite(CS, HIGH);
}


// Write to ADAS1000
void writeRegister(byte reg, int val) {
  // R/W:  [31]
  // Reg:  [30:24]
  // Data: [23:0]
  unsigned int payload = (0b1 << 31) | (reg << 24) | (val & 0x00FFFFFF);
  if (DEBUG) {Serial.print("Writing 0x"); Serial.println(payload, HEX);}
  // transfer payload
  digitalWrite(CS, LOW);
  SPI.transfer(payload);
  digitalWrite(CS, HIGH);
}
