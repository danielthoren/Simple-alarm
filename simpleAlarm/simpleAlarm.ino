
#include "Timer.h"
#include "Bounce2.h"

Timer t;

#define PASSWORD 0
#define NEW_PASSWORD 1

#define ENABLED 0
#define ENABLING 1
#define DISABLED 2
#define TRIGGERED 3
#define ACTIVATED 4

#define LED_RED 11
#define LED_GREEN 12

#define BTN1 2
#define BTN2 3
#define BTN3 4

#define DOOR_SW 5

#define RELAY 6
#define BEEP 7

int password[] = {1, 2, 3, 3};
int key_input[] = {0, 0, 0, 0};
int read_keys = 0;

int beep_event = -1;
int alarm_event = -1;

long debounce_delay = 50;
long last_debounce_time = 0;

int alarm_state;
int read_state =  PASSWORD;

Bounce btn1 = Bounce();
Bounce btn2 = Bounce();
Bounce btn3 = Bounce();
Bounce door_sw = Bounce();

bool btn1_prev = 0;
bool btn2_prev = 0;
bool btn3_prev = 0;

bool beep_on = 0;
int beep_counter = 0;
const int beep_count = 20;

void setup() {
  // put your setup code here, to run once:
 pinMode(LED_RED, OUTPUT);
 pinMode(LED_GREEN, OUTPUT);
 pinMode(BTN1, INPUT);
 pinMode(BTN2, INPUT);
 pinMode(BTN3, INPUT);
 pinMode(DOOR_SW, INPUT);
 pinMode(RELAY, OUTPUT);
 pinMode(BEEP, OUTPUT);

 digitalWrite(BTN1, 1);
 digitalWrite(BTN2, 1);
 digitalWrite(BTN3, 1);
 digitalWrite(DOOR_SW, 1);
 
 digitalWrite(RELAY, 0);
 digitalWrite(BEEP, 0);

 btn1.attach(BTN1);
 btn1.interval(10);
 btn2.attach(BTN2);
 btn2.interval(10);
 btn3.attach(BTN3);
 btn3.interval(10);
 door_sw.attach(DOOR_SW);
 door_sw.interval(10);

 Serial.begin(9600);
 Serial.println("Initial setup complete");

 if(digitalRead(DOOR_SW)){
  alarm_state = ENABLED;
  green();
 }
 else{
  alarm_state = DISABLED;
  orange();
 }
}

void loop() {
  t.update();
  btn1.update();
  btn2.update();
  btn3.update();
  door_sw.update();
  /*
   * Door opened, trigger alarm!
   */
  if(alarm_state == ENABLED && !door_sw.read()){
    Serial.println("Alarm triggered!");
    alarm_state = TRIGGERED;
    beep_event = t.every(500, toggle_beep, 10);
    alarm_event = t.after(10000, sound_alarm, 1);
    red();
  }

  if(read_keys > 3){
    Serial.println("Read keys > 4");
    if(read_state == PASSWORD){
      if(array_cmp(password, key_input, 4, 4)){
        if(alarm_state == DISABLED){
          enable_alarm();
        }
        else{
          disable_alarm();
        }
      }
      else{
        orange();
        delay(500);
        green();
      }
    }
    else{
      Serial.println("setting new password");
      green();
      password[0] = key_input[0];
      password[1] = key_input[1];
      password[2] = key_input[2];
      password[3] = key_input[3];
      delay(500);
      red();
      read_state = PASSWORD;
    }
    read_keys = 0;
  }
  
  /*
  * If all three buttons are pressed at once, wait a while and BEEP then wait for new password to be entered
  */
  if(!btn1.read() && !btn2.read() && !btn3.read() && alarm_state == DISABLED){
      Serial.println("New password state");
      for(int i = 0; i < 2; i++){
        orange();
        delay(500);
        green();
        delay(500);
      }
      read_state = NEW_PASSWORD;
      read_keys = 0;
    }
    if(!btn1.read() && btn1_prev && btn2.read() && btn3.read()){
      key_input[read_keys] = 1;
      read_keys++;
      Serial.println("Press key 1");
      btn1_prev = 0;
    }
    else if (btn1.read()){
      btn1_prev = 1;
    }
    if(btn1.read() && !btn2.read() && btn2_prev && btn3.read()){
      key_input[read_keys] = 2;
      read_keys++;
      Serial.println("Press key 2");
      btn2_prev = 0;
    }
    else if (btn2.read()){
      btn2_prev = 1;
    }
    if(btn1.read() && btn2.read() && !btn3.read() && btn3_prev){
      key_input[read_keys] = 3;
      read_keys++;
      Serial.println("Press key 3");
      btn3_prev = 0;
    }
    else if (btn3.read()){
      btn3_prev = 1;
    }
}

void sound_alarm(){
  Serial.println("Sound alarm!");
  digitalWrite(RELAY, 1);
  alarm_state = ACTIVATED;
}

void enable_alarm(){
  Serial.println("Enable alarm!");
  green();
  beep_event = t.every(500, toggle_beep, 12);
  alarm_event = t.after(6000, set_alarm_state_enabled, 1);
  alarm_state = ENABLING;
}

void set_alarm_state_enabled(){
  alarm_state = ENABLED;
}

void disable_alarm(){
  Serial.println("Disable alarm!");
  if(alarm_event != -1){
    t.stop(alarm_event);
    alarm_event = -1;
  }
  if(beep_event != -1){
    t.stop(beep_event);
    beep_event = -1;
  }
  digitalWrite(RELAY, 0);
  digitalWrite(BEEP, 0);
  alarm_state = DISABLED;
  orange();
  beep_counter = 0;
}

void toggle_beep(){
  if(beep_counter > beep_count){
    beep_counter = 0;
    t.stop(beep_event);
    digitalWrite(BEEP, 0);
    green();
  }
  else{
    beep_counter++;
    if(beep_on){
      digitalWrite(BEEP, 0);
      beep_on = 0;
      orange();
    }
    else{
      digitalWrite(BEEP, 1);
      beep_on = 1;
      green();
    }
  }
}

void green(){
  digitalWrite(LED_RED, 0);
  digitalWrite(LED_GREEN, 1);
}

void red(){
  digitalWrite(LED_RED, 1);
  digitalWrite(LED_GREEN, 0);
}

void orange(){
  digitalWrite(LED_RED, 1);
  digitalWrite(LED_GREEN, 1);
}

boolean array_cmp(int *a, int *b, int len_a, int len_b){
     int n;

     // if their lengths are different, return false
     if (len_a != len_b) return false;

     // test each element to be the same. if not, return false
     for (n=0;n<len_a;n++) if (a[n]!=b[n]) return false;

     //ok, if we have not returned yet, they are equal :)
     return true;
}



