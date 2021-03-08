#include <Coordinates.h>
#include <AlignedJoy.h>

#define PIN_X  A1
#define PIN_Y  A2
#define PIN_Z  A0

#define SCAN_INTERVAL 200

#define G_MIN 0
#define G_MAX 1023
#define G_MID G_MAX / 2
#define G_MAX_R 723
#define G_TOLERANCE 50

const float RAD_PER_SEG = M_PI / 8;

int xVal = 0;
int yVal = 0;
int zVal = 0;
int rVal = 0;

Coordinates point = Coordinates();

AlignedJoy joystick_1(PIN_X, PIN_Y);

void setup() {
  Serial.begin(115200);
  while(!Serial){}
}

void printData(int y, float angle, float r) {
  if(rVal < G_TOLERANCE) {
    Serial.print("CENTER");
  } else {
    if(angle < RAD_PER_SEG) {
      Serial.print("EAST");
    } else if(angle < 3*RAD_PER_SEG) {
      Serial.print("NORTHEAST");
    } else if(angle < 5*RAD_PER_SEG) {
      Serial.print("NORTH");
    } else if(angle < 7*RAD_PER_SEG) {
      Serial.print("NORTHWEST");
    } else if(angle < 9*RAD_PER_SEG) {
      Serial.print("WEST");
    } else if(angle < 11*RAD_PER_SEG) {
      Serial.print("SOUTHWEST");
    } else if(angle < 13*RAD_PER_SEG) {
      Serial.print("SOUTH");
    } else if(angle < 15*RAD_PER_SEG) {
      Serial.print("SOUTHEAST");
    } else if(angle <= 2*M_PI) {
      Serial.print("EAST");
    } else {
      Serial.print("DUNNO-");
      Serial.print(angle);
    }
  }
  
  Serial.print("|");
  Serial.print(r / G_MAX_R);
  
  Serial.print("\n");
}

void loop() {
  xVal = joystick_1.read(X);
  yVal = joystick_1.read(Y);

  xVal = G_MID - xVal;

  if(yVal >= G_MID) {
    yVal = yVal - G_MID;
  } else {
    yVal = 1 - (G_MID - yVal);
  }

  point.fromCartesian(xVal, yVal);
  rVal = point.getR();


 
//  Serial.print("X -> ");
//  Serial.print(xVal);
//  
//  Serial.print(" | Y -> ");
//  Serial.print(yVal);
//  
//  Serial.print(" r: ");
//  Serial.print(rVal);
//
//  Serial.print(" φ: ");
//  Serial.print(point.getAngle());
//  Serial.print(" rad ");

  printData(yVal, point.getAngle(), rVal);

END:
  delay(SCAN_INTERVAL);
}
