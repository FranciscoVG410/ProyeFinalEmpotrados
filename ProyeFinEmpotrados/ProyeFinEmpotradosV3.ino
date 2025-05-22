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

// Pines touch
const int touchPin = 32;
int touchValue = 0;
const int threshold = 40;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

float temperatura = 0.0;
float humedad = 0.0;
float tempOptima = 25.0;
float humOptima = 50.0;
String mensajeEstado = "";

// Función buildHTML con tu código
String buildHTML(float tempOptima, float humOptima, String mensaje) {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Monitor ESP32</title>";
  html += "<style>";
  html += "body{font-family:Arial;background:#f0f8ff;text-align:center;margin:20px;}";
  html += "h2{color:#0066cc;}p{font-size:1.2em;}form{margin-top:20px;}";
  html += "input[type=number]{padding:5px;margin:5px;width:100px;}";
  html += "input[type=submit]{padding:10px 20px;background:#0077cc;color:white;border:none;cursor:pointer;}";
  html += "img {max-width: 150px; margin-top: 10px;}";
  html += "</style>";
  html += "<script>";
  html += "const imagenes = {";
  html += "\"Condición óptima\": \"https://th.bing.com/th/id/OIP.hOmIUH2xdJgRFw0YFN-ZlwAAAA?rs=1&pid=ImgDetMain\",";
  html += "\"Fuera de óptimo\": \"https://images.emojiterra.com/twitter/v11/128px/1f614.png\",";
  html += "\"Temp óptima\": \"https://images.emojiterra.com/twitter/v14.0/128px/1f610.png\",";
  html += "\"Hum óptima\": \"https://images.emojiterra.com/twitter/v14.0/128px/1f610.png\",";
  html += "\"¡Modo reactivo!\": \"https://w7.pngwing.com/pngs/56/906/png-transparent-smiley-emoji-face-emoticon-blushing-emoji-miscellaneous-sticker-online-chat.png\",";
  html += "\"Error sensor DHT\": \"https://i.pinimg.com/originals/64/9e/d1/649ed18f8e4b1177f9cf20c9cb2dde2d.png\"";
  html += "};";
  html += "setInterval(() => {";
  html += "fetch('/datos').then(r => r.json()).then(data => {";
  html += "document.getElementById('temp').textContent = data.temperatura + ' °C';";
  html += "document.getElementById('hum').textContent = data.humedad + ' %';";
  html += "document.getElementById('estado').textContent = data.estado;";
  html += "document.getElementById('estadoImg').src = imagenes[data.estado] || '';";
  html += "});";
  html += "}, 2000);";
  html += "</script>";
  html += "</head><body>";
  html += "<h2>Estado Actual</h2>";
  html += "<p>Temperatura: <span id='temp'>--</span></p>";
  html += "<p>Humedad: <span id='hum'>--</span></p>";
  html += "<p><strong>Estado:</strong> <span id='estado'>--</span></p>";
  html += "<img id='estadoImg' src='' alt='Estado' />";
  html += "<h2>Configurar Valores Óptimos</h2>";
  html += "<form action='/guardar'>";
  html += "Temperatura óptima (°C): <input type='number' name='tempOpt' value='" + String(tempOptima) + "' step='0.1'><br>";
  html += "Humedad óptima (%): <input type='number' name='humOpt' value='" + String(humOptima) + "' step='0.1'><br><br>";
  html += "<input type='submit' value='Guardar'>";
  html += "</form></body></html>";
  return html;
}

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
  server.on("/config", HTTP_POST, [](){
    if(server.hasArg("plain")){
      StaticJsonDocument<64> doc;
      DeserializationError error = deserializeJson(doc, server.arg("plain"));
      if (error) {
        server.send(400, "application/json", "{\"error\":\"JSON inválido\"}");
        return;
      }
    
      tempOptima = doc["tempOpt"] | tempOptima;
      humOptima  = doc["humOpt"]  | humOptima;
    
      server.send(200);
    } else {
      server.send(400, "application/json", "{\"error\":\"Falta cuerpo\"}");
    } 
  });
  server.on("/datos", handleDatos);
  server.on("/guardar", handleGuardar);
  server.begin();
}

// Caritas para OLED (las básicas para ejemplo)
void caraFeliz() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(":) Condición óptima");
  display.display();
}
void caraNeutral() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(":| Condición parcial");
  display.display();
}
void caraTriste() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(":( Fuera de óptimo");
  display.display();
}
void caraReactiva() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(":O Modo reactivo!");
  display.display();
}

void enviarDatosTCP(float temp, float hum) {
  WiFiClient tcpClient;
  if (tcpClient.connect("192.168.100.14", 1234)) {  // Cambia IP y puerto
    String json = "{\"temp\":" + String(temp) + ",\"hum\":" + String(hum) + "}";
    tcpClient.print(json);
    tcpClient.stop();
  }
}
void enviarDatosSerial(float temp, float hum, const String& estado)
{
  String json = "{\"temp\":";
  json += String(temp, 1);          // 1 decimal
  json += ",\"hum\":";
  json += String(hum, 1);
  json += ",\"estado\":\"";
  json += estado;
  json += "\"}";
  
  Serial.println(json);             // <-- línea completa para Python
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
    touchValue = touchRead(touchPin);
    if (touchValue < threshold) {
      mensajeEstado = "¡Modo reactivo!";
      caraReactiva();
      delay(3000);
    } else {
      mensajeEstado = "Condición óptima";
      caraFeliz();
    }
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
    enviarDatosSerial(temperatura, humedad, mensajeEstado);
  }


  delay(2000);
}
