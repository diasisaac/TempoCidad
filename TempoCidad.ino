#include <dummy.h>

#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <WiFi.h>
//#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

// Dados pra o ThingSpeak
#define STASSID "wifi thales"
#define STAPSK "softex123"

#include "DHT.h"

#define DHTPIN 5    // pino que estamos conectado
#define DHTTYPE DHT11 // DHT 11

// Conecte pino 1 do sensor (esquerda) ao +5V
// Conecte pino 2 do sensor ao pino de dados definido em seu Arduino
// Conecte pino 4 do sensor ao GND
// Conecte o resistor de 10K entre pin 2 (dados)
// e ao pino 1 (VCC) do sensor
DHT dht(DHTPIN, DHTTYPE);

WiFiClient client;

//############### VARIAVEIS #########################

long prevMillisThingSpeak = 0;
int intervalThingSpeak = 15000; // Intervalo minímo para escrever no ThingSpeak write é de 15 segundos
const long CHANNEL = 1585685;
const char *WRITE_API = "4DP4NJYOK29CZV3J";

// ########## SENSOR DHT11 ###########

float humidadeAtual;
float temperaturaAtual;

// ############ ANEMOMETRO ###############
/*The ESP8266 supports interrupts in any GPIO, except GPIO16.
  Outra observação importante a dizer é que o pino D0 (GPIO16) do ESP8266 não tem suporte para interrupções por entrada.
  https://www.filipeflop.com/blog/utilizando-interrupcoes-por-nivel-logico-no-gpio-do-esp8266/
  https://randomnerdtutorials.com/interrupts-timers-esp8266-arduino-ide-nodemcu/

*/ //15
#define pinInterrupcao 15 // pino sinal Interrupção Pino ligado ao pino D0 do sensor
double rpm;
int pi = 3.14159265359;
float radius = 1.3;       // Raio do anemometro(mm) exemplo de 3 cm que é 30 mm
int tempoDelay = 1000; // em milisegundos
volatile byte pulsosContados;
unsigned long timeold;

float velocivento_kmh = 5;
// Em metros por segundos
float velociVentoMinima = 5;

// Altere o numero abaixo de acordo com a qtd de furos do disco do anemometro
unsigned int pulsos_por_volta = 20;

// ###################### PLUVIOMETRO ########################

//#define PLUVIPIN (numero do pino aq)
float tempoChuva;
int contML;
bool isPluv, pluv = 0;
bool stillPluv;
float pluvTime;

// ######################### SETUP ARDUINO ###########################

// ######################## PLUVIOMETRO ###############################

void pluviTime()
{

  // isPluv = digitalRead(PLUVIPIN);

  if (isPluv != pluv)
  {
    pluvTime = millis(); // marca o tempo inicial do sistema

    do
    {

      if (isPluv != pluv)
      { // registra se ouve alteracao no sensor
        contML++;
        pluv = isPluv; // faz a troca de valores para a proxima ativacao
      }

      if (isPluv == pluv && (millis() - pluvTime) == 10000)
        stillPluv = 0; // caso o sensor nao seja alterado por 10 segundos, parou de chover, sai do loop
      else
        stillPluv = 1;

    } while (!stillPluv);

    tempoChuva = (millis() - pluvTime) / 1000;
  }

  // return contML / 2; // ativa o dobro de vezes por ser a cada alteracao do sensor
}
/*
  int converMin(float tempo)
  { // FUNCAO APENAS PARA CONVERTER O TEMPO
  switch (tempo)
  {
  case >= 3600:
    return tempo / 3600; // SE FOR MAIOR QUE 3600 SEGUNDOS SE PASSOU 1 HORA E CONVERTE
    break;
  case < 3600 return tempo / 60; // SE FOR MENOR CONVERTE APENAS PARA MINUTOS
      break;
      default:
    continue;
    break;
  }
  }

*/
//##################### FUNCOES UTILIZADAS #############################

// ---------------- CALCULOS ------------------
void IRAM_ATTR contador()
{
  // Incrementa contador
  pulsosContados++;
}

double calculaRpm()
{
  return ((60 * 1000 / pulsos_por_volta) / (millis() - timeold) * pulsosContados);
}

