#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"
#include <EEPROM.h>


#define Relay1            D1
#define Relay2            D2
#define Relay3            D3
#define Relay4            D4

#define pbuttonPin1            D0
#define pbuttonPin2            D5
#define pbuttonPin3            D6
#define pbuttonPin4            D8

int val1=1;
int lightOn1=1;
int pushed1=1;

int val2=1;
int lightOn2=1;
int pushed2=1;

int val3=1;
int lightOn3=1;
int pushed3=1;

int val4=1;
int lightOn4=1;
int pushed4=1;

#define DHTPIN            D7


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "redmi"
#define WLAN_PASS       "123456789"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "sumitkv"
#define AIO_KEY         "1846b0a65e5e4dd292837ad35cd26df2"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe R1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/relay1");
Adafruit_MQTT_Subscribe R2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/relay2");
Adafruit_MQTT_Subscribe R3 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/relay3");
Adafruit_MQTT_Subscribe R4 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/relay4");



/************ Necessary declaration for DHT22 ******************/
#define DHTTYPE           DHT22     

DHT dht(DHTPIN, DHTTYPE);
uint32_t delayMS;


/*************************** Sketch Code ************************************/

void MQTT_connect();
void setup() {
  Serial.begin(115200);
  delay(10);
  
  pinMode(pbuttonPin1,INPUT_PULLUP);
  pinMode(Relay1, OUTPUT);

  pinMode(pbuttonPin2,INPUT_PULLUP);
  pinMode(Relay2, OUTPUT);

  pinMode(pbuttonPin3,INPUT_PULLUP);
  pinMode(Relay3, OUTPUT);

  pinMode(pbuttonPin4,INPUT_PULLUP);
  pinMode(Relay4, OUTPUT);

  EEPROM.begin(4);
  eeprom_state();

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(15);
    Serial.println(".");
    manual_relay();
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  
  //Setting up DHT sensor
  dht.begin();

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&R1);
  mqtt.subscribe(&R2);
  mqtt.subscribe(&R3);
  mqtt.subscribe(&R4);
}

uint32_t x = 0;

void loop() {
  
  MQTT_connect();
  
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(20000))) {
    if (subscription == &R1) {
      Serial.print(F("Got: "));
      Serial.println((char *)R1.lastread);
      int R1_State = atoi((char *)R1.lastread);
      digitalWrite(Relay1, R1_State);
      EEPROM.write(0,R1_State);
      EEPROM.commit(); 
    }
    if (subscription == &R2) {
      Serial.print(F("Got: "));
      Serial.println((char *)R2.lastread);
      int R2_State = atoi((char *)R2.lastread);
      digitalWrite(Relay2, R2_State);
      EEPROM.write(1,R2_State);
      EEPROM.commit();
    }
    if (subscription == &R3) {
      Serial.print(F("Got: "));
      Serial.println((char *)R3.lastread);
      int R3_State = atoi((char *)R3.lastread);
      digitalWrite(Relay3, R3_State);
      EEPROM.write(2,R3_State);
      EEPROM.commit();
    }
    if (subscription == &R4) {
      Serial.print(F("Got: "));
      Serial.println((char *)R4.lastread);
      int R4_State = atoi((char *)R4.lastread);
      digitalWrite(Relay4, R4_State);
      EEPROM.write(3,R4_State);
      EEPROM.commit();   
    }
  }

  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  if (! Humidity.publish(h)) {
    Serial.println(("Failed"));
  } else {
    Serial.print(("Humidity is "));
    Serial.println((h));
  }
  if (! Temperature.publish(t)) {
    Serial.println(("Failed"));
  } else {
    Serial.print(("Temperature is "));
    Serial.println((t));
  }

}

void MQTT_connect() {
  int8_t ret;
  
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      ESP.reset();
    }
  }
  Serial.println("MQTT Connected!");

}

void manual_relay(){ 
  val1 = digitalRead(pbuttonPin1);
  val2 = digitalRead(pbuttonPin2);
  val3 = digitalRead(pbuttonPin3);
  val4 = digitalRead(pbuttonPin4);
  
  if(val1==LOW || val2==LOW || val3==LOW || val4==LOW) {
    delay(10);
  //For Push button 1
  if(val1 == HIGH && lightOn1 == LOW){
    pushed1 = 1-pushed1;
    delay(100);
  }    
  lightOn1 = val1;
  if(pushed1 == HIGH){
    Serial.println("Relay1 OFF");
    digitalWrite(Relay1, LOW);
    EEPROM.write(0,0);
    EEPROM.commit();   
      }
   else{
    Serial.println("Relay1 ON");
    digitalWrite(Relay1, HIGH);
    EEPROM.write(0,1);
    EEPROM.commit();
      } 
      
  //For Push button 2
  if(val2 == HIGH && lightOn2 == LOW){
    pushed2 = 1-pushed2;
    delay(100);
  }    
  lightOn2 = val2;
  if(pushed2 == HIGH){
    Serial.println("Relay2 OFF");
    digitalWrite(Relay2, LOW);
    EEPROM.write(1,0);
    EEPROM.commit();   
      }
   else{
    Serial.println("Relay2 ON");
    digitalWrite(Relay2, HIGH);
    EEPROM.write(1,1);
    EEPROM.commit();
      }

  //For Push button 3
  if(val3 == HIGH && lightOn3 == LOW){
    pushed3 = 1-pushed3;
    delay(100);
  }    
  lightOn3 = val3;
  if(pushed3 == HIGH){
    Serial.println("Relay3 OFF");
    digitalWrite(Relay3, LOW);
    EEPROM.write(2,0);
    EEPROM.commit();   
      }
   else{
    Serial.println("Relay3 ON");
    digitalWrite(Relay3, HIGH);
    EEPROM.write(2,1);
    EEPROM.commit();
      }

  //For Push button 4
  if(val4 == HIGH && lightOn4 == LOW){
    pushed4 = 1-pushed4;
    delay(100);
  }    
  lightOn4 = val4;
  if(pushed4 == HIGH){
    Serial.println("Relay4 OFF");
    digitalWrite(Relay4, LOW);
    EEPROM.write(3,0);
    EEPROM.commit();   
      }
   else{
    Serial.println("Relay4 ON");
    digitalWrite(Relay4, HIGH);
    EEPROM.write(3,1);
    EEPROM.commit();
      }
    }
  }


void eeprom_state(){
    //For Relay 1
  if (EEPROM.read(0)==1){
    Serial.print("Relay1 is ON");
    digitalWrite(Relay1,HIGH);
    pushed1=0;
    }
  if (EEPROM.read(0)==0){
    Serial.print("Relay1 is OFF");
    digitalWrite(Relay1,LOW);
    pushed1=1;
    }
    
    //For Relay 2
  if (EEPROM.read(1)==1){
    digitalWrite(Relay2,HIGH);
    pushed2=0;
    }
  if (EEPROM.read(1)==0){
    digitalWrite(Relay2,LOW);
    pushed2=1;
    }

    //For Relay 3
  if (EEPROM.read(2)==1){
    digitalWrite(Relay3,HIGH);
    pushed3=0;
    }
  if (EEPROM.read(2)==0){
    digitalWrite(Relay3,LOW);
    pushed3=1;
    }
    
    //For Relay 4
  if (EEPROM.read(3)==1){
    digitalWrite(Relay4,HIGH);
    pushed4=0;
    }
  if (EEPROM.read(3)==0){
    digitalWrite(Relay4,LOW);
    pushed4=1;
    }
  }

