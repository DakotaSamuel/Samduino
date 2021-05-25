#include <Servo.h>

Servo motor;

char readVal = '-';
int speed = 90;



//=====setup==============================
void setup() {
  Serial.begin(9600);
  
  motor.attach(11);
  motor.write(speed);
}

//=====loop==============================
void loop() {
  switch (readVal) {
  case 'u':
    speed += 10;
    break;
  case 'd':
    speed -= 10;
    break;
  default:
    break;
}

  motor.write(speed);
  Serial.println(speed);
  
  //wait for a value, read it, and clear the extra
  while (Serial.available() <= 0) {
    delay(100);
  }
  readVal = Serial.read();
  while(Serial.available()){Serial.read();}
}
