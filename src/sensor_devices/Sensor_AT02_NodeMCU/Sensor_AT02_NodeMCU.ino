#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "cli.h"
#include "MQ7_LG.h"

#define MQ7_VRL_PIN             A0
#define MQ135_VRL_PIN           0
#define MQ7_DOUT_PIN           D4
#define MQ135_DOUT_PIN         D3

#define RED_PIN  D7      // red led 
#define GREEN_PIN D5     // green led 
#define BLUE_PIN D6      // blue led

#define BUZZER_PIN D0

bool isWarning_MQ7;
bool isWarning_MQ135;

#define SEND_PERIOD            3000
#define RECEIVE_PERIOD         5000

// Cac tham sof
uint32_t lastTime;
uint32_t cycle_time;
uint16_t reconnect_time;
char ssid[]= "Lotus";
char pass[]= "ktht$@1234";
//char ssid[]= "KTHT_NETWORK";
//char pass[]= "ktht!~2015";
//char ssid[]= "GW040_ACED35";
//char pass[]= "hoangphihung";
//char ssid1[]= "NUCE-TT&TT";
//char pass1[]= "19662018";
char data_topic[] = "test";
char data_topic_TB[] = "v1/devices/me/telemetry";
char ack_topic[] = "at_ack";
char device_id[] ="AT02";
uint16_t send_data_interval = SEND_PERIOD;
bool enableLed = true;
bool enableWarning = true;

MQ7_LG  mq7(MQ7_VRL_PIN, MQ7_DOUT_PIN);

WiFiClient espClient;
WiFiClient espClient_TB;
PubSubClient client(espClient);
PubSubClient client_TB(espClient_TB);
void (*restart_device) (void) = 0;//declare restart at address 0

void connect_wifi(char* ssid, char* pass)
{
 delay(10);
 // We start by connecting to a WiFi network
 Serial.println();
 Serial.print("Connecting to... ");
 Serial.println(ssid);

 WiFi.begin(ssid, pass);

 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
 }

 Serial.println("");
 Serial.println("WiFi connected");
 Serial.println("IP address: ");
 Serial.println(WiFi.localIP());
 lastTime = millis();
}

void setup() {
  // put your setup code here, to run once:
  // initialize cmd process
  Serial.begin(115200);
   send_data_interval = SEND_PERIOD;
   connect_wifi(ssid,pass);
   //connect to MQTT server
   client.setServer("iot.nuce.space", 1883);
   client_TB.setServer("iot.nuce.space", 18833);
   client.setCallback(callback);
   lastTime = millis();

   led_init();
   buzzer_init();
   mq7.init();
}

void buzzer_init()
{
  pinMode(BUZZER_PIN,OUTPUT);
  analogWrite(BUZZER_PIN,1023);
}

void led_init()
{
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

void led_down()
{
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);
}

void led_up()
{
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
}


void led_indicator(int co)
{
  if (co<=5)
  {
      // bat led xanh l�
      isWarning_MQ7=false;
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, HIGH);
      analogWrite(BUZZER_PIN,1023);
  }
  if(co>5&&co<=10)
  {
      // bat led vang
      if(isWarning_MQ7)
      {
        digitalWrite(RED_PIN, LOW);
        digitalWrite(GREEN_PIN, LOW);
        digitalWrite(BLUE_PIN, HIGH);
        // beep();
      }
      else isWarning_MQ7=true;
      
  }
  if(co>10&&co<=13)
  {
    // bat led xanh lam
    if(isWarning_MQ7)
    {
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, LOW);
      beep();
      beep();
      // beep();
    }
      else isWarning_MQ7=true;
  }
  
  if (co>13 && co<=16)
  {
    // bat led xanh duong
    if(isWarning_MQ7)
    {
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, LOW);
      beep();
      beep();
      beep();
      beep();
    }
      else isWarning_MQ7=true;
  }

  if (co>16)
  {
    // bat led do
    if(isWarning_MQ7)
    {
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, HIGH);
      analogWrite(BUZZER_PIN,100);
    }
      else isWarning_MQ7=true;
  }
}

void beep()
{
      analogWrite(BUZZER_PIN,100);
      delay(100);
      analogWrite(BUZZER_PIN,1023);
      delay(100);
}

void warning_MQ135()
{
    char msgWarning[30];
    sprintf(msgWarning,"MQ135_Warning!");
    Serial.println(msgWarning);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, HIGH);
    analogWrite(BUZZER_PIN,100);
    client.publish(data_topic,msgWarning);
}

void warning_MQ7()
{
    char msgWarning[30];
    sprintf(msgWarning,"MQ7_Warning!");
    Serial.println(msgWarning);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, HIGH);
    analogWrite(BUZZER_PIN,100);
    client.publish(data_topic,msgWarning);
}

void prepare_send_data (char * buf, unsigned int length)
{
     int co = mq7.getPPM_CO();
     Serial.println(co);
     sprintf(buf,"{\"id\":\"%s\",\"co\":%d}",
               device_id,co);
    if(enableLed) led_indicator(co); 
    if(enableWarning){ 
     if(digitalRead(MQ135_DOUT_PIN)==HIGH)
     {
        isWarning_MQ135=false;
        analogWrite(BUZZER_PIN,1023);
     }
     else 
     {
        if(isWarning_MQ135)
           warning_MQ135();
        else isWarning_MQ135=true;
     }
     if(digitalRead(MQ7_DOUT_PIN)==HIGH)
     {
        isWarning_MQ7=false;
        analogWrite(BUZZER_PIN,1023);
     }
     else 
     {
        if(isWarning_MQ7)
           warning_MQ7();
        else isWarning_MQ7=true;
     }
    }
}

