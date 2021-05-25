/**
 * Author: Samuel Ryckman
 * 
 * Slave device SPI code
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

// Commands
#define SET_SPEED        0x01
#define SET_POWER        0x02
#define SET_BRAKE        0x03
#define SET_INVERT_DIR   0x04

#define GETTER_COMAND    0x10
#define GET_SPEED        0x00 | GETTER_COMAND
#define GET_POWER        0x01 | GETTER_COMAND

// Values for the device (a motor controller)
int speed = 0;
int power = 0;
bool inverted = false;

// SPI Send/receive variables
int spiCommand = 0;        // Whether sending or receiving the data
int spiByteNum = 0;        // Current send/receive status
const int SPI_COM_END = 2; // Number of the final send/receive byte
int spiVal = 0;            // Value being sent or received

bool newData = false;

//===SETUP=============================
void setup (void) {
  Serial.begin(115200);

  // Initialize the SPI for the device
  initSpi();
  Serial.println("Beginning receive code.");
}  // end of setup


//===LOOP=============================
void loop (void) {

  if (newData) {
    // Get the value if doing a send
    if (spiCommand & GETTER_COMAND) {
      Serial.print("Value sent -> ");
      switch (spiCommand) {
        case GET_SPEED:
          Serial.print("speed: ");
          Serial.println(speed);
        break;
        case GET_POWER:
          Serial.print("power: ");
          Serial.println(power);
        break;
      }
    } else {
      Serial.print("Value set -> ");
      // Output the new value
      switch (spiCommand) {
        case SET_SPEED:
          Serial.print("speed: ");
          Serial.println(speed);
        break;
        case SET_POWER:
          Serial.print("power: ");
          Serial.println(power);
        break;
        case SET_BRAKE:
          Serial.println("Braking on.");
        break;
        case SET_INVERT_DIR:
          Serial.print("Inverted: ");
          Serial.println(inverted);
        break;
      }
    }
    newData = false;
  }
}  // end of loop


//=====SPI functions==========================
// Initialize the SPI communications for the slave device
void initSpi() {
  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);

  // turn on SPI in slave mode
  SPCR |= _BV(SPE);

  // turn on interrupts
  SPCR |= _BV(SPIE);
}


// SPI interrupt routine
ISR (SPI_STC_vect) {
  // Handle based on which byte this is
  if (spiByteNum == 0) {  // First byte
    // Read the fist byte (the command)
    spiCommand = SPDR;
    
    // Get the value if doing a send
    if (spiCommand & GETTER_COMAND) {
      switch (spiCommand) {
        case GET_SPEED:
          spiVal = speed;
        break;
        case GET_POWER:
          spiVal = power;
        break;
      }
      //First byte
      SPDR = spiVal & 0xFF;
    }
    spiByteNum++;
  } else if (spiByteNum == SPI_COM_END) {  // Last byte
    spiByteNum = 0;
    
    // Do the thing with the value if receiving
    if (!(spiCommand & GETTER_COMAND)) {
      // Last byte
      spiVal |= SPDR << 8;
      
      // Do what we need to with it
      switch (spiCommand) {
        case SET_SPEED:
          speed = spiVal;
        break;
        case SET_POWER:
          power = spiVal;
        break;
        case SET_BRAKE:
          
        break;
        case SET_INVERT_DIR:
          inverted = spiVal;
        break;
      }
    }
    newData = true;
  } else {
    // Send or receive the byte
    if (spiCommand & GETTER_COMAND) { // Sending
      SPDR = spiVal >> 8;
    } else { // Receiving
      spiVal = SPDR;
    }
    
    spiByteNum++;
  }
}
