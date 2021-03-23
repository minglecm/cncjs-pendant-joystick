#include <Coordinates.h>
#include <AlignedJoy.h>

#define PIN_BUTTON 14
#define PIN_X  A0
#define PIN_Y  A1
#define PIN_Z  A2

#define SCAN_INTERVAL 100

#define G_MIN 0
#define G_MAX 1023
#define G_MID G_MAX / 2
#define G_MAX_R 512 // pot is actually square :\, so this is the max radius of X=0.
#define G_R_TOLERANCE 50

const float RAD_PER_SEG = M_PI / 8;

int buttonVal = 0;
int xVal = 0;
int yVal = 0;
int zVal = 0;
int rVal = 0;

Coordinates point = Coordinates();

AlignedJoy joystick_1(PIN_X, PIN_Y);

void setup() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  Serial.begin(115200);
  while(!Serial){}
}

void printData(int x, int y, int z, float angle, float r, int buttonVal) {
  if(rVal < G_R_TOLERANCE) {
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
  Serial.print(min(r / G_MAX_R, 1.0));

  Serial.print("|");
  Serial.print((z - G_MID)/float(G_MID));

  Serial.print("|");
  if(buttonVal == 0) {
    Serial.print(1);
  } else {
    Serial.print(0);
  }

  Serial.print("\n");
}

void loop() {
  xVal = joystick_1.read(X);
  yVal = joystick_1.read(Y);
  zVal = analogRead(PIN_Z);
  buttonVal = digitalRead(PIN_BUTTON);

  xVal = G_MID - xVal;

  if(yVal >= G_MID) {
    yVal = yVal - G_MID;
  } else {
    yVal = 1 - (G_MID - yVal);
  }

  point.fromCartesian(xVal, yVal);
  rVal = point.getR();

  printData(xVal, yVal, zVal, point.getAngle(), rVal, buttonVal);

END:
  delay(SCAN_INTERVAL);
}
