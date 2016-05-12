/*
  Reflow_Oven
  
  v2.1

  created 10 May 2016
  by Taylor Daniska
 */
 
//MENWIZ ESAMPLE
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <buttons.h>
#include <MENWIZ.h>
#include <EEPROM.h> 
#include "max6675.h"
#include <PID_v1.h>

#define UP_BUTTON_PIN       4
#define DOWN_BUTTON_PIN     5
#define CONFIRM_BUTTON_PIN  2
#define ESCAPE_BUTTON_PIN   3
#define RELAY_PIN           7

double Setpoint, Input, Output;

//Define the aggressive and conservative Tuning Parameters
double aggKp=4, aggKi=0.2, aggKd=1;
double consKp=1, consKi=0.05, consKd=0.25;

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);

int WindowSize = 5000;
unsigned long windowStartTime;

menwiz menu;
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

//instantiate global variables to bind to menu
int     temp_dwell = 302;
int     temp_reflow = 437;
int     time_1 = 90;
int     time_2 = 180;
int     time_3 = 270;
int     time_4 = 360;

int      tp=0;
int    f=26;
boolean  bb=0;
byte     b=50;

int thermoDO = 11;
int thermoCS = 9;
int thermoCLK = 10;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

int buttonPinup = 4;
int buttonPindown = 5;
int buttonPinok = 2;
int buttonPinback = 3;
int buttonPinex = 6;
int heatPin = 7;
int x; 
int temp_set = 100;

//================================================== Setup =====================================================//

void setup(){
  pinMode(buttonPinup, INPUT);
  pinMode(buttonPindown, INPUT);
  pinMode(buttonPinok, INPUT);
  pinMode(buttonPinback, INPUT);
  pinMode(buttonPinex, INPUT);
  pinMode(heatPin, OUTPUT);

  _menu *r,*s1,*s2;
  _var *v; 
  int  mem;

  Serial.begin(19200);  
  mem=menu.freeRam();
  
  menu.begin(&lcd,16,2);
  
   

  r=menu.addMenu(MW_ROOT,NULL,F("Main Menu"));              //create a root menu at first (required)
                                                         //...associated to the terminal node and bind it to the app variable "bb" of type boolean
                                                         
    s1=menu.addMenu(MW_VAR,r,F("Reflow"));              
      s1->addVar(MW_ACTION,reflow);                         
      s1->setBehaviour(MW_ACTION_CONFIRM,false);
      
    s1=menu.addMenu(MW_VAR,r,F("Set Temp"));              
      s1->addVar(MW_ACTION,set_temp);                         
      s1->setBehaviour(MW_ACTION_CONFIRM,false);   

    s1=menu.addMenu(MW_VAR,r,F("Temp On"));              
      s1->addVar(MW_ACTION,temp_on);                         
      s1->setBehaviour(MW_ACTION_CONFIRM,false);      

    s1=menu.addMenu(MW_VAR,r,F("Get Temp"));       
      s1->addVar(MW_ACTION,get_temp);                    
      s1->setBehaviour(MW_ACTION_CONFIRM,false);

    s1=menu.addMenu(MW_SUBMENU,r,F("Settings"));     //add a child (submenu) node to the root menu
    
      s2=menu.addMenu(MW_VAR,s1,F("Dwell Temp"));        
        s2->addVar(MW_AUTO_INT,&temp_dwell,100,500,10); 
      
      s2=menu.addMenu(MW_VAR,s1,F("Reflow Temp"));        
        s2->addVar(MW_AUTO_INT,&temp_reflow,100,500,10);
        
      s2=menu.addMenu(MW_VAR,s1,F("Preheat Time"));        
        s2->addVar(MW_AUTO_INT,&time_1,5,400,5);
        
      s2=menu.addMenu(MW_VAR,s1,F("Dwell Time"));        
        s2->addVar(MW_AUTO_INT,&time_2,5,400,5);
        
      s2=menu.addMenu(MW_VAR,s1,F("Reflow Time"));        
        s2->addVar(MW_AUTO_INT,&time_3,5,400,5);
        
      s2=menu.addMenu(MW_VAR,s1,F("Cool Time"));        
        s2->addVar(MW_AUTO_INT,&time_4,5,400,5);

      s2=menu.addMenu(MW_VAR,s1,F("boolean var"));      
        s2->addVar(MW_BOOLEAN,&bb);                   
          
      s2=menu.addMenu(MW_VAR,s1,F("Save to EEPROM"));        
        s2->addVar(MW_ACTION,savevar);
        
      s1=menu.addMenu(MW_VAR,r,F("Show Info"));       
        s1->addVar(MW_ACTION,show);                    
        s1->setBehaviour(MW_ACTION_CONFIRM,false);

  menu.navButtons(UP_BUTTON_PIN,DOWN_BUTTON_PIN,ESCAPE_BUTTON_PIN,CONFIRM_BUTTON_PIN);
  
  menu.readEeprom();
  

  
 }

