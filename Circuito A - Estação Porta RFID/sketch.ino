#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN      5
#define RST_PIN     21
#define BUZZER_PIN  4
#define RED_LED_PIN 12
#define GREEN_LED_PIN 13
#define RELAY_PIN 26
#define DOOR_SENSOR_PIN 27

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);

  Serial.println("Iniciando...");

  SPI.begin();
  rfid.PCD_Init();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);

  pinMode(GREEN_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, LOW);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);

  Serial.println("MFRC522 Pronto");
  Serial.println("Escaneie um cartão");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println("Card detected! Buzzing...");

  tone(BUZZER_PIN, 1000);
  delay(1000);
  noTone(BUZZER_PIN);

  Serial.println("Buzzer stopped");

  digitalWrite(RED_LED_PIN, HIGH);
  delay(1000);
  digitalWrite(RED_LED_PIN, LOW);
  delay(1000);

  Serial.print("Antes do rele: ");
  Serial.println(digitalRead(DOOR_SENSOR_PIN));

  digitalWrite(RELAY_PIN, HIGH);
  delay(500);

  Serial.print("Depois do rele ligado: ");
  Serial.println(digitalRead(DOOR_SENSOR_PIN));

  digitalWrite(GREEN_LED_PIN, HIGH);
  delay(1000);
  digitalWrite(GREEN_LED_PIN, LOW);

  digitalWrite(RELAY_PIN, LOW);
  delay(500);

  Serial.print("Depois do rele desligado: ");
  Serial.println(digitalRead(DOOR_SENSOR_PIN));

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(1000);
}