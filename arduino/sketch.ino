#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define status_led BUILTIN_LED

#define led1 D1 // 1 room 
#define led2 D3 // 2 room 
#define led3 D4 // 3 room
#define sen D2  // temperature and humidity sensor
#define fan D5
#define DHTTYPE DHT11
DHT dht(sen, DHTTYPE);

const char* ssid = "YOUR-SSID";
const char* password = "YOUR-PASSWORD";
const char* mqtt_server = "192.168.0.109"; // адрес mqtt сервера

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int last_pub_time = 0;


void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.println('|' + msg + '|');
  if      (msg == "led1_ON")
    lightOn(led1);
  else if (msg == "led1_OFF")
    lightOff(led1);

  else if (msg == "led2_ON")
    lightOn(led2);
  else if (msg == "led2_OFF")
    lightOff(led2);

  else if (msg == "led3_ON")
    lightOn(led3);
  else if (msg == "led3_OFF")
    lightOff(led3);

  else if (msg == "fan_ON")
    fanOn();
  else if (msg == "fan_OFF")
    fanOff();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "lens_5kchta2dWhdRS2AQz0TeTuhplPE";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), "openhabian", "openhabian")) {
      Serial.println("connected");
      client.subscribe("smarthome");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(status_led, OUTPUT);
  dht.begin();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }

  getTemp();
  client.loop();
}

long previousMillis = 0;
long interval = 2000;

void getTemp() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.println(F("°C "));
    String ts = "";
    ts += t;
    char temp[ts.length()];
    ts.toCharArray(temp, ts.length());
    client.publish("temp", temp);
    String hs = "";
    hs += h;
    char hum[hs.length()];
    hs.toCharArray(hum, hs.length());
    client.publish("hum", hum);
  }

}


//void lightOn(int led) {
//  Serial.println("SWITCHING LED");
//  for (int i = 0; i < 255; i++) {
//    analogWrite(led, i);
//    delay(10);
//  }
//}
//void lightOff(int led) {
//  Serial.println("SWITCHING LED");
//  for (int i = 255; i > 0; i--) {
//    analogWrite(led, i);
//    delay(10);
//  }
//}

void lightOn(int led) {
  digitalWrite(led, HIGH);
}
void lightOff(int led) {
  digitalWrite(led, LOW);
}
void fanOn() {
  digitalWrite(fan, HIGH);
}
void fanOff() {
  digitalWrite(fan, LOW);
}
