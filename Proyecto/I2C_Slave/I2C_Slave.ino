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

// Variables para prueba con el LED y el buzzer
#define LED_2 31
#define buzz 53
#define IR_SENSOR_PIN 32  // Pin del sensor de infrarrojos (IR)

// Variables para el estado de los dispositivos
bool requestInProgress = false;
bool ledState = LOW;

// Variable global para actualizar las temperaturas cada 2 segundos
String response;

// Inicializar sensor DHT
void initDHT() {
  dht11.begin();  // Inicializa el sensor DHT
  Serial.println("DHT sensor initialized.");
}

void setup() {
  Serial.begin(9600);
  initDHT();
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);
  Wire.begin((uint8_t)I2C_DEV_ADDR);
  
  // Configurar pines
  pinMode(LED_2, OUTPUT);
  pinMode(buzz, OUTPUT);
  pinMode(IR_SENSOR_PIN, INPUT);  // Sensor IR como entrada
  digitalWrite(buzz, LOW);
  digitalWrite(LED_2, LOW);
}

void loop() {
  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();
  
  // Leer los sensores cada 2 segundos
  if (currentTime - lastReadTime >= 200) {
    lastReadTime = currentTime;
    response = getSensorReadings();
  }

  // Cambia el estado del LED y buzzer basado en el valor recibido
  digitalWrite(LED_2, ledState); 
  
   if (digitalRead(IR_SENSOR_PIN)) {  
    digitalWrite(buzz, LOW); 
  } else {
    digitalWrite(buzz, HIGH); 
  }
}

void onRequest() {
  if (requestInProgress) return; // Previene múltiples solicitudes
  requestInProgress = true;
  
  Serial.println("onRequest called");
  Serial.println(response);
  Wire.print(response.c_str());  // Enviar datos al maestro

  requestInProgress = false; 
}

String getSensorReadings() {
  // Leer temperatura y humedad
  float temperatureC = dht11.readTemperature();
  float humidity = dht11.readHumidity();
  float temperatureF = dht11.readTemperature(true);  // Leer en Fahrenheit
  int movement;
  if (digitalRead(IR_SENSOR_PIN)) {  // Si el sensor detecta proximidad
    movement = 0;
      // Enviar mensaje de movimiento detectado al maestro
  } else {
    movement = 1;
  }
  // Verificar que las lecturas no sean NaN
  if (!isnan(temperatureC) && !isnan(humidity) && !isnan(temperatureF)) {
    // Crear un String con los datos concatenados
    String readings = String(movement)+String(temperatureC) + String(humidity) + String(temperatureF);
    return readings;
  } else {
    Serial.println("No se pudo obtener lecturas");
    return "NaN";  // Retornar "NaN" si alguna lectura falla
  }
}


// Función que maneja los mensajes recibidos desde el maestro
void onReceive(int len) {
  Serial.print("onReceive: ");
  Serial.println(len);  // Imprimir la longitud del mensaje recibido
  String message = "";
  
  while (Wire.available()) {
    char c = Wire.read();
    message += c;        
  }
  
  // Si el mensaje recibido es "botonPresionado", cambiar el estado del LED
  if (message == "botonPresionado") {
    Serial.println("Botón presionado en la página. Cambiando estado del LED.");
    ledState = !ledState;     
  }
  
}
