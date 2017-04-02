#include<SPI.h>
#include<Ethernet.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define SENSORDATA_JSON_SIZE (JSON_OBJECT_SIZE(2))
#define DHTPIN 8 //pino em que o sensor está conectado

//MQTT Configuration 
const char* mqtt_server = "m10.cloudmqtt.com";
const char* mqtt_user = "ylfqyweh";
const char* mqtt_password = "OmyBRcCKjwpi";
const int mqtt_port = 17892;


DHT dht(DHTPIN, DHT22);
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
const int tamanhoMsg = 50;
char msg[tamanhoMsg];
const long taxaAtualizacao= 300000; //every 15 minutes!!!
long lastMsg=taxaAtualizacao*-1;//so it can run for the first time 

//Initialize both EthernetClient client and 
EthernetClient ethernetClient;
PubSubClient pubSubClient(ethernetClient);

void setup(){
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    Serial.println("connecting...");
    Ethernet.begin(mac);
    delay(1500);

    pubSubClient.setServer(mqtt_server, mqtt_port);
}

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
    StaticJsonBuffer<SENSORDATA_JSON_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["temperatura"] = t;
    root["umidade"] = h;
    root.printTo(json, maxSize); 
}

void loop(){
    reconnect();
    pubSubClient.loop();
    long now = millis();
    if(now - lastMsg > taxaAtualizacao){
        lastMsg = now;
        float h,t;
        do{
        h=dht.readHumidity(); //ler os dados de humidade do sensor
        t=dht.readTemperature();//ler os dados de temperatura do sensor
        }while(isnan(h) || isnan(t));
        toJson(t,h,msg,tamanhoMsg);//converte os dados para o formato json
        pubSubClient.publish("sensor/temp", msg);
        pubSubClient.disconnect();
    }
}
