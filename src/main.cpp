#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>  // Pustaka untuk koneksi SSL
#include "DHT.h"

// Definisikan pin dan jenis sensor DHT
#define DHTPIN D6          // Pin untuk sensor DHT
#define DHTTYPE DHT22      // Jenis sensor DHT
DHT dht(DHTPIN, DHTTYPE);  // Membuat objek DHT

// Informasi jaringan WiFi
const char* ssid = "Redmi 13"; // Ganti dengan SSID WiFi Anda
const char* password = "123456789"; // Ganti dengan password WiFi Anda
const char* serverUrl = "https://nursery-mbkm.research-ai.my.id/api/senddata"; // URL server

WiFiClientSecure wifiClientSecure; // Membuat objek WiFiClientSecure untuk koneksi HTTPS

void setup() {
  Serial.begin(9600); // Memulai serial komunikasi
  WiFi.begin(ssid, password); // Menghubungkan ke WiFi

  // Menunggu hingga terhubung ke WiFi
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Menampilkan IP Address ESP8266
  
  dht.begin(); // Memulai sensor DHT
}

void loop() {
  // Membaca kelembapan dan suhu dari sensor DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Memeriksa apakah pembacaan valid
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor"); // Menampilkan pesan error jika gagal membaca sensor
  } else {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print("%  Temperature: ");
    Serial.print(t);
    Serial.println("°C");

    // Kalibrasi suhu (+3°C) dan kelembapan (+4%)
    t -= 3;  // Kalibrasi suhu (tambah 3°C)
    h += 3.5;  // Kalibrasi kelembapan (tambah 4%)

    // Menampilkan nilai kalibrasi untuk debugging
    Serial.print("Calibrated Humidity: ");
    Serial.print(h);
    Serial.print("%  Calibrated Temperature: ");
    Serial.print(t);
    Serial.println("°C");

    sendData(t, h); // Mengirim data suhu dan kelembapan yang telah dikalibrasi
  }
  
  delay(10000); // Delay selama 10 detik sebelum pembacaan berikutnya
}

void sendData(float temperature, float humidity) {
  if (WiFi.status() == WL_CONNECTED) { // Memastikan terhubung ke WiFi
    HTTPClient http; // Membuat objek HTTPClient
    http.setTimeout(30000); // Mengatur timeout koneksi menjadi 30 detik
    Serial.println("Initializing connection...");

    // Menyediakan client untuk koneksi HTTPS
    http.begin(wifiClientSecure, serverUrl); // Menginisialisasi koneksi ke server menggunakan WiFiClientSecure

    // Menonaktifkan verifikasi SSL untuk pengujian
    wifiClientSecure.setInsecure();  // Menonaktifkan verifikasi SSL (hanya untuk pengujian lokal)

    http.addHeader("Content-Type", "application/json"); // Menambahkan header untuk format JSON

    // Membuat payload JSON
    String jsonPayload = "{\"id_alat\": \"2\", \"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + "}";

    // Menampilkan payload untuk debugging
    Serial.print("Sending Payload: ");
    Serial.println(jsonPayload);

    // Mengirimkan data ke server
    int httpResponseCode = http.POST(jsonPayload);

    // Memeriksa respons dari server
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    if (httpResponseCode > 0) {
      String response = http.getString(); // Menerima respons dari server
      Serial.println("Response: " + response);
    } else {
      Serial.print("Error occurred while sending data. Code: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // Mengakhiri koneksi HTTP
  } else {
    Serial.println("WiFi not connected"); // Menampilkan pesan jika tidak terhubung ke WiFi
  }