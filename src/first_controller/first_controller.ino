#include <Servo.h> // Use servo library

// Define constants
const int MODE_TEST = 0;
const int MODE_NORMAL = 1;
const int MODE_ERROR = 99;

const int PIN_BAT_LED    = 0;  // Battery LED
const int PIN_ENERGY_SYS = 1;  // There's energy on AC/DC bus - sync pin for second controller
const int PIN_BAT_SW     = 2;  // Battery switch
const int PIN_LIGHT_SW   = 3;  // External lighs ON/OFF switch
const int PIN_PUMP_SW    = 4;  // Pumps ON/OFF switch
const int PIN_GEN_SW     = 5;  // Generator ON/OFF switch
const int PIN_E1_RPM     = 6;  // Engine 1 RPM Servo
const int PIN_E2_RPM     = 7;  // Engine 2 RPM Servo
const int PIN_E1_STARTER = 8;  // Engine 1 starter
const int PIN_E2_STARTER = 9;  // Engine 2 starter
const int PIN_GEN_LED    = 10; // Generator LED
const int PIN_PUMP_LED   = 11; // Pump LED
const int PIN_LIGHT_LED  = 12; // External lights LED
const int PIN_E1_THROT   = 14; // Engine 1 throttle control
const int PIN_E2_THROT   = 15; // Engine 2 throttle control

// Define variables
Servo e1Rpm;
Servo e2Rpm;

int mode = MODE_TEST;
int counter = 0;
int max_counter = 60;
int err_led = PIN_BAT_LED;

bool bl_bat = false;

bool b_bat = false;
bool b_light = false;
bool b_pump = false;
bool b_gen = false;
bool b_e1_start = false;
bool b_e2_start = false;

bool bp_bat = false;
bool bp_light = false;
bool bp_pump = false;
bool bp_gen = false;
bool bp_e1_start = false;
bool bp_e2_start = false;

void setup() {  
  Serial.begin(9600);
  Serial.println("Startup");
  
  pinMode(PIN_BAT_LED, OUTPUT);
  pinMode(PIN_ENERGY_SYS, OUTPUT);
  pinMode(PIN_BAT_SW, INPUT);
  pinMode(PIN_LIGHT_SW, INPUT);
  pinMode(PIN_PUMP_SW, INPUT);
  pinMode(PIN_GEN_SW, INPUT);
  pinMode(PIN_E1_RPM, OUTPUT);
  pinMode(PIN_E2_RPM, OUTPUT);
  pinMode(PIN_E1_STARTER, INPUT);
  pinMode(PIN_E2_STARTER, INPUT);
  pinMode(PIN_GEN_LED, OUTPUT);
  pinMode(PIN_PUMP_LED, OUTPUT);
  pinMode(PIN_LIGHT_LED, OUTPUT);
//  pinMode(PIN_E1_THROT, INPUT);
//  pinMode(PIN_E2_THROT, INPUT);
    
  e1Rpm.attach(PIN_E1_RPM);
  e2Rpm.attach(PIN_E2_RPM);  

  digitalWrite(PIN_BAT_LED, HIGH);  
  digitalWrite(PIN_GEN_LED, HIGH);
  digitalWrite(PIN_PUMP_LED, HIGH);
  digitalWrite(PIN_LIGHT_LED, HIGH);

  e1Rpm.write(180);
  //e2Rpm.write(0);
}

void readSwitches() {
  b_bat = digitalRead(PIN_BAT_SW) == HIGH;
  b_light = digitalRead(PIN_LIGHT_SW) == HIGH;
  b_pump = digitalRead(PIN_PUMP_SW) == HIGH;
  b_gen = digitalRead(PIN_GEN_SW) == HIGH;
  b_e1_start = digitalRead(PIN_E1_STARTER) == HIGH;
  b_e2_start = digitalRead(PIN_E2_STARTER) == HIGH;

  //TODO: read throttles 
}

void setError() {
  Serial.println("Moved to error");
  max_counter = 10;
  
  digitalWrite(PIN_BAT_LED, LOW);
  digitalWrite(PIN_GEN_LED, LOW);
  digitalWrite(PIN_PUMP_LED, LOW);
  digitalWrite(PIN_LIGHT_LED, LOW);

  mode = MODE_ERROR;  
}

void setNormal() {
  mode = MODE_NORMAL;

  digitalWrite(PIN_BAT_LED, LOW);
  digitalWrite(PIN_GEN_LED, LOW);
  digitalWrite(PIN_PUMP_LED, LOW);
  digitalWrite(PIN_LIGHT_LED, LOW); 

  e1Rpm.write(180);
}

void loop() {
  readSwitches();

  Serial.print("Loop, mode=");
  Serial.print(mode);
  Serial.print(", c=");
  Serial.println(counter);
  Serial.print("BAT=");
  if(b_bat) Serial.print("ON"); else Serial.print("OFF");
  Serial.print(" LGTH=");
  if(b_light) Serial.print("ON"); else Serial.print("OFF");
  Serial.print(" PMP=");
  if(b_pump) Serial.print("ON"); else Serial.print("OFF");
  Serial.print(" GEN=");
  if(b_gen) Serial.print("ON"); else Serial.print("OFF");
  Serial.print(" E1=");
  if(b_e1_start) Serial.print("ON"); else Serial.print("OFF");
  Serial.print(" E2=");
  if(b_e2_start) Serial.print("ON"); else Serial.print("OFF");
  Serial.println("");
  
  if (mode == MODE_TEST) {
    if (b_bat || b_light || b_pump || b_gen || b_e1_start || b_e2_start) {
      if(b_bat)
        err_led = PIN_BAT_LED;
      if(b_light)
        err_led = PIN_LIGHT_LED;
      if(b_pump)
        err_led = PIN_PUMP_LED;
      if(b_gen)
        err_led = PIN_GEN_LED;
      
      setError();
    } else {
      e1Rpm.write(180 - (int)(counter * 1.8));
      //e2Rpm.write((int)(counter * 1.8));
    }

    if(counter >= max_counter) {
      setNormal();
    }
  } else if (mode == MODE_ERROR) {     
     if(counter == 5) {
        digitalWrite(err_led, HIGH);
     } else if (counter == 0) {
        digitalWrite(err_led, LOW);
     }
  } else if (mode == MODE_NORMAL) {
    if (bp_bat != b_bat) {
      if(b_bat)
        digitalWrite(PIN_BAT_LED, HIGH);
      else
        digitalWrite(PIN_BAT_LED, LOW);
      bp_bat = b_bat;
    }
    if (bp_light != b_light) {
      if(b_light)
        digitalWrite(PIN_LIGHT_LED, HIGH);
      else
        digitalWrite(PIN_LIGHT_LED, LOW);
      bp_light = b_light;
    }
    if (bp_pump != b_pump) {
      if(b_pump)
        digitalWrite(PIN_PUMP_LED, HIGH);
      else
        digitalWrite(PIN_PUMP_LED, LOW);
      bp_pump = b_pump;
    }
    if (bp_gen != b_gen) {
      if(b_gen)
        digitalWrite(PIN_GEN_LED, HIGH);
      else
        digitalWrite(PIN_GEN_LED, LOW);
      bp_gen = b_gen;
    }

  }

  if (counter >= max_counter){
     counter = 0;
  } else {
     counter++;
  }
  delay(100);
}
