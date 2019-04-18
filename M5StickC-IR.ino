/*
 * M5StickC-IR v0.0.2
 */
 
#include <M5StickC.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <WiFiManager.h> //https://github.com/Brunez3BD/WIFIMANAGER-ESP32
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <aJSON.h>       //https://github.com/interactive-matter/aJson  https://gitlab.com/xarduino/lightsw/blob/master/patch/ajson-void-flush.patch
#include <IRremote.h>    //https://github.com/SensorsIot/Arduino-IRremote

WebServer webServer(80);
byte SEND_PIN = 9; // IR:9
IRsend irsend(SEND_PIN);

void handleIndex(){
  Serial.println("handleIndex");
  String s = "<html lang=\"en\"><head><meta charset=\"utf-8\"/><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>M5StickC-IR</title><link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.7/css/bootstrap.min.css\" /><script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery/3.2.1/jquery.min.js\"></script><script src=\"https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.7/js/bootstrap.min.js\"></script></head><body><div class=\"container\"><div class=\"row\"><div class=\"col-md-12\"><h1>M5StickC-IR <small>";
  //s += localName;
  s += "m5c";
  s += ".local</small></h1><p>IP address: ";
  s += String(WiFi.localIP()[0]) + "." + String(WiFi.localIP()[1]) + "." + String(WiFi.localIP()[2]) + "." + String(WiFi.localIP()[3]);
  s += "</p><hr><div class=\"form-group\">";
  s += "<textarea class=\"form-control\" id=\"message\" rows=\"10\"></textarea></div>";
  s += "<div class=\"row\"> <button class=\"btn btn-success\" id=\"post\">POST</button> ";
  s += " <button class=\"btn btn-danger\" id=\"LEDON\">LED ON</button> ";
  s += " <button class=\"btn btn-info\" id=\"LEDOFF\">LED OFF</button>";
  //無線リセット
  s += " <button class=\"btn\" id=\"WIFIRESET\">WiFi reset</button> </div>";
  
  s += "</div>";
  s += "<script>var xhr = new XMLHttpRequest();var textarea = document.getElementById(\"message\");";
  //s += "document.getElementById(\"get\").addEventListener(\"click\", function () {xhr.open('GET', '/messages', true);xhr.setRequestHeader('X-Requested-With', 'curl');xhr.onreadystatechange = function() {if(xhr.readyState == 4) {textarea.value =xhr.responseText;}};xhr.send(null);});";
  s += "document.getElementById(\"post\").addEventListener(\"click\", function () {data = textarea.value;xhr.open('POST', '/messages', true);xhr.onreadystatechange = function() {if(xhr.readyState == 4) {alert(xhr.responseText);}};xhr.setRequestHeader('Content-Type', 'application/json');xhr.setRequestHeader('X-Requested-With', 'curl');xhr.send(data);});";

  //LED ON
  s += "document.getElementById(\"LEDON\").addEventListener(\"click\", function () {xhr.open('get', '/ledon', true);xhr.setRequestHeader('X-Requested-With', 'curl');xhr.onreadystatechange = function() {if(xhr.readyState == 4) {textarea.value =xhr.responseText;}};xhr.send(null);});";
  //LED OFF
  s += "document.getElementById(\"LEDOFF\").addEventListener(\"click\", function () {xhr.open('get', '/ledoff', true);xhr.setRequestHeader('X-Requested-With', 'curl');xhr.onreadystatechange = function() {if(xhr.readyState == 4) {textarea.value =xhr.responseText;}};xhr.send(null);});";
  //OnDemandAp Wifi設定 wifi reset
  s += "document.getElementById(\"WIFIRESET\").addEventListener(\"click\", function () {var result=confirm('Reset WiFi settings?');if(result){xhr.open('get', '/wifireset', true);xhr.setRequestHeader('X-Requested-With', 'curl');xhr.onreadystatechange = function() {if(xhr.readyState == 4) {textarea.value =xhr.responseText;}};xhr.send(null);}});";

  s += "</script></body></html>";
  webServer.send(200, "text/html", s);  
}

