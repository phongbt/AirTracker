    /*  AirTracker Sensor Firmware v1.0
 *  Copyright by AirTracker project
 */
#include <SoftwareReset.h>
#include <WiFiEspClient.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>  
#include <NTPClient.h>
#include <PubSubClient.h> 
#include "DHT.h"
#include "dust.h"
#include "cli.h"
#include "MQ7_LG.h"
#include "MQ135.h"


#ifdef __BLYNK__
  #define BLYNK_PRINT Serial
  #include <ESP8266_Lib.h>
  #include <BlynkSimpleShieldEsp8266.h>
#else
 // #include <SimpleTimer.h>
#endif

#define MQ7_VRL_PIN             A5
#define MQ135_VRL_PIN           A1
//#define MQ135_VRL_PIN_1       A12
#define DUST_VOUT_PIN           A3


// Optional parameter
#define MQ7_DOUT_PIN           11
#define MQ135_DOUT_PIN         3
//#define MQ135_DOUT_PIN_1       7
#define DUST_LED_PIN           7
#define DHT_PIN                4
#define Buzzer_PIN				37

#define RED_PIN  46      // red led 
#define GREEN_PIN 44     // green led 
#define BLUE_PIN 42      // blue led


//#define MQ7_INT_PIN  2   // INTERUPT PIN FOR MQ7 WARNING
//#define MQ135_INT_PIN 3  // INTERUPT PIN FOR MQ135 WARNING


#define SEND_PERIOD            3000
#define RECEIVE_PERIOD         5000
#define SENSOR_HEAT_TIME      30000  
#define TIME_ZONE  +7
#define espSerial Serial1  // for Arduino Mega with hardware serial


// Cac tham so
// unsigned long startTimeEpoch = 0;
unsigned long lastUpdateTime;
uint32_t currTime;
uint32_t lastTime;
uint32_t cycle_time;
uint16_t reconnect_time;
char ssid[]= "Lotus";
char pass[]= "ktht$@1234";
//char ssid[]= "KTHT_NETWORK";
//char pass[]= "ktht!~2015";
//char ssid1[]= "NUCE-TT&TT";
//char pass1[]= "19662018";
char data_topic[] = "test";
char data_topic_TB[] = "v1/devices/me/telemetry";
char ack_topic[] = "at_ack";
char device_id[] ="AT03";
uint16_t send_data_interval = SEND_PERIOD;

bool isWarning_MQ135;
bool isWarning_MQ7;
bool Is_warning = false;

bool enableWarning = true;
bool enableLed = true;

#ifdef __BLYNK__
  char auth[] = "672a839f6b9a4c21ad5acd75d9812890";
  ESP8266 wifi(&espSerial);
  BlynkTimer timer;
#else 
//  SimpleTimer timer;   
#endif

// Sensors
MQ7_LG  mq7(MQ7_VRL_PIN, MQ7_DOUT_PIN);
MQ135 mq135(MQ135_VRL_PIN, MQ135_DOUT_PIN);
//MQ135 mq135_1(MQ135_VRL_PIN_1, MQ135_DOUT_PIN_1);
DUST dust(DUST_VOUT_PIN, DUST_LED_PIN);
DHT  dht(DHT_PIN);
  
// Initialize the MQTT client object
WiFiEspClient espClient;
WiFiEspClient espClient_TB;
PubSubClient client(espClient);
PubSubClient client_TB(espClient_TB);


void (*restart_device) (void) = 0;//declare restart at address 0

//unsigned long getCurrentEpoch()
//{
//      return startTimeEpoch + (millis() - lastUpdateTime)/1000; 
//}


//void getNTPTime()
//{
//  // Get current time
//  WiFiEspUDP ntpUDP;
//  // By default 'time.nist.gov' is used with 60 seconds update interval and
//  // no offset
//  NTPClient timeClient(ntpUDP, TIME_ZONE*3600); // NTP Client (Time)
//  // Serial.println(get_time_info);
//  timeClient.begin();
//  timeClient.update();
//  startTimeEpoch = timeClient.getEpochTime();
//  lastTime = lastUpdateTime = millis();
//  timeClient.end();
//}



