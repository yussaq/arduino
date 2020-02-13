/*
 http://useaxes.com
 RC Car joystick
 DC motor controlled by L298n 
 SG90 servo to turn front wheels

L298n
in1 = 2;    // direction pin 1
in2 = 4;    // direction pin 2
ena = 3;    // PWM pin to change speed

Pin NRF24L01 Conections 
    CE   - 7
    CS   - 8
    MOSI - 11
    MISO - 12
    SCK  - 13
 */

#include <SPI.h>  
#include "RF24.h"
#include <Servo.h>
RF24 myRadio (7, 8);
Servo myservo;         // create servo object to control a servo

int Y;                // Y axis = forward/backward
int X;                // X axis = left/right
int fspeed;          // forward
int bspeed;         // backward
int iOldPos, iNewPos = 0;    // servo position
const int in1 = 2;    // direction pin 1
const int in2 = 4;    // direction pin 2
const int ena = 3;    // PWM pin to change speed

int buttonPin = 5;
int Led = 6;

long previousMillis = 0;      // timer for servo
const int interval = 250;      // interval to update servo position

struct package
{
  int msg  = 0;
  int Lin1 = 0;
  int Lin2 = 0;  
  int Lena = 0;
  int servopos = 0;  
};

byte addresses[][6] = {"0"};
typedef struct package Package;
Package data;

void setup()
{
  Serial.begin(9600);  
  pinMode(in1, OUTPUT);      // connection to L298n
  pinMode(in2, OUTPUT);      // connection to L298n
  pinMode(ena, OUTPUT);      // connection to L298n

  pinMode(buttonPin,INPUT); 
  pinMode(Led,OUTPUT); 
  
  myservo.attach(9);    // attaches the servo on pin 9 to the servo object
  myservo.write(87);    // center servo 

  myRadio.begin();  
  myRadio.setChannel(115);  //115 band above WIFI signals
  myRadio.setPALevel(RF24_PA_MIN); //MIN power low rage
  myRadio.setDataRate( RF24_250KBPS ) ;  //Minimum speed
    
  delay(50);           // give servo time to change position
}

void loop()
{
  if (digitalRead(buttonPin) == HIGH) {    // Baca PIN 5
      digitalWrite(Led, HIGH);       // Nyalakan Led Merah 
  } else { 
      digitalWrite(Led, LOW);        // Matikan Led Mera 
  }
  //Serial.println(digitalRead(buttonPin));    
  
  int X = analogRead(A1);  // joystick X axis
  int Y = analogRead(A0);  // joystick Y axis
    
  if (Y < 500)              // joystick forward
  {
    fspeed = (map(Y, 501, 0, 70, 250));
    forward(fspeed);
    //Serial.print("forward : ");
    //Serial.println(bspeed);    
  }

  if (Y > 540)              // joystick backward
  {
    bspeed = (map(Y, 541, 1023, 70, 250));
    backward(bspeed);
    //Serial.print(" backward : ");
    //Serial.println(bspeed);
  }
  if (Y >= 500 && Y <= 540)    // joystick is centered
  {
    stop();
  }

  if (X > 490 && X < 530 )    // joystick is centered
  {
    iNewPos = 87;     // 87 = servo in center position
  }

  if (X < 490)              // turn left
  {
    iNewPos = (map(X, 491, 0, 88, 130));
  }

  if (X > 530)              // turn right
  {
    iNewPos = (map(X, 531, 1023, 86, 35));
  }

  unsigned long currentMillis = millis();

  if(iOldPos != iNewPos && currentMillis - previousMillis > interval) {      // Issue command only if desired position changes and interval is over
    previousMillis = currentMillis;
    iOldPos = iNewPos;
    myservo.write(iNewPos);     // tell servo to go to position in variable 'pos' 
    //Serial.print("X: ");
    //Serial.println(iNewPos);
    data.servopos = iNewPos;
    WriteData();
  }
  
}

void stop()
{
  analogWrite(ena, 0);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  data.Lin1 = 0;
  data.Lin2 = 0;  
  data.Lena = 0;
  WriteData();
}

void forward(int fspeed)
{
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(ena, fspeed);
  data.Lin1 = 1;
  data.Lin2 = 0;  
  data.Lena = fspeed;
  WriteData();
}

void backward(int bspeed)
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(ena, bspeed);
  data.Lin1 = 0;
  data.Lin2 = 1;  
  data.Lena = bspeed;
  WriteData();
}

void WriteData()
{
  myRadio.stopListening();  //Stop Receiving and start transminitng 
  myRadio.openWritingPipe( 0xF0F0F0F0AA); //Sends data on this 40-bit address
  myRadio.write(&data, sizeof(data)); 
  Serial.print("Lin1: ");
  Serial.print(data.Lin1);    
  Serial.print(" | Lin2: ");  
  Serial.print(data.Lin2);      
  Serial.print(" | Speed: ");  
  Serial.print(data.Lena);        
  Serial.print(" | Servopos:");
  Serial.println(data.servopos);  
  delay(300);
}

void ReadData()
{ 
myRadio.openReadingPipe(1, 0xF0F0F0F066); // Which pipe to read, 40 bit Address
  myRadio.startListening(); //Stop Transminting and start Reveicing 
  if ( myRadio.available()) 
  {
    while (myRadio.available())
    {
      myRadio.read( &data, sizeof(data) );
    }
    Serial.print("Received:");
    Serial.println(data.msg);
  }
}
