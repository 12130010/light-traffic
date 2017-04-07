#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"
#include <string>

//wifi
// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "NhuocQuy2"           // cannot be longer than 32 characters!
#define WLAN_PASS       "12345678ab"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2


// TTL = Time to live
int lightTTL[] = {10000, 7000, 3000, 0}; // {red:0, green:1, yellow:2, mode:3}

int TIME_DELAY = 500;

//pin
int red = 16;
int green = 14;
int yellow = 15;

int red2 = 19;
int green2 = 17;
int yellow2 = 18;

int sensor = 21;
int bell = 7;

// first line of Request
String line = "";
char c;

// life of light
int redTimeLife = lightTTL[0];
int yellowTimeLife = 0;
int greenTimeLife = 0;


int redTimeLife2 = 0;
int yellowTimeLife2 = 0;
int greenTimeLife2 = lightTTL[1];

//server
Adafruit_CC3000_Server server(80);

void setup() {
  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);

  pinMode(red2, OUTPUT);
  pinMode(yellow2, OUTPUT);
  pinMode(green2, OUTPUT);

  pinMode(bell, OUTPUT);
  pinMode(sensor, INPUT);

  digitalWrite(bell, LOW);
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  Serial.println(F("Hello, CC3000!\n"));

  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);

  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while (1);
  }

  // Optional SSID scan
  // listSSIDResults();

  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while (1);
  }

  Serial.println(F("Connected!"));

  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP()) {
    delay(100); // ToDo: Insert a DHCP timeout!
  }
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
}

void loop() {
  Serial.println(F("Loop"));
  //client available
  Adafruit_CC3000_ClientRef client = server.available();
  if (client) {
    Serial.println(F("Client"));
    while (client.available()) {
      c = client.read();
      if (c != '\r') { // just read first line
        line += c;
      }else{
        break;
      }
    }
    Serial.println();
    Serial.println(line);
    handleRequest(line);
    line = "";
    sendResponse2(client);
    delay(1000);
    client.close();
  }
  if(lightTTL[3] != 0){ // Mode is not normal
	  alwaysTurnOnRedLight(lightTTL[3]);
  }else{// Mode is normal
	  control_Light();
	  control_Light2();
	  delay(TIME_DELAY);
  }
}

void handleRequest(String line) { //line = "/set?red=1&green=2&yellow=3&mode=0 HTTP/1.1"
   Serial.println(line);
  if (line.indexOf("red") == -1 ||  line.indexOf("green") == -1 || line.indexOf("yellow") == -1)
    return;
  Serial.println("Have param");
  String param = line.substring(line.indexOf('?') + 1); //"red=1&green=2&yellow=3&mode=0 HTTP/1.1"
  Serial.println(param);

  int equalChar = -1;
  int andChar = -1;

  for (int i = 0; i < 4; i++) {
    equalChar = param.indexOf('=', equalChar + 1);
    andChar = param.indexOf('&', andChar + 1);
    
    if (andChar == -1)
      andChar = param.indexOf(' ', andChar + 1);
    lightTTL[i] = param.substring(equalChar+1, andChar).toInt();
  }
  Serial.println(lightTTL[0]);
  Serial.println(lightTTL[1]);
  Serial.println(lightTTL[2]);
  Serial.println(lightTTL[3]);
  lightBlinkReset();
}
void checkVuotDen(){
  if(digitalRead(sensor) == HIGH){
//     for(int i = 0; i < 2; i++){
        digitalWrite(bell, HIGH);
        delay(500);
        digitalWrite(bell,LOW);
//     }
  }
}
void sendResponse(Adafruit_CC3000_ClientRef client) {
  
  client.fastrprintln(F("HTTP/1.1 200 OK"));
  client.fastrprintln(F("Content-Type: text/html"));
  client.fastrprintln(F("Connection: close"));
  client.fastrprintln(F("Server: Adafruit CC3000"));
  client.fastrprintln(F(""));
  client.fastrprintln(F("<!DOCTYPE html>"));
  client.fastrprintln(F("<html>"));
  client.fastrprintln(F("<head>"));
  client.fastrprintln(F("<meta charset=\"UTF-8\">"));
  client.fastrprintln(F("<title>PINK TEAM</title>"));
  client.fastrprintln(F("</head>"));
  client.fastrprintln(F("<body>"));
  client.fastrprintln(F("<h1>Hello Pink Team</h1>"));
  client.fastrprintln(F("<form action=\"\" method=\"get\">"));
  client.fastrprint(F(" Red: <input name=\"red\" type=\"text\" value=\""));
  client.print(lightTTL[0] );
  client.fastrprintln(F("\"/>"));
  client.fastrprintln(F("<br/>"));
  client.fastrprint(F("Green: <input name=\"green\" type=\"text\" value=\""));
  client.print(lightTTL[1] );
  client.fastrprintln(F("\"/>"));
  client.fastrprintln(F("<br/>"));
  client.fastrprint(F("Yellow: <input name=\"yellow\" type=\"text\" value=\""));
  client.print(lightTTL[2]);
  client.fastrprintln(F("\"/>"));
  client.fastrprintln(F("<br/>"));
  client.fastrprintln(F("<button type=\"submit\">Submit</button>"));
  client.fastrprintln(F("</form>"));
  client.fastrprintln(F("</body>"));
  client.fastrprintln(F("</html>"));
  
  Serial.println("Responese");
  
}

