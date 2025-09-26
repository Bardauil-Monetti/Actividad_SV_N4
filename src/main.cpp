#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>


const char* WIFI_SSID     = "Telecentro-da0d";
const char* WIFI_PASSWORD = "H4JTRX4NGL9L";


const int PIN_LED   = 2;   // LED controlado por PWM
const int PIN_POTE  = 14;  // Potenciómetro

float tensionLeida = 0.0;
int lecturaPote    = 0;
bool estadoLed     = true;

// Servidor Web en puerto 80
WebServer servidor(80);


const char HTML_BASE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>MONITOR DE TENSIÓN</title>
  <style>
    body { background-color: red; text-align: center; }
    .titulo { font-family: Courier; font-style: italic; font-size: 32px; color: white; margin-bottom: 20px; }
    .borde { width: 170px; height: 100px; border: 5px solid black; border-radius: 18px; background-color: black; margin: 0 auto; display: flex; align-items: center; justify-content: center; }
    .pantalla { width: 160px; height: 90px; border: 1px solid black; border-radius: 10px; background-color: cyan; margin: 4px; display: flex; align-items: center; justify-content: center; }
    .boton-container { display: flex; justify-content: center; margin-top: 10px; }
    .boton { width: 80px; height: 45px; background-color: yellow; color: black; border: 2px solid black; border-radius: 80px; font-weight: bold; font-size: 14px; cursor: pointer;
             display: flex; align-items: center; justify-content: center; padding: 0; text-align: center; }
  </style>
</head>
<body>
  <h1 class="titulo">MONITOR TENSIÓN</h1>
  <div class="borde">
    <div class="pantalla">Tensión: <span style="font-weight:bold;">X.XXV</span></div>
  </div>
  <div class="boton-container">
    <a href="/cambiarLed"><button class="boton">_ESTADO_BOTON_</button></a>
  </div>
</body>
</html>
)rawliteral";


void cambiarLed() {
    estadoLed = !estadoLed;  
    servidor.sendHeader("Location", "/");
    servidor.send(302, "text/plain", "");
}

void actualizarMedicion() {
    lecturaPote = analogRead(PIN_POTE);
    tensionLeida = (lecturaPote / 4095.0) * 3.3;

    int brillo = map(lecturaPote, 0, 4095, 0, 255);
    if (estadoLed) {
        ledcWrite(0, brillo);
    } else {
        ledcWrite(0, 0);
    }
}

void manejarRaiz() {
    String pagina = HTML_BASE;

    // Actualizar estado del botón
    pagina.replace("_ESTADO_BOTON_", estadoLed ? "APAGAR" : "ENCENDER");

    // Mostrar tensión
    String textoTension = String(tensionLeida, 2);
    pagina.replace("X.XX", textoTension);

    servidor.send(200, "text/html", pagina);
}


void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  ledcSetup(0, 5000, 8); 
  ledcAttachPin(PIN_LED, 0);

  Serial.print("Conectando a ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int intentos = 20;
  while (WiFi.status() != WL_CONNECTED && intentos > 0) {
    delay(500);
    Serial.print(".");
    intentos--;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nError de conexión. Reiniciando...");
    delay(1000);
    ESP.restart();
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("Dirección IP: http://");
  Serial.println(WiFi.localIP());

  // Rutas del servidor
  servidor.on("/", manejarRaiz);
  servidor.on("/cambiarLed", cambiarLed);

  servidor.begin();
}


void loop() {
    servidor.handleClient();
    aMedicion();
}
