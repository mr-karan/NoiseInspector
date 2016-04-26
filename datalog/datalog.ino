#include <SoftwareSerial.h>

//period between posts, set at 16 seconds
#define DELAY_PERIOD 16000


#define SSID "geek8"
#define PASS "123kkaabb"
#define PUBLIC_KEY "q5JMKnDJKXCMnjbYr0lG"
#define PRIVATE_KEY "BVBdMRDBMAt60G1J8WV5"

#define DEBUG 0

char serialbuffer[100];//serial buffer for request command
long nextTime;//next time in millis that the temperature read will fire
int wifiConnected = 0;

SoftwareSerial mySerial(10,11); // rx, tx

void setup()
{
    mySerial.begin(9600);//connection to ESP8266
    Serial.begin(9600); //serial debug

    if(DEBUG) {
      while(!Serial);
    }

    delay(1000);//delay

    //set mode needed for new boards
    mySerial.println("AT+RST");
    delay(3000);//delay after mode change       
    mySerial.println("AT+CWMODE=1");
    delay(300);
    mySerial.println("AT+RST");
    delay(500);

    nextTime = millis();//reset the timer
}

boolean connectWifi() {     
 String cmd="AT+CWJAP=\"";
 cmd+=SSID;
 cmd+="\",\"";
 cmd+=PASS;
 cmd+="\"";
 Serial.println(cmd);
 mySerial.println(cmd);
           
 for(int i = 0; i < 20; i++) {
   Serial.print(".");
   if(mySerial.find("OK"))
   {
     wifiConnected = 1;
     break;
   }
   
   delay(50);
 }
 
 Serial.println(
   wifiConnected ? 
   "OK, Connected to WiFi." :
   "Can not connect to the WiFi."
 );
 
 return wifiConnected;
}

void loop()
{

    if(!wifiConnected) {
      mySerial.println("AT");
      delay(1000);
      if(mySerial.find("OK")){
        Serial.println("Module Test: OK");
        connectWifi();
      } 
    }

    if(!wifiConnected) {
      delay(500);
      return;
    }

    //output everything from ESP8266 to the Arduino Micro Serial output
    while (mySerial.available() > 0) {
      Serial.write(mySerial.read());
    }

    //to send commands to the ESP8266 from serial monitor (ex. check IP addr)
    if (Serial.available() > 0) {
       //read from serial monitor until terminating character
       int len = Serial.readBytesUntil('\n', serialbuffer, sizeof(serialbuffer));

       //trim buffer to length of the actual message
       String message = String(serialbuffer).substring(0,len-1);
       
       Serial.println("message: " + message);

       //check to see if the incoming serial message is an AT command
       if(message.substring(0,2)=="AT"){
         //make command request
         Serial.println("COMMAND REQUEST");
         mySerial.println(message); 
       }
    }

    //wait for timer to expire
    if(nextTime<millis()){
      Serial.print("timer reset: ");
      Serial.println(nextTime);
      nextTime = millis() + DELAY_PERIOD;
      int val = 0;
  for(int i = 0; i < 10; i++) {
      val += analogRead(0);   
      delay(1500);
  }
      SendTempData(val);

    }
}


//web request needs to be sent without the http for now, https still needs some working
void SendTempData(int temperature){
 char temp[10];

 Serial.print("loudness val: ");     
 Serial.println(temperature);

 dtostrf(temperature,1,2,temp);

 //create start command
 String startcommand = "AT+CIPSTART=\"TCP\",\"data.sparkfun.com\", 80";
 
 mySerial.println(startcommand);
 Serial.println(startcommand);
 
 //test for a start error
 if(mySerial.find("Error")){
   Serial.println("error on start");
   return;
 }
 
 //create the request command
 String sendcommand = "GET /input/"; 
 sendcommand.concat(PUBLIC_KEY);
 sendcommand.concat("?private_key=");
 sendcommand.concat(PRIVATE_KEY);
 sendcommand.concat("&temp=");
 sendcommand.concat(String(temp));
 sendcommand.concat("\r\n");
 //sendcommand.concat(" HTTP/1.0\r\n\r\n");
 
 Serial.println(sendcommand);
 
 
 //send to ESP8266
 mySerial.print("AT+CIPSEND=");
 mySerial.println(sendcommand.length());
 
 //display command in serial monitor
 Serial.print("AT+CIPSEND=");
 Serial.println(sendcommand.length());
 
 if(mySerial.find(">"))
 {
   Serial.println(">");
 }else
 {
   mySerial.println("AT+CIPCLOSE");
   Serial.println("connect timeout");
   delay(1000);
   return;
 }
 
 //Serial.print("Debug:");
 //Serial.print(sendcommand);
 mySerial.print(sendcommand);
 delay(1000);
 mySerial.println("AT+CIPCLOSE");
}


