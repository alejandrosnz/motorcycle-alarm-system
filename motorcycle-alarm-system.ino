#include "Fsm.h"


// States
State state_disabled(NULL, &on_state_disabled, NULL);
State state_armed(&on_state_armed_enter, &on_state_armed, NULL);
State state_warn(NULL, &on_state_warn, NULL);
State state_alarm(NULL, &on_state_alarm, NULL);

// State DISABLED
void on_state_disabled(){
  
}

// Pre-State ARMED
void on_state_armed_enter(){
  
}

// State ARMED
void on_state_armed(){
  
}

// State WARN
void on_state_warn(){
  
}

// State ALARM
void on_state_alarm(){
  
}

void setup() {
  // Setup initial state
  Fsm fsm(&state_disabled);

  // Transations
  // Activate Alarm
  fsm.add_transition(&state_disabled, &state_armed,
                     NULL, NULL);
  // Deactivate Alarm
  fsm.add_transition(&state_armed, &state_disabled,
                     NULL, NULL);
  // Alert
  fsm.add_transition(&state_armed, &state_warn,
                     NULL, NULL);
  // Quiet
  fsm.add_transition(&state_warn, &state_armed,
                     NULL, NULL);
  fsm.add_transition(&state_alarm, &state_armed,
                     NULL, NULL);
  // Alarm
  fsm.add_transition(&state_armed, &state_alarm,
                     NULL, NULL);
  fsm.add_transition(&state_warn, &state_alarm,
                     NULL, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void log(String msg){
  Serial.println(msg);
}
