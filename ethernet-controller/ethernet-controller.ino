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

byte outs[6];                                      //array for digital outputs
int ins[6];                                        //array for analog inputs
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; //might need to change if there's a conflict with another device on network
IPAddress ip(192, 168, 0, 7);                      //fallback if DHCP doesn't work

String fname = ""; //for requested filename
String parms = ""; //requested file parameters (ie after ?)
String a = "";     //to hold request
long timecount;    //time counter for cycle measuring

EthernetServer server(80); //standard port for http

void setup()
{

  Serial.begin(115200); //fast so we don't slow down server tasks

  for (int i = 0; i < 6; i++)
  {
    pinMode(2 + i, OUTPUT);
  }

  delay(500);

  if (Ethernet.begin(mac) == 0)
  {
    //try to start with DHCP
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip); //try with fixed IP
  }
  else
  {
    Serial.println("DHCP configure OK");
  }
  Serial.print("Server IP: "); //print server IP
  Serial.println(Ethernet.localIP());

  server.begin(); //start the server
}

void loop()
{
  // listen for incoming clients
  timecount = millis();
  EthernetClient client = server.available();
  a = "";
  if (client)
  {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        a.concat(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank)
        {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();

          //find target file in GET request- from 5th character to before 1st space after 5th character
          fname = a.substring(5, a.indexOf(" ", 5));

          //parms= any chars after ? in url
          parms = fname.substring(fname.indexOf("?") + 1);
          if (fname.indexOf("?") == -1)
          {
            parms = "";
          }

          //adjust fname to not include parms
          fname = fname.substring(0, fname.indexOf("?"));

          Serial.print(fname); //dump requested info to serial debug
          Serial.print(" ");
          Serial.println(parms);

          if (parms == "someString")
          {
            // put some code here, if you want.
            // you can add multiple if statements to check for all sorts of params,
          }
          else
          {
            // we will parse it as a string, such as "21"
            int value = parms.toInt(); //get as int
            int pin = value / 10;      //first number;
            int mode = value % 10;     // second number

            outs[pin - 2] = (mode & 0x01); //true or false

            for (int i = 0; i < 6; i++)
            {
              digitalWrite(i + 2, outs[i]);
            } //set digital outputs
          }

          //send a response
          sendStatus(client);

          break;
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
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

void sendStatus(EthernetClient &client)
{
  client.println("<h1> Arduino Ethernet Controller </h1>");

  client.println("<h3>"); // in h3 tag
  // display state of pins, and offer control
  for (int i = 0; i < 6; i++)
  {
    // ---------------------------------------------
    client.print("Digital ");
    client.print(i + 2);
    //inline if statement, display " is ON" if outs[i] is true, otherwise " is OFF"
    client.print(outs[i] ? " is ON " : " is OFF ");
    // ---------------------------------------------

    // provide a link to turn on/off again.
    client.print("<a href=\"");
    client.print(fname);
    client.print("?");
    client.print(i + 2);
    client.print("0\">[OFF]</a>");

    client.print("<a href=\"");
    client.print(fname);
    client.print("?");
    client.print(i + 2);
    client.print("1\">[ON]</a><br>");
  }

  for (int i = 0; i < 6; i++)
  {
    ins[i] = analogRead(i);
  } //put the analog inputs in an array
  if (fname == "temp.htm")
  {
    for (int i = 0; i < 6; i++)
    { //convert analog ins to temps if temp is selected
      ins[i] = ins[i] / 10 - 24;
    }
  } //very rough conversion is +-2 degrees between 0-60
  for (int i = 0; i < 6; i++)
  { //show analog ins
    client.print("<br>Analog ");
    client.print(i);
    client.print(":  ");
    client.print(ins[i]);
    if (fname == "temp.htm")
    {
      client.print(" degrees");
    }
  }
  client.print("<br><a href=\"raw.htm\">Raw Data</a>"); //print some extra links at the end to change modes
  client.print("<br><a href=\"temp.htm\">Temperature Data</a>");
}