//#define STANDALONE

#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "XL320.h"

// Name your robot!
XL320 robot;

// Set some variables for incrementing position & LED colour
char rgb[] = "rgbypcwo";
int servoPosition = 0;
int ledColour = 0;

// Set the default servoID to talk to
int servoID = 1;
/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "roboy"
#define WLAN_PASS       "wiihackroboy"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "192.168.0.110"
#define AIO_SERVERPORT  8883
#define AIO_USERNAME    "roboy"
#define AIO_KEY         "11111"

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

//// Setup a feed called 'time' for subscribing to current time
//Adafruit_MQTT_Subscribe timefeed = Adafruit_MQTT_Subscribe(&mqtt, "time/seconds");
//
//// Setup a feed called 'slider' for subscribing to changes on the slider
//Adafruit_MQTT_Subscribe slider = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/slider", MQTT_QOS_1);
//
//// Setup a feed called 'onoff' for subscribing to changes to the button
//Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff", MQTT_QOS_1);

Adafruit_MQTT_Subscribe test = Adafruit_MQTT_Subscribe(&mqtt, "/test", MQTT_QOS_1);
Adafruit_MQTT_Subscribe posM1 = Adafruit_MQTT_Subscribe(&mqtt, "/position/M1", MQTT_QOS_1);
Adafruit_MQTT_Subscribe posM2 = Adafruit_MQTT_Subscribe(&mqtt, "/position/M2", MQTT_QOS_1);
Adafruit_MQTT_Subscribe posM3 = Adafruit_MQTT_Subscribe(&mqtt, "/position/M3", MQTT_QOS_1);
Adafruit_MQTT_Subscribe posM4 = Adafruit_MQTT_Subscribe(&mqtt, "/position/M4", MQTT_QOS_1);

/*************************** Sketch Code ************************************/

int sec;
int min;
int hour;

int timeZone = -4; // utc-4 eastern daylight time (nyc)

void onoffcallback(char *data, uint16_t len) {
  Serial.print("Hey we're in a onoff callback, the button value is: ");
  Serial.println(data);
}

void positionM1callback(double value){
    robot.Write(0, XL320::Address::GOAL_POSITION ,value);
}
void positionM2callback(int value){
    robot.Write(1, XL320::Address::GOAL_POSITION ,value);
}
void positionM3callback(int value){
    robot.Write(2, XL320::Address::GOAL_POSITION ,value);
}
void positionM4callback(int value){
    robot.Write(3, XL320::Address::GOAL_POSITION ,value);
}

void testcallback(double x) {
  #ifndef STANDALONE
    Serial.print("Hey we're in a test callback, the button value is: ");
  #endif

  if (x<=1) {
    
//    Serial.println(x);
//    // LED test.. select a random servoID and colour
    robot.Write(random(0,3), XL320::Address::LED, rgb[random(0,7)] );
  
    // LED colour test.. cycle between RGB, increment the colour and return 1 after 3
  //  robot.LED(servoID, &rgb[ledColour]);
    ledColour = (ledColour + 1) % 3;
  
    // Set a delay to account for the receive delay period
    delay(100);
  
    // Servo test.. let's randomly set the position (0-1023)
  //  robot.moveJoint(servoID, random(0, 1023));
  
    // Servo test.. select a random servoID and colour
    robot.Write(random(0,3), XL320::Address::GOAL_POSITION, random(0, 1023));
  
    // Servo test.. increment the servo position by 100 each loop
  //  robot.moveJoint(servoID, servoPosition);
    servoPosition = (servoPosition + 100) % 1023;
    
    // Set a delay to account for the receive delay period
    delay(100);
  }
  
//  Serial.println(x);
  
}


void setup() {
//  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
   
  #ifndef STANDALONE
    Serial.begin(1000000);
  #endif
    Serial.begin(1000000);
  delay(10);

  robot.Begin(Serial);
  
//  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  #ifndef STANDALONE
    Serial.println(); Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WLAN_SSID);
  #endif

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #ifndef STANDALONE
      Serial.print(".");
    #endif
  }
//  Serial.println();

  #ifndef STANDALONE
    Serial.println("WiFi connected");
    Serial.println("IP address: "); Serial.println(WiFi.localIP());
  #endif
  
//  timefeed.setCallback(timecallback);
//  slider.setCallback(slidercallback);
//  onoffbutton.setCallback(onoffcallback);
  test.setCallback(testcallback);
  posM1.setCallback(positionM1callback);
  
  
  // Setup MQTT subscription for time feed.
//  mqtt.subscribe(&timefeed);
//  mqtt.subscribe(&slider);
//  mqtt.subscribe(&onoffbutton);
  mqtt.subscribe(&test);
  mqtt.subscribe(&posM1);

}

uint32_t x=0;
double value=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets and callback em' busy subloop
  // try to spend your time here:
  mqtt.processPackets(10000);
  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  
  if(! mqtt.ping()) {
    mqtt.disconnect();
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

  #ifndef STANDALONE
    Serial.print("Connecting to MQTT... ");
  #endif
  
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       #ifndef STANDALONE
         Serial.println(mqtt.connectErrorString(ret));
         Serial.println("Retrying MQTT connection in 10 seconds...");
       #endif
       mqtt.disconnect();
       delay(10000);  // wait 10 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  #ifndef STANDALONE
    Serial.println("MQTT Connected!");
  #endif
}
