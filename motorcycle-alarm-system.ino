#include "Fsm.h"

// I/O
const int ARMED_SWITCH  = 2;
const int ARMED_LED     = 4;
const int BUZZER        = 3;
const int SIREN         = 8;
const int BLINKERS      = 12;
const int ACCMETER_I2C  = 0x68;

// States
State state_disabled(NULL, &on_state_disabled, NULL);
State state_prearmed(NULL, &on_state_prearmed, NULL);
State state_armed(NULL, &on_state_armed, NULL);
State state_warn(NULL, &on_state_warn, NULL);
State state_alarm(NULL, &on_state_alarm, NULL);

// Events
enum{
  EVENT_ACTIVATE,
  EVENT_DEACTIVATE,
  EVENT_ARM,
  EVENT_ALERT,
  EVENT_QUIET,
  EVENT_ALARM
}

// Setup initial state
Fsm fsm(&state_disabled);

// Constants
const int DELAY_PREARMED = 5;

// State DISABLED
void on_state_disabled(){
  log("State DISABLED");

  while(true){  
    if(digitalRead(ARMED_SWITCH) == LOW){
      fsm.trigger(EVENT_ACTIVATE);
      
      break;
    }
  }
}

// Pre-State ARMED
void on_state_prearmed(){
  log("State PRE-ARMED");

  // BUZZ

  delay(DELAY_PREARMED * 1000);

  fsm.trigger(EVENT_ARM);
}

// State ARMED
void on_state_armed(){
 log("State ARMED");

  while (true){
    if (digitalRead(ARMED_SWITCH) == HIGH){
      fsm.trigger(EVENT_DEACTIVATE);

      break;
    }
  }
}

// State WARN
void on_state_warn(){
  log("State WARN");
}

// State ALARM
void on_state_alarm(){
  log("State ALARM");
}

void setup() {
  Serial.begin(9600);

  // Transations
  // Activate Alarm
  fsm.add_transition(&state_disabled, &state_prearmed,
                     EVENT_ACTIVATE, NULL);
  // Arm Alarm
  fsm.add_transition(&state_prearmed, &state_armed,
                     EVENT_ACTIVATE, NULL);
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
  // put your main code here, to run repeatedly: 
  
}

void log(String msg){
  Serial.println(msg);
}
