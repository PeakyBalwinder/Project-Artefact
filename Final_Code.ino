#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711_ADC.h>

#define DHTPIN 2        // Pin to which the DHT22 is connected
#define DHTTYPE DHT22   // DHT 22 (AM2302)
#define RELAY_PIN 8     // Pin to which the relay module is connected
#define BUTTON_PLUS 3   // Pin for the "increase temperature" button
#define BUTTON_MINUS 10  // Pin for the "decrease temperature" button
#define LOAD_CELL_DT 6  // Pin connected to the HX711 load cell's data pin
#define LOAD_CELL_SCK 7 // Pin connected to the HX711 load cell's clock pin
#define ECG_LO_PLUS 4   // Pin for ECG leads off detection LO+
#define ECG_LO_MINUS 11 // Pin for ECG leads off detection LO-

DHT_Unified dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Replace 0x27 with the actual I2C address of your LCD
HX711_ADC LoadCell(LOAD_CELL_DT, LOAD_CELL_SCK);

float temperatureSetpoint = 30.0;  // Initial temperature setpoint
float tempRange = 1.0;            // Temperature range for adjustment

void setup() {
  Serial.begin(9600);
  lcd.init();                      // Initialize the LCD
  lcd.backlight();                 // Turn on the backlight
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  pinMode(RELAY_PIN, OUTPUT);      // Set the relay pin as an output
  digitalWrite(RELAY_PIN, LOW);    // Turn off the relay initially
  pinMode(BUTTON_PLUS, INPUT_PULLUP);  // Set the "increase temperature" button as input with pull-up resistor
  pinMode(BUTTON_MINUS, INPUT_PULLUP); // Set the "decrease temperature" button as input with pull-up resistor
  pinMode(LOAD_CELL_DT, INPUT);  // Set the HX711 data pin as an input
  pinMode(LOAD_CELL_SCK, OUTPUT); // Set the HX711 clock pin as an output
  pinMode(ECG_LO_PLUS, INPUT);  // Setup for ECG leads off detection LO+
  pinMode(ECG_LO_MINUS, INPUT); // Setup for ECG leads off detection LO-
}

void loop() {
  // Read the button states
  bool increaseButtonPressed = digitalRead(BUTTON_PLUS) == LOW;
  bool decreaseButtonPressed = digitalRead(BUTTON_MINUS) == LOW;

  // Adjust the temperature setpoint based on button presses
  if (increaseButtonPressed) {
    temperatureSetpoint += tempRange;
  } else if (decreaseButtonPressed) {
    temperatureSetpoint -= tempRange;
  }

  // Ensure the setpoint stays within a reasonable range
  if (temperatureSetpoint < 10.0) {
    temperatureSetpoint = 10.0;
  } else if (temperatureSetpoint > 30.0) {
    temperatureSetpoint = 30.0;
  }

  LoadCell.update(); // Retrieves data from the load cell
  float weight = LoadCell.getData(); // Get the weight reading

  delay(2000);  // Wait for 2 seconds between readings

  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Failed to read temperature!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(event.temperature);
    Serial.println(" °C");

    // Check if the temperature is below the setpoint and control the relay
    if (event.temperature < temperatureSetpoint) {
      // Temperature is below the setpoint, turn on the relay
      digitalWrite(RELAY_PIN, HIGH);
    } else {
      // Temperature is at or above the setpoint, turn off the relay
      digitalWrite(RELAY_PIN, LOW);
    }
  }

  // Check ECG leads-off detection here if needed.
  if (digitalRead(ECG_LO_PLUS) == 1) {
    Serial.println("ECG Lead Off: LO+");
  }
  if (digitalRead(ECG_LO_MINUS) == 1) {
    Serial.println("ECG Lead Off: LO-");
  }

  delay(2000); // 2-second delay
}
