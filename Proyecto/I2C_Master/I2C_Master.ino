/*********
  WebServer para actualizar temperatura y humedad por medio de websocket. 
  Los sensores están conectados en otro dispositivo, y se transmite la información 
  por medio de I2C.
  Este es el código del dispositivo "master" quien se conecta al WiFi para transmitir datos
  por medio de websocket 
*********/

// Incluir librerías
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include <Wire.h>

// Definir puertos del esclavo
#define I2C_DEV_ADDR 0x55

// Definir variables para conexión WiFi
const char* ssid = "iPhone-J4PNQC36G2";
const char* password = "IsaJuli2020";

// Crear objeto AsyncWebServer en el puerto 80
AsyncWebServer server(80);

// Definir objetos
AsyncWebSocket ws("/ws");

// Variable Json para almacenar las lecturas del sensor
JSONVar readings;

// Variables de tiempo para actualizar los datos
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

// Obtener lecturas del sensor y devolver un objeto JSON
String getSensorReadings() {
  Wire.requestFrom(I2C_DEV_ADDR, 17); // Solicita 17 bytes (1 byte para movimiento + 15 caracteres + terminador nulo)

  // Validar que se reciban únicamente 17 bytes
  if (Wire.available() == 17) {
    char data[17]; // Almacena la cadena recibida
    for (int i = 0; i < 17; i++) {
      data[i] = Wire.read(); // Leer los 17 bytes
    }
    
    data[16] = '\0'; // Asegurar que la cadena esté terminada en null

    // Definir los grupos (solo 5 caracteres cada uno)
    char motion[2];   // Para el primer byte (movimiento)
    char tempC[5];    // Para los siguientes 5 caracteres
    char humedad[5];  // Para los siguientes 5 caracteres
    char tempF[5];    // Para los últimos 5 caracteres
    
    // Copiar los caracteres a los grupos
    motion[0] = data[0];      // Primer byte (movimiento)
    motion[1] = '\0';         // Terminador nulo
    for (int i = 0; i < 5; i++) {
      tempC[i] = data[i + 1];      // Segundo grupo (data[1] a data[5])
      humedad[i] = data[i + 6];    // Tercer grupo (data[6] a data[10])
      tempF[i] = data[i + 11];     // Cuarto grupo (data[11] a data[15])
    }
    int motionDetected = atoi(motion); // Convierte a int
    float temperature = atof(tempC);   // Convierte a float
    float humidity = atof(humedad);    // Convierte a float
    float pressure = atof(tempF);      // Convierte a float

    if (motionDetected == 1) {
      notifyClients("Movimiento Detectado");
    }
    // Asignar valores a un objeto de lectura
    readings["temperature"] = String(temperature, 2); // Limita a 2 decimales
    readings["humidity"] = String(humidity, 2);       // Limita a 2 decimales
    readings["pressure"] = String(pressure, 2);       // Limita a 2 decimales
    String jsonString = JSON.stringify(readings);
    return jsonString;
  }
  return "";
}

// Inicializar LittleFS (cargar archivos de web server al ESP32, y leerlos de la memoria)
void initLittleFS() {
  if (!LittleFS.begin()) {
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
  // Mostrar en consola dirección IP
  Serial.println(WiFi.localIP());
}

// Enviar notificación websocket a todos los clientes conectados
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
      Wire.beginTransmission(I2C_DEV_ADDR);
      Wire.printf("botonPresionado");  //enviar mensaje al esclavo para cambiar el estado global del LED
      Wire.endTransmission();
    }
  }
}

// Mensajes en Serial de los eventos de websocket
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
  Serial.begin(9600);
  Wire.begin(); // Inicializa I2C
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
  // Enviar periódicamente las lecturas al websocket
  if ((millis() - lastTime) > timerDelay) {
    String sensorReadings = getSensorReadings();
    notifyClients(sensorReadings);
    lastTime = millis();
  }

  ws.cleanupClients();
}