// Velocidade do vento em kmh
float func_calculaVelociVento_kmh()
{
  // Calcula a velocidade do vento em m/s
  // divide por 1000 pq é 1 segundo.
  return (((2 * pi * radius * rpm) / 60) / 1000) * 3.6;

} // end func_calculaVelociVento_kmh

void func_atualizaVelocidadeVento()
{
  // Se velocidade atual é maior do que a velocidade minima do vento

  //if (velocivento_kmh >= velociVentoMinima)
  //{
  // Atualiza a contagem a cada tempoDelay dos intervalos entre as amostras

  if (millis() - timeold >= tempoDelay)
  {

    // Desabilita interrupcao durante o calculo
    detachInterrupt(digitalPinToInterrupt(pinInterrupcao));
    rpm = calculaRpm();
    velocivento_kmh = func_calculaVelociVento_kmh();
    timeold = millis();

    pulsosContados = 0;

    // Mostra o valor de RPM no serial monitor
    Serial.print("RPM = ");
    Serial.println(rpm);

    // Mostra o valor de RPM no serial monitor
    Serial.print("Velocidade do Vento em kmh = ");
    Serial.println(velocivento_kmh);

    // Habilita interrupcao
    attachInterrupt(digitalPinToInterrupt(pinInterrupcao), contador, FALLING);
  }

  //}
  /*else
    {
    Serial.print("Erro - Velocidade do vento menor que a mínima");
    }*/
}

// --------------------- HUMIDADE TEMPERATURA --------------------
void lerDHT11()
{
  // A leitura da temperatura e umidade pode levar 250ms  !
  // O atraso do sensor pode chegar a 2 segundos.
  humidadeAtual = dht.readHumidity();
  temperaturaAtual = dht.readTemperature();
  // testa se retorno é valido, caso contrário algo está errado.
  if (isnan(temperaturaAtual) || isnan(humidadeAtual))
  {
    Serial.println("Failed to read from DHT");
  }
  else
  {
    Serial.print("Umidade: ");
    Serial.print(humidadeAtual);
    Serial.print(" %t ");
    Serial.print("Temperatura: ");
    Serial.print(temperaturaAtual);
    Serial.println(" °C");
    delay(1000);
  }
}

// ------------------------ THINGSPEAK -------------------------
void enviarDadosThingSpeak()
{

  // Escrever os dados pra o ThingSpeak

  if (millis() - prevMillisThingSpeak > intervalThingSpeak)
  {
    Serial.print("Entrou");
    // Configura os campos com os valores

    ThingSpeak.setField(1, temperaturaAtual); // campo de temperatura

    ThingSpeak.setField(2, humidadeAtual); // campo de humidade
    
    ThingSpeak.setField(3, velocivento_kmh); // campo de velocidade do vento

    // Escreve no canal do ThingSpeak
    int x = ThingSpeak.writeFields(CHANNEL, WRITE_API);

    if (x == 200)
    {
      Serial.println("Update realizado com sucesso");
    }
    else
    {
      Serial.println("Problema no canal - erro HTTP " + String(x));
    }

    prevMillisThingSpeak = millis();
  }
}
void setup()
{

  Serial.begin(115200);

  //Serial.begin(921600);

  Serial.println("DHT11 teste!");
  dht.begin();

  // ----------- WIFI SETUP----------------
  Serial.println();
  Serial.println("Envia os dados do sensor para o ThingSpeak usando o ESP8266");
  Serial.println();

  ThingSpeak.begin(client); // Inicializa o ThingSpeak

  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
  }

  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // ---------- ANEMOMETRO SETUP -------------

  // Pino do sensor como entrada
  pinMode(pinInterrupcao, INPUT);
  // Vai encontrar o valor lógico da Interrupcao com a função digitalPinToInterrupt()  - que recebe do pino digital D2 da ESP 8266
  // Aciona o contador a cada pulso
  attachInterrupt(digitalPinToInterrupt(pinInterrupcao), contador, FALLING);
  pulsosContados = 0;
  rpm = 0;
  timeold = 0;



}


// ########################### LOOP ARDUINO ##############################
void loop()
{
  lerDHT11();
  func_atualizaVelocidadeVento();
  enviarDadosThingSpeak();

}