// Cac ham xu ly lenh nhan tu trung tam
// Reset thiet bi
void reset()
{
    Serial.println("RESET DEVICE");
    delay(200);
    //soft_restart();
    //restart_device();
    softwareReset::standard();
}

// thay doi khoang thoi gian do
void setInterval(uint16_t interval)
{
    Serial.print("SET INTERVAL ");
    Serial.println(interval);
    send_data_interval = interval;
}


int connect_wifi(char* ssid, char* pass)
{
    
    Serial.println("SET WIFI");
    Serial.println(ssid);
    Serial.println(pass);
    int status = WL_IDLE_STATUS;   // the Wifi radio's status
    WiFi.init(&espSerial);
    // check for the presence of the shield
    if (WiFi.status() == WL_NO_SHIELD) {
    // Serial.println("WiFi shield not present");
    // don't continue
       while (true);
    }
    // attempt to connect to WiFi network
    int count = 0;
    while ( status != WL_CONNECTED && count < 5) {
       // Serial.print("Attempting to connect to WPA SSID: ");
       // Serial.println(ssid);
       // Connect to WPA/WPA2 network
       status = WiFi.begin(ssid,pass);
       count++;
  }
  lastTime = millis();
  return status; 
}

void led_init()
{
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

void buzzer_init()
{
	pinMode(Buzzer_PIN,OUTPUT);
	analogWrite(Buzzer_PIN,224);
}

void beep()//phat ra tieng beep trong 500ms
{
  analogWrite(Buzzer_PIN,10);
  delay(500);
  analogWrite(Buzzer_PIN,224);
  delay(500);
}

void setup() {
  // put your setup code here, to run once:
  // initialize cmd process
  Serial.begin(115200);
  // initialize serial for ESP module
  espSerial.begin(57600);
  // initialize ESP module
  int wifi_status;
  wifi_status = connect_wifi(ssid,pass);
  if(wifi_status!= WL_CONNECTED)
  {
      wifi_status = connect_wifi(ssid,pass);
      while(wifi_status!= WL_CONNECTED);
  }
  // get current time
  // getNTPTime();
  // init sensor
  mq135.init();
  mq7.init();
 
  dust.init();
  dht.begin();

  // init led indicator 
  led_init();
  buzzer_init();
  beep();
  
  // count cycle and reconnection
  cycle_time = 0;
  reconnect_time = 0;

#ifdef __BLYNK__
  Blynk.begin(auth, wifi, ssid, pass);
#endif
  
  // timer.setInterval(RECEIVE_PERIOD, timer_event);
  //connect to MQTT server
  client.setServer("iot.nuce.space", 1883);
  client.setCallback(callback);
  client_TB.setServer("iot.nuce.space", 18833);
  lastTime = millis();
}

void mq7_warning()
{
    Serial.println("MQ7 Warning!");
}

void warning_MQ135()
{
    char msgWarning[30];
    sprintf(msgWarning,"MQ135_Warning!");
    Serial.println(msgWarning);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, HIGH);
    analogWrite(Buzzer_PIN,100);
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
    analogWrite(Buzzer_PIN,100);
    client.publish(data_topic,msgWarning);
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

void led_indicator(int DUST, int CO)
{
  if (DUST <= 50 && CO <= 4.4)
  {
      // bật led xanh lá
      if(Is_warning) Is_warning = false;
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, HIGH);
      analogWrite(Buzzer_PIN,224);
  }
  if ((DUST > 50 && DUST <= 100) || (CO >=4.5 && CO <= 9.4))
  {
     if(Is_warning)
     {
        // bat led vang
        digitalWrite(RED_PIN, LOW);
        digitalWrite(GREEN_PIN, LOW);
        digitalWrite(BLUE_PIN, HIGH);
        //beep();
     }
     else
        Is_warning = true;
  }
  if((DUST > 100 && DUST <= 150) || (CO >= 9.5 && CO <= 12.4))
  {
    if(Is_warning)
    {
      // bat led xanh lam
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, LOW);
      if(enableWarning){
        beep();
        beep();
      }
    }
    else
      Is_warning = true;
  }
  
  if ((DUST > 150 && DUST <= 200) || (CO >= 12.5 && CO <= 15.4))
  {
    if(Is_warning)
    {
      // bat led xanh duong
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, LOW);
      if(enableWarning){
        beep();
        beep();
        beep();
      }
    }
    else
      Is_warning = true;
  }
  if ((DUST > 200 && DUST <= 300) || (CO>=15.5 && CO <=30.4))
  {
    if(Is_warning)
    {
      // bat led tim
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, LOW);
      if(enableWarning){
        beep();
        beep();
        beep();
        beep();
      }
    }
    else
      Is_warning = true;
  }
  
  if (DUST > 300 || CO >= 30.4)
  {
    if(Is_warning)
    {
      // bat led do
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, HIGH);
      if(enableWarning) analogWrite(Buzzer_PIN,100);
      Serial.println(Is_warning);
    }
    else
      Is_warning = true;
  }
}
void prepare_send_data (char * buf, unsigned int length)
{
     char stime[24];
     float h = dht.readHumidity();
     // Read temperature as Celsius (the default)
     float t = dht.readTemperature();
     // Check if any reads failed and exit early (to try again).
     char t_buf[8];
     char h_buf[8];
     if (!isnan(t)) {
          //dtostrf(t, 3, 0, t_buf);
          sprintf(t_buf,"%d",(int)t);
     } else
          sprintf(t_buf,"");  
          
     if (!isnan(h)) {
          //dtostrf(h, 3, 0, h_buf);
          sprintf(h_buf,"%d",(int)h);
     } else
          sprintf(h_buf,"");  
     
     dust.run();
     int co = mq7.getPPM_CO();
     int d = (int)dust.getDust_ug();
     sprintf(buf,"{\"id\":\"%s\",\"co\":%d,\"dust\":%d,\"temp\":%s,\"humid\":%s}",
               device_id,co,d,t_buf,h_buf);
    
    if(enableWarning) {
      if(digitalRead(MQ135_DOUT_PIN)==HIGH)
      {
        isWarning_MQ135=false;
        analogWrite(Buzzer_PIN,1023);
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
        analogWrite(Buzzer_PIN,1023);
      }
      else 
      {
        if(isWarning_MQ7)
          warning_MQ7();
       else isWarning_MQ7=true;
      }
     }
     if(enableLed) led_indicator(d,co);
     Serial.print("H: ");
     Serial.println(h);
     Serial.print("T: ");
     Serial.println(t);
     Serial.print("CO: ");
     Serial.println(co);
     Serial.print("Dust: ");
     Serial.println(d);
 }


