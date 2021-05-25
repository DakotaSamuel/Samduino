/**
 * Author: Samuel Ryckman
 * 
 * Master device SPI code
 * 
 * Demo of SPI communications between two Arduinos. Communications are done in the following form.
 *     Data transmission: | Command | Data LSB | Data MSB |
 * For a send (Master -> Slave), the command indicates the data being sent and the following two 
 *   bytes are the data.
 * For a receive (Master <- Slave), the command indicates the data to send and the slave device 
 *   responds with the two data bytes.
 *   
 * Connections:
 * |--MASTER--|        |--SLAVE--|
 * |     MOSI |  <-->  | MOSI    |
 * |     MISO |  <-->  | MISO    |
 * |      SCK |  <-->  | SCK     |
 * |       SS |  <-->  | SS      |
 * |----------|        |---------|
 * 
 * Note: the SS (Slave select) pin could be any pin on the master (may have multiple when 
 * communicating with more than one device). But the SS pin must be used for at least one 
 * of the connections.
 */


#include <SPI.h>

// Commands
#define SET_SPEED        0x01
#define SET_POWER        0x02
#define SET_BRAKE        0x03
#define SET_INVERT_DIR   0x04

#define GETTER_COMAND    0x10
#define GET_SPEED        0x00 | GETTER_COMAND
#define GET_POWER        0x01 | GETTER_COMAND

// Menu handling
char menuChoice = 0;
int value = 0;


//===SETUP=============================
void setup (void) {
  Serial.begin (115200);
  Serial.setTimeout(100);
  
  initSpi();
}  // end of setup

//===LOOP=============================
void loop (void)
{
  printMenu();

  // Wait for command
  while (Serial.available() < 1) { }
  menuChoice = Serial.read();

  switch (menuChoice) {
     case '1': // Set speed
      Serial.println("Enter speed: ");
      value = readSerialVal();
      sendVal(SET_SPEED, value);
      Serial.print("Speed set to ");
      Serial.println(value);
     break;
     case '2': // Set power
      Serial.println("Enter power: ");
      value = readSerialVal();
      sendVal(SET_POWER, value);
      Serial.print("Power set to ");
      Serial.println(value);
     break;
     case '3': // Set brake
      Serial.println("Braking engaged.");
      sendVal(SET_BRAKE, 0);
     break;
     case '4': // Set invert direction
      Serial.println("Set direction inverted (1 or 0): ");
      value = readSerialVal();
      sendVal(SET_INVERT_DIR, value);
      Serial.print("Invert set to: ");
      Serial.println(value);
     break;
     case '5': // Get speed
      Serial.print("Current speed: ");
      value = getVal(GET_SPEED);
      Serial.println(value);
     break;
     case '6': // Get power
      Serial.print("Current power: ");
      value = getVal(GET_POWER);
      Serial.println(value);
     break;
  }
  Serial.println();
}  // end of loop


//=====SPI functions==========================
// Initialize the SPI connection
void initSpi() {
  pinMode(SS, OUTPUT);  // Shouldn't have to do this, but some devices need it
  digitalWrite(SS, HIGH);  // ensure SS stays high for now

  // Put SCK, MOSI, SS pins into output mode
  // also put SCK, MOSI into LOW state, and SS into HIGH state.
  // Then put SPI hardware into Master mode and turn SPI on
  SPI.begin ();

  // Slow down the master a bit
  //SPI.setClockDivider(SPI_CLOCK_DIV8);
}

// Send a value to the device for the given command.
void sendVal(int command, int value) {
  communicate(command, value);
}

// Get a value from the device for the given command.
int getVal(int command) {
  return communicate(command, 0);
}

/**
 * Communicate with the device. Send a command and then send a 16-bit value for the 
 * command and simultaneously read a response if present.
 */
int communicate(int command, int sendVal) {
  byte readVal[2];

  // Select the chip to control
  digitalWrite(SS, LOW);
  delayMicroseconds (TRANSFER_BYTE_DELAY/2);
  
  // Send the initial command
  SPI.transfer(command);
  delayMicroseconds (TRANSFER_BYTE_DELAY);

  // Send the value and (or) read the response
  readVal[0] = SPI.transfer(sendVal & 0xFF);
  delayMicroseconds (TRANSFER_BYTE_DELAY);
  readVal[1] = SPI.transfer(sendVal >> 8);
  
  // Done.
  delayMicroseconds (TRANSFER_BYTE_DELAY/2);
  digitalWrite(SS, HIGH);

  // Turn the two values into a 16-bit value
  return (readVal[1] << 8) | readVal[0];
}

//=====UTILITY funcitons=========================
void printMenu() {
  Serial.println("1 - Set Speed");
  Serial.println("2 - Set Power");
  Serial.println("3 - Brake");
  Serial.println("4 - Set Direction Inverted");
  Serial.println("5 - Get Speed");
  Serial.println("6 - Get Power");
  Serial.println();
}

int readSerialVal() {
  // Wait for value
  while(Serial.available() < 1) { }
  return Serial.parseInt();
}
