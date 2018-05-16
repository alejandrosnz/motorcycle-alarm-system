#include "Fsm.h"

// I/O
const int ARMED_SWITCH      = 2;
const int ARMED_LED         = 4;
const int BUZZER            = 3;
const int SIREN             = 8;
const int BLINKERS          = 12;
const int ACCELEROMETER_I2C = 0x68;

// Configuration
const bool REV_ARMED_SWITCH = true;
const long DELAY_PREARMED   = 5 * 1000;
const long DELAY_POSTWARN   = 120 * 1000L;
const long TIME_NOTIFY      = 600;
const long TIME_WARNED      = 2 * 1000;
const long TIME_ALARMED     = 30 * 1000;

// States
State state_disabled(NULL, &on_state_disabled, NULL);
State state_prearmed(NULL, &on_state_prearmed, NULL);
State state_armed(NULL, &on_state_armed, NULL);
State state_warn(NULL, &on_state_warn, NULL);
State state_alarm(NULL, &on_state_alarm, NULL);

// Events
const int EVENT_ACTIVATE    = 0xf0;
const int EVENT_DEACTIVATE  = 0xf1;
const int EVENT_ARM         = 0xf2;
const int EVENT_ALERT       = 0xf3;
const int EVENT_ALARM       = 0xf4;
const int EVENT_QUIET       = 0xf5;

// Setup initial state
Fsm fsm(&state_disabled);


// State DISABLED
void on_state_disabled(){
  log("State DISABLED");
  
  while(true){
    
    if(isArmed()){
      fsm.trigger(EVENT_ACTIVATE);
      
      break;
    }
  }
}

// Pre-State ARMED
void on_state_prearmed(){
  log("State PRE-ARMED");

  // BLINKERS
  int blinkersState = LOW;
  unsigned long blinkersPreviousMillis = 0;
  int blinkersOnTime = 100;
  int blinkersOffTime = 100;
  unsigned long currentMillis;
  unsigned long startTime = millis();

  while(true){
    currentMillis = millis();

    if (!isArmed()){
      digitalWrite(BLINKERS, LOW);
      
      fsm.trigger(EVENT_DEACTIVATE);
      break;
    }

    if(currentMillis >= startTime + TIME_NOTIFY){
      digitalWrite(BLINKERS, LOW);
      break;
    }

    if ((blinkersState == HIGH) && (currentMillis - blinkersPreviousMillis >= blinkersOnTime)){
      blinkersState = LOW;
      digitalWrite(BLINKERS, LOW);
      blinkersPreviousMillis = currentMillis;
    }else if ((blinkersState == LOW) && (currentMillis - blinkersPreviousMillis >= blinkersOffTime)){
      blinkersState = HIGH;
      digitalWrite(BLINKERS, HIGH);
      blinkersPreviousMillis = currentMillis;
    }
  }

  startTime = millis();

  while(true){
    currentMillis = millis();

    if (!isArmed()){
      fsm.trigger(EVENT_DEACTIVATE);
      break;
    }

    if(currentMillis >= startTime + DELAY_PREARMED){
      break;
    }
  }

  fsm.trigger(EVENT_ARM);
}

// State ARMED
void on_state_armed(){
 log("State ARMED");

  // ARMED_LED
  int armedLedState = LOW;
  unsigned long armedLedPreviousMillis = 0;
  int armedLedOnTime = 100;
  int armedLedOffTime = 1500;
  unsigned long currentMillis;
  unsigned long startTime = millis();

  while (true){
    currentMillis = millis();
    
    if (!isArmed()){
      digitalWrite(ARMED_LED, LOW);

      fsm.trigger(EVENT_DEACTIVATE);

      break;
    }

    if ((armedLedState == HIGH) && (currentMillis - armedLedPreviousMillis >= armedLedOnTime)){
      armedLedState = LOW;
      digitalWrite(ARMED_LED, LOW);
      armedLedPreviousMillis = currentMillis;
    }else if ((armedLedState == LOW) && (currentMillis - armedLedPreviousMillis >= armedLedOffTime)){
      armedLedState = HIGH;
      digitalWrite(ARMED_LED, HIGH);
      armedLedPreviousMillis = currentMillis;
    }

    // Read accelerometer

    // IF accelerometer read moves softly
    digitalWrite(ARMED_LED, LOW);
    fsm.trigger(EVENT_ALERT);

    // IF accelerometer read moves hardly
    digitalWrite(ARMED_LED, LOW);
    fsm.trigger(EVENT_ALARM);
  }
}

