#include <SPI.h>
#include <WiFi101.h>
#include "DHT.h"
#include <HttpClient.h>

//
// DHT22 related 
//
#define DHTPIN 10
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE, 15);


//
// Local WIFI module (ATWINC1500)
//
char  ssid[] = "5127";         //  your network SSID (name)
char  pass[] = "Uf4x7tkeHL";   // your network password (use for WPA, or use as key for WEP)
int   keyIndex = 0;             // your network key Index number (needed only for WEP)
void  printWifiStatus();

int status = WL_IDLE_STATUS;


//
// LEDS
//
#define GREEN_LED_PIN 11
#define ONBOARD_RED_LED_PIN 13


//
// Adafruit IO
//
// Adafruit IO
#define AIO_SERVER          "io.adafruit.com"
#define AIO_REST_API_PORT   80
#define AIO_USERNAME        "ppiquette"
#define AIO_KEY             "51452b0f8beb4ea7a73e7645b81da620"
#define AIO_Temperature_ID  "615913"
#define AIO_Humidity_ID     "616438"


// 
// HTTP Client related
//

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

WiFiClient client;
HttpClient http(client);

void DisplayAllFeeds();





void setup() {
  
  //
  // LEDS
  //
  // initialize digital LED pins as outputs.
  pinMode(ONBOARD_RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  digitalWrite(ONBOARD_RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);


  //
  // Serial port
  //
 
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  //
  // DHT module
  //
  dht.begin();


  //
  // Wifi module
  //
  // Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8,7,4,2);


  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait for connection
    delay(1000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
  digitalWrite(ONBOARD_RED_LED_PIN, HIGH);


  
//  Serial.println("\Trying connection (GET all feeds) to AdafruitIO server...");
//  DisplayAllFeeds();
}  


void loop() {

  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("Getting Temperature and Humidity");

  int humidity_data = (int)dht.readHumidity();
  int temperature_data = (int)dht.readTemperature();
  Serial.print("Temperature: ");
  Serial.println(temperature_data);
  Serial.print("Humidity: ");
  Serial.println(humidity_data);

  Serial.println();
  Serial.println("Sending to Adafruit IO");
  http.beginRequest();
  String s = "/api/feeds/";
  s += AIO_Temperature_ID;
  s += "/data";
  int connectStatus = http.startRequest( AIO_SERVER, AIO_REST_API_PORT, s.c_str() , HTTP_METHOD_POST, NULL);
  http.sendHeader("x-aio-key", AIO_KEY);
  
  //client.println();
  //client.println("\{\"value\": \"678\"\}");
  //http.sendHeader("value", "34");

  http.endRequest();
  DisplayResponse();

  delay(1000);
}


void DisplayResponse()
{
    int statusCode = http.responseStatusCode();
    if (statusCode >= 0)
    {
      Serial.print("Status code: ");
      Serial.println(statusCode);
  
      // Usually you'd check that the response code is 200 or a
      // similar "success" code (200-299) before carrying on,
      // but we'll print out whatever response we get
  
      int skipHeaderStatus = http.skipResponseHeaders();
      if (skipHeaderStatus >= 0)
      {
        int bodyLen = http.contentLength();
        Serial.print("Content length is: ");
        Serial.println(bodyLen);
        Serial.println("Body returned follows:");
        
        // Now we've got to the body, so we can print it out
        unsigned long timeoutStart = millis();
        char c;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ( (http.connected() || http.available()) && ((millis() - timeoutStart) < kNetworkTimeout) )
        {
          if (http.available())
          {
            c = http.read();
            // Print out this character
            Serial.print(c);
                 
            // We read something, reset the timeout counter
            timeoutStart = millis();
          }
          else
          {
             // We haven't got any data, so let's pause to allow some to
             // arrive
             delay(kNetworkDelay);
          }
        }
        Serial.println("");
        Serial.println("End of returned body");
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(skipHeaderStatus);
      }
    }
    else
    {    
      Serial.print("Getting response failed: ");
      Serial.println(statusCode);
    }
}



//
// Get request to retrieve all feeds in my account
//
void DisplayAllFeeds()
{
  http.beginRequest();
  int connectStatus = http.startRequest( AIO_SERVER, AIO_REST_API_PORT, "/api/feeds" , HTTP_METHOD_GET, NULL);
  http.sendHeader("x-aio-key", AIO_KEY);
  http.endRequest();
    
  if (connectStatus == 0)
  {
    Serial.println("Request Sent");
    DisplayResponse();
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(connectStatus);
  }
  http.stop();
}




void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}





