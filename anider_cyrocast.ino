#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

const char* ssid = "Realme X";
const char* password = "12345678";
#define MQ135_PIN A0
#define R0_AMMONIA 0.2 // R0 value for ammonia (in kOhm)
#define R0_CO2 10 // R0 value for carbon dioxide (in kOhm)
#define R0_NO2 6 // R0 value for nitrogen dioxide (in kOhm)

#define DHTPIN D4           // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11      // DHT 11

IPAddress local_IP(192, 168, 106, 240);
// Set your Gateway IP address
IPAddress gateway(192, 168, 106, 141);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8); // optional
IPAddress secondaryDNS(8, 8, 4, 4);

const int SOIL_MOISTURE_PIN = D1; 

DHT dht(DHTPIN, DHTTYPE);

ESP8266WebServer server(80);

void gasSensor() {
  int sensorValue = analogRead(MQ135_PIN);
  float rs = 10.0; // Resistance of sensor in clean air (in kOhm)
  float vC = 5.0; // Clean air voltage (in V)

  // Estimate ammonia concentration
  float ratioAmmonia = (1024.0 / sensorValue) - 1.0;
  float concentrationAmmonia = R0_AMMONIA / rs * ratioAmmonia * (vC / 5.0) * 4.4; // Conversion factor for ammonia

  // Estimate carbon dioxide concentration
  float ratioCO2 = (1024.0 / sensorValue) - 1.0;
  float concentrationCO2 = R0_CO2 / rs * ratioCO2 * (vC / 5.0) * 2.5; // Conversion factor for carbon dioxide

  // Estimate nitrogen dioxide concentration
  float ratioNO2 = (1024.0 / sensorValue) - 1.0;
  float concentrationNO2 = R0_NO2 / rs * ratioNO2 * (vC / 5.0) * 1.7; // Conversion factor for nitrogen dioxide

  // Create a JSON object
  String json = "{";
  json += "\"ammonia\": " + String(concentrationAmmonia, 2) + ",";
  json += "\"carbon_dioxide\": " + String(concentrationCO2, 2) + ",";
  json += "\"nitrogen_dioxide\": " + String(concentrationNO2, 2);
  json += "}";

  server.send(200, "application/json", json);
}

void dhtSensor() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    String json = "{\"error\": \"Failed to read from DHT sensor\"}";
    server.send(500, "application/json", json);
    return;
  }

  // Create a JSON object
  String json = "{";
  json += "\"temperature\": " + String(temperature, 2) + ",";
  json += "\"humidity\": " + String(humidity, 2);
  json += "}";

  server.send(200, "application/json", json);
}

void dataAll()
{
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  while(isnan(humidity) ==true && isnan(humidity) == true)
  {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
  }
  if (isnan(temperature) || isnan(humidity)) {
    String json = "{\"error\": \"Failed to read from DHT sensor\"}";
    server.send(500, "application/json", json);
    return;
  }
  int sensorValue = analogRead(MQ135_PIN);
  float rs = 10.0; // Resistance of sensor in clean air (in kOhm)
  float vC = 5.0; // Clean air voltage (in V)

  // Estimate ammonia concentration
  float ratioAmmonia = (1024.0 / sensorValue) - 1.0;
  float concentrationAmmonia = R0_AMMONIA / rs * ratioAmmonia * (vC / 5.0) * 4.4; // Conversion factor for ammonia

  // Estimate carbon dioxide concentration
  float ratioCO2 = (1024.0 / sensorValue) - 1.0;
  float concentrationCO2 = R0_CO2 / rs * ratioCO2 * (vC / 5.0) * 2.5; // Conversion factor for carbon dioxide

  // Estimate nitrogen dioxide concentration
  float ratioNO2 = (1024.0 / sensorValue) - 1.0;
  float concentrationNO2 = R0_NO2 / rs * ratioNO2 * (vC / 5.0) * 1.7; // Conversion factor for nitrogen dioxide

  // Create a JSON object
  String json = "{";
  json += "\"temp\": \"" + String(temperature, 2) + "\",";
  json += "\"humidity\": \"" + String(humidity, 2)+ "\",";
  json += "\"ammonia\": \"" + String(concentrationAmmonia, 2) + "\",";
  json += "\"carbon_dioxide\": \"" + String(concentrationCO2, 2) + "\",";
  json += "\"nitrogen_dioxide\": \"" + String(concentrationNO2, 2) + "\",";
  json += "\"weather\": \"Cloudy\"";
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  Serial.println(WiFi.localIP());

  server.on("/gas", gasSensor);
  server.on("/dht", dhtSensor);
  server.on("/data", dataAll);
  server.begin();
  Serial.println("HTTP server started");
}
void loop()
{
  server.handleClient();
  int value = analogRead(SOIL_MOISTURE_PIN);
  int moisturePercentage = map(value, 0, 1023, 0, 100); // Map the analog value to a percentage value

  if (moisturePercentage > 50) {
    WiFiClient client;
    HTTPClient http;

    String endpointURL = "http://192.168.106.250:5030/motor/on?delay=10000";

    http.begin(client, endpointURL);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.println("Error in HTTP request");
    }
    
    http.end();
    delay(10000);
  }
 
  Serial.print("MOISTURE: ");
  Serial.print(moisturePercentage);
  Serial.println("%");

  delay(1000);
}
