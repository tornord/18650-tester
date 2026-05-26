#include <Arduino.h>
#include <WiFiS3.h>
#include <Adafruit_INA219.h>

#include "arduino_secrets.h"
#include "Battery.h"

int status = WL_IDLE_STATUS;
WiFiServer server(80);
Adafruit_INA219 ina219_A(0x40);
Battery batA('A', 2, 3, &ina219_A);

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void setup()
{
  while (!Serial)
  {
    delay(1);
  }
  Serial.begin(115200);
  Serial.println("Serial open!");

  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("Communication with WiFi module failed!");
    while (true)
    {
      delay(1);
    }
  }
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(SECRET_SSID);
    status = WiFi.begin(SECRET_SSID, SECRET_PASS);
    delay(4000);
  }
  server.begin();
  printWifiStatus();

  if (!batA.ina219->begin())
  {
    Serial.println("Failed to find ina219 @ 0x40 chip");
    batA.state = S_NO_INA219;
  }
}

void wifiLoop(Battery &bat1)
{
  WiFiClient client = server.available(); // listen for incoming clients

  if (client)
  {                               // if you get a client,
    Serial.println("new client"); // print a message out the serial port
    String currentLine = "";      // make a String to hold incoming data from the client
    while (client.connected())
    {
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out to the serial monitor
        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Access-Control-Allow-Origin: *"); // allow all origins
            client.println("Access-Control-Allow-Methods: GET, POST, OPTIONS");
            client.println("Access-Control-Allow-Headers: Content-Type");
            client.println("Connection: close");
            client.println();
            client.println("[");
            bat1.printStatusJson(client);
            client.println();
            client.println("]");
            break;
          }
          else
          {
            currentLine = "";
          }
        }
        else if (c != '\r')
        {
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void loop(void)
{
  batA.loop();
  wifiLoop(batA);
  delay(100);
}