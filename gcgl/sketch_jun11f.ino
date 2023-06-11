#include <OneWire.h>
#include <DallasTemperature.h>
#include <LoRa.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi 信息
const char* ssid = "T10086";
const char* password = "1117072301050606";

// MQTT 服务器设置
const char* mqttServer = "192.168.31.234";
const int mqttPort = 1883;
const char* mqttUser = "emqx";
const char* mqttPassword = "public";

// DS18B20 温度传感器的引脚
const int DS18B20_Pin = 12;

// LoRa SPI 引脚
const int LoRa_NSS = 7;
const int LoRa_MISO = 10;
const int LoRa_MOSI = 3;
const int LoRa_SCK = 2;
const int LoRa_RST = 8;
const int LoRa_DIO0 = 1;

// 初始化 OneWire 和 DallasTemperature
OneWire oneWire(DS18B20_Pin);
DallasTemperature sensors(&oneWire);

// 初始化 WiFi 和 MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);

  // 设置 LoRa 接脚
  LoRa.setPins(LoRa_NSS, LoRa_RST, LoRa_DIO0);

  // 尝试复位 LoRa 模块
  pinMode(LoRa_RST, OUTPUT);     // 设置RST引脚为输出
  digitalWrite(LoRa_RST, LOW);   // 将RST引脚设置为低电平
  delay(100);                    // 等待100ms
  digitalWrite(LoRa_RST, HIGH);  // 将RST引脚设置为高电平
  delay(100);                    // 等待100ms

  // 配置WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // 连接MQTT
  client.setServer(mqttServer, mqttPort);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed to connect MQTT, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }

  // 设置LoRa
  if (!LoRa.begin(470E6)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }

  // 设置 DS18B20
  sensors.begin();
}

void loop() {
  // 从 DS18B20 读取温度
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println("C");

  // 发送 LoRa 数据包
  LoRa.beginPacket();
  LoRa.print(temp);
  LoRa.endPacket();

  // 发送 MQTT 消息
  char temperatureString[10];
  dtostrf(temp, 6, 2, temperatureString);
  client.publish("temperature", temperatureString);

  delay(5000);
}
