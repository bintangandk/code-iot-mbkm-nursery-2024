///////////////////////COPYRIGHT INOVOKASI 2024////////////////
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define DHTPIN D7
#define PINMQ A0
#define PINRELAY D6
#define PINLEDMerah D1 //
#define PINLEDBiru D2
#define PINLEDHijau D5
#define LED_INTERNAL LED_BUILTIN

#define DHTTYPE 21
DHT dht(DHTPIN, DHTTYPE);

bool DHTCondition = false;
bool MQ135Condition = false;

float humidity;
float temperature;
float nilai_Amonia;

unsigned long previousMillis = 0; // Variabel untuk menyimpan waktu sebelumnya
const long interval = 60000;      // Interval 1 menit (60000 milidetik)

// HTTP client Config
// Replace with your network credentials
const char *ssid = "Kontrak 1";
const char *password = "bayardulu123";
const int id_device = 1;

// Server Url
String URL_temperature = "http://iqacs-chick.research-ai.my.id/public/temperature_data.php";
String URL_amonia = "http://iqacs-chick.research-ai.my.id/public/amonia_data.php";
String URL_humidity = "http://iqacs-chick.research-ai.my.id/public/humidity_data.php";

// REPLACE with your Domain name and URL path or IP address with path
// const char *serverName = "https://example.com/post-esp-data.php";

//////
void readDHT21()
{
  delay(1);
  // Get DHT Temperature
  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temperature = dht.readTemperature();

  // Check Sensor Condition
  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    DHTCondition = false;
    return;
  }
  else
  {
    DHTCondition = true;
  }

  Serial.println("humidity :" + String(humidity));
  Serial.println("temperature :" + String(temperature));
  delay(1);
}

void readMQ135()
{
  nilai_Amonia = analogRead(PINMQ);
  Serial.println("Nilai Amonia : " + String(nilai_Amonia));
  delay(1);
  if (nilai_Amonia < 150 || nilai_Amonia > 1000)
  {
    MQ135Condition = false;
  }
  else
  {
    MQ135Condition = true;
  }
}

void showSensorError()
{
  // All Sensor OK
  if (DHTCondition == true && MQ135Condition == true)
  {
    Serial.println("AllSensor OK");
  }
  // DHT ok, MQ135 broke
  if (DHTCondition == true && MQ135Condition == false)
  {
    for (int i = 0; i < 2; i++)
    {
      digitalWrite(PINLEDBiru, HIGH);
      digitalWrite(PINLEDMerah, HIGH);
      delay(200);
      digitalWrite(PINLEDBiru, LOW);
      digitalWrite(PINLEDMerah, LOW);
      delay(200);
    }
  }
  // DHT broke, MQ135 ok
  if (DHTCondition == false && MQ135Condition == true)
  {
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(PINLEDBiru, HIGH);
      digitalWrite(PINLEDMerah, HIGH);
      delay(200);
      digitalWrite(PINLEDBiru, LOW);
      digitalWrite(PINLEDMerah, LOW);
      delay(200);
      Serial.println(i);
      delay(1);
    }
  }
  // All Sensor Broke
  if (DHTCondition == false && MQ135Condition == false)
  {
    for (int i = 0; i < 4; i++)
    {
      digitalWrite(PINLEDBiru, HIGH);
      digitalWrite(PINLEDMerah, HIGH);
      delay(200);
      digitalWrite(PINLEDBiru, LOW);
      digitalWrite(PINLEDMerah, LOW);
      delay(200);
    }
  }
}

void handleAllLed(bool ledMerah, bool ledKuning, bool ledBiru)
{
  digitalWrite(PINLEDMerah, ledMerah);
  digitalWrite(PINLEDBiru, ledKuning);
  digitalWrite(PINLEDHijau, ledBiru);
  delay(1);
}

void sendDataHandle(const int idalat, float nilaisensor, String URL)
{

  delay(20);
  String postData = (String) "id_alat=" + idalat + "&nilai=" + nilaisensor;

  WiFiClient client;
  HTTPClient http;
  http.begin(client, URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  auto httpCode = http.POST(postData);
  String payload = http.getString();
  Serial.println(URL);
  Serial.println(postData);
  Serial.println(payload);

  http.end();
}
///

// put function declarations here:
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Start Program");
  delay(1);

  Serial.println(F("DHTxx test!"));

  dht.begin();
  pinMode(PINMQ, INPUT);
  pinMode(LED_INTERNAL, OUTPUT);
  digitalWrite(LED_INTERNAL, HIGH);
  pinMode(PINRELAY, OUTPUT);
  digitalWrite(PINRELAY, HIGH);
  delay(1);

  pinMode(PINLEDMerah, OUTPUT);
  pinMode(PINLEDBiru, OUTPUT);
  pinMode(PINLEDHijau, OUTPUT);
  handleAllLed(0, 0, 0);

  digitalWrite(PINLEDMerah, HIGH);

  WiFi.begin(ssid, password);
  Serial.println(String("Connecting to :") + ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(PINLEDBiru, HIGH);
    delay(200);
    digitalWrite(PINLEDBiru, LOW);
    delay(200);
    Serial.print(".");
  }
  Serial.println("");

  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(PINLEDBiru, HIGH);
  }

  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  delay(1);
  digitalWrite(LED_INTERNAL, LOW);
  delay(500);
}

void loop()
{
  // put your main code here, to run repeatedly:
  Serial.println("Run Loop Program");
  readDHT21();
  readMQ135();
  // Check the Sensor

  delay(2000);
  // Serial.println(DHTCondition);
  // Serial.println(MQ135Condition);

  showSensorError();
  if (DHTCondition && MQ135Condition)
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
      // Menyimpan waktu sekarang sebagai waktu sebelumnya
      previousMillis = currentMillis;

      sendDataHandle(id_device,temperature,URL_temperature);
      digitalWrite(PINLEDHijau, HIGH);
      delay(200);
      digitalWrite(PINLEDHijau, LOW);
      delay(200);
      sendDataHandle(id_device,humidity,URL_humidity);
      digitalWrite(PINLEDHijau, HIGH);
      delay(200);
      digitalWrite(PINLEDHijau, LOW);
      delay(200);
      sendDataHandle(id_device,nilai_Amonia,URL_amonia);
      digitalWrite(PINLEDHijau, HIGH);
      delay(200);
      digitalWrite(PINLEDHijau, LOW);
      delay(200);
    }
  }
}
