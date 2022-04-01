// Code for PS4 Fan

#include<LiquidCrystal.h>

const int sensorPin = A0;
const int buttonUpPin = 10;
const int buttonDownPin = 9;
const int motorPin = 8;
const int ledPin = 7;
const int refreshInterval = 1000; // Only refresh once a second
const int minimumFanDuration = 10000; // Let the fan run for a minimum of 10 seconds

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int baselineTemp = 35;
float temperature = 0;
float previousTemperature = 0;
unsigned long lastFanActivation = 0;
unsigned long lastUpdate = 0;

void setup () {
//  Serial.begin(9600); // open a serial port
  lcd.begin(16, 2);

  pinMode(buttonUpPin, INPUT);
  pinMode(buttonDownPin, INPUT);
  pinMode(motorPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  lcd.print("Welcome");
  delay(1000);
}

void loop () {

  // Overflow protection - is this really required?
  if (lastFanActivation > millis()) {
    lastFanActivation = 0;
  }
  
  if (lastUpdate > millis()) {
    lastUpdate = 0;
  }

  // TODO - Convert these to interupts? - Can only use pints 2 and 3
  if(digitalRead(buttonUpPin)== HIGH) {
    baselineTemp++;
    updateLcd(); // Update the lcd on button press
  }

  if(digitalRead(buttonDownPin)== HIGH) {
    baselineTemp--;
    updateLcd(); // Update the lcd on button press
  }

//  Serial.print("Baseline Temp: ");
//  Serial.print(baselineTemp);
//  Serial.println();
    
  if (millis() - lastUpdate >= refreshInterval){
    updateTemperature();

    // Only update the LCD of the temperature reading changed
    if (temperature != previousTemperature) {
      updateLcd();
    }
    
    lastUpdate = millis();
  }
  
  delay(100);
}

void updateTemperature() {
  int sensorVal = analogRead(sensorPin);

  // Convert the ADC reading to voltage
  float voltage = (sensorVal / 1024.0) * 5.0;

  previousTemperature = temperature;

  temperature = (voltage - 0.5) * 100;

  if (temperature > baselineTemp)  {
    digitalWrite(ledPin, HIGH);
    digitalWrite(motorPin, HIGH);
    lastFanActivation = millis();   
  } else if (millis() - lastFanActivation >= minimumFanDuration){
    digitalWrite(ledPin, LOW);
    digitalWrite(motorPin, LOW);
  }
}

void updateLcd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  lcd.print("Baseline : ");
  lcd.print(baselineTemp);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Current : ");
  lcd.print(temperature);
  lcd.print(" C");
}