/*
 *  Xu ly cac lenh dieu khien nhan duoc tu trung tam
 *  1)  Lenh reset (reset): khoi dong lai thiet bi
 *  2)  Lenh setting (wifi): ket noi voi wifi theo ssid va password
 *  3)  Lenh setting (interval): dat khoang thoi gian giua 2 lan do (interval):  
 */
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
  // timer.run();
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
      char msg[96];
      prepare_send_data(msg, 96); 
      client.publish(data_topic,msg);
      client_TB.publish(data_topic_TB,msg);
      lastTime = millis();
      delay(1000L);
  }
  delay(1000L);
  client.loop();
}
void reconnect_TB() {
  // Loop until we're reconnected
  while (!client_TB.connected()) {
    // Serial.print("Attempting MQTT connection...");
    // Attempt to connect, just a name to identify the client
    char topic[] = "v1/devices/me/rpc/request/+"; 
    char token[]= "XIfToyWIgeC"; 
    char connection_ok[] = "OK! ThingsBoard MQTT Server connected";
    char connection_fail[] = "Failed, rc=";
    if (client_TB.connect(device_id,token,NULL)) {
      Serial.println(connection_ok);
      led_up();
      delay(1000);
      led_down();
      // Once connected, publish an announcement...
      // client.publish(reply_topic,login_msg);
      // ... and resubscribe
      //client_TB.subscribe(topic);
    } else {
       Serial.print(connection_fail);
       Serial.print(client.state());
       Serial.println("Wait");
      // Wait 5 seconds before retrying
      delay(3000);
    }
  } 
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