//================================================== Main Loop =====================================================//

void loop(){
  menu.draw(); 
  //PUT APPLICATION CODE HERE (if any)
  }
 
//================================================== Save Var =====================================================//
 
void savevar(){
  menu.writeEeprom();
  }

//================================================== Set Temp =====================================================//

void set_temp() {
  x = 1;
  while(x==1){
    delay(150);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set Temp");
    lcd.setCursor(12,0);
    lcd.print(temp_set);
    lcd.print("F");
    lcd.setCursor(0,1);
    lcd.print("OK to Start");
    if ((digitalRead(buttonPinup) == 0) && (temp_set < 500)){
      temp_set = temp_set + 10;
    }
    else if ((digitalRead(buttonPindown) == 0) && (temp_set > 100)){
      temp_set = temp_set - 10;
    }
    else if ((digitalRead(buttonPinok) == 0) || (digitalRead(buttonPinback) == 0)){
      x = 0;
    }
  }
  
  delay(500);
  int gap;
  int overshoot_correction = 0;
  boolean heat_state = 1;
  long previousMillis = 0;
  long interval = 150;
  long heat_on = 3000;
  long heat_off = 5000;
  
  boolean preheating = 1;
  boolean heat_var = 1;
  x = 1;
  while(x==1){
    
//================================================== Adaptive Tuning =====================================================//
gap = temp_set - temp_read(); 
unsigned long currentMillis = millis();

if (preheating == 1) {
  if (temp_read() > temp_set) {
    digitalWrite(RELAY_PIN, LOW);
  }
  else {
    if (gap > 60) {
      digitalWrite(RELAY_PIN, HIGH);
    }
    else  {
      if ((gap < 60) && (gap > 20)) {
        heat_on = 3000;
        heat_off = 5000;
      }
      else if ((gap < 20) && (gap > 5)) {
        heat_on = 1000;
        heat_off = 5000;
      }
      else if (gap < 5) {
        heat_on = 500;
        heat_off = 10000;
        preheating = false;
      }
      if(currentMillis - previousMillis > heat_on) {
        previousMillis = currentMillis; 
        digitalWrite(RELAY_PIN, HIGH);    
      }
      else {
        digitalWrite(RELAY_PIN, LOW);
      }
    }
  }
}
else {
  if (temp_read() > temp_set) {
    digitalWrite(RELAY_PIN, LOW);
  }
  else {
    digitalWrite(RELAY_PIN, HIGH);
  }
}
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

  
  
if (temp_read() > temp_set) {
  digitalWrite(RELAY_PIN, LOW);
}
else {
  
    
  }

    
    
    
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set Temp");
    lcd.setCursor(12,0);
    lcd.print(temp_set);
    lcd.print("F");
    lcd.setCursor(0,1);
    lcd.print("Heating...");
    lcd.setCursor(12,1);
    lcd.print(temp_read());
    lcd.print("F");
    if ((digitalRead(buttonPinok) == 0) || (digitalRead(buttonPinback) == 0)){
      x = 0;
      digitalWrite(heatPin, LOW);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Set Temp");
      lcd.setCursor(0,1);
      lcd.print("Ending...");
      delay(1000);
      return;
    } 
    //heat(temp_set);
    delay(90);
  }
}

//================================================== Set Temp =====================================================//

void temp_on() {
  x = 1;
  while(x==1){
    delay(150);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set Temp");
    lcd.setCursor(12,0);
    lcd.print(temp_set);
    lcd.print("F");
    lcd.setCursor(0,1);
    lcd.print("OK to Start");
    if ((digitalRead(buttonPinup) == 0) && (temp_set < 500)){
      temp_set = temp_set + 10;
    }
    else if ((digitalRead(buttonPindown) == 0) && (temp_set > 100)){
      temp_set = temp_set - 10;
    }
    else if ((digitalRead(buttonPinok) == 0) || (digitalRead(buttonPinback) == 0)){
      x = 0;
    }
  }
  
  delay(500);
  int gap;
  int overshoot_correction = 0;
  boolean heat_state = 1;
  long previousMillis = 0;
  long interval = 150;
  long heat_on = 3000;
  long heat_off = 5000;
  
  boolean preheating = 1;
  boolean heat_var = 1;
  x = 1;
  while(x==1){

if (temp_read() > temp_set) {
  digitalWrite(RELAY_PIN, LOW);
}
else {
  
    digitalWrite(RELAY_PIN, HIGH);
  }

    
    
    
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set Temp");
    lcd.setCursor(12,0);
    lcd.print(temp_set);
    lcd.print("F");
    lcd.setCursor(0,1);
    lcd.print("Heating...");
    lcd.setCursor(12,1);
    lcd.print(temp_read());
    lcd.print("F");
    if ((digitalRead(buttonPinok) == 0) || (digitalRead(buttonPinback) == 0)){
      x = 0;
      digitalWrite(heatPin, LOW);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Set Temp");
      lcd.setCursor(0,1);
      lcd.print("Ending...");
      delay(1000);
      return;
    } 
    //heat(temp_set);
    delay(90);
  }
}