// State WARN
void on_state_warn(){
  log("State WARN");

  // BLINKERS & SHORT SIREN
  int blinkersState = LOW;
  unsigned long blinkersPreviousMillis = 0;
  int blinkersOnTime = 200;
  int blinkersOffTime = 200;
  int sirenState = LOW;
  unsigned long sirenPreviousMillis = 0;
  int sirenOnTime = 500;
  int sirenOffTime = 300;
  unsigned long currentMillis;
  unsigned long startTime = millis();

  while(true){
    currentMillis = millis();

    if (!isArmed()){
      digitalWrite(BLINKERS, LOW);
      
      fsm.trigger(EVENT_QUIET);
      break;
    }

    if(currentMillis >= startTime + TIME_WARNED){
      digitalWrite(BLINKERS, LOW);
      digitalWrite(SIREN, LOW);
      break;
    }

    if ((blinkersState == HIGH) && (currentMillis - blinkersPreviousMillis >= blinkersOnTime)){
      blinkersState = LOW;
      digitalWrite(BLINKERS, LOW);
      blinkersPreviousMillis = currentMillis;
    }else if ((blinkersState == LOW) && (currentMillis - blinkersPreviousMillis >= blinkersOffTime)){
      blinkersState = HIGH;
      digitalWrite(BLINKERS, HIGH);
      blinkersPreviousMillis = currentMillis;
    }

    if ((sirenState == HIGH) && (currentMillis - sirenPreviousMillis >= sirenOnTime)){
      sirenState = LOW;
      digitalWrite(SIREN, LOW);
      sirenPreviousMillis = currentMillis;
    }else if ((sirenState == LOW) && (currentMillis - sirenPreviousMillis >= sirenOffTime)){
      sirenState = HIGH;
      digitalWrite(SIREN, HIGH);
      sirenPreviousMillis = currentMillis;
    }
  }

  startTime = millis();

  while(true){
    currentMillis = millis();

    if (!isArmed()){
      fsm.trigger(EVENT_QUIET);
      break;
    }

    if (currentMillis >= startTime + DELAY_POSTWARN){
      break;
    }

    // Read accelerometer
    // IF accelerometer read moves
    fsm.trigger(EVENT_ALARM);
  }

  fsm.trigger(EVENT_QUIET);
}

// State ALARM
void on_state_alarm(){
  log("State ALARM");

  // BLINKERS & LONG SIREN
  int blinkersState = LOW;
  unsigned long blinkersPreviousMillis = 0;
  int blinkersOnTime = 200;
  int blinkersOffTime = 200;
  int sirenState = LOW;
  unsigned long sirenPreviousMillis = 0;
  int sirenOnTime = 500;
  int sirenOffTime = 300;
  unsigned long currentMillis;
  unsigned long startTime = millis();

  while(true){
    currentMillis = millis();

    if (!isArmed()){
      digitalWrite(BLINKERS, LOW);
      digitalWrite(SIREN, LOW);

      fsm.trigger(EVENT_QUIET);
      break;
    }

    if(currentMillis >= startTime + TIME_ALARMED){
      digitalWrite(BLINKERS, LOW);
      digitalWrite(SIREN, LOW);

      break;
    }

    if ((blinkersState == HIGH) && (currentMillis - blinkersPreviousMillis >= blinkersOnTime)){
      blinkersState = LOW;
      digitalWrite(BLINKERS, LOW);
      blinkersPreviousMillis = currentMillis;
    }else if ((blinkersState == LOW) && (currentMillis - blinkersPreviousMillis >= blinkersOffTime)){
      blinkersState = HIGH;
      digitalWrite(BLINKERS, HIGH);
      blinkersPreviousMillis = currentMillis;
    }

    if ((sirenState == HIGH) && (currentMillis - sirenPreviousMillis >= sirenOnTime)){
      sirenState = LOW;
      digitalWrite(SIREN, LOW);
      sirenPreviousMillis = currentMillis;
    }else if ((sirenState == LOW) && (currentMillis - sirenPreviousMillis >= sirenOffTime)){
      sirenState = HIGH;
      digitalWrite(SIREN, HIGH);
      sirenPreviousMillis = currentMillis;
    }
  }

  fsm.trigger(EVENT_QUIET);
}

void setup() {
  Serial.begin(9600);

  // I/O
  pinMode(ARMED_SWITCH, INPUT);
  pinMode(ARMED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(SIREN, OUTPUT);
  pinMode(BLINKERS, OUTPUT);

  // Transations
  // Activate Alarm
  fsm.add_transition(&state_disabled, &state_prearmed,
                     EVENT_ACTIVATE, NULL);
  // Arm Alarm
  fsm.add_transition(&state_prearmed, &state_armed, 
                     EVENT_ARM, NULL);
  // Deactivate Alarm
  fsm.add_transition(&state_prearmed, &state_disabled,
                     EVENT_DEACTIVATE, NULL);
  fsm.add_transition(&state_armed, &state_disabled,
                     EVENT_DEACTIVATE, NULL);
  // Alert
  fsm.add_transition(&state_armed, &state_warn,
                     EVENT_ALERT, NULL);
  // Alarm
  fsm.add_transition(&state_armed, &state_alarm,
                     EVENT_ALARM, NULL);
  fsm.add_transition(&state_warn, &state_alarm,
                     EVENT_ALARM, NULL);
  // Quiet
  fsm.add_transition(&state_warn, &state_armed,
                     EVENT_QUIET, NULL);
  fsm.add_transition(&state_alarm, &state_armed,
                     EVENT_QUIET, NULL);

  // Start state machine
  fsm.run_machine();
}

void loop() { 
  // Empty

}

void log(String msg){
  Serial.println(msg);
}

bool isArmed(){
  return digitalRead(ARMED_SWITCH) ^ REV_ARMED_SWITCH;
}
