/*********
  WebServer para actualizar temperatura y humedad por medio de websocket. 
  basado en instrucciones por: https://RandomNerdTutorials.com/esp32-websocket-server-sensor/
  
*********/

// Incluir librerías

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include "DHT.h"
#include <Arduino_JSON.h>
#include <Wire.h>

// Definir puertos de los sensores/dispositivos
#define DHT_PIN 32
#define DHT_TYPE DHT11


// Definir variables para conexión WiFi
const char* ssid = "ssid";
const char* password = "pass";

// Crear objeto AsyncWebServer en el puerto 80
AsyncWebServer server(80);

// Definir objetos
DHT dht11(DHT_PIN, DHT_TYPE);
AsyncWebSocket ws("/ws");


// Variable Json para almacenar las lecturas del sensor

JSONVar readings;

// Variables de tiempo para actualizar los datos
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;
bool ledState = LOW;

//variable led para prueba websocket
#define LED_2 13

// inicializar sensor DHT
void initDHT(){
  dht11.begin();  // Inicializa el sensor DHT
  Serial.println("DHT sensor initialized.");
}

// Obtener lecturas del sensor y devolver un objeto JSON
String getSensorReadings(){
  readings["temperature"] = String(dht11.readTemperature());
  readings["humidity"] =  String(dht11.readHumidity());
  readings["pressure"] = String(dht11.readTemperature(true));
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// Inicializar LittleFS (cargar archivos de web server al ESP32, y leerlos de la memoria)
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// Inicializar WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  //mostrar en consola dirección IP
  Serial.println(WiFi.localIP());
}

//enviar notificación websocket a todos los clientes conectados
void notifyClients(String sensorReadings) {
  ws.textAll(sensorReadings);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0; // Asegurar que el mensaje recibido es tratado como un string
    String message = (char*)data;
    Serial.printf("Mensaje recibido: %s\n", message.c_str());

    // Si se recibe "getReadings", enviar las lecturas del sensor
    if (message == "getReadings") {
      String sensorReadings = getSensorReadings();
      Serial.println(sensorReadings);
      notifyClients(sensorReadings);
    }
    // Si se recibe "botonPresionado", encender o apagar el LED
    else if (message == "botonPresionado") {
      Serial.println("Botón presionado en la página. Cambiando estado del LED.");
      ledState = !ledState;  // Cambia el estado global del LED
    }
  }
}

//mensajes en Serial de los eventos de websocket
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      Serial.printf("Total clients: %u\n", server->count());

      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      Serial.printf("Total clients: %u\n", server->count());

      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_2, OUTPUT);
  digitalWrite(LED_2, LOW);

  initDHT();
  initWiFi();
  initLittleFS();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  // Inicializar server
  server.begin();
}


void loop() {

  //envia periodicamente las lecturas al websocket

  if ((millis() - lastTime) > timerDelay) {
    String sensorReadings = getSensorReadings();
    notifyClients(sensorReadings);
    lastTime = millis();
  }
  ws.cleanupClients();
  // Cambia el estado del Led con el recibido desde la pagina web
  digitalWrite(LED_2, ledState); 
}