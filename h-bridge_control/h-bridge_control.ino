#define BTN_DEBOUNCE_TIME 70    // timp de debounce
#define PULSE_WIDTH   300       // cat sa tii butonul apasat sa intre in auto

#define BTN_LEFT      1
#define BTN_RIGHT     2
#define RUN_MANUAL    1
#define RUN_AUTO      2
#define RUN_STOP_AUTO 3

#define RUN_TIME      1000    // timp rulare in ciclu
#define PAUSE_TIME    3000    // timp pauza in ciclu

/* CONTROL PUNTE H */
/* OUTPUT urile pentru control de punte H */
const int out1 = 8;
const int out2 = 9;
const int pwmo = 10;
const int stby = 11;


enum eHBridgeCMD
{
  LEFT,
  RIGHT,
  BRAKE,
  STBY
};

enum eMotorState{
  RUN,
  STOP,
  IDL
};

enum eBtnState {
  BTN_PRESS,
  BTN_RELEASE,
  BTN_HOLD,
  INIT
};

enum eMotorDir{
  CCW,
  CW
};

typedef struct{
  enum eMotorState eState;
  enum eMotorState ePrevState;
  enum eMotorDir eDir;
  enum eHBridgeCMD eCMD;
  int runType;
  unsigned long timeStarted;
  unsigned long timePaused;
}stMotor;

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
stMotor motor;

unsigned long startTime = millis();

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("Program started");
  Serial.println(startTime);

  // Init the structures
  leftButton.inputPin = 2;          // input buton 1
  leftButton.readState = HIGH;
  leftButton.prevState = HIGH;
  leftButton.lastDebounceTime = 0;
  leftButton.pressedTime = 0;
  leftButton.releasedTime = 0;
  leftButton.pulseTime = 0;
  leftButton.btnPos = BTN_LEFT;
  leftButton.btnState = INIT;

  rightButton.inputPin = 3;       // input buton 2
  rightButton.readState = HIGH;
  rightButton.prevState = HIGH;
  rightButton.lastDebounceTime = 0;
  rightButton.pressedTime = 0;
  rightButton.releasedTime = 0;
  rightButton.pulseTime = 0;
  rightButton.btnPos = BTN_RIGHT;
  rightButton.btnState = INIT;

  motor.timeStarted = 0;
  motor.timePaused = 0;
  motor.eState = IDL;

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
  cycleMotor();
  if(motor.eState != motor.ePrevState)
  {
    controlHBridge(motor.eCMD);
    motor.ePrevState = motor.eState;
  }
}

void updateCommand()
{
  if(motor.eState == RUN)
  {
    if(motor.eDir == CCW)
    {
      motor.eCMD = LEFT;
    }else{
      motor.eCMD = RIGHT;
    }
  }else{
    motor.eCMD = BRAKE;
  }
}

void cycleMotor()
{
  if(motor.runType == RUN_AUTO)
  {
    if(millis() - motor.timeStarted >= RUN_TIME && motor.eState != STOP)
    {
      //pause
      motor.timePaused = millis();
      motor.ePrevState = motor.eState;
      motor.eState = STOP;
      Serial.println("<---------CYCLE-------->");
      Serial.println(motor.eState);
    }
    if(millis() - motor.timePaused >= PAUSE_TIME && motor.eState == STOP)
    {
      //start
      motor.timeStarted = millis();
      motor.ePrevState = motor.eState;
      motor.eState = RUN;
      Serial.println("<---------CYCLE-------->");
      Serial.println(motor.eState);
    }
  }else if(motor.runType == RUN_STOP_AUTO)
  {
    motor.ePrevState = motor.eState;
    motor.eState = STOP;
    motor.runType = RUN_MANUAL;
    Serial.println("<---------CYCLE-------->");
    Serial.println(motor.eState);
  }
  updateCommand();
}

void controlHBridge(eHBridgeCMD motorState)
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
    case BRAKE:
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
  Serial.println(pButton->btnState);
  if(pButton->btnPos == BTN_LEFT)
  {
    if(motor.eState == IDL || motor.eState == STOP)
    {
      motor.eDir = CCW;
      motor.eState = RUN;
    }else{
      motor.eState = STOP;
    }
  }else if(pButton->btnPos == BTN_RIGHT)
  {
    if(motor.eState == IDL || motor.eState == STOP)
    {
      motor.eDir = CW;
      motor.eState = RUN;
    }else{
      motor.eState = STOP;
    }
  }
  updateCommand();
}

void interpretPulse(stButton* pButton)
{
  Serial.println("interpretPulse");
  if(pButton->btnState == BTN_HOLD)
  {
    motor.eState = STOP;
    pButton->btnState = BTN_RELEASE;
    motor.eCMD = BRAKE;
  }else{
     Serial.println("short PULSE");
    if(pButton->pulseTime < PULSE_WIDTH)
    {
      // put motor in continous run
      if(motor.runType == RUN_AUTO)
      {
        motor.runType = RUN_STOP_AUTO;
      }else{
        motor.runType = RUN_AUTO;
      }
      changeMotorPos(pButton);
    }
  }
}

void pollChanges(stButton* pButton)
{
  if(pButton->prevState == LOW)
  {
    unsigned long currentTime = millis();
    if(currentTime - pButton->pressedTime >= PULSE_WIDTH && pButton->btnState == BTN_PRESS)
    {
      // spin motor while btn is held
      pButton->btnState = BTN_HOLD;
      Serial.println("HOLD");
      if(motor.runType == RUN_AUTO)
      {
        motor.runType = RUN_STOP_AUTO;
      }else{
        changeMotorPos(pButton);
      } 
    }
  }
  
  if((millis() - pButton->lastDebounceTime) > BTN_DEBOUNCE_TIME)
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
        Serial.println("FIRST PRESS");
        pButton->btnState = BTN_PRESS;
      }else{
        // else the btn was released
        Serial.println("RELEASE");
        pButton->releasedTime = millis();
        pButton->pulseTime = pButton->releasedTime - pButton->pressedTime;
        pButton->btnState = BTN_RELEASE;
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
