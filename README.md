# Trabalho Robótica - Sensor RFID + Comunicação assíncrona

![RabbitMQ](https://img.shields.io/badge/RabbitMQ-FF6600?style=for-the-badge&logo=rabbitmq&logoColor=white)
![Arduino](https://img.shields.io/badge/-Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white)
![C++](https://img.shields.io/badge/C++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)

Este é o projeto final da matéria de Robótica do curso de ADS no IFRS Campus Osorio.

## Stack utilizada

- Sketchs e esboço dos componentes elétricos e sendores produzidos com [Wolki](https://wokwi.com).
- Código fonte produzido em C++.
- Comunicação assíncrona entre microcontroladores, sob o protocolo MQTT, utilizando a plataforma online [CloudAMQP](https://www.cloudamqp.com).

## Sensores e atuadores utilizados
1. Leitor RFID.
1. Cartões RFID.
1. Relé.
1. Leds: azul, amarelo, verde e vermelho.
1. Sensor magnetico de porta.
1. Buzzer.
1. Botões.
1. Display.

## Instruções do trabalho
### Circuito A: Estação Porta RFID
- Objetivo: Ler cartões RFID dos usuários, decidir localmente se libera a passagem (com base
em autorização vinda da central) e sinalizar o estado da porta.
- Sensores:
	- Leitor RFID de curto alcance (ex.: MFRC522 ou similar, alcance ~1–3 cm).
	- (Opcional) Sensor magnético de porta (aberta/fechada).
- Atuadores:
	- Relé simulando a fechadura elétrica (LED ligado = porta destravada).
	- LED verde: acesso liberado.
	- LED vermelho: acesso negado.
	- Buzzer: bipe rápido para feedback de leitura.
- Função Principal:
	- Detectar proximidade de um cartão RFID, enviar o ID para a central e agir conforme
a resposta (autorizar ou negar).
- Interação (publica):
	- portaRFID/cartao_id – publica o ID do cartão lido (string com UID).
	- portaRFID/solicitacao_acesso – "PENDENTE" toda vez que um cartão novo é lido.
	- (Opcional) portaRFID/porta_status – "ABERTA"/"FECHADA".
- Interação (assina):
	- centralRFID/autorizacao – "LIBERADO" / "NEGADO".
	- (Opcional) centralRFID/tempo_aberta – tempo, em segundos, que a porta deve ficar
destravada.
- Lógica de Interação:
	- Quando um cartão é aproximado do leitor:
	- Lê o UID, faz um bipe curto no buzzer e envia portaRFID/cartao_id com o UID.
	- Publica portaRFID/solicitacao_acesso = "PENDENTE".
	- Enquanto não chega resposta, mantém LEDs apagados (ou vermelho fixo).
	- Ao receber centralRFID/autorizacao:
	- Se "LIBERADO":
	- Acende LED verde, aciona o relé (porta destravada) pelo tempo configurado (por
padrão ou vindo em centralRFID/tempo_aberta).
	- Depois do tempo, desliga relé e apaga LED verde.
	- Se "NEGADO":
	- Acende LED vermelho por alguns segundos e toca um padrão de buzzer (por
exemplo, dois bipes curtos).
	- (Se existir sensor de porta):
	- Atualiza portaRFID/porta_status para que a central saiba se a porta realmente
abriu/fechou após a liberação.

### Circuito B: Estação Central de Autorização RFID
- Objetivo: Receber os IDs dos cartões lidos nas portas, verificar se estão em uma lista de usuários autorizados e responder com liberação ou negação, além de manter um log básico.
- Sensores:
	- (Opcional) Botões para:
	- Adicionar o último cartão lido à lista de autorizados.
	- Remover o último cartão lido da lista de autorizados.
	- (Opcional) Outro leitor RFID no próprio painel para cadastrar cartões diretamente.
- Atuadores:
	- Display (OLED ou LCD) mostrando:
	- ID do cartão lido.
	- Nome do usuário (se houver mapeamento).
	- Resultado: "LIBERADO" / "NEGADO".
	- LED azul indicando sistema online.
	- LED amarelo para indicar modo de cadastro (aprendizado de novos cartões).
	- (Opcional) Buzzer para alertar tentativas de acesso negadas.
- Função Principal:
	- Implementar a lógica de autorização:
	- Manter uma lista de IDs de cartões autorizados.
	- Decidir a resposta para cada solicitação de acesso.
	- Registrar eventos de entrada.
- Interação (publica):
	- centralRFID/autorizacao – "LIBERADO" / "NEGADO".
	- (Opcional) centralRFID/tempo_aberta – valor em segundos, ex.: "5".
	- (Opcional) centralRFID/log_evento – string com resumo do evento, ex.: "UID1234;LIBERADO;2026-05-01T10:30".
- Interação (assina):
	- portaRFID/cartao_id.
	- portaRFID/solicitacao_acesso.
	- (Opcional) portaRFID/porta_status.
- Lógica de Interação:
	- Ao receber portaRFID/cartao_id:
	- Armazena o último ID recebido e exibe no display.
	- Ao receber portaRFID/solicitacao_acesso = "PENDENTE":
	- Verifica se o ID do último cartão está na lista de autorizados.
	- Se estiver:
		- Publica centralRFID/autorizacao = "LIBERADO".
		- Publica centralRFID/tempo_aberta (ex.: "5" segundos).
		- Mostra no display “Acesso LIBERADO” com o ID (e nome, se houver).
		- Publica centralRFID/log_evento com o registro.
	- Se não estiver:
		- Publica centralRFID/autorizacao = "NEGADO".
		- Mostra “Acesso NEGADO” no display.
		- Opcionalmente aciona buzzer e registra no log.
	- Modo de cadastro:
		- Quando o botão “modo cadastro” é pressionado:
		- Ativa LED amarelo indicando que o próximo cartão lido será adicionado ou
removido.
		- Se outro botão for pressionado em sequência (ex.: “adicionar”), o ID atual é
inserido na lista de autorizados.
		- Se o botão “remover” for pressionado, o ID é retirado da lista.
		- Atualiza o display para indicar o resultado (“Cartão adicionado” / “Cartão
removido”).
