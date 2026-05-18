#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <MQTTPubSubClient.h>

#include "config.h"
#include "topic.h"

#define LARGURA_TELA 128
#define ALTURA_TELA 64

Adafruit_SSD1306 display(LARGURA_TELA, ALTURA_TELA, &Wire, -1);

WiFiClient wifiClient;
MQTTPubSubClient mqtt;

// Pinos do Circuito B
const int ledAzul = 26;
const int ledAmarelo = 27;
const int buzzer = 25;
const int botaoAdd = 32;
const int botaoRemove = 33;

const int MAX_CARTOES = 10;

String cartoesAutorizados[MAX_CARTOES] = {
  "A1:B2:C3:D4",
  "11:22:33:44",
  "FF:AA:CC:DD"
};

int quantidadeCartoes = 3;
String ultimoCartao = "";
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
  while (!mqtt.connect("central-rfid", MQTT_USERNAME, MQTT_PASSWORD)) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nMQTT conectado");

  mqtt.subscribe(CARD_ID_TOPIC, [](const String& payload, const size_t size) {
    ultimoCartao = payload;
    ultimoCartao.toUpperCase();

    Serial.println("Cartao recebido: " + ultimoCartao);

    processarCartao(ultimoCartao);
  });

  mqtt.subscribe(REQUEST_ACCESS_TOPIC, [](const String& payload, const size_t size) {
    Serial.println("Solicitacao recebida: " + payload);
  });
}

void setup() {
  Serial.begin(115200);

  pinMode(ledAzul, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(botaoAdd, INPUT_PULLUP);
  pinMode(botaoRemove, INPUT_PULLUP);

  digitalWrite(ledAzul, LOW);
  digitalWrite(ledAmarelo, LOW);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erro ao iniciar OLED");
    while (true);
  }

  mostrarMensagem("CENTRAL RFID", "Iniciando...");

  conectarWiFi();
  conectarMQTT();

  digitalWrite(ledAzul, HIGH);

  mostrarMensagem("CENTRAL RFID", "Sistema Online");
}

void loop() {
  mqtt.update();

  if (!mqtt.isConnected()) {
    digitalWrite(ledAzul, LOW);
    conectarMQTT();
    digitalWrite(ledAzul, HIGH);
  }

  if (digitalRead(botaoAdd) == LOW) {
    adicionarCartao(ultimoCartao);
    delay(700);
  }

  if (digitalRead(botaoRemove) == LOW) {
    removerCartao(ultimoCartao);
    delay(700);
  }
}

void processarCartao(String uid) {
  if (uid == "") {
    return;
  }

  bool liberado = verificarCartao(uid);

  if (liberado) {
    mqtt.publish(AUTHORIZATION_TOPIC, "LIBERADO");
    mqtt.publish(SEC_2B_OPEN, String(tempoAberta));

    mostrarResultado("ACESSO LIBERADO", uid);
    tone(buzzer, 1200, 150);

    Serial.println("Resposta enviada: LIBERADO");
  } else {
    mqtt.publish(AUTHORIZATION_TOPIC, "NEGADO");

    mostrarResultado("ACESSO NEGADO", uid);

    tone(buzzer, 500, 150);
    delay(200);
    tone(buzzer, 500, 150);

    Serial.println("Resposta enviada: NEGADO");
  }

  delay(2000);
  mostrarMensagem("CENTRAL RFID", "Aguardando cartao...");
}

bool verificarCartao(String uid) {
  for (int i = 0; i < quantidadeCartoes; i++) {
    if (cartoesAutorizados[i] == uid) {
      return true;
    }
  }

  return false;
}

void adicionarCartao(String uid) {
  if (uid == "") {
    mostrarMensagem("ERRO", "Nenhum cartao lido");
    return;
  }

  digitalWrite(ledAmarelo, HIGH);

  if (verificarCartao(uid)) {
    mostrarResultado("JA CADASTRADO", uid);
  } else if (quantidadeCartoes >= MAX_CARTOES) {
    mostrarResultado("LISTA CHEIA", uid);
  } else {
    cartoesAutorizados[quantidadeCartoes] = uid;
    quantidadeCartoes++;

    mostrarResultado("CARTAO ADICIONADO", uid);
    Serial.println("Cartao adicionado: " + uid);
  }

  tone(buzzer, 1000, 150);
  delay(1800);

  digitalWrite(ledAmarelo, LOW);
  mostrarMensagem("CENTRAL RFID", "Aguardando cartao...");
}

void removerCartao(String uid) {
  if (uid == "") {
    mostrarMensagem("ERRO", "Nenhum cartao lido");
    return;
  }

  digitalWrite(ledAmarelo, HIGH);

  int posicao = -1;

  for (int i = 0; i < quantidadeCartoes; i++) {
    if (cartoesAutorizados[i] == uid) {
      posicao = i;
      break;
    }
  }

  if (posicao == -1) {
    mostrarResultado("NAO ENCONTRADO", uid);
  } else {
    for (int i = posicao; i < quantidadeCartoes - 1; i++) {
      cartoesAutorizados[i] = cartoesAutorizados[i + 1];
    }

    quantidadeCartoes--;

    mostrarResultado("CARTAO REMOVIDO", uid);
    Serial.println("Cartao removido: " + uid);
  }

  tone(buzzer, 800, 150);
  delay(1800);

  digitalWrite(ledAmarelo, LOW);
  mostrarMensagem("CENTRAL RFID", "Aguardando cartao...");
}

void mostrarMensagem(String titulo, String mensagem) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println(titulo);
  display.println("----------------");
  display.println(mensagem);
  display.println("");
  display.print("Total cards: ");
  display.println(quantidadeCartoes);

  display.display();
}

void mostrarResultado(String titulo, String uid) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println(titulo);
  display.println("----------------");
  display.println("Cartao:");
  display.println(uid);
  display.println("");
  display.print("Total cards: ");
  display.println(quantidadeCartoes);

  display.display();
}