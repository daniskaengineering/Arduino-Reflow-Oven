/*
  Reflow_Oven

  created 10 May 2016
  by Taylor Daniska
 */
 
#include <Wire.h>  
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include "max6675.h"

int thermoDO = 11;
int thermoCS = 9;
int thermoCLK = 10;

int temp_offset = 10;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

int buttonPinup = 4;
int buttonPindown = 5;
int buttonPinok = 2;
int buttonPinback = 3;
int buttonPinex = 6;
int heatPin = 7;

int temp = 100;
float temp_c,temp_f;
int x = 1;           
int sel=0;
unsigned long up_time_1,up_time_2,up_time;
double time_1,time_2,time;
int last_st=0,st=0;
int menu = 0;
int state = 0;
int frame = 0;
int mem_temp[] = {302, 437, 0, 0};
int mem_time[] = {90, 180, 270, 360};

int submenu = 0;
int submenulist = 1;

int address = 0;
byte value;

void setup()  
{
  pinMode(buttonPinup, INPUT);
  pinMode(buttonPindown, INPUT);
  pinMode(buttonPinok, INPUT);
  pinMode(buttonPinback, INPUT);
  pinMode(buttonPinex, INPUT);
  pinMode(heatPin, OUTPUT);
  Serial.begin(9600);

  lcd.begin(16,2);  

  for(int i = 0; i< 2; i++)
  {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  lcd.backlight(); 
  lcd.print("  DANISKA OVEN");
  delay(500);
  lcd.setCursor(0,1);
  lcd.print("Loading...");
  mem_read();
  delay(1000); 
  lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("  DANISKA OVEN  ");
      lcd.setCursor(0,1);
      lcd.print("Choose an option");
 
 
up_time_1 = millis();
  time_1 = millis();



}


void loop()   
{
   
  time_2 = millis();
  last_st = state;
  read_state();
  st = state;
  if(st != last_st){
    process_state();
    process_display();
    //delay(45);
    time_1 = millis();
  }
  time = time_2 - time_1;
  time = time/1000;
  delay(90);
 
}

void process_display(){
 
  if (submenu == 0) {
  switch (menu) {
    case 1:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("1.Solder Reflow");
      lcd.setCursor(0,1);
      lcd.print("Press OK");
      if(sel == 1){
        sel = 0;
        solder_reflow();
      }
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("2.Set Temp");
      lcd.setCursor(0,1);
      lcd.print("Press OK");
      if(sel == 1){
        sel = 0;
        set_temp();
      }
      break;
     
    case 3:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("3.Get Info");
      lcd.setCursor(0,1);
      lcd.print("Press OK");
      if(sel == 1){
        sel = 0;
        get_info(); 
      }
      break;
      case 4:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("4.Settings");
      lcd.setCursor(0,1);
      lcd.print("Press OK");
      if(sel == 1){
        sel = 0;
        submenu = 1;
        submenulist = 1;
      }
      break;
       
  }
  
  }
  else if (submenu == 1) {
   switch (menu) {
    case 4:
      switch (submenulist) {
    case 1:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("1.");
      lcd.setCursor(0,1);
      lcd.print("Press OK");
      if(sel == 1){
        sel = 0;
      }
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("2.");
      lcd.setCursor(0,1);
      lcd.print("Press OK");
      if(sel == 1){
        sel = 0;
      }
      break;
     
    case 3:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("3.");
      lcd.setCursor(0,1);
      lcd.print("Press OK");
      if(sel == 1) {
        sel = 0;
      }
      break;
      case 4:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("4.");
      lcd.setCursor(0,1);
      lcd.print("Press OK");
      if(sel == 1){
        sel = 0;
      }
      break;
       
  }
      break;
       
  } 
    
    
  }
  
  
  
  
}
 
void process_state(){
 
  switch (state) {
    case 3:
    if (menu > 1) {
      menu = menu - 1;
      
    }
    submenulist = submenulist - 1;
      break;
    case 4:
    if (menu < 4) {
      menu = menu + 1;
      
    }
    submenulist = submenulist + 1;
      break;
    case 5:
    break;
   
  }
 
}
 
void read_state(){
  state = 0;
  if (digitalRead(buttonPinok) == 1) {
    state = 1;
    Serial.print("ok");\
    sel = 1;
  }
  else if (digitalRead(buttonPinback) == 1) {
    state = 2;
      Serial.print("back");
  }
  else if (digitalRead(buttonPinup) == 1){
    state = 3;
    Serial.print("up");
  }
  else if (digitalRead(buttonPindown) == 1){
    state = 4;
    Serial.print("back");
  }
  else if (digitalRead(buttonPinex) == 1){
    state = 5;
    Serial.print("ex");
    
  }
 
}
 
void get_up_time(){
 
  up_time_2 = millis();
  up_time = up_time_2 - up_time_1;
  up_time = up_time/1000;        //time in seconds
 
 
     
}
void set_temp() {
  x = 1;
  while(x==1){
      delay(200);

 lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Set Temp");
        lcd.setCursor(12,0);
        lcd.print(temp);
        lcd.print("F");
        lcd.setCursor(0,1);
        lcd.print("OK to Start");
  if ((digitalRead(buttonPinup) == 1) && (temp < 500)){
    temp = temp + 10;
  }
  else if ((digitalRead(buttonPindown) == 1) && (temp > 100)){
    temp = temp - 10;
  }
  else if (digitalRead(buttonPinback) == 1){
    x = 0;
  }
  else if (digitalRead(buttonPinok) == 1){
    x = 0;
  }
}
x = 1;
delay(500);
frame = 1;
while(x==1){
  
  lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Set Temp");
        lcd.setCursor(12,0);
        lcd.print(temp);
        lcd.print("F");
        lcd.setCursor(0,1);
        lcd.print("Heating...");
        lcd.setCursor(12,1);
        lcd.print(temp_read());
        lcd.print("F");
  if (digitalRead(buttonPinok) == 1){
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
  heat(temp);
  delay(90);
}

}



int temp_read(){
    int temp_value;
    
    temp_value = thermocouple.readFahrenheit() - temp_offset;
    
    delay(50);
    
    return temp_value;
}

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

void solder_reflow(){
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
  if (digitalRead(buttonPinok) == 1){
      x = 0;
   }
   else if (digitalRead(buttonPinback) == 1){
      x = 0;
      return;
    }
  }
  x = 1;
  while (x == 1){
    time = time + 1;
    
    if (digitalRead(buttonPinback) == 1){
      x = 0;
      break;
    }
    if (time < 90) {
      stage = 1;
    }
    else if ((time > mem_time[1]) && (time < mem_time[2])) {
      stage = 2;
    }
    else if ((time > mem_time[2]) && (time < mem_time[3])) {
      stage = 3;
    }
    else if (time > mem_time[3]) {
      stage = 4;
    }
    if (time > mem_time[4]) {
      x = 0;
      break;
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
          break;
        case 2:
          lcd.print("Dwelling");
          break;
        case 3:
          lcd.print("Reflowing");
          break;
        case 4:
          lcd.print("Cooling");
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

void mem_read() {
  int n = 0;
  address = 2;
  
  if (EEPROM.read(1) == 200) {
  }
  else {
    EEPROM.write(1, 200);
    while (n < 4) {
      EEPROM.write(address, mem_temp[n]); 
      address = address + 1;
      n = n + 1; 
      delay(500);
      

    }
    n = 0;
    while (n < 4) {

      EEPROM.write(address, mem_temp[n]); 
      address = address + 1;
      n = n +1;
      delay(500); 
    } 
  }

  n = 0;
  address = 2;

    while (n < 4) {
     mem_temp[n] = EEPROM.read(address); 
     address = address + 1;
    n = n + 1; 
     delay(500);
    }
    while (n < 4) {
     mem_time[n] = EEPROM.read(address); 
     address = address + 1;
     n = n +1;
    delay(500); 
    }  
  

}


void get_info() {
  delay(100);
  x = 1;
        while (x == 1){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Temperature");
          lcd.setCursor(0,1);
          lcd.print(thermocouple.readFahrenheit() - temp_offset);
          lcd.print("F");
          if (digitalRead(buttonPinok) == 1){
            x = 0;
          }
          else if (digitalRead(buttonPinback) == 1){
            x = 0;
          }
          delay(500);
       } 
}
