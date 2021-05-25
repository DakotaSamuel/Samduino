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
#define SET_VAL        0x01
#define GET_VAL        0x02

// Values stored by the device
int val = 0;

// SPI Send/receive variables
int spiCommand = 0;        // Whether sending or receiving the data
int spiByteNum = 0;        // Current send/receive status
const int SPI_COM_END = 2; // Number of the final send/receive byte
int spiVal = 0;            // Value being sent or received
const long SPI_TIMEOUT = 120; // Max time a byte should take (in us)

bool newData = false;

unsigned long start = 0;
unsigned long done = 0;

//===SETUP=============================
void setup (void) {
  //Serial.begin(115200);

  // Initialize the SPI for the device
  start = micros();
  initSpi();
  Serial.println("Beginning receive code.");
}  // end of setup


//===LOOP=============================
void loop (void) {
  /*static bool printed = false;

  if (printed && micros() - start < 1000000) {
    printed = false;
  } else if (!printed && micros() - start > 1000000) {
    Serial.println("------------");
    printed = true;
  }*/
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
  // Read the fist byte (the command)
  spiCommand = SPDR;
  
  // Get the value if doing a send
  if (spiCommand == GET_VAL) {
    spiVal = val;
  } else {
    spiVal = 0;
  }
  
  // Do the data transfer
  spiVal = transferData(spiVal);
  
  val = spiVal;
  newData = true;
}

inline int16_t transferData(int16_t data) {
    union { int16_t val; struct { uint8_t lsb; uint8_t msb; }; } in, out;
    static unsigned long start = 0;
  
    in.val = data;
    
    // Send/Receive the LSB
    SPDR = in.lsb;
    asm volatile("nop");
    start = micros();
    while (!(SPSR & _BV(SPIF))) {
      if (micros() - start > (SPI_TIMEOUT << 1)) {
        return 0;
      }
    }
    out.lsb = SPDR;
    
    // Send/Receive the MSB
    SPDR = in.msb;
    asm volatile("nop");
    start = micros();
    while (!(SPSR & _BV(SPIF))) {
      if (micros() - start > SPI_TIMEOUT) {
        return 0;
      }
    }
    out.msb = SPDR;
    return out.val;
}
