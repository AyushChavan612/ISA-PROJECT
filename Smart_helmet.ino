#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>

#define TRIGGER_PIN D2
#define ECHO_PIN D1
#define ANALOG_PIN A0
#define FORCE_SENSOR_PIN A0  // Define the pin for the force sensor
#define DHT_PIN D4
#define DHT_TYPE DHT11

// Define sensor types
#define MQ135_SENSOR 0
#define FORCE_SENSOR 1
#define DHT_SENSOR 2

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "ABC"
#define WIFI_PASSWORD "123456789"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAwPc6mM5NLmqnujzYyPcNvax3XNepL5tc"

// Insert RTDB URL
#define DATABASE_URL "https://trialultrasonic-default-rtdb.asia-southeast1.firebasedatabase.app/"

int ID = 101;

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;  // Since we are doing an anonymous sign in

// Calibration values for the FSR sensor
const float R0 = 1000.0; // Resistance of FSR with no force applied (in Ohms)
const float Rf = 500.0;  // Resistance of FSR when a known force is applied (in Ohms)
const float Fmax = 10.0; // Maximum force applied (in Newtons)

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(FORCE_SENSOR_PIN, INPUT);  // Set the force sensor pin as input

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the API key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Authentication successful");
    signupOK = true;
  } else {
    Serial.printf("Authentication failed: %s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  dht.begin();
}

void loop() {
  long duration, distance;

  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;

  // Read from MQ135 sensor
  int sensorValueMQ135 = analogRead(ANALOG_PIN);
  int co2Level = calculateCO2Level(sensorValueMQ135);

  // Read from force sensor
  int sensorValueForce = analogRead(FORCE_SENSOR_PIN);
  float forceInNewtons = calculateForce(sensorValueForce);

  // Read from DHT sensor
  float temperature = dht.readTemperature();
  if ( isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }


  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Enter distance into the Ultrasonic sensor table
    if (Firebase.RTDB.setInt(&fbdo, "Real/Ultrasonic/Distance", distance)) {
      Serial.print("Distance: ");
      Serial.println(distance);
    } else {
      Serial.println("Failed to write to the database");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Enter CO2 level into the MQ135 table
    if (Firebase.RTDB.setInt(&fbdo, "Real/MQ135/CO2Level", co2Level)) {
      Serial.print("CO2 Level: ");
      Serial.println(co2Level);
    } else {
      Serial.println("Failed to write CO2 level to the database");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Enter force value into the force sensor table
    if (Firebase.RTDB.setFloat(&fbdo, "Real/ForceSensor/Force", forceInNewtons)) {
      Serial.print("Force: ");
      Serial.println(forceInNewtons);
    } else {
      Serial.println("Failed to write force value to the database");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Enter temperature into the DHT sensor table
    if (!isnan(temperature)) {
      if (Firebase.RTDB.setFloat(&fbdo, "Real/Temperature/Temp", temperature)) {
        Serial.print("Temperature: ");
        Serial.println(temperature);
      } else {
        Serial.println("Failed to write temperature to the database");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }

    // Send one real update and 5 dummy updates
    sendDataRealUpdate();
    sendDataDummyValues();
    
  }
}

// Function to calculate CO2 level in ppm based on sensor's analog reading
int calculateCO2Level(int sensorValue) {
  // Perform necessary calibration and conversion
  // You need to replace the calibration values with your sensor's actual values
  // Here's an example calibration curve:
  float ppm = map(sensorValue, 0, 1023, 300, 800);  // Map the sensor value to the desired range of CO2 levels
  
  // Constrain ppm value within the acceptable range
  ppm = max(min((int)ppm, 800), 300);
  
  return (int)ppm; // Return the ppm as an integer
}

// Function to calculate force in Newtons based on sensor's analog reading
float calculateForce(int sensorValue) {
  // Convert analog reading to resistance
  float fsrResistance = (1023.0 / sensorValue - 1) * R0;

  // Calculate force using the conversion formula
  float force = (1.0 / (Rf - R0)) * (fsrResistance - R0) * Fmax;

  // Constrain the force value within the acceptable range
  force = max(min(force, Fmax), 0.0f);  // Ensure all arguments are of type float

  return force; // Return the force in Newtons
}

// Function to send one real update to the database
// Function to send one real update to the database
void sendDataRealUpdate() {
  // Generate real values for distance, CO2 level, force, and temperature
  long duration, distance;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;
  int sensorValueMQ135 = analogRead(ANALOG_PIN);
  int co2Level = calculateCO2Level(sensorValueMQ135);
  int sensorValueForce = analogRead(FORCE_SENSOR_PIN);
  float forceInNewtons = calculateForce(sensorValueForce);
  float temperature = dht.readTemperature();

  // Enter real distance
  if (Firebase.RTDB.setInt(&fbdo, "Real/Ultrasonic/Distance", distance)) {
    Serial.print("Real Update - Distance: ");
    Serial.println(distance);
  } else {
    Serial.println("Failed to write real distance to the database");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  // Enter real CO2 level
  if (Firebase.RTDB.setInt(&fbdo, "Real/MQ135/CO2Level", co2Level)) {
    Serial.print("Real Update - CO2 Level: ");
    Serial.println(co2Level);
  } else {
    Serial.println("Failed to write real CO2 level to the database");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  // Enter real force value
  if (Firebase.RTDB.setFloat(&fbdo, "Real/ForceSensor/Force", forceInNewtons)) {
    Serial.print("Real Update - Force: ");
    Serial.println(forceInNewtons);
  } else {
    Serial.println("Failed to write real force value to the database");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  // Enter real temperature
  if (!isnan(temperature)) {
    if (Firebase.RTDB.setFloat(&fbdo, "Real/Temperature/Temp", temperature)) {
      Serial.print("Real Update - Temperature: ");
      Serial.println(temperature);
    } else {
      Serial.println("Failed to write real temperature to the database");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}

// Function to send dummy values to the database
void sendDataDummyValues() {
  for (int i = 1; i <= 5; i++) {
    String dummyPath = "Dummy" + String(i) + "/";
    // Generate random values for distance, CO2 level, force, and temperature
    int randomDistance = random(10, 100); // Random distance between 10 cm and 100 cm
    int randomCO2Level = random(300, 800); // Random CO2 level between 300 ppm and 800 ppm
    float randomForce = random(10, 100) * 0.1; // Random force between 1 N and 10 N
    float randomTemperature = random(20, 35); // Random temperature between 20°C and 35°C

    // Enter random distance
    if (Firebase.RTDB.setInt(&fbdo, dummyPath + "Ultrasonic/Distance", randomDistance)) {
      Serial.print("Dummy ");
      Serial.print(i);
      Serial.print(" - Random Distance: ");
      Serial.print(randomDistance);
      Serial.println(" cm");
    } else {
      Serial.println("Failed to write random distance to the database");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Enter random CO2 level
    if (Firebase.RTDB.setInt(&fbdo, dummyPath + "MQ135/CO2Level", randomCO2Level)) {
      Serial.print("Dummy ");
      Serial.print(i);
      Serial.print(" - Random CO2 Level: ");
      Serial.print(randomCO2Level);
      Serial.println(" ppm");
    } else {
      Serial.println("Failed to write random CO2 level to the database");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Enter random force value
    if (Firebase.RTDB.setFloat(&fbdo, dummyPath + "ForceSensor/Force", randomForce)) {
      Serial.print("Dummy ");
      Serial.print(i);
      Serial.print(" - Random Force: ");
      Serial.print(randomForce);
      Serial.println("N");
    } else {
      Serial.println("Failed to write random force value to the database");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Enter random temperature
    if (Firebase.RTDB.setFloat(&fbdo, dummyPath + "Temperature/Temp", randomTemperature)) {
      Serial.print("Dummy ");
      Serial.print(i);
      Serial.print(" - Random Temperature: ");
      Serial.print(randomTemperature);
      Serial.println("°C");
    } else {
      Serial.println("Failed to write random temperature to the database");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}
