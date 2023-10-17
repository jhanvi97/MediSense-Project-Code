#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_MLX90614.h>

#define REPORTING_PERIOD_MS 1000

PulseOximeter pox;
uint32_t tsLastReport = 0;

Adafruit_MLX90614 mlx;

void onBeatDetected() {
  Serial.println("Beat!");
}

void setup() {
  Serial.begin(9600);
  Serial.print("Initializing pulse oximeter..");

  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply
  // or wrong target chip
  if (!pox.begin()) {
    Serial.println("FAILED");
    for(;;);
  } else {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  mlx.begin();
  pinMode(10, INPUT); // Setup for leads off detection LO +
  pinMode(11, INPUT); // Setup for leads off detection LO -
}

void loop() {
  // Make sure to call update as fast as possible
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Heart rate:");
    Serial.print(pox.getHeartRate());
    Serial.print(" bpm / SpO2:");
    Serial.print(pox.getSpO2());
    Serial.println("%");

    float ambientTemp = mlx.readAmbientTempC();
    float objectTemp = mlx.readObjectTempC();

    Serial.print("Ambient Temperature: ");
    Serial.print(ambientTemp);
    Serial.println(" °C");

    Serial.print("Object Temperature: ");
    Serial.print(objectTemp);
    Serial.println(" °C");

    if ((digitalRead(10) == 1) || (digitalRead(11) == 1)) {
      Serial.println('!');
    } else {
      // send the value of analog input 0:
      Serial.print("ECG Value: ");
      Serial.println(analogRead(A0));
    }

    tsLastReport = millis();
  }
  
  // Wait for a bit to keep serial data from saturating
  // Adjust the delay as needed
}