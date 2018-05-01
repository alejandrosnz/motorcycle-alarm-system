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
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
