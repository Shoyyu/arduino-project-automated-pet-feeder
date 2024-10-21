#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <DHT.h>

// WiFi parameters
const char* ssid = "hidayah";
const char* password = "fari1234";

// Thingsboard parameters
const char* mqtt_server = "thingsboard.cloud";
const int mqtt_port = 1883;
const char* mqtt_user = "fari77";
const char* mqtt_pass = "1234";

WiFiClient espClient;
PubSubClient client(espClient);

// Define pins and variables
Servo myservo;
int pos = 0;
const int irPin = 2;
const int trigPin = 18;
const int echoPin = 19;

const int maxDistance = 25; // in cm
const int minDistance = 10;  // in cm

#define DHTPIN 22
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  myservo.attach(13);

  pinMode(irPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  dht.begin();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  client.setServer(mqtt_server, mqtt_port);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {
  // ... (Kode yang sudah ada sebelumnya)
  // Check IR sensor for obstacle
  int irValue = digitalRead(irPin);
  if (irValue == LOW) {
    Serial.println("Obstacle detected! Opening servo...");

    for (pos = 0; pos <= 90; pos += 1) {
      myservo.write(pos);
      delay(15);
    }

    Serial.println("Servo opened!");

    delay(10000);

    for (pos = 90; pos >= 0; pos -= 1) {
      myservo.write(pos);
      delay(15);
    }

    Serial.println("Servo closed!");
  } else {
    Serial.println("No obstacle");
  }

  // Read ultrasonic sensor distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.print(distance, 1);
  Serial.println(" cm");

  // Kirim data distance ke ThingsBoard
  String distanceStr = String(distance);
  client.publish("v1/devices/me/telemetry", ("{\"distance\":\""+distanceStr+"\"}").c_str(), true);

  // ... (Kode yang sudah ada sebelumnya)

  if (distance > maxDistance) {
    Serial.println(" - Makanan Full");
  } else if (distance < minDistance) {
    Serial.println(" - Makanan Habis");
  } else {
    Serial.println("");
  }

  // Read DHT sensor values
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C ");
    String hStr = String(h);
    String tStr = String(t);
    client.publish("v1/devices/me/telemetry", ("{\"humidity\":\""+hStr+"\",\"temperature\":\""+tStr+"\"}").c_str(), true);
  }

  delay(100);
  client.loop();
}
