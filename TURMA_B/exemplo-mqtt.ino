#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHTesp.h" 


const char* ssid = "Wokwi-GUEST";
const char* password = "";


const char* mqtt_server = "broker.qubitro.com";
const int mqtt_port = 1883;
const char* mqtt_user = "SEU_USER";
const char* mqtt_password = "SUA_SENHA";
const char* mqtt_topic = "SEU_TÓPICO";
const char* client_id = "SEU_CLIENT_ID";


#define DHT_PIN 15
DHTesp dht;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const long interval = 5000;

void setup_wifi() {
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());

  dht.setup(DHT_PIN, DHTesp::DHT22);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect(client_id, mqtt_user, mqtt_password)) {
      Serial.println("conectado!");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    int sensor1 = analogRead(34); 
    int sensor2 = analogRead(35);

    TempAndHumidity tempH = dht.getTempAndHumidity();
    
    StaticJsonDocument<128> doc;
    doc["sensor1"] = sensor1;
    doc["sensor2"] = tempH.temperature;
    char buffer[128];
    serializeJson(doc, buffer);

    Serial.print("Enviando payload: ");
    Serial.println(buffer);

    client.publish(mqtt_topic, buffer);
  }
}
