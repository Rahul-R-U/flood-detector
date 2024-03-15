#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const int raindropPin = A0;      // Raindrop sensor pin
int vs = 4;                      // Vibration sensor pin
int waterLevelPin = 16;          // Water level sensor pin

// Define threshold values for your sensors
const int raindropThreshold = 700;
const long vibrationThreshold = 500; /* your vibration threshold value */
const int waterLevelThreshold = 700; /* your water level threshold value */

const char* ssid = "Galaxy A03 Core33ff";
const char* password = "hellomrthanku";
const char* thingsboardServer = "thingsboard.cloud";
const char* token = "iattojpjndp9j8k9i4l3";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

const char* recipients[] = {"number1", "number2"}; // Array of phone numbers

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(thingsboardServer, 1883);
  pinMode(vs, INPUT);
  pinMode(waterLevelPin, INPUT);
  pinMode(raindropPin, INPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  int raindropValue = analogRead(raindropPin);
  long vibrationValue = vibration();
  int waterLevelValue = digitalRead(waterLevelPin);

  Serial.print("Raindrop Value: ");
  Serial.println(raindropValue);

  Serial.print("Vibration Value: ");
  Serial.println(vibrationValue);

  Serial.print("Water Level Value: ");
  Serial.println(waterLevelValue);

  if (raindropValue < raindropThreshold) {
    Serial.println("Heavy rain detected.");
    sendSMS("FLOOD ALERT: HEAVY RAIN DETECTED", recipients, sizeof(recipients) / sizeof(recipients[0]));
  }

  if (vibrationValue > vibrationThreshold) {
    Serial.println("Vibration detected.");
    sendSMS("EARTHQUAKE ALERT: VIBRATION DETECTED", recipients, sizeof(recipients) / sizeof(recipients[0]));
  }

  if (waterLevelValue > waterLevelThreshold) {
    Serial.println("High water level detected.");
    sendSMS("FLOOD ALERT: HIGH WATER LEVEL", recipients, sizeof(recipients) / sizeof(recipients[0]));
  }

  sendTelemetry(raindropValue, vibrationValue, waterLevelValue);

  delay(1);
  client.loop();
}

long vibration() {
  return pulseIn(vs, HIGH);
}

void sendSMS(const char* message, const char* numbers[], int count) {
  for (int i = 0; i < count; i++) {
    Serial.print("Sending SMS to ");
    Serial.println(numbers[i]);

    Serial.println("AT");
    delay(1000);
    Serial.println("AT+CMGF=1");
    delay(1000);
    Serial.print("AT+CMGS=\"");
    Serial.print(numbers[i]);
    Serial.println("\"");
    delay(1000);
    Serial.print(message);
    delay(5000);
    Serial.write(26);
    delay(1000);
    Serial.println("SMS sent");
  }
}

void sendTelemetry(int raindropSensorValue, long vibrationSensorValue, int waterLevelSensorValue) {
  DynamicJsonDocument doc(500);
  doc["raindrop"] = raindropSensorValue;
  doc["vibration"] = vibrationSensorValue;
  doc["water_level"] = waterLevelSensorValue;
  char jsonBuffer[500];
  serializeJson(doc, jsonBuffer);
  client.publish("v1/devices/me/telemetry", jsonBuffer);
}

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client", token, NULL)) {
      Serial.println("Connected to ThingsBoard");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Try again in 5 seconds");
      delay(5000);
    }
  }
}
