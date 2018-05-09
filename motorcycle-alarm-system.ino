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
const int DELAY_PREARMED    = 5;
const int DELAY_POSTWARN    = 5;
const int TIME_ALARMED      = 5;

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
  int blinkersOnTime = 500;
  int blinkersOffTime = 500;
  int totalTime = 3 * 1000;
  unsigned long currentMillis = millis();
  unsigned long startTime = currentMillis;

  while(currentMillis < (startTime + totalTime)){
    currentMillis = millis();

    if (!isArmed()){
      digitalWrite(BLINKERS, LOW);
      
      fsm.trigger(EVENT_DEACTIVATE);
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
}

// State ARMED
void on_state_armed(){
 log("State ARMED");

  while (true){
    if (!isArmed()){
      fsm.trigger(EVENT_DEACTIVATE);

      break;
    }

    // Read accelerometer

    // IF accelerometer read moves softly
    fsm.trigger(EVENT_ALERT);

    // IF accelerometer read moves hardly
    fsm.trigger(EVENT_ALARM);
  }
}

// State WARN
void on_state_warn(){
  log("State WARN");

  // BLINKERS

  // SHORT SIREN

  while(true){
    // Read accelerometer
    
    // IF accelerometer read moves
    fsm.trigger(EVENT_ALARM);
  }
}

// State ALARM
void on_state_alarm(){
  log("State ALARM");

  while (true){
    if (!isArmed()){
      fsm.trigger(EVENT_QUIET);

      break;
    }
    
    // BLINKERS

    // SIREN
  }
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
  fsm.add_timed_transition(&state_prearmed, &state_armed, 
                     DELAY_PREARMED * 1000, NULL);
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
  fsm.add_timed_transition(&state_warn, &state_armed,
                     DELAY_POSTWARN * 1000, NULL);
  fsm.add_transition(&state_alarm, &state_armed,
                     EVENT_QUIET, NULL);
  fsm.add_timed_transition(&state_alarm, &state_warn,
                     TIME_ALARMED * 1000, NULL);

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
  return digitalWrite(ARMED_SWITCH) ^ REV_ARMED_SWITCH;
}