void handleNotFound() {
  Serial.println(webServer.uri().c_str());
  Serial.println("404");
  webServer.send(404, "text/plain", "Not found.");
}

void handleMessages() {
  Serial.println(webServer.uri().c_str()); //add

  if (webServer.method() == HTTP_POST) {
    //irrecv.disableIRIn();  //
    String req = webServer.arg(0);
    char json[req.length() + 1];
    req.toCharArray(json, req.length() + 1);
    Serial.println(json); //受信データ確認
    aJsonObject* root = aJson.parse(json);
    if (root != NULL) {
      aJsonObject* freq = aJson.getObjectItem(root, "freq");
      aJsonObject* data = aJson.getObjectItem(root, "data");
      if (freq != NULL && data != NULL) {
        const uint16_t d_size = aJson.getArraySize(data);
        //uint16_t rawData[d_size];
        unsigned int  rawData[d_size]; //型変更
        for (int i = 0; i < d_size; i++) {
          aJsonObject* d_int = aJson.getArrayItem(data, i);
          rawData[i] = d_int->valueint;
        }
        irsend.sendRaw(rawData, d_size, (uint16_t)freq->valueint);
        //irrecv.enableIRIn(); //
        req = "";
        aJson.deleteItem(root);
        webServer.sendHeader("Access-Control-Allow-Origin", "*");
        webServer.send(200, "text/plain", "ok");
      } else {
        webServer.send(400, "text/plain", "Invalid JSON format");
      }
    } else {
      webServer.send(400, "text/plain", "Request body is empty");
    }
  }
}

void handleLedOn() {
  Serial.println("LED ON");
  digitalWrite(M5_LED, LOW); //LED ON
  webServer.send(200, "text/plain", "ok");
}
void handleLedOff() {
  Serial.println("LED OFF");
  digitalWrite(M5_LED, HIGH);  //LED OFF
  webServer.send(200, "text/plain", "ok");
}

void handleWiFiReset(); //宣言

