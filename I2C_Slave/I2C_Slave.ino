/*********
  Dispositivo esclavo I2C para lectura de sensores  
*********/

// Incluir librerías

#include "DHT.h"
#include <Wire.h>
#define I2C_DEV_ADDR 0x55

// Definir puertos de los sensores/dispositivos
#define DHT_PIN 33
#define DHT_TYPE DHT11

// Definir objetos
DHT dht11(DHT_PIN, DHT_TYPE);

//variable led para prueba websocket
#define LED_2 31
//variables para 
bool requestInProgress = false;
bool ledState = LOW;

//variable global para actualizar las temperaturas cada 2 segundos:
String response;

// inicializar sensor DHT
void initDHT(){
  dht11.begin();  // Inicializa el sensor DHT
  Serial.println("DHT sensor initialized.");
}

void setup() {
  Serial.begin(9600);
  initDHT();
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);
  Wire.begin((uint8_t)I2C_DEV_ADDR);
  pinMode(LED_2, OUTPUT);
  digitalWrite(LED_2, LOW);
}


void loop() {

  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastReadTime >= 2000) {
    lastReadTime = currentTime;
    response = getSensorReadings();
  }

  // Cambia el estado del Led con el recibido desde la pagina web
  digitalWrite(LED_2, ledState); 
}

void onRequest() {
    if (requestInProgress) return; // Previene múltiples llamados
    requestInProgress = true;
    Serial.println("onRequest called");
    Serial.println(response);
    Wire.print(response.c_str());
    requestInProgress = false; 
}

String getSensorReadings() {
    // Leer temperatura y humedad
    float temperatureC = dht11.readTemperature();
    float humidity = dht11.readHumidity();
    float temperatureF = dht11.readTemperature(true);  // Leer en Fahrenheit

    // Verificar que las lecturas no sean NaN
    if (!isnan(temperatureC) && !isnan(humidity) && !isnan(temperatureF)) {
        // Crear un String con los datos concatenados
        String readings = String(temperatureC) + String(humidity) + String(temperatureF);
        return readings;
    } else {
      Serial.println("no se pudo obtener lecturas");
      return "NaN";  // Retornar "NaN" si alguna lectura falla
    }
}



void onReceive(int len) {
 Serial.print("onReceive: ");
 Serial.println(len);  // Print length of received message
 String message = "";
    while (Wire.available()) {
        char c = Wire.read();
        message += c;        
    }
    if (message == "botonPresionado") {
        Serial.println("Botón presionado en la página. Cambiando estado del LED.");
        ledState = !ledState;
        // Cambia el estado global del LED
    }
}
