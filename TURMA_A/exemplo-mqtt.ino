#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHTesp.h" 


const char* ssid = "Wokwi-GUEST";
const char* password = "";


const char* mqtt_server = "broker.qubitro.com";
const int mqtt_port = 1883;
const char* mqtt_user = "92a9c650-9b11-49b5-9fdc-4c3a540bc0c9";
const char* mqtt_password = "174708933582616639746bdc0019406a046d4c0e1311948d68cf7cc16ff083b5afb87760aa29b4f6773553d7b11b6d9afc7";
const char* mqtt_topic = "92a9c650-9b11-49b5-9fdc-4c3a540bc0c9";
const char* client_id = "92a9c650-9b11-49b5-9fdc-4c3a540bc0c9";


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
  
    TempAndHumidity tempH = dht.getTempAndHumidity();
    const int length = 256;
    
    StaticJsonDocument<length> doc;
    /* sensor_tanque_PH, sensor_tanque_TEMP, sensor_plantacao_1_PH, sensor_plantacao_1_HUM,
     sensor_plantacao_2_PH, sensor_plantacao_2_HUM, sensor_plantacao_3_PH, sensor_plantacao_3_HUM,
     sensor_plantacao_4_PH, sensor_plantacao_4_HUM   
    */

    doc["sensor_plantacao_1_PH"] = tempH.temperature;
    doc["sensor_plantacao_1_HUM"] = tempH.humidity;

    doc["sensor_plantacao_2_PH"] = tempH.temperature;
    doc["sensor_plantacao_2_HUM"] = tempH.humidity;

    char buffer[length];
    serializeJson(doc, buffer);

    Serial.print("Enviando payload: ");
    Serial.println(buffer);

    client.publish(mqtt_topic, buffer);
  }
}
