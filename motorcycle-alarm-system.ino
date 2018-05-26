#include <Wire.h>

// I/O
const int ARMED_SWITCH      = 2;          // Pin for switch to arm alarm
const int ARMED_LED         = 4;          // Pin for notify that alarm is armed
const int BUZZER            = 3;          // Pin for buzzer/piezo speaker
const int SIREN             = 8;          // Pin for siren/claxon
const int BLINKERS          = 12;         // Pin for blinkers
const int ACCELEROMETER_I2C = 0x68;       // I2C address for accelerometer
const int SERIAL_BAUD_RATE  = 9600;       // Serial baud rate for debug

// Configuration
const bool REV_ARMED_SWITCH = true;       // If set to true arm alarm on low
const long DELAY_PREARMED   = 5 * 1000;   // Time delay to arm alarm
const long DELAY_POSTWARN   = 120 * 1000L;// Time delay to calm down from warning
const long TIME_NOTIFY      = 600;        // Time to mantain armed notification
const long TIME_WARNED      = 2 * 1000;   // Time to mantain warning sound
const long TIME_ALARMED     = 30 * 1000;  // Time to mantain alarm sound
const long THRESHOLD_WARN   = 2500;       // Force threshold to warn
const long THRESHOLD_ALARM  = 3500;       // Force threshold to alarm
const long DELAY_ACCEL_READ = 100;        // Read accelerometer every x millis
const long BUZZER_TONE      = 1000;       // Play buzzer at 1 KHz

int currentState;

// STATES
const int STATE_DISABLED    = 0xf0;
const int STATE_PREARMED    = 0xf1;
const int STATE_ARMED       = 0xf2;
const int STATE_WARN        = 0xf3;
const int STATE_ALARM       = 0xf4;

// Events 
const int EVENT_ACTIVATE    = 0xe0; 
const int EVENT_DEACTIVATE  = 0xe1; 
const int EVENT_ARM         = 0xe2; 
const int EVENT_ALERT       = 0xe3; 
const int EVENT_ALARM       = 0xe4; 
const int EVENT_QUIET       = 0xe5;

int16_t AcX, initAcX, diffAcX,
        AcY, initAcY, diffAcY,
        AcZ, initAcZ, diffAcZ;

