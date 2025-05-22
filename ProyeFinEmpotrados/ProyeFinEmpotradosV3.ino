#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid = "Francisco";
const char* password = "valdez1010";

WebServer server(80);

float temperature = 0.0;
float humidity = 0.0;

void setup() {
  Serial.begin(115200);
  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("No se pudo inicializar la pantalla OLED"));
    for(;;);
  }

  display.clearDisplay();
  display.display();

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Sensor DHT11</title>
      </head>
      <body>
        <h1>Lectura de Temperatura y Humedad</h1>
        <p>Temperatura: <span id="temp">--</span> °C</p>
        <p>Humedad: <span id="hum">--</span> %</p>

        <script>
          setInterval(() => {
            fetch('/data')
              .then(response => response.json())
              .then(data => {
                document.getElementById('temp').textContent = data.temperature;
                document.getElementById('hum').textContent = data.humidity;
              });
          }, 1000);
        </script>
      </body>
      </html>
    )rawliteral");
  });

  server.on("/data", HTTP_GET, []() {
    String json = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("Temp: ");
  display.print(temperature);
  display.println(" C");

  display.print("Humedad: ");
  display.print(humidity);
  display.println(" %");

  // Dibujar la cara según la temperatura
  if (temperature >= 28.0) {
    // Cara triste
    display.drawCircle(96, 32, 30, SSD1306_WHITE);
    display.fillCircle(86, 24, 3, SSD1306_WHITE);
    display.fillCircle(106, 24, 3, SSD1306_WHITE);
    display.drawPixel(84, 48, SSD1306_WHITE);
    display.drawPixel(88, 46, SSD1306_WHITE);
    display.drawPixel(92, 44, SSD1306_WHITE);
    display.drawPixel(96, 43, SSD1306_WHITE);
    display.drawPixel(100, 44, SSD1306_WHITE);
    display.drawPixel(104, 46, SSD1306_WHITE);
    display.drawPixel(108, 48, SSD1306_WHITE);
  } else if (temperature >= 24.0) {
    // Cara neutral
    display.drawCircle(96, 32, 30, SSD1306_WHITE);
    display.fillCircle(86, 24, 3, SSD1306_WHITE);
    display.fillCircle(106, 24, 3, SSD1306_WHITE);
    display.drawLine(84, 42, 108, 42, SSD1306_WHITE);
  } else if (temperature >= 20.0) {
    // Cara feliz
    display.drawCircle(96, 32, 30, SSD1306_WHITE);
    display.fillCircle(86, 24, 3, SSD1306_WHITE);
    display.fillCircle(106, 24, 3, SSD1306_WHITE);
    display.drawPixel(84, 42, SSD1306_WHITE);
    display.drawPixel(88, 44, SSD1306_WHITE);
    display.drawPixel(92, 46, SSD1306_WHITE);
    display.drawPixel(96, 47, SSD1306_WHITE);
    display.drawPixel(100, 46, SSD1306_WHITE);
    display.drawPixel(104, 44, SSD1306_WHITE);
    display.drawPixel(108, 42, SSD1306_WHITE);
  } else {
    // Cara reactiva (ojos cerrados y boca abierta)
    display.drawCircle(96, 32, 30, SSD1306_WHITE);
    display.drawLine(81, 24, 91, 24, SSD1306_WHITE); // ojo izquierdo
    display.drawLine(101, 24, 111, 24, SSD1306_WHITE); // ojo derecho
    display.drawPixel(84, 42, SSD1306_WHITE);
    display.drawPixel(88, 44, SSD1306_WHITE);
    display.drawPixel(92, 46, SSD1306_WHITE);
    display.drawPixel(96, 47, SSD1306_WHITE);
    display.drawPixel(100, 46, SSD1306_WHITE);
    display.drawPixel(104, 44, SSD1306_WHITE);
    display.drawPixel(108, 42, SSD1306_WHITE);
    display.drawLine(86, 43, 106, 43, SSD1306_WHITE); // línea horizontal dentro de la boca
  }

  display.display();
  server.handleClient();
  delay(200);
}
