#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LARGURA_TELA 128
#define ALTURA_TELA 64

Adafruit_SSD1306 display(LARGURA_TELA, ALTURA_TELA, &Wire, -1);

// Pinos
const int ledAzul = 26;
const int ledAmarelo = 27;
const int buzzer = 25;
const int botaoAdd = 32;
const int botaoRemove = 33;

// Cartão recebido simulado
String ultimoCartao = "A1 B2 C3 D4";

// Lista fixa de cartões autorizados
String cartoesAutorizados[] = {
  "A1 B2 C3 D4",
  "11 22 33 44",
  "FF AA CC DD"
};

int quantidadeCartoes = 3;

unsigned long tempoAnterior = 0;
int contadorSimulacao = 0;

void setup() {
  Serial.begin(9600);

  pinMode(ledAzul, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(buzzer, OUTPUT);

  pinMode(botaoAdd, INPUT_PULLUP);
  pinMode(botaoRemove, INPUT_PULLUP);

  digitalWrite(ledAzul, HIGH);
  digitalWrite(ledAmarelo, LOW);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erro ao iniciar OLED");
    while (true);
  }

  mostrarTelaInicial();

  Serial.println("Central RFID iniciada");
}

void loop() {
  if (digitalRead(botaoAdd) == LOW) {
    modoCadastro("ADICIONAR");
    delay(500);
  }

  if (digitalRead(botaoRemove) == LOW) {
    modoCadastro("REMOVER");
    delay(500);
  }

  // Simula recebimento de um cartão a cada 5 segundos
  if (millis() - tempoAnterior >= 5000) {
    tempoAnterior = millis();
    simularCartaoRecebido();
  }
}

void simularCartaoRecebido() {
  if (contadorSimulacao % 2 == 0) {
    ultimoCartao = "A1 B2 C3 D4"; // autorizado
  } else {
    ultimoCartao = "99 88 77 66"; // negado
  }

  contadorSimulacao++;

  bool liberado = verificarCartao(ultimoCartao);

  if (liberado) {
    respostaLiberado(ultimoCartao);
  } else {
    respostaNegado(ultimoCartao);
  }
}

bool verificarCartao(String uid) {
  for (int i = 0; i < quantidadeCartoes; i++) {
    if (uid == cartoesAutorizados[i]) {
      return true;
    }
  }

  return false;
}

void respostaLiberado(String uid) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("ACESSO LIBERADO");
  display.println("----------------");
  display.println("Cartao:");
  display.println(uid);
  display.println("");
  display.println("Resposta: OK");
  display.display();

  Serial.println("Cartao: " + uid);
  Serial.println("Status: LIBERADO");

  tone(buzzer, 1200, 150);
  delay(2000);

  mostrarTelaInicial();
}

void respostaNegado(String uid) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("ACESSO NEGADO");
  display.println("----------------");
  display.println("Cartao:");
  display.println(uid);
  display.println("");
  display.println("Resposta: NEGADO");
  display.display();

  Serial.println("Cartao: " + uid);
  Serial.println("Status: NEGADO");

  tone(buzzer, 500, 150);
  delay(200);
  tone(buzzer, 500, 150);

  delay(2000);

  mostrarTelaInicial();
}

void modoCadastro(String acao) {
  digitalWrite(ledAmarelo, HIGH);

  tone(buzzer, 1000, 150);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("MODO CADASTRO");
  display.println("----------------");
  display.print("Acao: ");
  display.println(acao);
  display.println("");
  display.print("Cartao: ");
  display.println(ultimoCartao);
  display.display();

  Serial.print("Botao pressionado: ");
  Serial.println(acao);

  delay(1500);

  digitalWrite(ledAmarelo, LOW);
  mostrarTelaInicial();
}

void mostrarTelaInicial() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("CENTRAL RFID");
  display.println("----------------");
  display.println("Sistema Online");
  display.println("");
  display.println("Aguardando cartao...");
  display.display();
}