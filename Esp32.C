#include <LiquidCrystal_I2C.h>
#include "DHTesp.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST"; // SSID da sua rede Wi-Fi
const char* password = ""; // Senha da sua rede Wi-Fi
const char mqttBroker[] = "broker.emqx.io";
const int mqttPort = 1883;

const int DHT_PIN = 15;


HTTPClient cliente; // HTTP client
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

DHTesp dhtSensor;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Declaração e inicialização do objeto lcd

void setup() {
  lcd.init(); // Inicializa o LCD
  lcd.clear(); // Limpa o LCD
  lcd.backlight(); // Liga a luz de fundo do LCD
  Serial.begin(115200);

  // Configuração do Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Conectado ao Wi-Fi com sucesso!");
  
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  mqttClient.setServer(mqttBroker, mqttPort);

  while (!mqttClient.connected()) {
    String clientId = "nsifb-";
    clientId += String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("Conectando ao Broker");
    } else {
      delay(500);
      Serial.println("Falha ao conectar ao Broker");
    }
  }
}

void loop() {   
  String url = "https://api.thingspeak.com/update?api_key=WVIO62AF3Z1YM6RB&field1=";
  lcd.setCursor(0, 0);      
  lcd.print("temperatura: "); // Imprime o texto "Temp: " no LCD
  float humidity = dhtSensor.getHumidity();
  float temperature = dhtSensor.getTemperature();
  lcd.print(temperature, 1); // Imprime a temperatura no LCD com uma casa decimal
  lcd.print(" C"); // Imprime a unidade de temperatura no LCD
  lcd.setCursor(0, 1); // Define o cursor para a segunda linha do LCD
  lcd.print("umidade: ");
  lcd.print(humidity, 1); // Imprime a umidade no LCD com uma casa decimal
  lcd.print(" %"); // Imprime o símbolo de porcentagem no LCD
  Serial.println("temperatura: " + String(temperature, 1) + "°C");
  Serial.println("umidade: " + String(humidity, 2) + "%");
  Serial.println("---");

  url = url + String(temperature) + "&field2=" + String(humidity);
  Serial.println(url);

  // Configuração do HTTPClient
  delay(500);
  String payload = "";
  char payloadChar[1024];
  cliente.begin(url);
  int httpCode = cliente.GET();
  Serial.println(httpCode);
  if (httpCode > 0) {
    Serial.println("Informações enviadas com sucesso ");
    payload = cliente.getString();
    payload.toCharArray(payloadChar, sizeof(payloadChar));
    Serial.println(payload);
  } else {
    Serial.println("Problemas no envio da informação");
  }

  String data = "temperatura:"+String(temperature)+"; humidade:"+String(humidity);

//configuração do mqtt
  if (mqttClient.connected()) {
    mqttClient.publish("professor", data.c_str());
    Serial.println("Dados publicados no tópico senai.");
  } else {
    Serial.println("Cliente MQTT não conectado ao broker.");
  }

  delay(2000); // Espera por uma nova leitura do sensor (DHT22 tem uma taxa de amostragem de ~0.5Hz)
}