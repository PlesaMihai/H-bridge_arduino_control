#define DEBOUNCE_TIME 100

/* Button structure */
typedef struct{
  int inputPin;
  int outputPin;
  int readState;
  int prevState;
  unsigned long lastDebounceTime;
  unsigned long pressedTime;
  unsigned long releasedTime;
  unsigned long pulseTime;
}stButton;

stButton leftButton;
stButton rightButton;

int changes = 0;

void setup() {
  // Init the structures
  leftButton.inputPin = 2;
  leftButton.outputPin = 8;
  leftButton.prevState = HIGH;
  leftButton.lastDebounceTime = 0;

  rightButton.inputPin = 3;
  rightButton.outputPin = 9;
  rightButton.prevState = HIGH;
  rightButton.lastDebounceTime = 0;
  
  // Configure input and outputs
  pinMode(leftButton.inputPin, INPUT_PULLUP);
  pinMode(leftButton.outputPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(leftButton.inputPin), leftIsr, CHANGE);

  pinMode(rightButton.inputPin, INPUT_PULLUP);
  pinMode(rightButton.outputPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(rightButton.inputPin), rightIsr, CHANGE);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  pollChanges(&leftButton);
  pollChanges(&rightButton);
  if(changes != 0)
  {
    changes = 0;
    Serial.println("presTime");
    Serial.println(leftButton.pressedTime);
    Serial.println("releasedTime");
    Serial.println(leftButton.releasedTime);
    Serial.println("pulseTime");
    Serial.println(leftButton.pulseTime);
  }
}

void pollChanges(stButton* pButton)
{
  if((millis() - pButton->lastDebounceTime) > DEBOUNCE_TIME)
  {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if(pButton->readState != pButton->prevState)
    {
      pButton->prevState = pButton->readState;
      if(pButton->readState == LOW)
      {
        // if the state is low the btn is pressed
        pButton->pressedTime = millis();
      }else{
        // else the btn was released
        pButton->releasedTime = millis();
        pButton->pulseTime = pButton->releasedTime - pButton->pressedTime;
        changes = 1;
      }
    }
  }
}

void debounceInput(stButton* pButton)
{
  // read the pin state
  pButton->readState = digitalRead(pButton->inputPin);
  // if the sw changed, due to noise or pressing
  if(pButton->readState != pButton->prevState)
  {
    pButton->lastDebounceTime = millis();
  }
}

void leftIsr()
{
  debounceInput(&leftButton);
}

void rightIsr()
{
  debounceInput(&rightButton);
}
