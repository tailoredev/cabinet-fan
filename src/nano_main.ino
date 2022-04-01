#include "DHT.h"
#include "U8glib.h"

const int dhtPin = 2;
const int dhtType = DHT11;
const int buttonUpPin = 9;
const int buttonDownPin = 8;
const int relayPin = 10;
const unsigned long refreshInterval = 1000ul; // Only refresh once a second (minimum for DHT11)
const unsigned long minimumFanDuration = 30000ul; // Let the fan run for a minimum of 30 seconds
const unsigned long screenTimeOut = 300000ul; // 5 minutes
const unsigned long wakeUpInterva = 3600000ul; // 1 hour

// Test Values
//const unsigned long screenTimeOut = 60000ul; // 1 minute (test value)
//const unsigned long wakeUpInterva = 60000ul; // 1 minute (test value)

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(dhtPin, dhtType, 6);
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);

char str[10];
int thresholdTemperature = 35;
float temperature = 0;
float previousTemperature = 0;
float humidity = 0;
float previousHumidity = 0;
float heatIndex = 0;
bool fanActive = false;
bool asleep = false;
unsigned long lastFanActivation = 0;
unsigned long lastUpdate = 0;
unsigned long lastActivity = 0;

void setup() {
  Serial.begin(9600); 
  Serial.println("Hello");

  pinMode(buttonUpPin, INPUT);
  pinMode(buttonDownPin, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  dht.begin();
  u8g.setRot180(); // We'll be mounting the screen upside down
  u8g.setFont(u8g_font_helvB14);
  drawWelcome();
  u8g.setFont(u8g_font_helvB08);
}

void loop() {
  // Overflow protection - is this really required?
  if (lastFanActivation > millis()) {
    lastFanActivation = 0;
  }
  
  if (lastUpdate > millis()) {
    lastUpdate = 0;
  }

  if (lastActivity > millis()) {
    lastActivity = 0;
  }

  if ((digitalRead(buttonUpPin) == HIGH) || (digitalRead(buttonDownPin) == HIGH)) {
    Serial.println("Button pressed");
    if (!asleep) {
      if (digitalRead(buttonUpPin) == HIGH) {
        Serial.println("Not asleep, increasing temperature");
        thresholdTemperature++;
      }
    
      if (digitalRead(buttonDownPin) == HIGH) {
        Serial.println("Not asleep, descreasing temperature");
        thresholdTemperature--;
      }   
    } else {
      asleep = false;
    }

    lastActivity = millis();
    updateOled(); // Update the lcd on button press
  }
  
  if (millis() - lastUpdate >= refreshInterval){
    Serial.println("Running update");
    updateTemperature();

    // Only update the OLED of the temperature or humidity reading changed, and we're not in sleep mode
    if (!asleep && ((temperature != previousTemperature) || (humidity != previousHumidity))) {
      Serial.println("Not asleep and changes detected, updating oled");
      updateOled();
    }
    
    lastUpdate = millis();
  }

   // Time to go to sleep, don't sleep if the fan is active
  if (!asleep && (millis() - lastActivity >= screenTimeOut) && !fanActive) {
    Serial.println("Going to sleep");
    lastActivity = millis();
    asleep = true;
    clearScreen();
  }

  // Time to wake up
  if (asleep && (millis() - lastActivity >= wakeUpInterva)) {
    Serial.println("Waking up");
    lastActivity = millis();
    asleep = false;
    updateOled();
  }
  
  delay(100);
}

void drawWelcome() {
  u8g.firstPage();  
  do {
    //u8g.setFont(u8g_font_unifont);
    u8g.drawStr( 0, 20, "Welcome");
  } while (u8g.nextPage());
}

void updateTemperature() {
  previousTemperature = temperature;
  previousHumidity = humidity;

  humidity = dht.readHumidity();
  // Read temperature in Celsius
  temperature = dht.readTemperature(false);
  
  // Exit if any reads failed
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor, returning");
    return;
  }

  // Compute heat index, false is for Celcius
  heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  if (temperature >= thresholdTemperature) {
    Serial.println("Threshold reached, activating fan (or keeping it active)");
    digitalWrite(relayPin, HIGH);
    lastFanActivation = millis();
    lastActivity = millis();
    asleep = false;
    fanActive = true;
  } else if (fanActive && (millis() - lastFanActivation >= minimumFanDuration)){
    Serial.println("Deactivating fan");
    digitalWrite(relayPin, LOW);
    lastActivity = millis();
    fanActive = false;
  }
}

void updateOled() {
  Serial.println("Updating oled");
  u8g.firstPage();  
  do {
//    u8g.drawStr( 0, 8, "Test:");
    
    u8g.drawStr( 0, 15, "Humidity:");
    u8g.drawStr( 80, 15, dtostrf(humidity, 5, 2, str));
    u8g.drawStr( 120, 15, "%");
    
    u8g.drawStr( 0, 30, "Temperature:");
    u8g.drawStr( 80, 30, dtostrf(temperature, 5, 2, str));
    u8g.drawStr( 120, 30, "\260C");

    u8g.drawStr( 0, 45, "Threshold:");
    u8g.drawStr( 80, 45, dtostrf(thresholdTemperature, 5, 2, str));
    u8g.drawStr( 120, 45, "\260C");
    
    u8g.drawStr( 0, 60, "Heat index:");
    u8g.drawStr( 80, 60, dtostrf(heatIndex, 5, 2, str));
    u8g.drawStr( 120, 60, "\260C");
  } while(u8g.nextPage());
}

void clearScreen() {
  u8g.firstPage();
  do {
    
  } while( u8g.nextPage() );
}

