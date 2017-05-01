#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(2) + 2*JSON_OBJECT_SIZE(3) + 50;
StaticJsonBuffer<bufferSize> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();
JsonObject& location = root.createNestedObject("location");
JsonArray&  coordinates  = location.createNestedArray("coordinates");
JsonObject& data = root.createNestedObject("data");


#define DHTPIN 2 //pino em que o sensor está conectado

//WiFi configuration
const char* ssid     = "VIVOFIBRA-8EE3";
const char* password = "958261792";

//MQTT Configuration 
const char* mqtt_server = "m10.cloudmqtt.com";
const char* mqtt_user = "ylfqyweh";
const char* mqtt_password = "OmyBRcCKjwpi";
const int mqtt_port = 17892;


DHT dht(DHTPIN, DHT22);
const long taxaAtualizacao= 3e+8;//updates every 5 minutes (microseconds)

//Initialize both WiFiClient client and 
WiFiClient espClient;
PubSubClient pubSubClient(espClient);
void reconnect() {
  while (!pubSubClient.connected()) {
    Serial.print("Attempting to connect to the broker ");
    Serial.print(mqtt_server); Serial.print("...");
    if (pubSubClient.connect("otherClient",mqtt_user,mqtt_password)) {
      Serial.println("connected");
      pubSubClient.publish("outTopic", "o módulo está conectado!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubSubClient.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void toJson(float t, float h, char *json,size_t maxSize)
{

    data["temperature"] =  t;
    data["humidity"] = h;
    root.prettyPrintTo(Serial);
    root.printTo(json, maxSize); 

}

void setup(){
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    root["device_id"] = 10001;
    location["city"] = "Santo Andre";
    location["country"] = "Brazil";
    coordinates.add(-23.637059, 7);
    coordinates.add(-46.541065, 7);

    data["temperature"] =0 ;
    data["humidity"] =0 ;

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    pubSubClient.setServer(mqtt_server, mqtt_port);

    reconnect();
    float h,t;
    do{
      h=dht.readHumidity(); //ler os dados de humidade do sensor
      t=dht.readTemperature();//ler os dados de temperatura do sensor
    }while(isnan(h) || isnan(t));
    const int tamanhoMsg = 170;
    char msg[tamanhoMsg];
    toJson(t,h,msg,tamanhoMsg);//converte os dados para o formato json
    Serial.println(msg);
    pubSubClient.publish("sensor/temp",msg);
    pubSubClient.disconnect();
    ESP.deepSleep(taxaAtualizacao, WAKE_RF_DEFAULT);
}

void loop(){

}
