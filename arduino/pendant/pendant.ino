#include <AlignedJoy.h>
#include <Coordinates.h>
#include <PinChangeInterrupt.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>

#define JOYSTICK_MIN 0
#define JOYSTICK_MAX 1023
#define JOYSTICK_MID JOYSTICK_MAX / 2
#define JOYSTICK_MAX_R 512 // pot is actually square :\, so this is the max radius of X=0.
#define JOYSTICK_R_TOLERANCE 50
#define JOYSTICK_REPORTING_MS 100

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C

static int PIN_BUTTON_AXIS = 6;
static int PIN_BUTTON_DISTANCE = 7;
static int PIN_BUTTON_JOYSTICK = 14;
static int PIN_JOYSTICK_X = A0;
static int PIN_JOYSTICK_Y = A1;
static int PIN_JOYSTICK_Z = A2;
static int PIN_LED_JOYSTICK = 10;
static int PIN_LED_MPG = 16;
static int PIN_MPG_A = 8;
static int PIN_MPG_B = 9;
static int PIN_SWITCH_JOYSTICK_ENABLE = 4;
static int PIN_SWITCH_MPG_ENABLE = 5;

static int MODE_OFF = 0;
static int MODE_JOYSTICK = 1;
static int MODE_MPG = 2;

// system
bool firstBoot = true;
int mode = MODE_OFF;

// mpg
static int AXIS_X = 0;
static int AXIS_Y = 1;
static int AXIS_Z = 2;
static int DISTANCE_A = 0;
static float DISTANCE_A_VALUE = 1.0;
static int DISTANCE_B = 1;
static float DISTANCE_B_VALUE = 0.5;
static int DISTANCE_C = 2;
static float DISTANCE_C_VALUE = 0.25;
static int DISTANCE_D = 3;
static float DISTANCE_D_VALUE = 0.025;
static int MAX_DISTANCE = DISTANCE_D;

int aVal = 0;
int bVal = 0;
volatile int encoderPos = 0;
volatile int oldEncPos = 0;
int axis = 0;
int distance = 0;

// joystick
Coordinates point = Coordinates();
AlignedJoy joystick_1(PIN_JOYSTICK_X, PIN_JOYSTICK_Y);
unsigned long lastJoystickPrintTime;

int rVal = 0;
int xVal = 0;
int yVal = 0;
int zVal = 0;
const float RAD_PER_SEG = M_PI / 8;

// screen stuff
int lastDistance = -1;
int distanceX = 0;
int distanceY = 0;
int distanceW = 0;
int distanceH = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// buttons
Bounce2::Button axisButton = Bounce2::Button();
Bounce2::Button distanceButton = Bounce2::Button();
Bounce2::Button joystickButton = Bounce2::Button();

// switches
Bounce joystickEnable = Bounce();
Bounce mpgEnable = Bounce();

void setup() {
  axisButton.attach(PIN_BUTTON_AXIS, INPUT_PULLUP);
  axisButton.setPressedState(LOW);

  distanceButton.attach(PIN_BUTTON_DISTANCE, INPUT_PULLUP);
  distanceButton.setPressedState(LOW);

  joystickButton.attach(PIN_BUTTON_JOYSTICK, INPUT_PULLUP);
  joystickButton.setPressedState(LOW);

  joystickEnable.attach(PIN_SWITCH_JOYSTICK_ENABLE, INPUT_PULLUP);
  joystickEnable.interval(5);

  mpgEnable.attach(PIN_SWITCH_MPG_ENABLE, INPUT_PULLUP);
  mpgEnable.interval(5);
  
  pinMode(PIN_LED_JOYSTICK, OUTPUT);
  pinMode(PIN_LED_MPG, OUTPUT);

  pinMode(PIN_MPG_A, INPUT);
  pinMode(PIN_MPG_B, INPUT);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(PIN_MPG_A), MPG_PinA_Interrupt, RISING);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(PIN_MPG_B), MPG_PinB_Interrupt, RISING);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  clearDisplay();

  Serial.begin(115200);
  while(!Serial){}
  Serial.println("on");
}

void MPG_PinA_Interrupt(){
  bVal = digitalRead(PIN_MPG_B);
  if(!bVal) {
    encoderPos++;
  }
}

void MPG_PinB_Interrupt(){
    aVal = digitalRead(PIN_MPG_A);
    if(!aVal) {
      encoderPos --;
    }
}

void MPG_Reset() {
  encoderPos = 0;
  oldEncPos = 0;
}

