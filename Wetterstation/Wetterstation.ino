#include <SPI.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
//#include <DHT.h>
//#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


//Oled
#define OLED_RESET LED_BUILTIN  //4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Replace with your network credentials
const char* ssid = "CoworkingHN";
const char* password = "ilovecowohn";

//Bme280
Adafruit_BME280 bme;
char temperatureString[6];
char humidityString[6];
char pressureString[7];

//Webserver
ESP8266WebServer server(80);   //instantiate server at port 80 (http port)

//Wetterdaten
double Bodenfeuchtigkeit;
double Luftfeuchtigkeit;
double Temperatur;
String Regen = "";
double Bodenfeuchtigkeitweb;
double Luftfeuchtigkeitweb;
double Luftdruck;
double Luftdruckweb;
double Temperaturweb;
String Regenweb = "";

//Webpage
String page = "";

//DHT-sensor
//#define DHTPIN            13         // Pin which is connected to the DHT sensor.
// Uncomment the type of sensor in use:
//#define DHTTYPE           DHT11     // DHT 11 
//#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)
//DHT_Unified dht(DHTPIN, DHTTYPE);
//uint32_t delayMS;

//Rain-sensor
const byte interruptPin = 12; //GPIO
volatile boolean checkInterrupt = false;
int numberOfInterrupts = 0;

//Soil-sensor
unsigned long debounceTime = 1000; //time between measurements
unsigned long lastDebounce = 0;
int sense_Pin = 0; // sensor input at Analog pin A0
float value = 0; //soil measurement

void setup() {
  //Oled Display
  bme.begin();
  Wire.begin(16, 5);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  //Open serial port for serial monitor
  Serial.begin(115200);

  //Rainsensor
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
  delay(2000);

  //Connect to Wifi
  WiFi.begin(ssid, password); //begin WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //Webserver Html
  server.on("/", []() {
    page = "<style>h1 { text-align: center; border-bottom: 6px solid red; background-color: lightgrey;}h2 { margin-left: 37.5%; width: 25%; border: 2px solid black; text-align: center;}</style><h1>Wetterstation </h1><h2>Regen: " + String(Regenweb) + "</h2><h2>Temperatur: " + String(Temperaturweb) + "C </h2><h2>Luftfeuchtigkeit :" + String(Luftfeuchtigkeitweb) + "% </h2><h2>Bodenfeuchtigkeit :" + String(Bodenfeuchtigkeitweb) + "% </h2><script>window.addEventListener('load', function(){window.setInterval(function(){location.reload(true);}, 5000);});</script>";
    server.send(200, "text/html", page);
  });

  //Webserver json
  server.on("/json", []() {
    server.send(200, "application/json", createsensor());
  });
  server.begin();
  Serial.println("Web server started!");


/*dht initzialisation  dht.begin();

  // Print temperature sensor details.*/
  /*sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");
  Serial.println("------------------------------------");

  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");
  Serial.println("------------------------------------");

  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;*/
}

void soil() {
  Serial.print("Bodenfeuchtigkeit : ");
  value = analogRead(sense_Pin);
  value = map(value, 0, 1023, 100, 0);
  Serial.print(value);
  Serial.println("%");
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 28);
  display.print(value);
  display.print("%");
}

void handleInterrupt() {
  checkInterrupt = true;
}

void rain() {
  if (checkInterrupt == true && ( (millis() - lastDebounce)  > debounceTime )) {

    lastDebounce = millis();
    checkInterrupt = false;
    numberOfInterrupts++;
    Regen = "Es regnet!";
    Serial.println("Rain detected ");
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(20, 28);
    display.print("Rain detected");
  }
  else {
    checkInterrupt = false;
    Regen = "Es regnet nicht!";
  }
}
String createsensor() {
  return "{ \"Temperatur\":" + String(Temperaturweb) + ", \"Bodenfeuchtigkeit\": " + String(Bodenfeuchtigkeitweb) + ", \"Humidity\":" + String(Luftfeuchtigkeitweb) + ", \"Rain\":\"" + String(Regenweb) + "\"}";;
}

/*void temp() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Serial.print("Temperature: ");
    Serial.print(event.temperature);
    Temperatur = event.temperature;
    Serial.println(" °C");
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(20, 28);
    display.print(event.temperature);
    display.print(" °C");
  }

}
void humidity() {
  sensors_event_t event;
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    Serial.print("Humidity: ");
    Serial.print(event.relative_humidity);
    Luftfeuchtigkeit = event.relative_humidity;
    Serial.println("%");
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(20, 28);
    display.print(event.relative_humidity);
    display.print("%");
  }
}
*/
void Temperature() {
  Temperatur = bme.readTemperature();
  Serial.println("Temperatur: ");
  Serial.println(Temperatur);
  delay(100);
}
void Luftfeuchtigkeite() {
  Luftfeuchtigkeit = bme.readHumidity();
  Serial.println("Luftfeuchtigkeit: ");
  Serial.println(Luftfeuchtigkeit);
  delay(100);
}
void Luftdrucke() {
  Luftdruck = bme.readPressure()/100.0F;
  Serial.println("Temperatur: ");
  Serial.println(Luftdruck);
  delay(100);
}
void loop(void) {
  rain();
  Regenweb = Regen;
  server.handleClient();
  delay(1000);
  soil();
  Bodenfeuchtigkeit = value;
  Bodenfeuchtigkeitweb = Bodenfeuchtigkeit;
  server.handleClient();
  delay(1000);
  Temperature();
  Temperaturweb = Temperatur;
  server.handleClient();
  delay(1000);
  Luftfeuchtigkeite();
  Luftfeuchtigkeitweb = Luftfeuchtigkeit;
  server.handleClient();
  delay(1000);
  Luftdruckweb = Luftdruck;
  server.handleClient();
  delay(1000);
}
