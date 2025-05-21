#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi
const char* ssid = "MEGACABLE-2.4G-74A5";
const char* password = "seVfjks29B";

// Pines
#define DHTPIN 16
#define DHTTYPE DHT11
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

// Variables
float temperatura = 0.0;
float humedad = 0.0;
float tempOptima = 25.0;
float humOptima = 50.0;
String mensajeEstado = "";

// Cara feliz
void caraFeliz() {
  display.clearDisplay();
  display.drawCircle(32, 32, 30, SSD1306_WHITE);
  display.fillCircle(22, 24, 3, SSD1306_WHITE); // Ojo izquierdo
  display.fillCircle(42, 24, 3, SSD1306_WHITE); // Ojo derecho

  // Boca feliz (curva con píxeles)
  display.drawPixel(20, 42, SSD1306_WHITE);
  display.drawPixel(24, 44, SSD1306_WHITE);
  display.drawPixel(28, 46, SSD1306_WHITE);
  display.drawPixel(32, 47, SSD1306_WHITE);
  display.drawPixel(36, 46, SSD1306_WHITE);
  display.drawPixel(40, 44, SSD1306_WHITE);
  display.drawPixel(44, 42, SSD1306_WHITE);

  display.display();
}

// Cara neutral
void caraNeutral() {
  display.clearDisplay();
  display.drawCircle(32, 32, 30, SSD1306_WHITE);
  display.fillCircle(22, 24, 3, SSD1306_WHITE);
  display.fillCircle(42, 24, 3, SSD1306_WHITE);
  display.drawLine(20, 42, 44, 42, SSD1306_WHITE); // boca recta
  display.display();
}

// Cara triste
void caraTriste() {
  display.clearDisplay();
  display.drawCircle(32, 32, 30, SSD1306_WHITE);
  display.fillCircle(22, 24, 3, SSD1306_WHITE);
  display.fillCircle(42, 24, 3, SSD1306_WHITE);

  // Boca triste (curva con píxeles)
  display.drawPixel(20, 48, SSD1306_WHITE);
  display.drawPixel(24, 46, SSD1306_WHITE);
  display.drawPixel(28, 44, SSD1306_WHITE);
  display.drawPixel(32, 43, SSD1306_WHITE);
  display.drawPixel(36, 44, SSD1306_WHITE);
  display.drawPixel(40, 46, SSD1306_WHITE);
  display.drawPixel(44, 48, SSD1306_WHITE);

  display.display();
}

// Página web
String buildHTML(float tempOptima, float humOptima, String mensaje) {
  String html = R"=====(<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Monitor ESP32 - Valores Óptimos</title>
<script>
function actualizarDatos() {
  fetch('/datos')
    .then(res => res.json())
    .then(data => {
      document.getElementById('temp').innerText = data.temperatura + " °C";
      document.getElementById('hum').innerText = data.humedad + " %";
      document.getElementById('estado').innerText = data.estado;
    });
}
setInterval(actualizarDatos, 2000);
window.onload = actualizarDatos;
</script>
</head>
<body>
<h2>Estado Actual</h2>
<p>Temperatura: <span id="temp">--</span></p>
<p>Humedad: <span id="hum">--</span></p>
<p><strong>Estado:</strong> <span id="estado">--</span></p>

<h2>Configurar Valores Óptimos</h2>
<form action="/guardar">
  Temperatura óptima (°C): <input type="number" name="tempOpt" value=")=====";
  html += String(tempOptima);
  html += "\" step=\"0.1\"><br>";
  html += "Humedad óptima (%): <input type=\"number\" name=\"humOpt\" value=\"";
  html += String(humOptima);
  html += "\" step=\"0.1\"><br><br>";
  html += R"=====(<input type="submit" value="Guardar">
</form>
</body>
</html>)=====";
  return html;
}

// Ruta /datos
void handleDatos() {
  String json = "{";
  json += "\"temperatura\":" + String(temperatura, 1) + ",";
  json += "\"humedad\":" + String(humedad, 1) + ",";
  json += "\"estado\":\"" + mensajeEstado + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleGuardar() {
  if (server.hasArg("tempOpt")) tempOptima = server.arg("tempOpt").toFloat();
  if (server.hasArg("humOpt")) humOptima = server.arg("humOpt").toFloat();
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT_PULLUP);
  dht.begin();
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("No se detectó OLED");
    while (true);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Conectando WiFi...");
  display.display();

  IPAddress ip(192, 168, 100, 50);
  IPAddress gateway(192, 168, 100, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  if (WiFi.status() == WL_CONNECTED) {
    display.println("WiFi conectado");
    display.println(WiFi.localIP());
  } else {
    display.println("WiFi NO conectado");
  }
  display.display();
  delay(1500);

  server.on("/", []() {
    server.send(200, "text/html", buildHTML(tempOptima, humOptima, mensajeEstado));
  });
  server.on("/datos", handleDatos);
  server.on("/guardar", handleGuardar);
  server.begin();
}

void enviarDatosTCP(float temp, float hum) {
  WiFiClient tcpClient;
  if (tcpClient.connect("192.168.100.14", 1234)) {
    String json = "{\"temp\":" + String(temp) + ",\"hum\":" + String(hum) + "}";
    tcpClient.print(json);
    tcpClient.stop();
  }
}

void loop() {
  server.handleClient();
  temperatura = dht.readTemperature();
  humedad = dht.readHumidity();

  if (isnan(temperatura) || isnan(humedad)) {
    mensajeEstado = "Error sensor DHT";
    caraTriste();
    delay(2000);
    return;
  }

  float tol = 2.0;
  bool tempOk = abs(temperatura - tempOptima) <= tol;
  bool humOk = abs(humedad - humOptima) <= tol;

  if (tempOk && humOk) {
    mensajeEstado = "Condición óptima";
    caraFeliz();
  } else if (tempOk || humOk) {
    mensajeEstado = tempOk ? "Temp óptima" : "Hum óptima";
    caraNeutral();
  } else {
    mensajeEstado = "Fuera de óptimo";
    caraTriste();
  }

  static unsigned long lastEnvio = 0;
  if (millis() - lastEnvio > 5000) {
    enviarDatosTCP(temperatura, humedad);
    lastEnvio = millis();
  }

  delay(2000);
}
