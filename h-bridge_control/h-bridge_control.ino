#define DEBOUNCE_TIME 50
#define PULSE_WIDTH   300
#define BTN_LEFT      1
#define BTN_RIGHT     2

enum eMotorState{
  LEFT,
  RIGHT,
  STOP,
  STBY
};

enum eBtnState {
  BTN_PRESS,
  BTN_RELEASE,
  BTN_HOLD,
  INIT
};

/* CONTROL PUNTE H */
const int out1 = 8;
const int out2 = 9;
const int pwmo = 10;
const int stby = 11;

/* Button structure */
typedef struct{
  int inputPin;
  int outputPin;
  int readState;
  int prevState;
  int btnPos;
  enum eBtnState btnState;
  unsigned long lastDebounceTime;
  unsigned long pressedTime;
  unsigned long releasedTime;
  unsigned long pulseTime;
}stButton;

stButton leftButton;
stButton rightButton;

enum eMotorState motorPos;
enum eMotorState prevMotorPos;

unsigned long startTime = millis();

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("Program started");
  Serial.println(startTime);

  motorPos = STBY;
  prevMotorPos = STBY;
  // Init the structures
  leftButton.inputPin = 2;
  leftButton.readState = HIGH;
  leftButton.prevState = HIGH;
  leftButton.lastDebounceTime = 0;
  leftButton.pressedTime = 0;
  leftButton.releasedTime = 0;
  leftButton.pulseTime = 0;
  leftButton.btnPos = BTN_LEFT;
  leftButton.btnState = INIT;

  rightButton.inputPin = 3;
  rightButton.readState = HIGH;
  rightButton.prevState = HIGH;
  rightButton.lastDebounceTime = 0;
  rightButton.pressedTime = 0;
  rightButton.releasedTime = 0;
  rightButton.pulseTime = 0;
  rightButton.btnPos = BTN_RIGHT;
  rightButton.btnState = INIT;

  // Configure input and outputs
  pinMode(leftButton.inputPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(leftButton.inputPin), leftIsr, CHANGE);

  pinMode(rightButton.inputPin, INPUT_PULLUP);  
  attachInterrupt(digitalPinToInterrupt(rightButton.inputPin), rightIsr, CHANGE);

  pinMode(out1, OUTPUT);
  pinMode(out2, OUTPUT);
  pinMode(pwmo, OUTPUT);
  pinMode(stby, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  pollChanges(&leftButton);
  pollChanges(&rightButton);
  if(motorPos != prevMotorPos)
  {
    controlHBridge(motorPos);
    prevMotorPos = motorPos;
  }
}

void controlHBridge(eMotorState motorState)
{
  switch(motorState)
  {
    case LEFT:
      // motor spin CCW
      Serial.println("LEFT");
      digitalWrite(out1, LOW);
      digitalWrite(out2, HIGH);
      digitalWrite(pwmo, HIGH);
      digitalWrite(stby, HIGH);
    break;
    case RIGHT:
      // motor spin CW
      Serial.println("RIGHT");
      digitalWrite(out1, HIGH);
      digitalWrite(out2, LOW);
      digitalWrite(pwmo, HIGH);
      digitalWrite(stby, HIGH);
    break;
    case STOP:
      // stop the motor
      Serial.println("STOP");
      digitalWrite(out1, LOW);
      digitalWrite(out2, LOW);
      digitalWrite(pwmo, HIGH);
      digitalWrite(stby, HIGH);
    break;
    case STBY:
      // motor on power saving
      Serial.println("Stand By");
      digitalWrite(out1, LOW);
      digitalWrite(out2, LOW);
      digitalWrite(pwmo, LOW);
      digitalWrite(stby, LOW);
    break;
    default:
      // motor on power saving
      Serial.println("Stand By");
      digitalWrite(out1, LOW);
      digitalWrite(out2, LOW);
      digitalWrite(pwmo, LOW);
      digitalWrite(stby, LOW);
    break;
  }
}

void changeMotorPos(stButton* pButton)
{
  if(pButton->btnPos == BTN_LEFT)
  {
    if(motorPos == STBY || motorPos == STOP)
    {
      motorPos = LEFT;
    }else{
      motorPos = STOP;
    }
  }else if(pButton->btnPos == BTN_RIGHT)
  {
    if(motorPos == STBY || motorPos == STOP)
    {
      motorPos = RIGHT;
    }else{
      motorPos = STOP;
    }
  }
}

void interpretPulse(stButton* pButton)
{
  Serial.println("interpretPulse");
  if(pButton->btnState == BTN_HOLD)
  {
    motorPos = STOP;
    pButton->btnState = BTN_RELEASE;
  }else{
     Serial.println("Interpret PULSE");
    if(pButton->pulseTime < PULSE_WIDTH)
    {
      // put motor in continous run
      changeMotorPos(pButton);
    }
  }
}

void pollChanges(stButton* pButton)
{
  if(pButton->prevState == LOW)
  {
    int currentTime = millis();
    if(currentTime - pButton->pressedTime >= PULSE_WIDTH && pButton->btnState != BTN_HOLD)
    {
      // spin motor while btn is held
      pButton->btnState = BTN_HOLD;
      changeMotorPos(pButton);
    }
  }
  
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
        pButton->btnState = BTN_PRESS;
      }else{
        // else the btn was released
        pButton->releasedTime = millis();
        pButton->pulseTime = pButton->releasedTime - pButton->pressedTime;
        if(pButton->btnState == BTN_PRESS)
        {
          pButton->btnState == BTN_RELEASE;
        }
        interpretPulse(pButton); 
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
