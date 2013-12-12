#include <Wire.h>

int outputpin8 = 8;
int outputpin9 = 9;
#define ECHOPIN2 2
#define TRIGPIN3 3

void setup()

{
  //Console.begin();
  Wire.begin();                // join i2c bus (address optional for master)
  
  Serial.begin(9600);          // start serial communication at 9600bps
  pinMode(outputpin8, OUTPUT);
  pinMode(outputpin9, OUTPUT);
  pinMode(ECHOPIN2, INPUT);
  pinMode(TRIGPIN3, OUTPUT);

}



int reading = 0;
int distance2 = 0;


void loop()

{
  //Serial.print("it should print");
  // step 1: instruct sensor to read echoes

  Wire.beginTransmission(112); // transmit to device #112 (0x70)

                               // the address specified in the datasheet is 224 (0xE0)

                               // but i2c adressing uses the high 7 bits so it's 112

  Wire.write(byte(0x00));      // sets register pointer to the command register (0x00)  

  Wire.write(byte(0x51));      // command sensor to measure in "inches" (0x50)

                               // use 0x51 for centimeters

                               // use 0x52 for ping microseconds

  Wire.endTransmission();      // stop transmitting


  // step 2: wait for readings to happen

  delay(58);                   // datasheet suggests at least 65 milliseconds


  //read from srf05  
  digitalWrite(TRIGPIN3, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN3, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN3, LOW);
  distance2 = pulseIn(ECHOPIN2, HIGH);
  distance2 = distance2/58;


  // step 3: instruct sensor to return a particular echo reading

  Wire.beginTransmission(112); // transmit to device #112

  Wire.write(byte(0x02));      // sets register pointer to echo #1 register (0x02)

  Wire.endTransmission();      // stop transmitting



  // step 4: request reading from sensor

  Wire.requestFrom(112, 2);    // request 2 bytes from slave device #112



  // step 5: receive reading from sensor
  //Serial.println("Displaying distance");
  if(2 <= Wire.available())    // if two bytes were received

  {

    reading = Wire.read();  // receive high byte (overwrites previous reading)

    reading = reading << 8;    // shift high byte to be high 8 bits

    reading |= Wire.read(); // receive low byte as lower 8 bits
    
    //printout of each of the ultrasonic sensors
    Serial.print("Sensor 1:");
    Serial.println(reading);   // print the reading


    //if ultrasonics sees any obstacle
    if(reading  < 5){
      Serial.println("Sensor1 obstacle");
      digitalWrite(outputpin8, HIGH);
      //delay(1000);
    }
    else{
      Serial.println("Sensor1 clear");
      digitalWrite(outputpin8,LOW);
      //delay(1000);
    }
    
  }
  Serial.print("Sensor 2:");
  Serial.println(distance2);
  if(distance2 < 5){
    Serial.println("Sensor2 obstacle");
    digitalWrite(outputpin9, HIGH);
  }
  else{
    Serial.println("Sensor2 Clear");
    digitalWrite(outputpin9, LOW);
  }

  delay(250);                  // wait a bit since people have to read the output :)

}





/*



// The following code changes the address of a Devantech Ultrasonic Range Finder (SRF10 or SRF08)

// usage: changeAddress(0x70, 0xE6);



void changeAddress(byte oldAddress, byte newAddress)

{

  Wire.beginTransmission(oldAddress);

  Wire.write(byte(0x00));

  Wire.write(byte(0xA0));

  Wire.endTransmission();



  Wire.beginTransmission(oldAddress);

  Wire.write(byte(0x00));

  Wire.write(byte(0xAA));

  Wire.endTransmission();



  Wire.beginTransmission(oldAddress);

  Wire.write(byte(0x00));

  Wire.write(byte(0xA5));

  Wire.endTransmission();



  Wire.beginTransmission(oldAddress);

  Wire.write(byte(0x00));

  Wire.write(newAddress);

  Wire.endTransmission();

}



*/
