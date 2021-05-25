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
#define SET_VAL        0x01
#define GET_VAL        0x02

#define SIMULATE_LOSSES false  // Whether random bytes will be not sent

const unsigned long UPDATE_INTERVAL = 2500;

const int TRANSFER_BYTE_DELAY = 80;

//===SETUP=============================
void setup (void) {
  Serial.begin (115200);
  Serial.setTimeout(100);
  
  initSpi();
  
  randomSeed(analogRead(0));
}  // end of setup

//===LOOP=============================
void loop (void)
{
  int randNumber;
  int receivNumber;
  static unsigned long lastUpdate = micros();
  static unsigned long sendNum = 0;
  static unsigned long errorNum = 0;


  // Get the power level from the speed controller
  if (micros() - lastUpdate > UPDATE_INTERVAL) {
    randNumber = random(0,10000);
    sendVal(randNumber);
    delayMicroseconds(100);
    receivNumber = getVal();

    sendNum++;
    if (receivNumber != randNumber) {
      errorNum++;
    }

    // Print if error
    if (sendNum > 1000) {
      Serial.print(errorNum / float(sendNum) * 100.0);
      Serial.println("%");

      errorNum = 0;
      sendNum = 0;
    }
    
    lastUpdate = micros();
  }
  
    
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
  SPI.setClockDivider(SPI_CLOCK_DIV64);
}

// Send a value to the device for the given command.
void sendVal(int value) {
  communicate(SET_VAL, value);
}

// Get a value from the device for the given command.
int getVal() {
  return communicate(GET_VAL, 0);
}

/**
 * Communicate with the device. Send a command and then send a 16-bit value for the 
 * command and simultaneously read a response if present.
 */
int communicate(int command, int sendVal) {
  union { int16_t val; struct { uint8_t lsb; uint8_t msb; }; } in, out;
  
  bool dropBytes[] = {false, false, false};
  const int PROB = 10;

  // If simulating losses, determine which one(s) to drop
  if (SIMULATE_LOSSES) {
    dropBytes[0] = (random(0, PROB) == 0);
    dropBytes[1] = (random(0, PROB) == 0);
    dropBytes[2] = (random(0, PROB) == 0);

    if (dropBytes[0] || dropBytes[1] || dropBytes[2]) {
      Serial.print("DROP BYTE {");
      Serial.print(dropBytes[0]);
      Serial.print(",");
      Serial.print(dropBytes[1]);
      Serial.print(",");
      Serial.print(dropBytes[2]);
      Serial.println("}");
    }
  }

  out.val = sendVal;

  noInterrupts();
  
  // Select the chip to control
  digitalWrite(SS, LOW);
  delayMicroseconds (TRANSFER_BYTE_DELAY/2);
  
  // Send the initial command
  if (!dropBytes[0]) {
    SPI.transfer(command); // 20us at 32 clock div, 36us at 64 clock div, 47us at 128 clock div
  }
  delayMicroseconds(TRANSFER_BYTE_DELAY);

  // Send the value and (or) read the response
  if (!dropBytes[1]) {
    in.lsb = SPI.transfer(out.lsb);
  }
  delayMicroseconds (TRANSFER_BYTE_DELAY/2);
  if (!dropBytes[2]) {
    in.msb = SPI.transfer(out.msb);
  }
  
  // Done.
  delayMicroseconds(TRANSFER_BYTE_DELAY/2);
  digitalWrite(SS, HIGH);
  delayMicroseconds(TRANSFER_BYTE_DELAY/2);
  interrupts();

  // Turn the two values into a 16-bit value
  return in.val;
}