void setup() {  // ---------------------------------------------------------------
  M5.begin();
  M5.Lcd.fillScreen(BLACK);
  M5.Axp.ScreenBreath(9); //7 - 16 暗めにしたい

  // LED
  pinMode(M5_LED, OUTPUT);
  digitalWrite(M5_LED, LOW); //LEDON
  delay(200);
  digitalWrite(M5_LED, HIGH); //LEDOFF
  pinMode(M5_BUTTON_HOME, INPUT);
  pinMode(M5_BUTTON_RST, INPUT);
  
  //start serial connection
  Serial.begin(115200);
  Serial.println("\n Starting");

  WiFiManager wifiManager; 

  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(5,0);
  //M5.Lcd.println("AP mode: \r\nAutoConnectAP");
  //M5.Lcd.println("IP address: \r\n192.168.4.1");

  M5.Lcd.println("M5StickC-IR");
  M5.Lcd.println("\nIf you want to setup WiFi, \nPower off, \nPress Button RST \nand Power On!!!");
  //M5.Lcd.println("IP address: \r\n192.168.4.1");
  //delay(5000);

  if(digitalRead(M5_BUTTON_RST) == LOW){ //起動時RSTボタンが押されてたら
      M5.Lcd.fillScreen(BLACK); //clear
      M5.Lcd.setTextSize(1);
      M5.Lcd.setCursor(5,0);
      M5.Lcd.println("AP mode: \nOnDemandAP");
      M5.Lcd.println("\nconnect to WiFi, \nM5StickC-AP");
      M5.Lcd.println("\nOpen \n192.168.4.1");

      digitalWrite(M5_LED, LOW); //LED ON
      while(digitalRead(M5_BUTTON_RST) == LOW);
      digitalWrite(M5_LED, HIGH); //LED OFF
      for (int i=0; i <= 3; i++){
        delay(200);
        digitalWrite(M5_LED, LOW); //LED ON
        delay(200);
        digitalWrite(M5_LED, HIGH); //LED OFF
      }
      //起動時のボタンでWiFi設定起動 WiFiManager
      if (!wifiManager.startConfigPortal("M5StickC-AP")) {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
      }
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(2000);

  } else {
      //wifiManager.autoConnect("M5StickC-AP"); //AutoConnectAP
      Serial.println("WiFi.begin()");
      WiFi.begin();
      M5.Lcd.setCursor(5,120);
      int i = 0;
      while (WiFi.status() != WL_CONNECTED) { //接続まで最大10秒待つ
        if(i > 20){break;}
        delay(500); 
        i++;
        Serial.print(".");
        M5.Lcd.print(".");
      }
        Serial.println("!");
        M5.Lcd.println("! ");
  }

  IPAddress ipadr = WiFi.localIP();

  if(WiFi.status() == WL_CONNECTED){
    Serial.println("connected..");
    M5.Lcd.println("connected..");
    Serial.println(WiFi.SSID());
  } else {
    Serial.println("not connected!!!");
    M5.Lcd.println("not connected!!!");
  }
  
  delay(1000); 
  M5.Lcd.fillScreen(BLACK); //clear
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println("M5StickC-IR");//AutoConnectAP

  //M5.Lcd.setCursor(0,20);
  //M5.Lcd.println("SSID: ");
  //M5.Lcd.print(WiFi.SSID());
  M5.Lcd.setCursor(0,60);
  M5.Lcd.println("IP address: ");
  M5.Lcd.print((String)ipadr[0] + "." + (String)ipadr[1] + "." + (String)ipadr[2] + "." + (String)ipadr[3]);

  //mDNSの設定
  if (MDNS.begin("m5c")) {
      MDNS.addService("http", "tcp", 80);
      Serial.println("mDNS responder started");
  }

  // ウェブサーバーの設定
  webServer.on("/", handleIndex); // root
  webServer.on("/messages", handleMessages);  //messages
  webServer.on("/ledon", handleLedOn);  //LED ON  
  webServer.on("/ledoff"    , handleLedOff);  //LED OFF
  webServer.on("/wifireset", handleWiFiReset); //設定やり直し
  webServer.on("/favicon.ico", handleNotFound); //faviconなし

  webServer.onNotFound(handleNotFound);  
  webServer.begin();
  Serial.println("Web server started");
}

void loop() {
  //右ボタン 
  if(digitalRead(M5_BUTTON_RST) == LOW){
    Serial.println("RST");
    digitalWrite(M5_LED, LOW);  //LED ON
    irsend.sendNEC(0x02fd58a7, 32); //TOSHIBA REGZA VOLUME UP 02fd58a7 
    delay(200);
    irsend.sendPanasonic(0x4004, 0x0D00818C); //Panasonic DIGA Back 40040D00818C
    while(digitalRead(M5_BUTTON_RST) == LOW);
    digitalWrite(M5_LED, HIGH);  //LED OFF
  }
  //HOMEボタン 
  if(digitalRead(M5_BUTTON_HOME) == LOW){
    Serial.println("HOME");
    digitalWrite(M5_LED, LOW);  //LED ON
    irsend.sendNEC(0x02fd7887, 32); //TOSHIBA REGZA VOL DOWN 02fd7887
    delay(200);
    irsend.sendPanasonic(0x4004, 0x0D00FAF7); //Panasonic DIGA display 40040D00FAF7
    while(digitalRead(M5_BUTTON_HOME) == LOW);
    digitalWrite(M5_LED, HIGH);  //LED OFF
  }
  
  //クライアントからの要求に応じて処理を行う
  webServer.handleClient();
}

void handleWiFiReset() { //WiFi reset
  Serial.println("reset WiFi settings");
  webServer.send(200, "text/plain", "reset WiFi settings...");

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,4);
  M5.Lcd.println("Reset WiFi settings");

  WiFi.disconnect(true);   // still not erasing the ssid/pw. Will happily reconnect on next start
  WiFi.begin("0","0");       // adding this effectively seems to erase the previous stored SSID/PW
  ESP.restart();
  delay(1000);
}