// State DISABLED
void on_state_disabled(){
  log("State DISABLED");
  
  while(true){

    if (currentState != STATE_DISABLED)
      break;

    if(isArmed()){
      trigger(EVENT_ACTIVATE);
      
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
    
    if (currentState != STATE_PREARMED)
      break;

    if (!isArmed()){
      digitalWrite(BLINKERS, LOW);
      
      trigger(EVENT_DEACTIVATE);
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

    if(currentState != STATE_PREARMED)
      break;

    if (!isArmed()){
      trigger(EVENT_DEACTIVATE);
      break;
    }

    if(currentMillis >= startTime + DELAY_PREARMED){
      break;
    }
  }

  trigger(EVENT_ARM);
}

// State ARMED
void on_state_armed(){
 log("State ARMED");

  // Read initial accelerometer values
  readAccelerometer();
  initAcX = AcX;
  initAcY = AcY;
  initAcZ = AcZ;

  // ARMED_LED
  int armedLedState = LOW;
  unsigned long armedLedPreviousMillis = 0;
  int armedLedOnTime = 100;
  int armedLedOffTime = 1500;
  unsigned long accelerometerPreviousMillis = 0;
  unsigned long currentMillis;
  unsigned long startTime = millis();

  while (true){
    currentMillis = millis();

    if (currentState != STATE_ARMED)
      break;

    if (!isArmed()){
      digitalWrite(ARMED_LED, LOW);

      trigger(EVENT_DEACTIVATE);

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

    // Read accelerometer each DELAY_ACCEL_READ millis
    if(currentMillis >= accelerometerPreviousMillis + DELAY_ACCEL_READ){
      // IF accelerometer read movement trigger alert/alarm event
      readAccelerometer();
      diffAcX = abs(initAcX - AcX);
      diffAcY = abs(initAcY - AcY);
      diffAcZ = abs(initAcZ - AcZ);
      
      if (diffAcX > THRESHOLD_ALARM || diffAcY > THRESHOLD_ALARM || diffAcZ > THRESHOLD_ALARM){
        digitalWrite(ARMED_LED, LOW);

        trigger(EVENT_ALARM);
        break;
      }

      if (diffAcX > THRESHOLD_WARN || diffAcY > THRESHOLD_WARN || diffAcZ > THRESHOLD_WARN){
        digitalWrite(ARMED_LED, LOW);

        trigger(EVENT_ALERT);
        break;
      }

      accelerometerPreviousMillis = currentMillis;
    }
  }
}

// State WARN
void on_state_warn(){
  log("State WARN");

  // Read initial accelerometer values
  readAccelerometer();
  initAcX = AcX;
  initAcY = AcY;
  initAcZ = AcZ;

  // BLINKERS & SHORT SIREN
  int blinkersState = LOW;
  unsigned long blinkersPreviousMillis = 0;
  int blinkersOnTime = 200;
  int blinkersOffTime = 200;
  int sirenState = LOW;
  unsigned long sirenPreviousMillis = 0;
  int sirenOnTime = 500;
  int sirenOffTime = 300;
  unsigned long accelerometerPreviousMillis = 0;
  unsigned long currentMillis;
  unsigned long startTime = millis();

  while(true){
    currentMillis = millis();

    if (currentState != STATE_WARN)
      break;

    if (!isArmed()){
      digitalWrite(BLINKERS, LOW);
      digitalWrite(SIREN, LOW);
      noTone(BUZZER);

      trigger(EVENT_QUIET);
      break;
    }

    if(currentMillis >= startTime + TIME_WARNED){
      digitalWrite(BLINKERS, LOW);
      digitalWrite(SIREN, LOW);
      noTone(BUZZER);
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
      noTone(BUZZER);
      sirenPreviousMillis = currentMillis;
    }else if ((sirenState == LOW) && (currentMillis - sirenPreviousMillis >= sirenOffTime)){
      sirenState = HIGH;
      digitalWrite(SIREN, HIGH);
      tone(BUZZER, BUZZER_TONE);
      sirenPreviousMillis = currentMillis;
    }

    // Read accelerometer each DELAY_ACCEL_READ millis
    if (currentMillis >= accelerometerPreviousMillis + DELAY_ACCEL_READ){
      // IF accelerometer read movement trigger alarm event
      readAccelerometer();
      diffAcX = abs(initAcX - AcX);
      diffAcY = abs(initAcY - AcY);
      diffAcZ = abs(initAcZ - AcZ);

      if (diffAcX > THRESHOLD_ALARM || diffAcY > THRESHOLD_ALARM || diffAcZ > THRESHOLD_ALARM){
        trigger(EVENT_ALARM);
        break;
      }

      accelerometerPreviousMillis = currentMillis;
    }
  }

  // Read initial accelerometer values
  readAccelerometer();
  initAcX = AcX;
  initAcY = AcY;
  initAcZ = AcZ;
  
  startTime = millis();

  while(true){
    currentMillis = millis();
    
    if (currentState != STATE_WARN)
      break;
    
    if (!isArmed()){
      trigger(EVENT_QUIET);
      break;
    }

    if (currentMillis >= startTime + DELAY_POSTWARN){
      break;
    }

    // Read accelerometer each DELAY_ACCEL_READ millis
    if (currentMillis >= accelerometerPreviousMillis + DELAY_ACCEL_READ){
      // IF accelerometer read movement trigger alarm event
      readAccelerometer();
      diffAcX = abs(initAcX - AcX);
      diffAcY = abs(initAcY - AcY);
      diffAcZ = abs(initAcZ - AcZ);
      
      if (diffAcX > THRESHOLD_WARN || diffAcY > THRESHOLD_WARN || diffAcZ > THRESHOLD_WARN){
        trigger(EVENT_ALARM);
        break;
      }

      accelerometerPreviousMillis = currentMillis;
    }
  }

  trigger(EVENT_QUIET);
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

    if (currentState != STATE_ALARM)
      break;

    if (!isArmed()){
      digitalWrite(BLINKERS, LOW);
      digitalWrite(SIREN, LOW);
      noTone(BUZZER);

      trigger(EVENT_QUIET);
      break;
    }

    if(currentMillis >= startTime + TIME_ALARMED){
      digitalWrite(BLINKERS, LOW);
      digitalWrite(SIREN, LOW);
      noTone(BUZZER);
      
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
      noTone(BUZZER);
      sirenPreviousMillis = currentMillis;
    }else if ((sirenState == LOW) && (currentMillis - sirenPreviousMillis >= sirenOffTime)){
      sirenState = HIGH;
      digitalWrite(SIREN, HIGH);
      tone(BUZZER, BUZZER_TONE);
      sirenPreviousMillis = currentMillis;
    }
  }

  trigger(EVENT_QUIET);
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  // Setup I/O
  pinMode(ARMED_SWITCH, INPUT);
  pinMode(ARMED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(SIREN, OUTPUT);
  pinMode(BLINKERS, OUTPUT);

  // Setup Accelerometer
  Wire.begin();
  Wire.beginTransmission(ACCELEROMETER_I2C);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  // Set up initial state
  currentState = STATE_DISABLED;
  on_state_disabled();
}

void loop() { 
  // Empty
  
}

// Trigger state machine events
void trigger(int event){  
  switch (event){
    case EVENT_ACTIVATE:
      log("EVENT_ACTIVATE");
      if(currentState == STATE_DISABLED){
        currentState = STATE_PREARMED;
        on_state_prearmed();
      }   
      break;

    case EVENT_ARM:
      log("EVENT_ARM");
      if (currentState == STATE_PREARMED){
        currentState = STATE_ARMED;
        on_state_armed();
      }
      break;

    case EVENT_DEACTIVATE:
      log("EVENT_DEACTIVATE");
      if (currentState == STATE_PREARMED ||
          currentState == STATE_ARMED){
        currentState = STATE_DISABLED;
        on_state_disabled();
      }
      break;

    case EVENT_ALERT:
      log("EVENT_ALERT");
      if (currentState == STATE_ARMED){
        currentState = STATE_WARN;
        on_state_warn();
      }
      break;

    case EVENT_ALARM:
      log("EVENT_ALARM");
      if (currentState == STATE_ARMED ||
          currentState == STATE_WARN){
        currentState = STATE_ALARM;
        on_state_alarm();
      }
      break;

    case EVENT_QUIET:
      log("EVENT_QUIET");
      if (currentState == STATE_WARN ||
          currentState == STATE_ALARM){
        currentState = STATE_ARMED;
        on_state_armed();
      }
      break;
  

    default:
      log("Exception! Wrong event called!");
      currentState = STATE_DISABLED;
      break;
  }
}

// Write message to serial port
void log(String msg){
  Serial.println(msg);
}

// Return if the alarm is armed or not
bool isArmed(){
  return digitalRead(ARMED_SWITCH) ^ REV_ARMED_SWITCH;
}

// Read accelerometer values and store in AcX, AcY and AcZ
void readAccelerometer(){
  Wire.beginTransmission(ACCELEROMETER_I2C);
  Wire.write(0x3B);                     // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);

  Wire.requestFrom(ACCELEROMETER_I2C, 14, true);  // request a total of 14 registers
  AcX=Wire.read()<<8|Wire.read();       // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();       // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();       // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

  Wire.endTransmission(true);
}
