#include<SPI.h>
#include<Ethernet.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(2) + 2*JSON_OBJECT_SIZE(3) + 50;
StaticJsonBuffer<bufferSize> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();
JsonObject& location = root.createNestedObject("location");
JsonArray&  coordinates  = location.createNestedArray("coordinates");
JsonObject& data = root.createNestedObject("data");


#define DHTPIN 8 //pino em que o sensor está conectado

//MQTT Configuration 
const char* mqtt_server = "m10.cloudmqtt.com";
const char* mqtt_user = "ylfqyweh";
const char* mqtt_password = "OmyBRcCKjwpi";
const int mqtt_port = 17892;


DHT dht(DHTPIN, DHT22);
const long taxaAtualizacao= 30000;//600000; //every 10 minutes!!!
long lastMsg=taxaAtualizacao*-1;//so it can run for the first time 
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//Initialize both EthernetClient client and 
EthernetClient ethernetClient;
PubSubClient pubSubClient(ethernetClient);

void setup(){
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    root["device_id"] = 10000;
    location["city"] = "São Caetano do Sul";
    location["country"] = "Brazil";
    coordinates.add(-23.6377431, 7);
    coordinates.add(-46.5788834, 7);
    data["temperature"] =0 ;
    data["humidity"] =0 ;

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

    data["temperature"] =  t;
    data["humidity"] = h;
    root.prettyPrintTo(Serial);
    root.printTo(json, maxSize); 

}

void loop(){
    //pubSubClient.loop();
    long now = millis();
    if(now - lastMsg > taxaAtualizacao){
        reconnect();
        lastMsg = now;
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
   
    }
}
