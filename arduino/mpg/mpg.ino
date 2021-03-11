static int PIN_A = 2;
static int PIN_B = 3;
static int PIN_ENABLE = 5;
static int PIN_LED = 15;

int aVal = 0;
int bVal = 0;
int switchVal = 0;
int enabled = 0;

volatile int encoderPos = 0;
volatile int oldEncPos = 0;

void setup() {
  pinMode(PIN_A, INPUT);
  pinMode(PIN_B, INPUT);
  pinMode(PIN_ENABLE, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_A), PinA_Interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_B), PinB_Interrupt, RISING);

  Serial.begin(115200); // start the serial monitor link
  while(!Serial){}
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
    Serial.println(encoderPos - oldEncPos);
    oldEncPos = encoderPos;
  }
}