void sendResponse2(Adafruit_CC3000_ClientRef client) {
	client.fastrprintln(F("HTTP/1.1 302 Found"));
	client.fastrprint(F("Location: http://192.168.43.122:8080/arduino?"));
	
	client.fastrprint(F("red="));
	client.print(lightTTL[0]);
	
	client.fastrprint(F("&green="));
	client.print(lightTTL[1]);
	
	client.fastrprint(F("&yellow="));
	client.print(lightTTL[2]);
	
	client.fastrprint(F("&mode="));
	client.print(lightTTL[3]);
	
	client.fastrprintln(F("&status=success"));
}

void lightBlinkReset() {
  Serial.println("Blink");
  for (int i = 0; i < 3 ; i++) {
    digitalWrite(red, HIGH);
    digitalWrite(green, HIGH);
    digitalWrite(yellow, HIGH);
	
	digitalWrite(red2, HIGH);
    digitalWrite(green2, HIGH);
    digitalWrite(yellow2, HIGH);
    delay(250);
    digitalWrite(red, LOW);
    digitalWrite(green, LOW);
    digitalWrite(yellow, LOW);
	
	digitalWrite(red2, LOW);
    digitalWrite(green2, LOW);
    digitalWrite(yellow2, LOW);
  }
  greenTimeLife2 = lightTTL[1];
  redTimeLife = lightTTL[0];
  
}
void control_Light() {
  if (redTimeLife != 0) {
    red_Light();
    redTimeLife -= TIME_DELAY;

    if (redTimeLife == 0) {
      greenTimeLife = lightTTL[1];
    }
  }
  if ( yellowTimeLife != 0) {
    yellow_Light();
    yellowTimeLife -= TIME_DELAY;

    if ( yellowTimeLife == 0) {
      redTimeLife = lightTTL[0];
    }
  }
  if (greenTimeLife != 0) {
    green_Light();
    greenTimeLife -= TIME_DELAY;
    if (greenTimeLife != 0) {
      yellowTimeLife = lightTTL[2];
    }
  }
}
void control_Light2() {
  if (redTimeLife2 != 0) {
    red_Light2();
    redTimeLife2 -= TIME_DELAY;

    if (redTimeLife2 == 0) {
      greenTimeLife2 = lightTTL[1];
    }
  }
  if ( yellowTimeLife2 != 0) {
    yellow_Light2();
    yellowTimeLife2 -= TIME_DELAY;

    if ( yellowTimeLife2 == 0) {
      redTimeLife2 = lightTTL[0];
    }
  }
  if (greenTimeLife2 != 0) {
    green_Light2();
    greenTimeLife2 -= TIME_DELAY;
    if (greenTimeLife2 != 0) {
      yellowTimeLife2 = lightTTL[2];
    }
  }
}
void red_Light() {
  checkVuotDen();
  digitalWrite(red, HIGH);
  digitalWrite(green, LOW);
  digitalWrite(yellow, LOW);
}

void yellow_Light() {
  digitalWrite(red, LOW);
  digitalWrite(green, LOW);
  digitalWrite(yellow, HIGH);
}
void green_Light() {
  digitalWrite(red, LOW);
  digitalWrite(green, HIGH);
  digitalWrite(yellow, LOW);
}

void red_Light2() {
  digitalWrite(red2, HIGH);
  digitalWrite(green2, LOW);
  digitalWrite(yellow2, LOW);
}

void yellow_Light2() {
  digitalWrite(red2, LOW);
  digitalWrite(green2, LOW);
  digitalWrite(yellow2, HIGH);
}
void green_Light2() {
  digitalWrite(red2, LOW);
  digitalWrite(green2, HIGH);
  digitalWrite(yellow2, LOW);
}

void alwaysTurnOnRedLight(int number){
	if(number == 1){
		red_Light();
		green_Light2();
	}
	if(number == 2){
		red_Light2();
		green_Light();
	}
}
void printWifiStatus() {
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if (!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
  }
}
