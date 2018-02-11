/*
 * TCP/IP communication ESP8266WiFi
 * Copyright (C) 2017 http://www.projetsdiy.fr - https://www.diyprojects.io
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <ESP8266WiFi.h>

#define VSCP_DUMB_NODE  16384

const char* ssid     = "grodansparadis";       // SSID
const char* password = "xxxxxxxxxxxxx";        // Password
const char* host = "192.168.1.6";              // IP server
const int   port = 9598;                       // Server Port
const char* vscpuser = "admin";
const char* vscppassword = "secret";
String GUID;
const int freqHeartbeats = 30000;               // Heartbeat frequency
unsigned long previousMillis = millis(); 
int pidState = 0;                               // Status for pid
int lastPidState = 0;                           // Status for last pid
bool bPid = false;                              // Send detect event
bool bHeartbeat = false;                        // Flag for hartbeat send
int cnt = 0;                                    // Counter for sent frames

// Event data 
uint8_t vscpindex = 0;
uint8_t vscpzone = 0;
uint8_t vscpsubzone = 0;

// Forward declarations 
void setup(); 
void loop();
bool checkValidResponse( WiFiClient& client, unsigned long to );

///////////////////////////////////////////////////////////////////////////////////////////////////////
// setup
//

void setup() {

  // Initialize I/O
  pinMode(D0, OUTPUT);      // Blue on-board LED
  digitalWrite(D0, HIGH);   // Turn LED off
  pinMode(D1, INPUT);       // PID detector

  Serial.begin(115200);
  delay(500);
  
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // MAC
  Serial.println();
  Serial.println("MAC: " +  WiFi.macAddress() );

  // GUID ( two LSB's are free to use for this module giving up to
  // 65536 individual sensors.
  GUID = "FF:FF:FF:FF:FF:FF:FF:FE:" + WiFi.macAddress() + ":00:00";
  Serial.println("GUID: " + GUID );

  // Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
  //   would try to act as both a client and an access-point and could cause
  //   network-issues with your other WiFi-devices on your WiFi-network. 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println( WiFi.localIP( ));
}
 
///////////////////////////////////////////////////////////////////////////////////////////////////////
// loop
//
 
void loop() 
{
  unsigned long currentMillis = millis();

  // Check if it's time to send heartbat event
  if ( currentMillis - previousMillis > freqHeartbeats ) {    
    previousMillis = currentMillis;
    bHeartbeat = true;  // Send heartbeat event
  }

  // Check pid detector
  pidState = digitalRead( D1 );
  digitalWrite( D0, !pidState );

  // Set detect flag if not set
  if (pidState && !lastPidState && !bPid ) {
    bPid = true;
  }

  lastPidState = pidState;
 
  if ( bHeartbeat || bPid ) {
    
    WiFiClient client;

    client.setTimeout(2000);
    
    if ( !client.connect( host, port ) ) {
      Serial.println("connection failed");
      client.stop();
      return;
    }

    Serial.print("Connected.\n");

    if ( !checkValidResponse( client, 1000 ) ) {
      Serial.print("No valid response from server.\n");
      client.stop();
      return;
    }
        
    client.print( String("user ") + vscpuser + "\r\n" );
    if ( !checkValidResponse( client, 1000 ) ) {
      Serial.print("User not accepted.\n");
      client.stop();
      return;
    }

    Serial.print("User accepted.\n");

    client.print( String("pass ") + vscppassword + "\r\n" );
    if ( !checkValidResponse( client, 1000 ) ) {
      Serial.print("User not accepted.\n");
      client.stop();
      return;
    }

    Serial.println("Password accepted.");

    Serial.print("Time to connect: ");
    Serial.print( millis() - currentMillis, DEC  );
    Serial.println(" ms ");


    if ( bHeartbeat ) {

      Serial.println("------------------- Heartbeat ----------------------");

      // Send the node heartbeat event
      // IMPORTANT: Set the dumb node bit in the header as this is
      //    is a dumb node that does not implement registers, MDF etc.
      // CLASS1.INFORMATION (20), Type=9, node heartbeat
      //
      // send head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3
      //
      String strEvent = "send " + 
        String(VSCP_DUMB_NODE + (cnt & 7) )  + "," + // header (dumb node bit set)
        String(20) + "," +            // VSCP class
        String(9) + "," +             // VSCP type
        ",,," +                       // obid,datetime,timestamp = all set to defaults
        GUID + "," +                  // GUID for this node
        String(vscpindex) + "," +     // index for sensor
        String(vscpzone) + "," +      // Zone we are on/belong to
        String(vscpsubzone) + "\r\n"; // Subzone we are on/belong to

      client.print( strEvent );       // Send evet to the host
      if ( checkValidResponse( client, 1000 ) ) {
        cnt++;
        bHeartbeat = false;
      }

    }

    if ( bPid ) {

      Serial.println("---------------------- PID --------------------------");

      // Send the detected event
      // IMPORTANT: Set the dumb node bit in the header as this is
      //    is a dumb node that does not implement registers, MDF etc.
      // CLASS1.INFORMATION (20), Type=49, detect
      //
      // send head,class,type,obid,daettime,timestamp,GUID,data1,data2,data3
      //
      String strEvent = "send " + 
        String(VSCP_DUMB_NODE + (cnt & 7) )  + "," + // header (dumb node bit set)
        String(20) + "," +            // VSCP class
        String(49) + "," +            // VSCP type
        ",,," +                       // obid,datetime,timestamp = all set to defaults
        GUID + "," +                  // GUID for this node
        String(vscpindex) + "," +     // index for sensor
        String(vscpzone) + "," +      // Zone we are on/belong to
        String(vscpsubzone) + "\r\n"; // Subzone we are on/belong to

      client.print( strEvent );       // Send evet to the host
      if ( checkValidResponse( client, 1000 ) ) {
        bPid = false;
      }

    }

    /*
    delay(3000);
    while(client.available()){
      rplyline = client.readStringUntil('\r');
      Serial.print(rplyline);
    }

    if ( !rplyline.startsWith("+OK") ) {
      Serial.print("Password not accepted.");  
      return;
    }
    */

    client.stop();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// checkValidResponse
//

bool checkValidResponse( WiFiClient& client, unsigned long to )
{
    String rplyline;    
    unsigned long start = millis();

    // Loop until "+OK" is found or timeout
    while ( ( millis() - start ) < to  ) {
      
      if ( client.available() ) {
        rplyline = client.readStringUntil('\r');
        Serial.print( rplyline );
      }

      if ( !rplyline.startsWith("+OK") ) {
        return true;
      }
    }

    return false;
}