void reset()
{
  Serial.println("RESET DEVICE");
  restart_device(); 
}

// thay doi khoang thoi gian do
void setInterval(uint16_t interval)
{
 
  Serial.print("SET INTERVAL ");
  Serial.println(interval);
  send_data_interval = interval;
}


//print any message received for subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[");
  Serial.print(topic);
  Serial.print("]");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  char* cmdBuf;
  char* cmd_ArgArray[4];
  int cmd_ArgCount;
  int cid;
  char ack[32];
  cmdBuf = (char*)payload;
  cmdBuf[length] = '\0';
  // phan tich lenh
  cid = parseCmd(cmdBuf, cmd_ArgCount, cmd_ArgArray);
  // thuc hien lenh
  // doCmd(cid, cmd_ArgCount, cmd_ArgArray);
  if (strcmp(cmd_ArgArray[0], "setting") == 0 && cmd_ArgCount == 3)
  {
    sprintf(ack,"{\"id\":\"%s\",\"ack\":1}",device_id); 
    client.publish(ack_topic, ack);
    delay(100);
    connect_wifi(cmd_ArgArray[1], cmd_ArgArray[2]);
    delay(200);
  }
  else if (strcmp(cmd_ArgArray[0], "reset") == 0 && cmd_ArgCount == 1)
  {
    sprintf(ack,"{\"id\":\"%s\",\"ack\":1}",device_id); 
    client.publish(ack_topic, ack);
    delay(100);
    reset();
    delay(200);
  }
  else if (strcmp(cmd_ArgArray[0], "setting") == 0 && cmd_ArgCount == 4)
  {
    String val(cmd_ArgArray[3]);
    sprintf(ack,"{\"id\":\"%s\",\"ack\":1}",device_id); 
    client.publish(ack_topic, ack);
    delay(100);
    setInterval(val.toInt());
    connect_wifi(cmd_ArgArray[1], cmd_ArgArray[2]);
    delay(200);
  }
  else if (strcmp(cmd_ArgArray[0], "setting") == 0 && cmd_ArgCount == 2)
  {
    String val(cmd_ArgArray[1]);
    sprintf(ack,"{\"id\":\"%s\",\"ack\":1}",device_id); 
    client.publish(ack_topic, ack);
    delay(100);
    setInterval(val.toInt());
    delay(200);
  }
  else if (strcmp(cmd_ArgArray[0], "led") == 0 && cmd_ArgCount == 2)
  {
    
    if(strcmp(cmd_ArgArray[1], "off") == 0)
    {
       enableLed = false;
       led_down();
    }
    else {
       enableLed = true;
       led_up();
    }
    sprintf(ack,"{\"id\":\"%s\",\"ack\":1}",device_id); 
    client.publish(ack_topic, ack);
    delay(200);
  }
  else if (strcmp(cmd_ArgArray[0], "alarm") == 0 && cmd_ArgCount == 2)
  {
    if(strcmp(cmd_ArgArray[1], "off") == 0)
       enableWarning = false;
    else
       enableWarning = true;
    sprintf(ack,"{\"id\":\"%s\",\"ack\":1}",device_id); 
    client.publish(ack_topic, ack);
    delay(200);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  #ifdef __BLYNK______
     Blynk.run();
  #endif
  // reconnect if connection loss
  cycle_time++;
  if (!client.connected()) {
    reconnect();
    reconnect_time++;
  }
  if(!client_TB.connected()) {
      reconnect_TB();
      reconnect_time++;
  } 
  if(millis() - lastTime > send_data_interval)
  { 
      char msg[64];
      prepare_send_data(msg,64); 
      client.publish(data_topic,msg);
      client_TB.publish(data_topic_TB,msg);
      lastTime = millis();
      delay(1000L);
  }
  delay(1000L);
  client.loop();
}

void reconnect() {
  // Loop until we're reconnected
  
  while (!client.connected()) {
    // Serial.print("Attempting MQTT connection...");
    // Attempt to connect, just a name to identify the client
    char user[] = "at_ktht_03";    //"wiqusarz";
    char pass[] = "ktht@2018";    //"rj3naMiQKof4";
    char connection_ok[] = "OK! RabbitMQ MQTT Server connected";
    char connection_fail[] = "Failed, rc=";
    if (client.connect(device_id,user,pass)) {
      Serial.println(connection_ok);
      led_up();
      delay(1000);
      led_down();
      // Once connected, publish an announcement...
      // ... and resubscribe
      client.subscribe(device_id);
    } else {
       Serial.print(connection_fail);
       Serial.print(client.state());
       Serial.println("Wait");
      // Wait 5 seconds before retrying
      delay(3000);
    }
  }  
}

void reconnect_TB() {
  // Loop until we're reconnected
  char device_id[] = "AT02";
  char msg[] = "OK";
  char connection_fail[] = "Failed, rc=";
  while (!client_TB.connected()) {
    char token[] = "vKWJOFUqx";
    char connection_ok[] = "OK! Thingsboard MQTT Server connected";

    if (client_TB.connect(device_id, token, NULL)) {
      Serial.println(connection_ok);
      led_up();
      delay(1000);
      led_down();
      // Once connected, publish an announcement...
      // ... and resubscribe

      // client.subscribe(topic);
      delay(200);
    } else {
      Serial.print(connection_fail);
      Serial.print(client.state());
      Serial.println("Wait");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