//================================================== Heat =====================================================//

void heat(int temp_heat){
  int temp_real;
  temp_real = temp_read();  
  if (temp_real < temp_heat) {
    digitalWrite(heatPin, HIGH); 
  }
  else if (temp_real > temp_heat) {
    digitalWrite(heatPin, LOW);    
  }
  delay(50);
}

//================================================== Temperature Read =====================================================//

int temp_read(){
    int temp_value;
    
    temp_value = thermocouple.readFahrenheit();
    
    delay(150);
    
    return temp_value;
}

//================================================== Reflow =====================================================//

void reflow() {
  int temp_set = temp_dwell;
  int stage = 1;
  int time = 0;
  int time_remaining = 0;
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Reflow");
  delay(500);
  lcd.setCursor(0,1);
  lcd.print("Loading...");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Reflow");
  lcd.setCursor(0,1);
  lcd.print("OK to start");
  x = 1;
  while (x == 1){
  if (digitalRead(buttonPinok) == 0){
      x = 0;
   }
   else if (digitalRead(buttonPinback) == 0){
      x = 0;
      return;
    }
  }
  x = 1;
  while (x == 1){
    time = time + 1;
    
    if (digitalRead(buttonPinback) == 0){
      x = 0;
      break;
    }
    if (time < 90) {
      stage = 1;
    }
    else if ((time > time_1) && (time < time_2)) {
      stage = 2;
    }
    else if ((time > time_2) && (time < time_3)) {
      stage = 3;
    }
    else if (time > time_3) {
      stage = 4;
    }
    if (time > time_4) {
      x = 0;
      break;
    }  
    if (temp_read() > temp_set) {
      digitalWrite(heatPin, LOW);
    }
    else if (temp_read() < temp_set) {
      digitalWrite(heatPin, HIGH);
    } 
      
    
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Reflow");
      lcd.setCursor(12,0);
      time_remaining = 360 - time;
      lcd.print(time_remaining);
      lcd.print("S");
      lcd.setCursor(0,1);
      
      switch (stage) {
        case 1:
          lcd.print("Preheating");
          temp_set = temp_dwell;
          break;
        case 2:
          lcd.print("Dwelling");
          temp_set = temp_dwell;
          break;
        case 3:
          lcd.print("Reflowing");
          temp_set = temp_reflow;
          break;
        case 4:
          lcd.print("Cooling");
          temp_set = 0;
          break;
        }
      lcd.setCursor(12,1);  
      lcd.print(temp_read());
      lcd.print("F");  
  delay(1000);  
  } 
  lcd.clear();
  lcd.print("Reflow");
  delay(500);
  lcd.setCursor(0,1);
  lcd.print("Ending...");
  delay(1000); 
}

//================================================== Get Temp =====================================================//

void get_temp() {
  long previousMillis = 0;
  long interval = 150;
  int temperature = 77;
  delay(500);
  x = 1;
  while(x==1){
    
    
    
    
    
    
    unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis > interval) {
     
    previousMillis = currentMillis;   

    temperature = thermocouple.readFahrenheit();
    
    delay(500);
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Temperature");
    lcd.setCursor(0,1);
    lcd.print(temperature);
    lcd.print("F");
  }
    
    
    
    
    
    
    
    
    
    
    
    
    if ((digitalRead(buttonPinok) == 0) || (digitalRead(buttonPinback) == 0)){
      x = 0;
    }
    
  }
 delay(500); 
}

//================================================== Show =====================================================//

void show(){
  delay(500);
  x = 1;
  while(x==1){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Show");
    lcd.setCursor(0,1);
    lcd.print(temp_dwell);
    if ((digitalRead(buttonPinok) == 0) || (digitalRead(buttonPinback) == 0)){
      x = 0;
    }
    delay(90);
  }
 delay(500);  
  
}
