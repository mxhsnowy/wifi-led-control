#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>

/* Put your SSID & Password */
const char* ssid = "ESP32";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

static const int spiClk = 1000000; // 1 MHz

//uninitalised pointers to SPI objects
SPIClass * vspi = NULL;

uint8_t LEDpin = 4;
bool LEDstatus = LOW;
bool MODstatus = LOW;

uint8_t CRC;
uint8_t datatf;
bool CRCstatus=LOW;
void setup() {
  Serial.begin(115200);
  pinMode(LEDpin, OUTPUT);
  
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
  server.on("/", handle_OnConnect);
  server.on("/ledon", handle_ledon);
  server.on("/ledoff", handle_ledoff);
  server.on("/mod1", handle_mod1);
  server.on("/mod2", handle_mod2);
  server.on("/crcfalse", handle_CRCfalse);
  server.on("/crctrue", handle_CRCtrue);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");

  vspi = new SPIClass(VSPI);
  vspi->begin();
   pinMode(5, OUTPUT); //VSPI SS



}
void loop() {
  server.handleClient();
  if(LEDstatus)
  {
  if (MODstatus)
    {vspiCommand(0x00000011);}
  else
    {vspiCommand(0b00000010);}
  }
  else
  {vspiCommand(1);}
  
  if (CRC == 1)
  {handle_CRCtrue();}
  else
  {handle_CRCfalse();}
}

void vspiCommand(uint8_t data) {
  //use it as you would the regular arduino SPI API
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(5, LOW); //pull SS slow to prep other end for transfer
  CRC=vspi->transfer(data);
  digitalWrite(5, HIGH);
  vspi->endTransaction();
}

void handle_OnConnect() {
  LEDstatus = LOW;
  MODstatus = LOW;
  CRCstatus = LOW;
  Serial.println("GPIO4 Status: OFF ");
  server.send(200, "text/html", SendHTML(LEDstatus,MODstatus,CRCstatus)); 
}

void handle_ledon() {
  LEDstatus = HIGH;
  Serial.println("GPIO4 Status: ON");
  server.send(200, "text/html", SendHTML(true,MODstatus,CRCstatus)); 
}

void handle_ledoff() {
  LEDstatus = LOW;
  Serial.println("GPIO4 Status: OFF");
  server.send(200, "text/html", SendHTML(false,MODstatus,CRCstatus)); 
}


void handle_mod1() {
  MODstatus = HIGH;
  server.send(200, "text/html", SendHTML(LEDstatus,true,CRCstatus)); 
}

void handle_mod2() {
  MODstatus = LOW;
  server.send(200, "text/html", SendHTML(LEDstatus,false,CRCstatus)); 
}

void handle_CRCfalse() {
  CRCstatus = LOW;
  server.send(200, "text/html", SendHTML(LEDstatus,MODstatus,false));
 }
  
void handle_CRCtrue() {
  CRCstatus = HIGH;
   server.send(200, "text/html", SendHTML(LEDstatus,MODstatus,true));
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t ledstart,uint8_t modestart,uint8_t crcstart){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>EMBEDDED SYSTEM DESIGN</h1>\n";
  ptr +="<h3>TEAM 6 - LIGHT CONTROL</h3>\n";

   if(crcstart)
   {ptr +="<p>CHECK</p><a class=\"button button-on\" href=\"/crctrue\">TRUE</a>\n";}
  else
  {ptr +="<p>CHECK</p><a class=\"button button-on\" href=\"/crcfalse\">FALSE</a>\n";}
  
   if(ledstart)
   {
  {ptr +="<p>LED Status: ON</p><a class=\"button button-off\" href=\"/ledoff\">OFF</a>\n";}
  if(modestart)
  {ptr +="<p>MODE: 2</p><a class=\"button button-on\" href=\"/mod2\">2</a>\n";}
  else
  {ptr +="<p>MODE: 1</p><a class=\"button button-on\" href=\"/mod1\">1</a>\n";}
  }
   else
  {ptr +="<p>LED Status: OFF</p><a class=\"button button-on\" href=\"/ledon\">ON</a>\n";}
  ptr +="</body>\n";
  ptr +="</html>\n";
  

  return ptr;
}
