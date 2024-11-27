
# Sistemas digitales y ensambladores

Este repositorio se creó para cargar el código de los laboratorios y el proyecto para la asignatura de sistemas digitales y ensambladores de la universidad Politécnico Gran Colombiano.

Para los proyectos incluidos en esta carpeta se usa exclusivamente Arduino.IDE y un módulo ESP32




## Librerias y complementos requeridos

Para cargar los archivos HTML, CSS y JavaScript necesarios para construir este proyecto en la memoria flash ESP32 (LittleFS), usaremos un complemento para Arduino IDE: LittleFS Filesystem uploader. 

[Se puede seguir este corto tutorial para agregarlo al IDE](https://randomnerdtutorials.com/arduino-ide-2-install-esp32-littlefs/)

Librerias:

- [Arduino_JSON library by Arduino version 0.1.0 (Arduino Library Manager)](https://github.com/arduino-libraries/Arduino_JSON)
- [DHT sensor library)](https://github.com/adafruit/DHT-sensor-library)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)

Si se instala directamente desde el library manager de Arduino.IDE instalar todas las dependencias (AsyncTCP es dependencia de ESPAsyncWebServer).

# Proyecto

En este proyecto, estamos desarrollando un sistema basado en un ESP32 con conexión Wi-Fi como dispositivo maestro y un Arduino como esclavo, comunicados mediante el protocolo I2C. El sistema utiliza un sensor de proximidad para detectar cuando una mascota se acerca a un lugar restringido, generando una alerta sonora. Esta información se transmite al ESP32, que a su vez la despliega en tiempo real en una página web mediante un websocket.

Adicionalmente, el sistema incluye un sensor de temperatura y humedad para monitorear el ambiente donde se encuentra la mascota, asegurando que las condiciones sean óptimas. Este enfoque combina tecnologías de sensores, comunicación inalámbrica y desarrollo web para ofrecer una solución integral de monitoreo y alerta.




https://github.com/user-attachments/assets/cffedd05-6a8e-4d96-ae7d-901e624d6b9e






