#include <DHTesp.h> //Librería para manejo de sensor del tipo DHTX en ESP32
#include <WiFi.h> //Librería para conectividad WiFI
#include <ThingsBoard.h> //Librería para manejo de ThingsBoard
#include <Arduino_MQTT_Client.h> //Librería para uso de MQTT en Arduino

#define pinDht 32 //Pin por donde llegan los datos del sensor
DHTesp dhtSensor;//Instancia del sensor

#define WIFI_AP "" //Nombre del WiFi del entorno
#define WIFI_PASS "" //Contraseña del WiFi del entorno

#define TB_SERVER "thingsboard.cloud" //Servidor con el que se va a generar conectividad (192.168.X.X)
#define TOKEN "" //TOKEn del dispositvo en la plataforma

constexpr uint16_t MAX_MESSAGE_SIZE = 128U; //Tamaño de los paquetes

WiFiClient espClient; //Instancia del cliente Wifi
Arduino_MQTT_Client mqttClient(espClient); //Conectar al cliente WiFI al entorno MQTT
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE); //Crear una instancia de ThingsBoard con el cliente MQTT y máximo tamaño de mensaje

//Función para generar conectividad a Wifi
void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  int attempts = 0;
  //Probar conectividad WIFI
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    WiFi.begin(WIFI_AP, WIFI_PASS, 6);
    delay(500);
    Serial.print(".");
    attempts++;
  }
  //Verificar conectividad WiFI
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect to WiFi.");
  } else {
    Serial.println("\nConnected to WiFi");
  }
}

//Función para generar conexión con el servidor de ThingsBoard
void connectToThingsBoard() {
  //Probando conectividad con el servidor de ThingsBoard
  if (!tb.connected()) {
    Serial.println("Connecting to ThingsBoard server");
    //Verificar conectividad con ThingsBoard
    if (!tb.connect(TB_SERVER, TOKEN)) {
      Serial.println("Failed to connect to ThingsBoard");
    } else {
      Serial.println("Connected to ThingsBoard");
    }
  }
}

//Generar paquete con datos del sensor y enviar la telemetría
void sendDataToThingsBoard(float temp, int hum) {
  String jsonData = "{\"temperature\":" + String(temp) + ", \"humidity\":" + String(hum) + "}";
  //Enviar telemetría a la instancia de ThingsBoard
  tb.sendTelemetryJson(jsonData.c_str());
  Serial.println("Data sent");
}

//Inicializar instancia del sensor y funciones
void setup() {
  Serial.begin(115200);
  dhtSensor.setup(pinDht,DHTesp::DHT11); //Instancia del sensor DHT11
  connectToWiFi(); //Conectarse a WiFI
  connectToThingsBoard(); //Conectarse con ThingsBoard
}

void loop() {
  //Conectarse a Wifi
  connectToWiFi();
  
  //Obtener datos del sensor DHT11
  TempAndHumidity data =dhtSensor.getTempAndHumidity();
  float temp = data.temperature;
  int hum = data.humidity;
  
  //Se muestran los datos en el serial
  Serial.println(temp);
  Serial.println(hum);
  
  //Verificar conexión con Thinsgboard
  if (!tb.connected()) {
    connectToThingsBoard();
  }
  
  //Enviar datos a la Thinsgboard Cloud
  sendDataToThingsBoard(temp, hum);

  delay(3000);
  
  tb.loop();
}