#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

DHT dht;

#define RELAY 12
#define LED 13
#define BUTTON 0
#define GPIO14 14

const char* ssid = "TIM WiFi Fon";
const char* password = "estoucomfome";

const char *mqtt_server = "m11.cloudmqtt.com"; //"192.168.1.10 //"m11.cloudmqtt.com";
const int mqtt_port = 14458; //1883; //14458
const char *mqtt_user = "jgojknep"; //"localBroker"; //jgojknep
const char *mqtt_pass = "muJlxlc58d0v"; //"1234"; //"muJlxlc58d0v"

const char *mqtt_client_name = "SonoffClient1"; // Client connections cant have the same connection name

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
long now;
int lastTemp, temp, cont;
char msg[50];
boolean relayStatus;

const char *topics[] = {"keepAlive", "outTopic", "sonoff","temp"};
int numTopics;
String message;

void setup() {
  pinMode(BUTTON, INPUT);
  relayStatus = false;
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  dht.setup(GPIO14); // data pin 2
  temp = getTemp();
  
  Serial.begin(115200);
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
    digitalWrite(LED,LOW);
    if (digitalRead(BUTTON) == LOW){btnPress();}
    delay(250);
    Serial.print(".");
    digitalWrite(LED,HIGH);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED,HIGH);
}

void callback(char* topic, byte* payload, unsigned int length) {
  digitalWrite(LED, HIGH);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  payload[length] = '\0';
  message = String((char*)payload);
  Serial.println(message);
  if (String(topic) == "keepAlive") {
    if (message == "Sonoff/check" || message == "check") {
      client.publish("keepAlive", "Sonoff/Alive");
      Serial.println("Send Keep Alive");
      }
  }else if (String(topic) == "sonoff"){
    if (message == "relay/OFF"){
        btnPress();
        relayStatus = false;
    }else if (message == "relay/ON"){
      btnPress();
      relayStatus = true;
    }
  }else if (String(topic) == "temp"){
    if (message =="getTemp"){publishTemp(lastTemp); Serial.println("Send Temp");}
  }else if (String(topic) == "humi"){
    if (message =="getHumi"){publishHumi(getHumi()); Serial.println("Send Humi");}
  }
      
  delay(100);
  digitalWrite(LED,LOW);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (digitalRead(BUTTON) == LOW){btnPress();}
    // Attempt to connect
    if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("keepAlive", "Sonoff/Alive");
      // ... and resubscribe
      for (int i = 0; i <= numTopics; i++) {
        client.subscribe(topics[i]);
      }
    } else {
      digitalWrite(LED,LOW);
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(500);
      digitalWrite(LED,HIGH);delay(500);if (digitalRead(BUTTON) == LOW){btnPress();}
      digitalWrite(LED,LOW);delay(500);if (digitalRead(BUTTON) == LOW){btnPress();}
      digitalWrite(LED,HIGH);delay(500);if (digitalRead(BUTTON) == LOW){btnPress();}
      digitalWrite(LED,LOW);delay(500);if (digitalRead(BUTTON) == LOW){btnPress();}
      digitalWrite(LED,HIGH);delay(500);if (digitalRead(BUTTON) == LOW){btnPress();}
      digitalWrite(LED,LOW);delay(500);if (digitalRead(BUTTON) == LOW){btnPress();}
      digitalWrite(LED,HIGH);delay(500);if (digitalRead(BUTTON) == LOW){btnPress();}
      digitalWrite(LED,LOW);delay(500);if (digitalRead(BUTTON) == LOW){btnPress();}
      digitalWrite(LED,HIGH);delay(500);if (digitalRead(BUTTON) == LOW){btnPress();}
    }
  }
}

void loop() {
  if (!client.connected()) {
    digitalWrite(LED,HIGH);
    if (digitalRead(BUTTON) == LOW){btnPress();}
    reconnect();
  }
  digitalWrite(LED,LOW);
  client.loop();
  if (digitalRead(BUTTON) == LOW){btnPress();}
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

void btnPress(){
    if (relayStatus == false){
      digitalWrite(LED, HIGH);
      digitalWrite(RELAY,HIGH);
      Serial.println("Button Press");
      relayStatus = true;
      delay(200);
      digitalWrite(LED, LOW);
      if (client.connected()) {
        client.publish("sonoff", "relayOnSucess" );
      }
    }else{
      digitalWrite(LED, HIGH);
      digitalWrite(RELAY,LOW);
      Serial.println("Button Press");
      relayStatus = false;
      delay(200);
      digitalWrite(LED, LOW);
      if (client.connected()) {
        client.publish("sonoff", "relayOffSucess" );
      }
    }
}

void publishTemp(int value) {
  String temp = String(value);
  String msg = "tempex";
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

