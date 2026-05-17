#include <SPI.h>
#include <MFRC522.h>
#include "config.h"
#include "topic.h"
#include <WiFi.h>
#include <MQTTPubSubClient.h>

WiFiClient wifiClient;
MQTTPubSubClient mqtt;

#define SS_PIN      5
#define RST_PIN     21
#define BUZZER_PIN  4
#define RED_LED_PIN 12
#define GREEN_LED_PIN 13
#define RELAY_PIN 26
#define DOOR_SENSOR_PIN 27

MFRC522 rfid(SS_PIN, RST_PIN);

String autorizacao = "";
int tempoAberta = 3;

void conectarWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
}

void conectarMQTT() {
  wifiClient.connect(MQTT_HOST_NAME, MQTT_PORT);
  mqtt.begin(wifiClient);

  Serial.print("Conectando MQTT");
  while (!mqtt.connect(CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nMQTT conectado");

  mqtt.subscribe(AUTHORIZATION_TOPIC, [](const String& payload, const size_t size) {
    autorizacao = payload;
    autorizacao.toUpperCase();
  });

  mqtt.subscribe(SEC_2B_OPEN, [](const String& payload, const size_t size) {
    tempoAberta = payload.toInt();

    if (tempoAberta <= 0) {
      tempoAberta = 3;
    }
  });
}

String lerUID() {
  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      uid += "0";
    }

    uid += String(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) {
      uid += ":";
    }
  }

  uid.toUpperCase();
  return uid;
}

void bipCurto() {
  tone(BUZZER_PIN, 1000);
  delay(150);
  noTone(BUZZER_PIN);
}

void negarAcesso() {
  digitalWrite(RED_LED_PIN, HIGH);

  for (int i = 0; i < 2; i++) {
    tone(BUZZER_PIN, 400);
    delay(150);
    noTone(BUZZER_PIN);
    delay(150);
  }

  delay(1000);
  digitalWrite(RED_LED_PIN, LOW);

  mqtt.publish(DOOR_STATUS_TOPIC, "FECHADA");
}

void liberarAcesso() {
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RELAY_PIN, HIGH);

  mqtt.publish(DOOR_STATUS_TOPIC, "ABERTA");

  delay(tempoAberta * 1000);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  mqtt.publish(DOOR_STATUS_TOPIC, "FECHADA");
}

void setup() {
  Serial.begin(115200);

  Serial.println("Iniciando...");

  SPI.begin();
  rfid.PCD_Init();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);

  conectarWiFi();
  conectarMQTT();

  Serial.println("Escaneie um cartão");
}

void loop() {
  mqtt.update();

  if (!mqtt.isConnected()) {
    conectarMQTT();
  }

  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  String uid = lerUID();

  bipCurto();

  autorizacao = "";

  mqtt.publish(CARD_ID_TOPIC, uid);
  mqtt.publish(REQUEST_ACCESS_TOPIC, "PENDENTE");

  digitalWrite(RED_LED_PIN, HIGH);

  while (autorizacao == "") {
    mqtt.update();
    delay(10);
  }

  digitalWrite(RED_LED_PIN, LOW);

  if (autorizacao == "LIBERADO") {
    liberarAcesso();
  } else {
    negarAcesso();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(1000);
}