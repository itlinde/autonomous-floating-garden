/* ======================================== 
#### NOTES:
- make sure laptop is connected to hot spot before running!

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
String soilState = "wet"; // "wet": moisture levels over threshold
                          // "dry": moisture levels under threshold

// output variables for GPIO pins
const int pumpOutput = 25;
const int sensorInput = 32;

int moistureReading = 0;
float moisturePercent = 0;
int threshold = 429;

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

  // saturation level calcs
  moistureReading = analogRead(sensorInput);
  Serial.print("Moisture sensor reading: ");
  Serial.println(moistureReading);
  if (moistureReading > threshold) { // higher reading = drier soil
    soilState = "dry";
  } else {
    soilState = "wet";
  }
  // moisture percent:
  moisturePercent = 100 * (1 - (moistureReading - 30) / (800 - 30));

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
              Serial.println("pump on");
              pumpState = "on";
              digitalWrite(pumpOutput, HIGH);
            } else if (header.indexOf("GET /25/off") >= 0) {
              Serial.println("pump off");
              pumpState = "off";
              digitalWrite(pumpOutput, LOW);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html>");
            client.println("<html lang=\"en\">");
            client.println("<head>");
            client.println("  <meta charset=\"UTF-8\">");
            client.println("  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
            client.println("  <title>My Floating Garden</title>");
            client.println("  <link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">");
            client.println("  <link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin>");
            client.println("  <link href=\"https://fonts.googleapis.com/css2?family=Inclusive+Sans:ital,wght@0,300..700;1,300..700&family=Jua&family=Varela+Round&display=swap\" rel=\"stylesheet\">");
            client.println("  <style>");
            client.println("    :root {");
            client.println("      --green1: #60693D;");
            client.println("      --dark-brown: #69584C;");
            client.println("      --tan: #C7B39A;");
            client.println("      --off-white: #ECEBE8;");
            client.println("      --beige: #D2C9BF;");
            client.println("    }");
            client.println("    .varela-round-regular {");
            client.println("      font-family: 'Varela Round', sans-serif;");
            client.println("      font-weight: 400;");
            client.println("      font-style: normal;");
            client.println("    }");
            client.println("    .inclusive-sans {");
            client.println("      font-family: 'Inclusive Sans', sans-serif;");
            client.println("      font-optical-sizing: auto;");
            client.println("      font-weight: 400;");
            client.println("      font-style: normal;");
            client.println("    }");
            client.println("    .jua-regular {");
            client.println("      font-family: 'Jua', sans-serif;");
            client.println("      font-weight: 400;");
            client.println("      font-style: normal;");
            client.println("    }");
            client.println("    html, body {");
            client.println("      background-color: var(--off-white);");
            client.println("      margin: 0;");
            client.println("      padding: 0;");
            client.println("      overflow-x: hidden;");
            client.println("      width: 100%;");
            client.println("      text-align: center;");
            client.println("      display: flex;");
            client.println("      flex-direction: column;");
            client.println("      justify-content: center;");
            client.println("    }");
            client.println("    .main {");
            client.println("      display: flex;");
            client.println("      flex-direction: column;");
            client.println("      justify-content: center;");
            client.println("    }");
            client.println("    .title {");
            client.println("      font-family: 'Jua', sans-serif;");
            client.println("      color: var(--green1);");
            client.println("      margin: 0;");
            client.println("      margin-top: 2rem;");
            client.println("      font-size: 2rem;");
            client.println("    }");
            client.println("    .planter-info {");
            client.println("      display: flex;");
            client.println("      margin-top: 30px;");
            client.println("      text-align: left;");
            client.println("      gap: 70px;");
            client.println("    }");
            client.println("    .planter-title {");
            client.println("      font-family: 'Jua', sans-serif;");
            client.println("      color: var(--dark-brown);");
            client.println("      font-size: 1.7rem;");
            client.println("      margin: 0;");
            client.println("    }");
            client.println("    p {");
            client.println("      font-family: 'Inclusive Sans', sans-serif;");
            client.println("      margin: 0;");
            client.println("      padding: 2px;");
            client.println("    }");
            client.println("    .garden-preview {");
            client.println("      background-color: var(--tan);");
            client.println("      height: 15rem;");
            client.println("      width: 15rem;");
            client.println("      margin: 2rem;");
            client.println("      columns: 2;");
            client.println("      gap: 16px;");
            client.println("      padding: 16px;");
            client.println("      border-radius: 20px;");
            client.println("      place-self: center;");
            client.println("    }");
            client.println("    .garden-info {");
            client.println("      place-self: center;");
            client.println("    }");
            client.println("    .planter-box {");
            client.println("      background-color: var(--beige);");
            client.println("      height: inherit;");
            client.println("      border-radius: 10px;");
            client.println("    }");
            client.println("    .water-button {");
            client.println("      border-radius: calc(infinity * 1px);");
            client.println("      width: 70px;");
            client.println("      height: 70px;");
            client.println("      border-style: none;");
            client.println("      background: var(--beige);");
            client.println("    }");
            client.println("    .water-button:active {");
            client.println("      border-radius: calc(infinity * 1px);");
            client.println("      width: 70px;");
            client.println("      height: 70px;");
            client.println("      border-style: none;");
            client.println("      background: var(--tan);");
            client.println("    }");
            client.println("    .water-button-disabled {");
            client.println("      border-radius: calc(infinity * 1px);");
            client.println("      width: 70px;");
            client.println("      height: 70px;");
            client.println("      border-style: none;");
            client.println("      background: #b8b8b8;");
            client.println("    }");
            client.println("  </style>");
            client.println("</head>");
            client.println("<body>");
            client.println("  <section class=\"header\">");
            client.println("    <h1 class=\"title\">My Floating Garden</h1>");
            client.println("    <div>");
            client.println("      <!-- Weather Icon goes here -->");
            client.println("      <p>Vancouver, BC</p>");
            client.println("    </div>");
            client.println("  </section>");
            client.println("  <section class=\"main\">");
            client.println("    <div class=\"garden-preview\">");
            client.println("      <div id=\"planter-box-1\" class=\"planter-box\"></div>");
            client.println("      <div id=\"planter-box-2\" class=\"planter-box\"></div>");
            client.println("    </div>");
            client.println("    <div class=\"garden-info\">");
            client.println("      <div id=\"planter-info-1\" class=\"planter-info\">");
            client.println("        <div>");
            client.println("          <h3 class=\"planter-title\">Sage Planter</h3>");
            client.println("          <p>last watered: 26/03/2025</p>");
            client.println("          <p>saturation: " + String(moisturePercent) + "%</p>");
            client.println("        </div>");
            if (pumpState == "off") { // if pump is inactive
              client.println("        <button class=\"water-button\">");
              client.println("          <p>water!</p>");
              client.println("        </button>");
            } else {                  // if pump is active
              client.println("        <button class=\"water-button-disabled\">");
              client.println("          <p>watering</p>");
              client.println("        </button>");
            }
            client.println("          <p>water!</p>");
            client.println("        </button>");
            client.println("      </div>");
            client.println("      <div id=\"planter-info-2\" class=\"planter-info\">");
            client.println("        <div>");
            client.println("          <h3 class=\"planter-title\">Planter 2</h3>");
            client.println("          <p>last watered: 26/03/2025</p>");
            client.println("          <p>saturation: " + String(moisturePercent) + "%</p>");
            client.println("        </div>");
            client.println("        <button class=\"water-button\">");
            client.println("          <p>water!</p>");
            client.println("        </button>");
            client.println("      </div>");
            client.println("    </div>");
            client.println("  </section>");
            client.println("  <section class=\"footer\">");
            client.println("  </section>");
            client.println("</body>");
            client.println("</html>");
            
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
