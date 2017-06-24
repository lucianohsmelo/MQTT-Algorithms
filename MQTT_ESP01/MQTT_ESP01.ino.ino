#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define D0 0

DHT dht;
const char* ssid = "TIM WiFi Fon";
const char* password = "estoucomfome";

const char *mqtt_server = "192.168.1.10"; //"192.168.1.10 //"m11.cloudmqtt.com";
const int mqtt_port = 1883; //1883; //14458
const char *mqtt_user = "localBroker"; //"localBroker"; //jgojknep
const char *mqtt_pass = "1234"; //"1234"; //"muJlxlc58d0v"

const char *mqtt_client_name = "Esp01Client1"; // Client connections cant have the same connection name

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
int lastTemp, temp, cont;
long now;
char msg[50];

const char *topics[] = {"keepAlive", "outTopic", "temp", "ESP01GPIO", "humi"};
int numTopics;
String message;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(D0,OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(D0,LOW);
  
  Serial.begin(115200);
  
  dht.setup(2); // data pin 2
  temp = getTemp();
  numTopics = (sizeof(topics) / sizeof(char*)) - 1;
  
  delay(10);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(" ");
    Serial.print(cont++);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  digitalWrite(LED_BUILTIN, LOW);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  payload[length] = '\0';
  message = String((char*)payload);
  Serial.println(message);
 if (String(topic) == "keepAlive") {
    if (message == "ESP01/check" || message == "check") {
      client.publish("keepAlive", "ESP01/Alive");
      Serial.println("Send Keep Alive");
    }
  }else if (String(topic) == "temp"){
    if (message =="getTemp"){publishTemp(lastTemp); Serial.println("Send Temp");}
  }else if (String(topic) == "humi"){
    if (message =="getHumi"){publishHumi(getHumi()); Serial.println("Send Humi");}
  }else if (String(topic) == "ESP01GPIO"){
    if (message == "D0/ON"){digitalWrite(D0,HIGH);client.publish("outTopic", "Sucess: ESP01GPIO/D0 ON");}
    else if (message == "D0/OFF"){digitalWrite(D0,LOW); client.publish("outTopic", "Sucess: ESP01GPIO/D0 OFF");}
  }
  digitalWrite(LED_BUILTIN, HIGH);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("keepAlive", "ESP01/Alive");
      publishTemp(temp);
      // ... and resubscribe
      for (int i = 0; i <= numTopics; i++) {
        client.subscribe(topics[i]);
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  now = millis();
  if (now - lastMsg > 5*1000) {
    lastMsg = now;
    temp = getTemp();
    if (lastTemp != temp){
      lastTemp = temp;
      publishTemp(temp);
    }else{
      Serial.print("Temperatura: ");
      Serial.print(temp);
      Serial.println("°C");
      Serial.print("Humidade: ");
      Serial.print(getHumi());
      Serial.println("%");
    }
  }
}

void publishTemp(int value) {
  String temp = String(value);
  String msg = "tempInter/";
  msg += temp;
  char message[58];
  msg.toCharArray(message, 58);
  client.publish("temp", message);
}

void publishHumi(int value) {
  String msg = String(value);
  char message[58];
  msg.toCharArray(message, 58);
  client.publish("humi", message);
}

float getTemp(){
  float temperature = dht.getTemperature();
  return temperature;
}

float getHumi(){
  float humidity = dht.getHumidity();
  return humidity;
}

