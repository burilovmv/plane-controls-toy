#include <Servo.h> // Use servo library

// Define constants
const int MODE_TEST = 0;
const int MODE_NORMAL = 1;
const int MODE_ERROR = 99;

const int PIN_BAT_LED    = 13;  // Battery LED
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

int i_e1_throttle = 0;
int i_e2_throttle = 0;

int ip_e1_throttle = 0;
int ip_e2_throttle = 0;

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

bool b_e1_started = false;
bool b_e2_started = false;

bool b_e1_starting = false;
bool b_e2_starting = false;

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
  pinMode(PIN_E1_THROT, INPUT);
  pinMode(PIN_E2_THROT, INPUT);

  e1Rpm.attach(PIN_E1_RPM);
  e2Rpm.attach(PIN_E2_RPM);

  digitalWrite(PIN_BAT_LED, HIGH);
  digitalWrite(PIN_GEN_LED, HIGH);
  digitalWrite(PIN_PUMP_LED, HIGH);
  digitalWrite(PIN_LIGHT_LED, HIGH);

  e1Rpm.write(0);
  e2Rpm.write(0);
}

void readSwitches() {
  b_bat = digitalRead(PIN_BAT_SW) == HIGH;
  b_light = digitalRead(PIN_LIGHT_SW) == HIGH;
  b_pump = digitalRead(PIN_PUMP_SW) == HIGH;
  b_gen = digitalRead(PIN_GEN_SW) == HIGH;
  b_e1_start = digitalRead(PIN_E1_STARTER) == HIGH;
  b_e2_start = digitalRead(PIN_E2_STARTER) == HIGH;

  i_e1_throttle = analogRead(PIN_E1_THROT);
  i_e2_throttle = analogRead(PIN_E2_THROT);

  //TODO: read throttles
}

void setError() {
  Serial.println("Moved to error");
  max_counter = 20;

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
  e2Rpm.write(180);
}

void loop() {
  readSwitches();

  Serial.print("Loop, e1=");
  Serial.print(i_e1_throttle);
  Serial.print(", e2=");
  Serial.print(i_e2_throttle);
  Serial.println("");

  if (mode == MODE_TEST) {
    if (b_bat || b_light || b_pump || b_gen || b_e1_start || b_e2_start || i_e1_throttle < 1020 || i_e2_throttle < 1020) {
      if (b_bat)
        err_led = PIN_BAT_LED;
      if (b_light)
        err_led = PIN_LIGHT_LED;
      if (b_pump)
        err_led = PIN_PUMP_LED;
      if (b_gen)
        err_led = PIN_GEN_LED;

      setError();
    } else {
      e1Rpm.write((int)((double)counter * 9));
      e2Rpm.write((int)((double)counter * 9));
    }

    if (counter >= max_counter) {
      setNormal();
    }
  } else if (mode == MODE_ERROR) {
    if (counter == 5) {
      digitalWrite(err_led, HIGH);
    } else if (counter == 0) {
      digitalWrite(err_led, LOW);
    }
  } else if (mode == MODE_NORMAL) {
    bool energy = bp_bat || (bp_gen && (b_e1_started || b_e2_started));
    
    if (bp_bat != b_bat) {
      if (b_bat)
        digitalWrite(PIN_BAT_LED, HIGH);
      else
        digitalWrite(PIN_BAT_LED, LOW);
      bp_bat = b_bat;
    }

    if (bp_light != b_light && energy) {
      if (b_light)
        digitalWrite(PIN_LIGHT_LED, HIGH);
      else
        digitalWrite(PIN_LIGHT_LED, LOW);
      bp_light = b_light;
    } else if(!energy) digitalWrite(PIN_LIGHT_LED, LOW);
    
    if (bp_pump != b_pump && energy) {
      if (b_pump)
        digitalWrite(PIN_PUMP_LED, HIGH);
      else
        digitalWrite(PIN_PUMP_LED, LOW);
      bp_pump = b_pump;
    } else if(!energy) digitalWrite(PIN_PUMP_LED, LOW);
    
    if (bp_gen != b_gen && energy) {
      if (b_gen)
        digitalWrite(PIN_GEN_LED, HIGH);
      else
        digitalWrite(PIN_GEN_LED, LOW);
      bp_gen = b_gen;
    } else if(!energy) digitalWrite(PIN_GEN_LED, LOW);

    if(b_e1_starting && (!energy || !bp_pump)) {
       b_e1_starting = false;
       e1Rpm.write(180);         
    }

    if(b_e1_started && (!energy || !bp_pump)) {
       b_e1_started = false;
       e1Rpm.write(180);         
    }

    if(b_e2_starting && (!energy || !bp_pump)) {
       b_e2_starting = false;
       e2Rpm.write(180);         
    }

    if(b_e2_started && (!energy || !bp_pump)) {
       b_e2_started = false;
       e2Rpm.write(180);         
    }

    if (b_e1_start != bp_e1_start && !b_e1_started) {
      if(b_e1_start) {
        if (bp_bat && bp_pump) {
          b_e1_starting = true;
          e1Rpm.write(180);
          counter = 0;
        } else {
          b_e1_starting = false;
          e1Rpm.write(180);
        }
      } else {
         b_e1_starting = false;
         e1Rpm.write(180);         
      }
      bp_e1_start = b_e1_start;
    }

    if(b_e1_starting) {
      e1Rpm.write(180 - counter);
    }    

    if (b_e2_start != bp_e2_start && !b_e2_started) {
      if(b_e2_start) {
        if (bp_bat && bp_pump) {
          b_e2_starting = true;
          e2Rpm.write(180);
          counter = 0;
        } else {
          b_e2_starting = false;
          e2Rpm.write(180);
        }
      } else {
         b_e2_starting = false;
         e2Rpm.write(180);         
      }
      bp_e2_start = b_e2_start;
    }

    if(b_e2_starting) {
      e2Rpm.write(180 - counter);
    }      


    if(b_e1_started && i_e1_throttle != ip_e1_throttle) {
      e1Rpm.write(i_e1_throttle / 5.6);
      ip_e1_throttle = i_e1_throttle;      
    }
    if(b_e2_started && i_e2_throttle != ip_e2_throttle) {
      e2Rpm.write(i_e2_throttle / 5.6);
      ip_e2_throttle = i_e2_throttle;      
    }
 }

  if (counter >= max_counter) {
    counter = 0;

    if(b_e1_starting) {
      b_e1_started = true;
      b_e1_starting = false;
    }
    if(b_e2_starting) {
      b_e2_started = true;
      b_e2_starting = false;
    }
  } else {
    counter++;
  }
  delay(100);
}
