// Incluir librerías: WiFi.h para conectar a WiFi, DHT.h para leer el sensor de humedad, ThingSpeak.h para conectar a ThingSpeak
#include <WiFi.h>
#include "DHT.h"
#include "ThingSpeak.h"

// Definir puertos de los sensores/dispositivos
#define LED_1 13
#define POT 34
#define DHT_PIN 32
#define DHT_TYPE DHT11

// Definir variables para conexión WiFi
const char* ssid = "NOMBRE DE RED";
const char* password = "CLAVE DE RED";

// Definir variables del canal de ThingSpeak
unsigned long myChannelNumber = NUMERO_DE_CANAL;
const char* myWriteAPIKey = "WRITE_API";

// Definir objetos
DHT dht11(DHT_PIN, DHT_TYPE);  // Objeto DHT para el sensor de humedad
WiFiClient client;             // Objeto WiFiClient para la conexión a WiFi con ThingSpeak

// Definir variables adicionales
float brillo;          // Variable para definir brillo porcentual a mostrar en ThingsPeak
float potenciometro;   // Variable para definir valor de brillo en LED
float humedad;        // Variable para humedad
float temperatura_C;  // Variable para temperatura en Celsius
float temperatura_F;  // Variable para temperatura en Fahrenheit

void setup() {
  Serial.begin(9600);  // Iniciar la comunicación serial
  dht11.begin();       // Iniciar el sensor de humedad/temperatura

  // Definir inputs y outputs
  pinMode(LED_1, OUTPUT);

  WiFi.begin(ssid, password);  // Conectar a la red WiFi
  Serial.print("Conectando a ");
  Serial.print(ssid);
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {  // Esperar conexión de WiFi
    delay(1000);
    Serial.print(++i);
    Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Conexion establecida!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());  // Mostrar la dirección IP asignada por consola

  ThingSpeak.begin(client);  // Inicializar ThingSpeak
}

void loop() {

  // Leer datos de sensores
  humedad = dht11.readHumidity();
  temperatura_C = dht11.readTemperature();
  temperatura_F = dht11.readTemperature(true);
  potenciometro = analogRead(POT) / 16;
  brillo = (potenciometro * 100) / 255;

  // Validar lectura de sensores
  if (isnan(humedad) || isnan(temperatura_C) || isnan(temperatura_F)) {
    Serial.println("Fallo al leer el sensor DHT!");
  } else {
    // Mostrar valores leídos por consola serial
    Serial.print("Humedad: ");
    Serial.print(humedad);
    Serial.print("%");
    Serial.print(" | ");
    Serial.print("Temperatura: ");
    Serial.print(temperatura_C);
    Serial.print("°C ~ ");
    Serial.print(temperatura_F);
    Serial.println("°F");
    Serial.print("Brillo: %");
    Serial.println(brillo);

    // Agregar los datos que serán enviados a ThingSpeak en los distintos campos:
    ThingSpeak.setField(1, humedad);
    ThingSpeak.setField(2, temperatura_C);
    ThingSpeak.setField(3, temperatura_F);
    ThingSpeak.setField(4, brillo);

    // Enviar todas las lecturas a ThingSpeak en un solo envío 
    int codigoRespuesta = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    //validar que la información fue correctamente recibida por el servidor:
    if (codigoRespuesta == 200) {
      Serial.println("Datos actualizados correctamente.");
    } else {
      Serial.println("Error al actualizar el canal. Código de error HTTP " + String(codigoRespuesta));
    }
    // Encender el LED con el valor del potenciometro
    analogWrite(LED_1, potenciometro);
  }
  
  // Esperar 15 segundos antes de la siguiente actualización (limitación de ThingSpeak)
  delay(15000);
}
