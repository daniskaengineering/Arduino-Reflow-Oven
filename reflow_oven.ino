/*
  Reflow_Oven
  runs reflow cycle for toaster oven.

  created 16 June 2015
  by Taylor Daniska
 */
 
const int buttonPin = 2;     // the number of the pushbutton pin
const int ledReflow = 5;     // the number of the pushbutton pin
const int relayHeat = 6;     // the number of the pushbutton pin
const int main1 = 7;
const int main2 = 8;
const int main3 = 9;
const int main4 = 10;
const int main5 = 11;
const int main6 = 12;


int buttonState = 0;
int mainstate = 0;
boolean runCycle = false;

boolean preheatCycle = false;
boolean soakingCycle = false;
boolean reflowCycle = false;

int temperatureVar = 150;
int temperature = 0;
int var = 0;

void setup() {
  
  pinMode(main1, INPUT);
  pinMode(main2, INPUT);
  pinMode(main3, INPUT);
  pinMode(main4, INPUT);
  pinMode(main5, INPUT);
  pinMode(main6, INPUT);
  
  pinMode(buttonPin, INPUT);
  pinMode(ledPreheat, OUTPUT);
  pinMode(ledSoaking, OUTPUT);
  pinMode(ledReflow, OUTPUT);
  pinMode(relayHeat, OUTPUT);
  
  digitalWrite(ledPreheat, HIGH);
  digitalWrite(ledSoaking, HIGH);
  digitalWrite(ledReflow, HIGH);
  delay(100);
  digitalWrite(ledPreheat, LOW);
  digitalWrite(ledSoaking, LOW);
  digitalWrite(ledReflow, LOW);
}


void loop() {
  buttonState = digitalRead(buttonPin);
  
  if (digitalRead(main1) == true) {
    mainstate = 1;
  }
  if (digitalRead(main2) == true) {
    mainstate = 2;
  }
  if (digitalRead(main3) == true) {
    mainstate = 3;
  }
  if (digitalRead(main4) == true) {
    mainstate = 4;
  }
  if (digitalRead(main5) == true) {
    mainstate = 5;
  }
  if (digitalRead(main6) == true) {
    mainstate = 6;
  }
  
  if (buttonState == HIGH) {
    runCycle = true; 
    preheatCycle = true;
  }
  
  if (runCycle == true) {
    PreHeatCycleFunction(); 
    SoakingCycleFunction();
    ReflowCycleFunction();
  } 
}
/**********************************************FUCNTIONS************************************************************/
int temperatureFunction() {
  int sensorValue = analogRead(A5);
  int temperature = (sensorValue * (226 / 1023.0)) + 24;
  delay(1);
  return temperature;
}

void PreHeatCycleFunction() {
  if (preheatCycle == true){
      delay(1); 
      digitalWrite(ledPreheat, HIGH);   
      digitalWrite(relayHeat, HIGH);
      
      temperature = temperatureFunction();
        
     if (temperature > 130) {    
       soakingCycle = true;
       preheatCycle = false;
       digitalWrite(ledPreheat, LOW); 
       digitalWrite(relayHeat, LOW);
       digitalWrite(ledSoaking, HIGH);
       delay(1875);
      }
    } 
}

void SoakingCycleFunction() {
  if (soakingCycle == true) {
    digitalWrite(ledSoaking, HIGH); 
      
    temperature = temperatureFunction();
        if (var < 7500) {
      if (temperature < 190) {
        digitalWrite(relayHeat, HIGH);
      } else if (temperature > 210) {
        digitalWrite(relayHeat, LOW);
      }  
      var++;
        } else {  
    soakingCycle = false;  
    digitalWrite(ledSoaking, LOW); 
    reflowCycle = true;
        }
    
    }
}

void ReflowCycleFunction() {
  if (reflowCycle == true) {
    digitalWrite(ledReflow, HIGH);   
    digitalWrite(relayHeat, HIGH);
    delay(1);    
    temperature = temperatureFunction();
    
    if (temperature >= 245) {
       reflowCycle = false;  
       digitalWrite(ledReflow, LOW);
       digitalWrite(relayHeat, LOW); 
       runCycle = false;
    }
  }
}
