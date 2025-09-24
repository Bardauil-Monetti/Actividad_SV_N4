#include <Arduino.h>

#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Telecentro-ece8";
const char* password = "DPYD4HHFQ37C";

const int ledPin = 25; 
const int pote = 34;

float volts=0.0;
int valorPote;

bool ledEncendido = true;
WebServer server(80);


// pagina
const char pagina_template[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title><b>MONITOR DE TENSIÓN</b></title>
  <style>
    body { 
      background-color: red; 
      text-align: center; 
    }
    
    .titulo { 
      font-family: Courier;
      font-style: italic; 
      font-size: 32px; 
      color: white; 
      margin-bottom: 20px;
    }

    .borde {
      width: 170px;
      height: 100px;
      border: 5px solid black;
      border-radius: 18px;
      background-color: black;
      margin: 0 auto; 
      display: flex;
      align-items: center;
      justify-content: center;
    }

    .pantalla {
      width: 160px;
      height: 90px;
      border: 1px solid black;
      border-radius: 10px;
      background-color: cyan;
      margin: 4px;
      display: flex;
      align-items: center;
      justify-content: center;
    }

    .boton-container {
      display: flex;
      justify-content: center; 
      margin-top: 10px;
      
    }

    .boton {
      width: 80px;
      height: 45px;
      background-color: yellow;
      color: black;
      border: 2px solid black;
      border-radius: 80px;
      font-weight: bold;
      font-size: 14px;
      cursor: pointer;

      display: flex;
      align-items: center;     
      justify-content: center; 
      padding: 0;
      text-align: center;
    }

  </style>
</head>
<body>
  <h1 class="titulo">MONITOR TENSIÓN</h1>

  <div class="borde">
    <div class="pantalla">  Tensión: <span style="font-weight:bold;">X.XXV</span></div>
  </div>

  <div class="boton-container">
    <a href="/toggleLED"><button class="boton">__ACCIONAR__</button></a>
  </div>
  
</body>
)rawliteral";


void toggle(){
    valorPote = analogRead(pote);
    int pwm = map(valorPote, 0, 4095, 0, 255);
    if(ledEncendido) {
    ledcWrite(0, pwm); 
    }else{
    ledcWrite(0, 0);   
    }
    ledEncendido = !ledEncendido;
    server.sendHeader("Location", "/"); // dice que la página se vaya a /
    server.send(302, "text/plain", ""); // 302 dice que el /toggle no está y que vaya a la locación de antes
}
void monitor(){
    valorPote = analogRead(pote);
    volts = (valorPote/4095.0) * 3.3;
    int pwm = map(valorPote, 0, 4095, 0, 255);
    ledcWrite(0, pwm);
}
void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  ledcSetup(0, 5000, 8); // canal 0, 5 kHz, 8 bits
  ledcAttachPin(ledPin, 0);

  // conexion al wifi como antes
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int timeout = 20; // 20 intentos de 500ms = 10 segundos
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    Serial.print(".");
    timeout--;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFallo la conexion. Reiniciando...");
    delay(1000);
    ESP.restart();
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("Dirección IP: http://");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    
    String pagina = pagina_template; //hago una variable para no modificar a la original
    //parte para cambiar lo que dice el boton una vez que se prende o apaga
    String textoBoton;
    
    if (ledEncendido == true)  textoBoton = "APAGAR";
    else textoBoton = "ENCENDER";
    //pagina.replace busca el texto y lo reemplaza con lo que pongas. 
    pagina.replace("__ACCIONAR__", textoBoton);
    String lectura = String(volts, 2); // 2 decimales
    pagina.replace("X.XX", lectura);


    server.send(200, "text/html", pagina);
    
  });

  server.on("/toggleLED",toggle);


  server.begin();
}

void loop() {
    server.handleClient();
    monitor();
}