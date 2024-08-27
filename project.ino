#include <WiFi.h>  // Include the Wi-Fi library
#include <WebServer.h>  // Include the WebServer library

const char* ssid = "MITWPU-EXAM";     // Your WiFi SSID
const char* password = "january@2023"; // Your WiFi password

const int pressureInput1 = A0; // Analog input pin for the first pressure transducer
const int pressureInput2 = A3; // Analog input pin for the second pressure transducer

const float pressureZero = 409.5; // Analog reading of pressure transducers at 0 bar
const float pressureMax = 3685.5; // Analog reading of pressure transducers at 16 bar

const int sensorreadDelay = 500; // Constant integer to set the sensor read delay in milliseconds

float pressureValue1 = 0; // Variable to store the value coming from the first pressure transducer
float pressureValue2 = 0; // Variable to store the value coming from the second pressure transducer
float pressureDifference = 0; // Variable to store the difference between pressure values

const float densityWater = 1000.0; // Density of water in kg/m^3
const float gravity = 9.81; // Acceleration due to gravity in m/s^2

WebServer server(80);  // Create a web server object that listens for HTTP requests on port 80

void setupWiFi() {
  WiFi.begin(ssid, password);  // Connect to the WiFi network

  while (WiFi.status() != WL_CONNECTED) { // Wait for connection
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());  // Print the IP address
}

void handleRoot() {
  if (server.args() > 0) { // If there are arguments in the URL
    float length;
    float diameter;

    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == "length") {
        length = server.arg(i).toFloat(); // Convert length value to float
      }
      if (server.argName(i) == "diameter") {
        diameter = server.arg(i).toFloat(); // Convert diameter value to float
      }
    }

    pressureDifference = pressureValue2 - pressureValue1; // Calculate pressure difference
    float pressureLoss = calculatePressureLoss(length, diameter); // Calculate pressure loss
    float criticalLength = calculateCriticalLength(diameter); // Calculate critical length
    String html = generateHTML(pressureLoss, criticalLength); // Generate HTML response with pressure loss and critical length
    server.send(200, "text/html", html); // Send HTTP response with HTML page
  } else {
    String html = generateHTML(0, 0); // Generate HTML response with initial values
    server.send(200, "text/html", html); // Send HTTP response with HTML page
  }
}

String generateHTML(float pressureLoss, float criticalLength) {
  String html = "<!DOCTYPE html><html><head><title>Pressure Sensor Data</title></head><body>";
  html += "<h1>Pressure Sensor Data</h1>";
  html += "<p>Pressure Sensor 1 Value: ";
  html += pressureValue1;
  html += " bar</p>";
  html += "<p>Pressure Sensor 2 Value: ";
  html += pressureValue2;
  html += " bar</p>";
  html += "<p>Pressure Difference: ";
  html += pressureDifference;
  html += " bar</p>";
  html += "<p>Pressure Loss due to Friction: ";
  html += pressureLoss;
  html += " Pa</p>"; // Pressure loss in Pascal
  html += "<p>Critical Length (Flow reaches zero): ";
  html += criticalLength;
  html += " meters</p>"; 
  html += "<h2>Select Pipe Length and Diameter:</h2>";
  html += "<form method='get'>";
  html += "<label for='length'>Select Length:</label>";
  html += "<select name='length'>";
  html += "<option value='100'>100 meters</option>";
  html += "<option value='200'>200 meters</option>";
  html += "<option value='300'>300 meters</option>";
  html += "</select><br><br>";
  html += "<label for='diameter'>Select Diameter:</label>";
  html += "<select name='diameter'>";
  html += "<option value='0.0254'>1 inch</option>";  // 1 inch to meters
  html += "<option value='0.0127'>1/2 inch</option>";  // 1/2 inch to meters
  html += "<option value='0.00635'>1/4 inch</option>";  // 1/4 inch to meters
  html += "</select><br><br>";
  html += "<input type='submit' value='Calculate Pressure Loss'>";
  html += "</form>";
  html += "</body></html>";

  return html;
}

void setup() {
  Serial.begin(9600);
  pinMode(pressureInput1, INPUT);
  pinMode(pressureInput2, INPUT);

  setupWiFi();  // Connect to WiFi network

  server.on("/", HTTP_GET, handleRoot); // Associate the handler function to the path
  server.begin(); // Start the server
}

void loop() {
  pressureValue1 = analogRead(pressureInput1); // Read value from first input pin
  pressureValue1 = ((pressureValue1 - pressureZero) / ((pressureMax - pressureZero)*10)) * 16; // Conversion equation to convert analog reading to bar

  pressureValue2 = analogRead(pressureInput2); // Read value from second input pin
  pressureValue2 = ((pressureValue2 - pressureZero) / ((pressureMax - pressureZero)*10)) * 16; // Conversion equation to convert analog reading to bar

  server.handleClient(); // Handle client requests
  delay(sensorreadDelay); // Delay in milliseconds between read values
}

float calculatePressureLoss(float length, float diameter) {
  // Darcy-Weisbach equation for pressure loss
  return 0.079 * (length / diameter) * pow((densityWater * gravity * pressureDifference * diameter) / pow((densityWater / 2), 2), 0.25);
}

float calculateCriticalLength(float diameter) {
  // Calculate the critical length where pressure loss equals pressure difference
  return pressureDifference * diameter * pow((densityWater / 2), 2) / (0.079 * densityWater * gravity);
}
