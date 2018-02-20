// Datalogging ESP8266 -12 sketch 

#include "DHT.h"
#include <ESP8266WiFi.h>

#define DHTPIN1 13    // Sensor 1
#define DHTPIN2 12    // Sensor 2
#define DHTPIN3 2     // Sensor 3
#define DHTPIN4 16    // Sensor 4

// Type of DHT sensor
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
//
// DAT: 15 needed as were running on ESP8266 and 3.3v auto seems to screw up
DHT dht1(DHTPIN1, DHTTYPE, 15);
DHT dht2(DHTPIN2, DHTTYPE, 15);
DHT dht3(DHTPIN3, DHTTYPE, 15);
DHT dht4(DHTPIN4, DHTTYPE, 15);

const char* ssid     = "Attic";
const char* password = "wanderonintotexel";

const char* host = "10.0.0.30";
const char* port = "1010";


void setup() {
  // Init Serial for when we are bench testing.
  Serial.begin(115200);
  Serial.println("Hay temperatures!");

  // Set all required pins to input just in case
  pinMode(DHTPIN1, INPUT_PULLUP);
  pinMode(DHTPIN2, INPUT_PULLUP);
  pinMode(DHTPIN3, INPUT_PULLUP);
  pinMode(DHTPIN4, INPUT_PULLUP);
  
  connectToWifi();

  // Initialise all dht sensors
  dht1.begin();
  dht2.begin();
  dht3.begin();
  dht4.begin();
}

void loop() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  
  // Read temperature as Celsius (the default)
  float h1 = dht1.readHumidity();
  float t1 = dht1.readTemperature();
  delay(200);
  float h2 = dht2.readHumidity();
  float t2 = dht2.readTemperature();
  delay(200);
  float h3 = dht3.readHumidity();
  float t3 = dht3.readTemperature();
  delay(200);
  float h4 = dht4.readHumidity();
  float t4 = dht4.readTemperature();

  // Check if any reads failed and exit early (to try again).
  // DAT: Removed as we want to transmit "nan" if we have a problem, 
  // let the webserver sort it out
  //  if (isnan(h1) || isnan(t1) ) {
  //    Serial.println("Failed to read from DHT sensor!");
  //    return;
  //  }

  // Debug temps to serial
  printTemp(h1, t1);
  printTemp(h2, t2);
  printTemp(h3, t3);
  printTemp(h4, t4);

  Serial.println(WiFi.localIP());
  Serial.println("------");

  sendToServer(t1, h1, t2, h2, t3, h3, t4, h4);

  // Wait ten minutes between measurements.
  delay(600000);
}

void printTemp(float h, float t) {
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.println("");
}

void connectToWifi() {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendToServer(
  float t1, float h1, 
  float t2, float h2, 
  float t3, float h3, 
  float t4, float h4) {
    Serial.print("connecting to ");
    Serial.println(host);
    
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 1010;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }
    
    // We now create a URI for the request
    String url = "/gpio/hay";
    url += "?t1=";
    url += t1;
    url += "&h1=";
    url += h1;

    url += "&t2=";
    url += t2;
    url += "&h2=";
    url += h2;

    url += "&t3=";
    url += t3;
    url += "&h3=";
    url += h3;

    url += "&t4=";
    url += t4;
    url += "&h4=";
    url += h4;
    
    
    Serial.print("Requesting URL: ");
    Serial.println(url);
    
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
    delay(10);
    
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    
    Serial.println();
    Serial.println("closing connection");
}