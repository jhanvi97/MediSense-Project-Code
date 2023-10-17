#include <WiFiNINA.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_MLX90614.h>
#include "Firebase_Arduino_WiFiNINA.h"

#define DATABASE_URL "sensordata-5b22b-default-rtdb.asia-southeast1.firebasedatabase.app" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "ytCfLvAQzvWVI0xLq0xFnwFGfotNZ7Nc7egXGp9H"
#define WIFI_SSID "MyWifiCar"
#define WIFI_PASSWORD "jhanvi098765"

// Define Firebase data object
FirebaseData fbdo;

#define REPORTING_PERIOD_MS 1000
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

PulseOximeter pox;
uint32_t tsLastReport = 0;

const char* ssid = "MyWifiCar";
const char* password = "jhanvi098765";

void onBeatDetected() {
  Serial.println("Beat!");
}

// Ifttt settings
const char* server = "maker.ifttt.com";
String eventName = "Savedata";
String IFTTT_Key = "lS4Qfe5jLcxQwrlGYXqmVaGahugbw62oou_KCbYNb87";

float value1 = 0.0;
float value2 = 0.0;
float value3 = 0.0;

String path = "/Patient Data";

void setup() {
  Serial.begin(9600);

  Serial.print("Connecting to Wi-Fi");
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Provide the authentication data
  Firebase.begin(DATABASE_URL, DATABASE_SECRET, WIFI_SSID, WIFI_PASSWORD);
  Firebase.reconnectWiFi(true);

  Serial.println("Setting Sensor Data");

  // Initializing Pulse Oximeter sensor
  Serial.println();
  Serial.print("Initializing pulse oximeter..");

  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply
  // or wrong target chip
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;) {
    }
  } else {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Initializing the MLX90614 temperature sensor
  Wire.begin();
  mlx.begin();

  pinMode(10, INPUT); // Setup for leads off detection LO +
  pinMode(11, INPUT); // Setup for leads off detection LO -
}

void sendDataToIFTTT() {
  // Prepare the URL with event name and values
  String url = "/trigger/" + eventName + "/with/key/" + IFTTT_Key +
               "?value1=" + String(value1) + "&value2=" + String(value2) +
               "&value3=" + String(value3);

  Serial.println("Sending data to IFTTT: " + url);

  // Create a WiFiClient object
  WiFiClient client;

  // Connect to the server
  if (client.connect(server, 80)) {
    // Send an HTTP request
    client.print("GET " + url + " HTTP/1.1\r\n");
    client.print("Host: " + String(server) + "\r\n");
    client.print("Connection: close\r\n\r\n");

    // Read and print the response
    while (client.connected()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }

    // Disconnect from the server
    client.stop();
  } else {
    Serial.println("Failed to connect to the server");
  }
}

void loop() {
  // Read sensor values
  // Make sure to call update as fast as possible
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    // Read heart rate and SpO2 values
    int16_t heartRate = pox.getHeartRate();
    int8_t SpO2Value = pox.getSpO2();

    if (heartRate > 0 && SpO2Value > 0) {
      Serial.print("Heart rate: ");
      Serial.print(heartRate);
      Serial.print(" bpm / SpO2: ");
      Serial.print(SpO2Value);
      Serial.println("%");

      value2 = static_cast<float>(heartRate);
      value3 = static_cast<float>(SpO2Value);
      //Measuring Temp
      value1=static_cast<float>(mlx.readAmbientTempC());
      // Send data to IFTTT
      sendDataToIFTTT();

      // Sending data to Firebase
      if (Firebase.setFloat(fbdo, path + "/Heartrate", value2)) {
        Serial.println("Heart rate uploaded to Firebase");
      } else {
        Serial.println("Error uploading Heart rate to Firebase: " + fbdo.errorReason());
      }

      if (Firebase.setFloat(fbdo, path + "/Spo2", value3)) {
        Serial.println("SpO2 uploaded to Firebase");
      } else {
        Serial.println("Error uploading SpO2 to Firebase: " + fbdo.errorReason());
      }
    } else {
      Serial.println("Invalid Heart rate or SpO2 values");
    }

    tsLastReport = millis();
  }

  // Continue with other code...
}
