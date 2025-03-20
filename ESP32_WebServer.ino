/* ======================================== 
#### NOTES:
- make sure laptop is connected to ubcvisitor before running!

#### TROUBLESHOOTING: 
- if upload error 2, make sure upload speed = 112500

======================================== */

// Load Wi-Fi library
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "isabellas phone"; // network name
const char* password = "hohoheeha"; // network password 

// const char* ssid = "SHAW-7C2E"; // network name
// const char* password = "collar8216camel"; // network password 


// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// auxiliar variables to store current output states
String pumpState = "off";

// output variables for GPIO pins
const int output25 = 25;
const int output32 = 32;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  delay(200);

  WiFi.mode(WIFI_STA);
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); // (if connecting to a password protected wifi, use .begin(ssid, password))
  while (WiFi.status() != WL_CONNECTED) { // repeatedly print "." until the server is connected
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin(); 
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {   
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // HTTPS routes
            if (header.indexOf("GET /25/on") >= 0) {
              Serial.println("GPIO 25 on");
              pumpState = "on";
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, LOW);
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              output27State = "on";
              digitalWrite(output27, HIGH);
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              output27State = "off";
              digitalWrite(output27, LOW);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("</head>"); 
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server TESTTTTT </h1>");  
            
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }         
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
    delay(500);
  } else {
    Serial.println("Waiting for connection...");
    delay(500);
  }
}