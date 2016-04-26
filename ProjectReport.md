#SNU Library Noise Monitor

##Domain: Smart Buildings
This project serves to help making the library noise-free. Since it uses wireless sensor networking capabilities to interact with the building and automate a task, we think it jutfies the `Smart Buildings` category.

##Objective:
The objective of this project is to actively monitor noise level inside the library. We are using a Sound Sensor with ESP8266 (Wifi Module) to monitor noise levels in the library. We aggregate the sensor data every 15 seconds and if it crosses the threshold level, we trigger an SMS to the guard's mobile phone present in the library. 
## Flowchart of the application:
![](http://i.imgur.com/juSDDup.png)

##Discuss 2-3 real world applications of the project:
We have made our project as a real word application which can be used in our university's campus. Apart from this, this project can be used in areas like schools, hospitals and other buildings where silence has to be maintained. 
Multiple arduino boards can be set up in the building where each can measure the noise levels and notify the nearest concerned authority. 

#Hardware Used:
| Components      | Units           | Cost |
| :-------------: |:-------------:| :-----:|
| Arduino UNO     | 1 | ₹600 |
| ESP8266 Wifi Module      | 1  |   ₹250 |
| Analog Sound Sensor | 1   | ₹100   |
| Breadboard | 1   | ₹130  |
| Jumper Wires | 15   | ₹50   |

##Snapshot of circuit:
![](http://i.imgur.com/D9g4FvH.jpg)

#Breadboard View:
![](http://i.imgur.com/kQwXOEv.png)

#Screenshots: 

![](http://i.imgur.com/febtaZm.png)
![](http://i.imgur.com/hJJvUmE.png)
![](http://i.imgur.com/M3pcg9y.png)


##Code (prototype + GUI):

###Arduino(For getting data+ communicating wth server):
```
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



```



###Processing (For showing spikes in voltage values):
```
import processing.serial.*;
// at the top of the program:
float xPos = 0; // horizontal position of the graph
float yPos = 0; // vertical position of the graph
Serial myPort;
void setup () {
  size(800, 600);        // window size
String portName = "/dev/ttyACM1";
myPort = new Serial(this, portName, 9600);
background(#4C0F1C);
}

void serialEvent (Serial myPort) {
  // get the byte:
  int inByte = myPort.read()*6;
  // print it:
  println(inByte);
  if (inByte==0){
    yPos=0;
  }
  yPos = height - inByte;
}

void draw () {
   // draw the line in a pretty color:
  stroke(#DAC0A7);
  line(xPos, height, xPos, yPos);
  // at the edge of the screen, go back to the beginning:
  if (xPos >= width) {
    xPos = 0;
    // clear the screen by resetting the background:
    background(#4C0F1C);
  } else {
    // increment the horizontal position for the next reading:
    xPos++;
  }
}
```


###Python (For SMS):
```
import sys
from phant import Phant
from twilio.rest import TwilioRestClient
from time import sleep
 
# Your Account Sid and Auth Token from twilio.com/user/account
account_sid = "AC70e3378bc320d80f6d7de6f8c621cc91"
auth_token  = "af1003842796fd42560d7dae9565ed88"
client = TwilioRestClient(account_sid, auth_token)

p = Phant(public_key='q5JMKnDJKXCMnjbYr0lG', fields=['temp'], private_key='BVBdMRDBMAt60G1J8WV5')

while(True):
    data = p.get()
    print("Latest Loudness Value is: {}".format(data[0]['temp']))
    if float(data[0]['temp']) > 0:
        message = client.messages.create(body="HCL Collection Area is making noise, please take action",
            to="+919650318721",    # Replace with your phone number
            from_="+12018905183") # Replace with your Twilio number
        print (message.sid)
    sleep(15)

```


###Web View(For Graph plotting using Google Charts)

```
<!DOCTYPE html>
<html>
  <head>
    <!-- EXTERNAL LIBS-->
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.1/jquery.min.js"></script>
    <script src="https://www.google.com/jsapi"></script>

    <!-- EXAMPLE SCRIPT -->
    <script>

      // onload callback
      function drawChart() {

        var public_key = 'q5JMKnDJKXCMnjbYr0lG';

        // JSONP request
        var jsonData = $.ajax({
          url: 'https://data.sparkfun.com/output/' + public_key + '.json',
          data: {page: 1},
          dataType: 'jsonp',
        }).done(function (results) {

          var data = new google.visualization.DataTable();
          data.addColumn('datetime', 'Time');
          data.addColumn('number', 'Temp');

          $.each(results, function (i, row) {
            data.addRow([
              (new Date(row.timestamp)),
              parseFloat(row.temp)            ]);
          });

          var chart = new google.visualization.LineChart($('#chart').get(0));

          chart.draw(data, {
            title: 'SNU Library Noise Level Monitor'
          });

        });

      }

      // load chart lib
      google.load('visualization', '1', {
        packages: ['corechart']
      });

      // call drawChart once google charts is loaded
      google.setOnLoadCallback(drawChart);

    </script>

  </head>
  <body>
    <div id="chart" style="width: 100%;"></div>
  </body>
</html>

```


