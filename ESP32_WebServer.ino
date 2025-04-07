/* ======================================== 
#### TROUBLESHOOTING:
- make sure laptop is connected to ubcvisitor before running!
- if upload error 2, make sure upload speed = 112500

#### CORE FEATURES: 
- Wifi connection 
- Moisture sensor integration 
- web server
- saving & exporting moisture data
- Convert ms to human-readable date/time 

======================================== */

// Load Wi-Fi library
#include <WiFi.h>
#include <time.h>

// network credentials
const char* ssid = "############"; // network name
const char* password = "############"; // network password 

// Time conversion -- NTP Server details
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -28800; // adjust for your timezone (example: PST is -8 hours)
const int   daylightOffset_sec = 3600;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// output states
String pumpState = "off";
String soilState = "wet"; // "wet": moisture levels over threshold
                          // "dry": moisture levels under threshold
String isExporting = "false"; // "true" sends user to export page

// GPIO pins
const int pumpOutput = 25;
const int sensorInput = 32;

// moisture variables 
float moistureReading = 0.00;
float moisturePercent = 0.00;
int threshold = 2755; // og value was 2457, 2755 was the moisture reading on dry soil
                      // wet soil goes down to ~1300

// Time variables for WIFI
unsigned long currentTimeoutTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000; // Define WIFI timeout time in milliseconds (example: 2000ms = 2s)

// Time variables for RECORDING 
unsigned long currentReadingTime = millis();
unsigned long lastReadingTime = 0;
const unsigned long readingInterval = 60000; // 15 minutes in milliseconds 

// setting up the data export array
#define MAX_READINGS 1440

struct Reading {
  time_t timestamp;
  float moistureReading;
};

Reading readings[MAX_READINGS]; // creates an array "readings" of type Reading
int readingIndex = 0; // start index for adding readings 

void setup() {
  Serial.begin(115200);
  delay(200);
  pinMode(pumpOutput, OUTPUT);
  pinMode(sensorInput, INPUT);
  pinMode(2, OUTPUT);

  WiFi.mode(WIFI_STA);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); // (if connecting to a password protected wifi, use .begin(ssid, password))
  while (WiFi.status() != WL_CONNECTED) { // repeatedly print "." until the server is connected
    delay(500);
    Serial.print(".");
  }

  // Sync time with NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("\nWaiting for time synchronization...");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTime synchronized.");


  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin(); 
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients
  
  readMoistureSensors(); 
  
  if (client) {                             // If a new client connects,
    currentTimeoutTime = millis();
    previousTime = currentTimeoutTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTimeoutTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTimeoutTime = millis();
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
              Serial.println("turning pump on...");
              pumpState = "on";
              waterPlant();
            } else if (header.indexOf("GET /export") >= 0) {
              Serial.println("exporting data...");
              isExporting = "true";

              Serial.print("isExporting: ");
              Serial.println(isExporting);  

            } else if (header.indexOf("GET /") >= 0) {
              Serial.println("displaying home page...");
              isExporting = "false";
            };

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
            // css styling 
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

            // if not trying to export... 
            if (isExporting == "false") {
              // show the main home page
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
              client.println("      <div id=\"planter-box-1\" class=\"planter-box\"><p>Sage</p></div>");
              client.println("      <div id=\"planter-box-2\" class=\"planter-box\"><p>Basil</p></div>");
              client.println("    </div>");
              client.println("    <div class=\"garden-info\">");
              client.println("      <div id=\"planter-info-1\" class=\"planter-info\">");
              client.println("        <div>");
              client.println("          <h3 class=\"planter-title\">Plant Status</h3>");
              client.println("          <p>last watered: 26/03/2025</p>");
              client.println("          <p>saturation: " + String(moisturePercent) + "%</p>");
              client.println("        </div>");
              if (pumpState == "off") { // if pump is inactive
                client.println("        <a href=\"/25/on\">");
                client.println("        <button class=\"water-button\">");
                client.println("          <p>water!</p>");
                client.println("        </button></a>");
              } else {                  // if pump is active (button is NOT linked)
                client.println("        <button class=\"water-button-disabled\">");
                client.println("          <p>watering</p>");
                client.println("        </button><");
              }
              client.println("      </div>");
              client.println("      <p> </p>");
              client.println("      <p> </p>");
              client.println("      <p> </p>");
              client.println("      <p> </p>");
              client.println("      <p> </p>");
              client.println("      <a href=\"/export\">");
              client.println("        <button class=\"water-button\">");
              client.println("        <p>EXPORT DATA</p>");
              client.println("        </button>");
              client.println("      </a>");
              client.println("    </div>");
              // for (int i = 0; i < readingIndex; i++) {
              //   client.println("<p>" + formatTimestamp(readings[i].timestamp) + "   |   " + String(readings[i].moistureReading) + "</p>");
              //   client.println("<p> </p>");
              // };
              client.println("  </section>");
              client.println("  <section class=\"footer\">");
              client.println("  </section>");
              client.println("</body>");
            
            // if trying to export...
            } else { 
              // print moisture data since start-up
              client.println("<body>");
              client.println("<p>Your data:</p>");
              
              // for loop to print data
              for (int i = 0; i < readingIndex; i++) {
                client.println("<p>" + formatTimestamp(readings[i].timestamp) + "   |   " + String(readings[i].moistureReading) + "</p>");
                client.println("<p> </p>");
              };
              client.println("</body>");
            }
              client.println("</html>");
            
            // The HTTP response ends with another blank line
            client.println();
            break;
          } else { // if we get a newline, then clear currentLine
            currentLine = "";
          }         
        } else if (c != '\r') {  // if you got anything else but a carriage return character...
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    header = ""; // Clear the header variable
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  delay(500);
}

// ## waterPlant() :
// - turns on/off the pump
// - turns on/off the blue LED on the ESP32 board
void waterPlant() {
  digitalWrite(2, HIGH); // blue LED thats on the ESP32 board
  // turn on pump for 5s, then turn it off
  analogWrite(pumpOutput, 200);
  delay(5000);
  analogWrite(pumpOutput, 0);
  pumpState = "off";
  digitalWrite(2, LOW); // blue LED thats on the ESP32 board
}

// ## readMoistureSensors() :
// - retrieves moisture reading
// - sets soilState to dry/wet based on moisture reading and threshold
// - computes the soil's moisture percent
// - saves moistureReading & timestamp to data array 
void readMoistureSensors() {
  // saturation level calculations
  moistureReading = analogRead(sensorInput);
  if (moistureReading > threshold) { // higher reading = drier soil
    soilState = "dry";
    waterPlant();
  } else {
    soilState = "wet";
  }
  // compute moisture percent
  moisturePercent = 100 * float((1 - float((moistureReading - 0) / (4095 - 0))));

  currentReadingTime = millis();
  // save reading if it's been a minute
  if (currentReadingTime - lastReadingTime >= readingInterval) {
    lastReadingTime = currentReadingTime;

    // get the current real time
    time_t now;
    time(&now);

    // save new reading & timestamp to array
    if (readingIndex < MAX_READINGS) {
      readings[readingIndex].timestamp = now;
      readings[readingIndex].moistureReading = moistureReading;
      readingIndex++;
    } else {
      readingIndex = 0; // if MAX_READING is reached, go back to 0
    }
  }
}

// formats ms to a date-time format
String formatTimestamp(time_t rawTime) {
  struct tm timeinfo;
  localtime_r(&rawTime, &timeinfo);
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}
