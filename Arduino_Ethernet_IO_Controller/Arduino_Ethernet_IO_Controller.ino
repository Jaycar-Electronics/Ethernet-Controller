/*
Basic ethernet shield+uno acting as a web-server
Takes commands via URL parameters and also feeds back info from analog inputs
eg go to temp.htm?21 to view temperatures and set digital 2 on
Sketch generates links so all features can be accessed by one click
Check serial monitor for server IP address
Much shamelessly borrowed from Ethernet server example
*/

#include <SPI.h>
#include <Ethernet.h>

byte outs[6];    //array for digital outputs
int ins[6];      //array for analog inputs
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  //might need to change if there's a conflict with another device on network
IPAddress ip(192,168,0,7);    //fallback if DHCP doesn't work

String fname="";    //for requested filename
String parms="";    //requested file parameters (ie after ?)
String a="";        //to hold request
long timecount;    //time counter for cycle measuring

EthernetServer server(80);    //standard port for http

void setup(){
  Serial.begin(115200);    //fast so we don't slow down server tasks
  for(int i=0;i<6;i++){pinMode(i+2,OUTPUT);}      //set digital pins as outputs
  delay(500);  //let everything stabilise
  if (Ethernet.begin(mac) == 0) {                    //try to start with DHCP
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac,ip);                          //try with fixed IP
  }else{
    Serial.println("DHCP configure OK");
  }
  Serial.print("Server IP:");                        //print server IP
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();
  server.begin();   //start the server
}

void loop()
{
  // listen for incoming clients
  timecount=millis();  
  EthernetClient client = server.available();
  a="";
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        a.concat(c); 
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();

        //find target file in GET request- from 5th character to before 1st space after 5th character
        fname=a.substring(5,a.indexOf(" ",5));

        //parms= any chars after ? in url
        parms=fname.substring(fname.indexOf("?")+1);
        if(fname.indexOf("?")==-1){parms="";}
          
        //adjust fname to not include parms
        fname=fname.substring(0,fname.indexOf("?"));

        Serial.print(fname);      //dump requested info to serial debug
        Serial.print(" ");
        Serial.println(parms);
        //check digital output selection (can't use switch case because it's a string)
        if(parms=="20"){outs[0]=0;}
        if(parms=="21"){outs[0]=1;}
        if(parms=="30"){outs[1]=0;}
        if(parms=="31"){outs[1]=1;}
        if(parms=="40"){outs[2]=0;}
        if(parms=="41"){outs[2]=1;}
        if(parms=="50"){outs[3]=0;}
        if(parms=="51"){outs[3]=1;}
        if(parms=="60"){outs[4]=0;}
        if(parms=="61"){outs[4]=1;}
        if(parms=="70"){outs[5]=0;}
        if(parms=="71"){outs[5]=1;}

        for(int i=0;i<6;i++){digitalWrite(i+2,outs[i]);}        //set digital outputs        
        client.println("<h1>ARDUINO IO CONTROL</h1><h3>");        //display heading
        for(int i=0;i<6;i++){                        //display output states and controls
          client.print("Digital ");
          client.print(i+2);
          if(outs[i]){client.print(" is ON   ");}else{client.print(" is OFF ");}
          client.print("<a href=\"");
          client.print(fname);
          client.print("?");
          client.print(i+2);
          client.print("0\">[OFF]</a>");
          client.print("<a href=\"");
          client.print(fname);
          client.print("?");
          client.print(i+2);
          client.print("1\">[ON]</a><br>");
        }

        for(int i=0;i<6;i++){ins[i]=analogRead(i);}      //put the analog inputs in an array
        if(fname=="temp.htm"){for(int i=0;i<6;i++){            //convert analog ins to temps if temp is selected
          ins[i]=ins[i]/10-24;}}                  //very rough conversion is +-2 degrees between 0-60
        for(int i=0;i<6;i++){                    //show analog ins
          client.print("<br>Analog ");
          client.print(i);
          client.print(":  ");
          client.print(ins[i]);
          if(fname=="temp.htm"){client.print(" degrees");}
        }
        client.print("<br><a href=\"raw.htm\">Raw Data</a>");        //print some extra links at the end to change modes
        client.print("<br><a href=\"temp.htm\">Temperature Data</a>");
        
        break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}
