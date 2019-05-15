#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"



//Relays for switching appliances
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

//DHT22 for reading temperature and humidity value
#define DHTPIN            D7


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Moto"
#define WLAN_PASS       "amitv1234"

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
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  
  MQTT_connect();
  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(20000))) {
    if (subscription == &R1) {
      Serial.print(F("Got: "));
      Serial.println((char *)R1.lastread);
      int R1_State = atoi((char *)R1.lastread);
      digitalWrite(Relay1, R1_State);
      
    }
    if (subscription == &R2) {
      Serial.print(F("Got: "));
      Serial.println((char *)R2.lastread);
      int R2_State = atoi((char *)R2.lastread);
      digitalWrite(Relay2, R2_State);
    }
    if (subscription == &R3) {
      Serial.print(F("Got: "));
      Serial.println((char *)R3.lastread);
      int R3_State = atoi((char *)R3.lastread);
      digitalWrite(Relay3, R3_State);
    }
    if (subscription == &R4) {
      Serial.print(F("Got: "));
      Serial.println((char *)R4.lastread);
      int R4_State = atoi((char *)R4.lastread);
      digitalWrite(Relay4, R4_State);
      
    }
  }

  
   
  // Now we can publish stuff!

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
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

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
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
      // basically die and wait for WDT to reset me
      //while (1);
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
      }
   else{
    Serial.println("Relay1 ON");
    digitalWrite(Relay1, HIGH);
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
      }
   else{
    Serial.println("Relay2 ON");
    digitalWrite(Relay2, HIGH);
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
      }
   else{
    Serial.println("Relay3 ON");
    digitalWrite(Relay3, HIGH);
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
      }
   else{
    Serial.println("Relay4 ON");
    digitalWrite(Relay4, HIGH);
      }
    }
  }

