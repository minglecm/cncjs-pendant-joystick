#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>
#include <PinChangeInterrupt.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2

#define DISTANCE_A 0
#define DISTANCE_B 1
#define DISTANCE_C 2
#define DISTANCE_D 3

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static int PIN_DISTANCE = 6;
static int PIN_AXIS = 7;
static int PIN_A = 8;
static int PIN_B = 9;
static int PIN_ENABLE = 5;
static int PIN_LED = 15;

int aVal = 0;
int bVal = 0;
int switchVal = 0;
int enabled = 0;

volatile int encoderPos = 0;
volatile int oldEncPos = 0;

Bounce2::Button axisButton = Bounce2::Button();
Bounce2::Button distanceButton = Bounce2::Button();

void setup() {
  pinMode(PIN_A, INPUT);
  pinMode(PIN_B, INPUT);
  pinMode(PIN_ENABLE, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  
  axisButton.attach(PIN_AXIS, INPUT_PULLUP);
  axisButton.setPressedState(LOW);

  distanceButton.attach(PIN_DISTANCE, INPUT_PULLUP);
  distanceButton.setPressedState(LOW);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(PIN_A), PinA_Interrupt, RISING);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(PIN_B), PinB_Interrupt, RISING);

  Serial.begin(115200);
  while(!Serial){}

  updateDisplay();
  digitalWrite(PIN_LED, 0);
}

int axis = 0;
int distance = 0;

void toggleAxis() {
  axis++;

  if(axis > AXIS_Z) {
    axis = AXIS_X;
  }
}

void toggleDistance() {
  distance++;

  if(distance > DISTANCE_D) {
    distance = DISTANCE_A;
  }
}

String getAxisLabel() {
  if(axis == AXIS_X) {
    return "X";
  } else if(axis == AXIS_Y) {
    return "Y";
  } else if(axis == AXIS_Z) {
    return "Z";
  }
}

float getDistance() {
  if(distance == DISTANCE_A) {
    return 1.0;
  } else if(distance == DISTANCE_B) {
    return 0.5;
  } else if(distance == DISTANCE_C) {
    return 0.25;
  } else if(distance == DISTANCE_D) {
    return 0.025;
  }
}

String getDistanceLabel() {
  return String(getDistance(), 3) + " mm";
}

void PinA_Interrupt(){
  bVal = digitalRead(PIN_B);
  if(!bVal) {
    encoderPos++;
  }
}

void PinB_Interrupt(){
    aVal = digitalRead(PIN_A);
    if(!aVal) {
      encoderPos --;
    }
}

void reset() {
  encoderPos = 0;
  oldEncPos = 0;
}

void loop(){
  axisButton.update();
  distanceButton.update();

  if ( axisButton.pressed() ) {
    toggleAxis();
    updateDisplay();
  }

  if ( distanceButton.pressed() ) {
    toggleDistance();
    updateDisplay();
  }
  
  switchVal = !digitalRead(PIN_ENABLE);

  if(switchVal) {
    if(!enabled) {
      // enabling
      reset();
      enabled = 1;
    }
  } else {
    enabled = 0;
  }

  if(enabled) {
    digitalWrite(PIN_LED, 1);
  } else {
    digitalWrite(PIN_LED, 0);
  }

  if(oldEncPos != encoderPos && enabled) {
    Serial.print(encoderPos - oldEncPos);
    Serial.print("|" + getAxisLabel());
    Serial.print("|" + String(getDistance(), 3));
    Serial.print("\n");
    
    oldEncPos = encoderPos;
  }
}

int lastDistance = -1;
int distanceX = 0;
int distanceY = 0;
int distanceW = 0;
int distanceH = 0;

void updateDisplay() {
  String axisLabel = getAxisLabel();
  String distanceLabel = getDistanceLabel();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 5);
  display.setTextSize(4);
  display.print(axisLabel);
  
  display.setTextSize(2);

  if(lastDistance != distance) {
    display.getTextBounds(distanceLabel, 0, 0, &distanceX, &distanceY, &distanceW, &distanceH);
    lastDistance = distance;
  }
  
  display.setCursor(SCREEN_WIDTH - distanceW, 10);
  display.print(distanceLabel);

  display.display();
}
