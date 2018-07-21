/*  AirTracker Sensor Firmware v1.0
    Copyright by AirTracker project
*/
#include <SoftwareReset.h>
#include <WiFiEspClient.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include "dust.h"
#include "cli.h"
#include "DHT22.h"
#include "S8.h"

#define DUST_VOUT_PIN           A1
// Optional parameter
#define DUST_LED_PIN          11

#define DHT22_PIN                7

#define RED_PIN  8      // red led 
#define GREEN_PIN 9     // green led 
#define BLUE_PIN 10     // blue led
#define BUZZER_PIN                6

#define PRINT_PERIOD          5000
#define SEND_PERIOD           3000
#define SENSOR_HEAT_TIME      30000
#define TIME_ZONE  +7



void (*restart_device) (void) = 0; //declare restart at address 0


// Initialize the MQTT client object
WiFiEspClient espClient;
WiFiEspClient espClient_TB;
PubSubClient client(espClient);
PubSubClient client_TB(espClient_TB);

DUST dust(DUST_VOUT_PIN, DUST_LED_PIN);
DHT22 dht(DHT22_PIN);
S8  s8;
SoftwareSerial espSerial(5, 4); // RX, TX  for Arduino Uno or Nano with software serial


// Cac ham xu ly lenh nhan tu trung tam

uint32_t  lastTime;
uint16_t send_data_interval;
bool Is_warning = false;
bool enableLed = true;
bool enableWarning = true;
char device_id[] ="AT01";

void setup() {
  // put your setup code here, to run once:
  // initialize serial for debugging
  s8.init();
  Serial.begin(9600);
  // initialize serial for ESP module
  espSerial.begin(9600);
  // initialize ESP module
  send_data_interval = SEND_PERIOD;
  char ssid[]= "Lotus";
  char pass[]="ktht$@1234";
  //char ssid[] = "KTHT_NETWORK";
  //char pass[] = "ktht!~2015";
  int status = WL_IDLE_STATUS;   // the Wifi radio's status
  WiFi.init(&espSerial);
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    // Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    // Serial.print("Attempting to connect to WPA SSID: ");
    // Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
    delay(500);
  }


  //connect to MQTT server
  client.setServer("iot.nuce.space", 1883);
   client_TB.setServer("iot.nuce.space", 18833);
  client.setCallback(callback);

  // set interupt for  sending warning
  // pinMode(S8_INT_PIN,INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(S8_INT_PIN), s8_warning, LOW);
  led_init();
  dust.init();
  buzzer_init();
  dht.begin();
  
  lastTime = millis();

}

void connect_wifi(char* id, char* p)
{
  delay(200);
  int status = WL_IDLE_STATUS;   // the Wifi radio's status
  WiFi.init(&espSerial);
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    // Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    // Serial.print("Attempting to connect to WPA SSID: ");
    // Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(id, p);
  }
  lastTime = millis();
}

void buzzer_init()
{
  pinMode(BUZZER_PIN, OUTPUT);
  delay(500);
  analogWrite(BUZZER_PIN, 255);
}

void led_init()
{
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

void beep()//phat ra tieng beep trong 500ms
{
  analogWrite(BUZZER_PIN,10);
  delay(500);
  analogWrite(BUZZER_PIN,255);
  delay(500);
}

void reset()
{
  Serial.println("RESET DEVICE");
  delay(200);
  softwareReset::standard();
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

  char ack_topic[] ="at_ack";
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

void led_indicator(int DUST, int CO2)
{
  if (DUST <= 50 && CO2 < 800)
  {
      // bat led xanh la cay
      if(Is_warning) 
      {
        Is_warning = false;
        analogWrite(BUZZER_PIN,255);  
      }
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, HIGH);
  }
  if ((DUST > 50 && DUST <= 100) || (CO2 >800 && CO2 <1200))
  {
     //if(Is_warning)
     //{
        // bat led vang
        digitalWrite(RED_PIN, LOW);
        digitalWrite(GREEN_PIN, LOW);
        digitalWrite(BLUE_PIN, HIGH);
        //beep();
//     }
//     else
//        Is_warning = true;
  }
  if((DUST > 100 && DUST <= 150) || (CO2 >= 1200 && CO2 <2000))
  {
    if(Is_warning)
    {
      // bat led xanh lam
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, LOW);
      beep();
      beep();
    }
    else
      Is_warning = true;
  }
  
  if ((DUST > 150 && DUST <= 200) || (CO2 >= 2000 && CO2 <4000))
  {
    if(Is_warning)
    {
      // bat led xanh duong
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, LOW);
      beep();
      beep();
      beep();
    }
    else
      Is_warning = true;
  }
  if ((DUST > 200 && DUST <= 300) || (CO2>=4000 && CO2 <10000))
  {
    if(Is_warning)
    {
      // bat led tim
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, LOW);
      analogWrite(BUZZER_PIN,100);
    }
    else
      Is_warning = true;
  }
  
  if (DUST > 300 || CO2 >10000)
  {
    if(Is_warning)
    {
      // bat led do
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, HIGH);
      analogWrite(BUZZER_PIN,100);
      Serial.println(Is_warning);
    }
    else
      Is_warning = true;
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  // reconnect if connection loss
  if (!client.connected()) {
    reconnect();
  }
  if(!client_TB.connected()) {
    reconnect_TB();
    }

  if (millis() - lastTime > send_data_interval)
  {
    char msg[64];
    char topic[] = "test";
    char topic_TB[] = "v1/devices/me/telemetry";
  
    dust.run();  
    int d = (int)dust.getDust_ug();
    int co2 = s8.readCO2();
    sprintf(msg,"{\"id\":\"%s\",\"co2\":%d,\"dust\":%d,\"temp\":%d,\"humid\":%d}",
               device_id,co2,(int)dust.getDust_ug(),(int)dht.readTemperature(),(int)dht.readHumidity());
    Serial.print("CO2: ");
    Serial.println(co2);
    espSerial.listen();
    client.publish(topic, msg);
    client_TB.publish(topic_TB,msg);
    lastTime = millis();
    if(enableLed) led_indicator(d, co2);
    delay(1000L);
  }
  // receive message from server
  delay(2000L);
  client.loop();
}

void reconnect() {
  // Loop until we're reconnected
  char device_id[] = "AT01";
  char msg[] = "OK";
  char connection_fail[] = "Failed, rc=";
  while (!client.connected()) {
    char user[] = "at_ktht_03";  //"wiqusarz";
    char pass[] = "ktht@2018";  //"rj3naMiQKof4";
    char connection_ok[] = "OK! Rabbit MQTT Server connected";
    if (client.connect(device_id, user, pass)) {
      Serial.println(connection_ok);
      led_up();
      delay(1000);
      led_down();
      // Once connected, publish an announcement...
      // client.publish(topic,msg);
      // ... and resubscribe
      client.subscribe(device_id);
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
void reconnect_TB() {
  // Loop until we're reconnected
  char device_id[] = "AT01";
  char msg[] = "OK";
  char connection_fail[] = "Failed, rc=";
  while (!client_TB.connected()) {
    char token[] = "8cKUbSJvC";
    char connection_ok[] = "OK! Thingsboard MQTT Server connected";

    if (client_TB.connect(device_id, token, NULL)) {
      Serial.println(connection_ok);
      led_up();
      delay(1000);
      led_down();
      // Once connected, publish an announcement...
      // client.publish(topic,msg);
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