void setModeLEDState() {
  if(mode == MODE_OFF) {
    digitalWrite(PIN_LED_JOYSTICK, 0);
    digitalWrite(PIN_LED_MPG, 0);
  } else if(mode == MODE_JOYSTICK) {
    digitalWrite(PIN_LED_JOYSTICK, 1);
    digitalWrite(PIN_LED_MPG, 0);    
  } else if(mode == MODE_MPG) {
    digitalWrite(PIN_LED_JOYSTICK, 0);
    digitalWrite(PIN_LED_MPG, 1);
  }
}

void printJoystickData(int x, int y, int z, float angle, float r) {
  Serial.print("JD|");

  if(rVal < JOYSTICK_R_TOLERANCE) {
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
  Serial.print(angle);
  
  Serial.print("|");
  Serial.print(min(r / JOYSTICK_MAX_R, 1.0));

  Serial.print("|");
  Serial.print((z - JOYSTICK_MID)/float(JOYSTICK_MID));

  Serial.print("\n");
}

void clearDisplay() {
  display.clearDisplay();
  display.display();
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

void toggleAxis() {
  axis++;

  if(axis > AXIS_Z) {
    axis = AXIS_X;
  }
}

float getDistance() {
  if(distance == DISTANCE_A) {
    return DISTANCE_A_VALUE;
  } else if(distance == DISTANCE_B) {
    return DISTANCE_B_VALUE;
  } else if(distance == DISTANCE_C) {
    return DISTANCE_C_VALUE;
  } else if(distance == DISTANCE_D) {
    return DISTANCE_D_VALUE;
  }
}

String getDistanceLabel() {
  return String(getDistance(), 3) + " mm";
}

void toggleDistance() {
  distance++;

  if(distance > MAX_DISTANCE) {
    distance = DISTANCE_A;
  }
}

void updateDisplay() {
  String axisLabel = getAxisLabel();
  String distanceLabel = getDistanceLabel();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 5);
  display.setTextSize(4);
  display.print(axisLabel);
  
  display.setTextSize(2);

  display.getTextBounds(distanceLabel, 0, 0, &distanceX, &distanceY, &distanceW, &distanceH);
  
  display.setCursor(SCREEN_WIDTH - distanceW, 10);
  display.print(distanceLabel);

  display.display();
}

void setModeJoystick() {
  mode = MODE_JOYSTICK;
}

void setModeMPG() {
  mode = MODE_MPG;
  MPG_Reset();
  updateDisplay();
}

void setModeOff() {
  mode = MODE_OFF;
}

void handleJoystick() {
    if (joystickButton.pressed()) {
      Serial.println("JB");
    }
  
    xVal = joystick_1.read(X);
    yVal = joystick_1.read(Y);
    zVal = analogRead(PIN_JOYSTICK_Z);

    xVal = JOYSTICK_MID - xVal;

    if(yVal >= JOYSTICK_MID) {
      yVal = 1 - (yVal - JOYSTICK_MID);
    } else {
      yVal = JOYSTICK_MID - yVal;
    }

    point.fromCartesian(xVal, yVal);
    rVal = point.getR();

    unsigned long now = millis();
    
    if(now - lastJoystickPrintTime > JOYSTICK_REPORTING_MS) {
      printJoystickData(xVal, yVal, zVal, point.getAngle(), rVal);
      lastJoystickPrintTime = now;
    }
}

void handleMPG() {
  bool shouldUpdateDisplay = false;

  if (axisButton.pressed()) {
    toggleAxis();
    shouldUpdateDisplay = true;
  }

  if (distanceButton.pressed()) {
    toggleDistance();
    shouldUpdateDisplay = true;
  }

  if(shouldUpdateDisplay) {
    updateDisplay();
  }

  if(oldEncPos != encoderPos) {
    Serial.print("M|");
    Serial.print(encoderPos - oldEncPos);
    Serial.print("|" + getAxisLabel());
    Serial.print("|" + String(getDistance(), 3));
    Serial.print("\n");
    
    oldEncPos = encoderPos;
  }
}

void loop() {
  axisButton.update();
  distanceButton.update();
  joystickButton.update();
  joystickEnable.update();
  mpgEnable.update();

  if(firstBoot) {
      if (joystickEnable.read() == LOW) {
        setModeJoystick();
      } else if (mpgEnable.read() == LOW) {
        setModeMPG();
      }
  } else {
    if(joystickEnable.changed()) {
      if (joystickEnable.read() == LOW) {
        setModeJoystick();
      } else {
        setModeOff();
      }
    }

    if(mpgEnable.changed()) {
      if (mpgEnable.read() == LOW) {
        setModeMPG();
      } else {
        clearDisplay();
        setModeOff();
      }
    }
  }

  setModeLEDState();

  if(mode == MODE_JOYSTICK) {
    handleJoystick();
  } else if(mode == MODE_MPG) {
    handleMPG();
  }

  firstBoot = false;
}