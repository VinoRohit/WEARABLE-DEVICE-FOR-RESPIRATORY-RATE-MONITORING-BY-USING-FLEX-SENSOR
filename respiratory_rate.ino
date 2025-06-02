#include <WiFi.h>
#include <WebServer.h>

// Pin definition
const int flexPin = 34;  // For ESP32, A0 maps to GPIO34 or another appropriate ADC pin
int flexValue;           // Variable to store flex sensor readings
int threshold = 1900;     // Adjusted threshold for breath detection

// Time variables
unsigned long currentTime = 0;
unsigned long previousTime = 0;
unsigned long breathTime = 0; // Time when the last breath was detected
int breathCount = 0;          // Counter for the number of breaths
int respiratoryRate = 0;      // Respiratory rate in breaths per minute

// Constants
const int timeWindow = 60000;  // One minute time window for respiratory rate calculation
const int debounceTime = 1000; // Minimum time between breaths (1000ms = 1 second)

// Variables for smoothing the sensor readings
const int numReadings = 10;    // Number of readings for smoothing (moving average)
int readings[numReadings];     // Array to hold readings
int readIndex = 0;             // Current reading index
int total = 0;                 // Total of all readings
int average = 0;               // The average reading

// Wi-Fi Credentials
const char* ssid = "VinoRohit";         // Replace with your Wi-Fi SSID
const char* password = "123456789"; // Replace with your Wi-Fi password

// Web Server
WebServer server(80);

// Function to connect to Wi-Fi
void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Attempting to connect... Status: ");
    Serial.println(WiFi.status()); // Prints status code
  }
  Serial.println("Wi-Fi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Function to handle the root endpoint
void handleRoot() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><title>Respiratory Monitor</title></head>";
  html += "<body>";
  html += "<h1>Respiratory Monitor</h1>";
  html += "<p>Flex Sensor Avg Value: " + String(average) + "</p>";
  html += "<p>Respiratory Rate (bpm): " + String(respiratoryRate) + "</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);          // Initialize serial communication
  pinMode(flexPin, INPUT);       // Set the flex sensor pin as input

  // Initialize all readings to 0
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  // Connect to Wi-Fi
  connectToWiFi();

  // Define the root endpoint
  server.on("/", handleRoot);

  // Start the server
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  currentTime = millis();  // Record the current time

  // Read the flex sensor value
  flexValue = analogRead(flexPin);

  // Update the moving average
  total = total - readings[readIndex];        // Subtract the last reading
  readings[readIndex] = flexValue;            // Read the new sensor value
  total = total + readings[readIndex];        // Add the new reading to the total
  readIndex = (readIndex + 1) % numReadings;  // Advance to the next position in the array

  average = total / numReadings;  // Calculate the moving average

  // Print the average sensor value for debugging
  Serial.print("Flex Sensor Avg Value: ");
  Serial.println(average);

  // Breath detection: Check if the sensor value crosses the threshold
  if (average > threshold) {
    if (currentTime - breathTime > debounceTime) { // Debounce to avoid multiple counts for one breath
      breathCount++;
      breathTime = currentTime;  // Update last breath time

      // Print breath detection for debugging
      Serial.println("Breath detected.");
    }
  }

  // Calculate and display respiratory rate every minute
  if (currentTime - previousTime >= timeWindow) {
    respiratoryRate = breathCount;  // Respiratory rate is the number of breaths in the last minute

    // Print respiratory rate
    Serial.print("Respiratory Rate (bpm): ");
    Serial.println(respiratoryRate);

    // Reset for the next minute window
    breathCount = 0;
    previousTime = currentTime;
  }

  // Real-time display of respiratory rate (updated every loop)
  Serial.print("Current Respiratory Rate (bpm): ");
  Serial.println(respiratoryRate);

  // Handle web server requests
  server.handleClient();

  delay(100);  // Delay for stability
}
