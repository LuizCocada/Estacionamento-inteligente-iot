#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo motorCancela;
const int PINO_SERVO = 4;
const int ANGULO_SERVO_ABERTO = 0;
const int ANGULO_SERVO_FECHADO = 100;

const int PINO_SENSOR_ENTRADA = 2;
const int PINO_SENSOR_SAIDA = 3;

const int MAX_VAGAS = 4;
int vagasDisponiveis = MAX_VAGAS;

const unsigned long TIMEOUT_PASSAGEM_MS = 10000;

enum EstadoSistema {
  REPOUSO,
  AGUARDANDO_CONFIRMACAO_ENTRADA,
  AGUARDANDO_CONFIRMACAO_SAIDA
};
EstadoSistema estadoAtual = REPOUSO;
unsigned long tempoAcionamentoPrimeiroSensor = 0;

bool sensorEntradaEstavaAtivo = false;
bool sensorSaidaEstavaAtivo = false;

void setup() {
  Serial.begin(9600);


  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Estacionamento");
  lcd.setCursor(0, 1);
  lcd.print("Inteligente IoT");
  delay(2000);
  lcd.clear();

  motorCancela.attach(PINO_SERVO);
  fecharCancela(); 

  pinMode(PINO_SENSOR_ENTRADA, INPUT_PULLUP);
  pinMode(PINO_SENSOR_SAIDA, INPUT_PULLUP);

  atualizarDisplayLCD();
}

void loop() {

  bool sensorEntradaAcionado = (digitalRead(PINO_SENSOR_ENTRADA) == LOW);
  bool sensorSaidaAcionado = (digitalRead(PINO_SENSOR_SAIDA) == LOW);

  bool bordaSensorEntrada = sensorEntradaAcionado && !sensorEntradaEstavaAtivo;
  sensorEntradaEstavaAtivo = sensorEntradaAcionado;

  bool bordaSensorSaida = sensorSaidaAcionado && !sensorSaidaEstavaAtivo;
  sensorSaidaEstavaAtivo = sensorSaidaAcionado;


  switch (estadoAtual) {
    case REPOUSO:
      // ENTRADA 
      if (bordaSensorEntrada) {
        Serial.println("Sensor de ENTRADA acionado (RE_E).");
        if (vagasDisponiveis > 0) {
          abrirCancela();
          vagasDisponiveis--;  
          estadoAtual = AGUARDANDO_CONFIRMACAO_ENTRADA;
          tempoAcionamentoPrimeiroSensor = millis();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Entrando...");
          Serial.println("Estado: AGUARDANDO_CONFIRMACAO_ENTRADA");
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Desculpe :(");
          lcd.setCursor(0, 1);
          lcd.print("Lotado!");
          Serial.println("Tentativa de entrada: LOTADO.");
          delay(2000);  
        }
      }
      // SAÍDA 
      else if (bordaSensorSaida) {
        Serial.println("Sensor de SAIDA acionado (RE_S).");
        abrirCancela();
        estadoAtual = AGUARDANDO_CONFIRMACAO_SAIDA;
        tempoAcionamentoPrimeiroSensor = millis();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Saindo...");
        Serial.println("Estado: AGUARDANDO_CONFIRMACAO_SAIDA");
      }
      break;

    case AGUARDANDO_CONFIRMACAO_ENTRADA:
      if (bordaSensorSaida) {
        Serial.println("Sensor de SAIDA acionado (ACE_S) - Entrada Confirmada.");
        delay(1000);  
        fecharCancela();
        estadoAtual = REPOUSO;
        Serial.println("Estado: REPOUSO. Entrada concluída.");
      }
      // Timeout
      else if (millis() - tempoAcionamentoPrimeiroSensor > TIMEOUT_PASSAGEM_MS) {
        Serial.println("TIMEOUT em AGUARDANDO_CONFIRMACAO_ENTRADA.");
        fecharCancela();
        vagasDisponiveis++;  
        estadoAtual = REPOUSO;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Entrada Cancel.");
        delay(1500);
        Serial.println("Estado: REPOUSO. Entrada cancelada por timeout.");
      }
      break;

    case AGUARDANDO_CONFIRMACAO_SAIDA:
      if (bordaSensorEntrada) {
        Serial.println("Sensor de ENTRADA acionado (ACS_E) - Saída Confirmada.");
        if (vagasDisponiveis < MAX_VAGAS) {
          vagasDisponiveis++;
        }
        delay(1000);  
        fecharCancela();
        estadoAtual = REPOUSO;
        Serial.println("Estado: REPOUSO. Saída concluída.");
      }
      // Timeout
      else if (millis() - tempoAcionamentoPrimeiroSensor > TIMEOUT_PASSAGEM_MS) {
        Serial.println("TIMEOUT em AGUARDANDO_CONFIRMACAO_SAIDA.");
        fecharCancela();
        estadoAtual = REPOUSO;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Saida Cancel.");
        delay(1500);
        Serial.println("Estado: REPOUSO. Saída cancelada por timeout.");
      }
      break;
  }


  if (estadoAtual == REPOUSO) {
    static unsigned long ultimoTempoDisplay = 0;
    if (millis() - ultimoTempoDisplay > 500) {  
      atualizarDisplayLCD();
      ultimoTempoDisplay = millis();
    }
  }
}

void abrirCancela() {
  motorCancela.write(ANGULO_SERVO_ABERTO);
  Serial.println("Cancela: ABERTA");
}

void fecharCancela() {
  motorCancela.write(ANGULO_SERVO_FECHADO);
  Serial.println("Cancela: FECHADA");
}

void atualizarDisplayLCD() {
  if (estadoAtual == REPOUSO) {
    lcd.clear(); 

    if (vagasDisponiveis == 0) {
      lcd.setCursor(0, 0); 
      lcd.print("Estacionamento");
      lcd.setCursor(0, 1); 
      lcd.print("Lotado!     "); 
    } else {
      lcd.setCursor(0, 0);
      lcd.print(" Bem-vindo!   ");
      lcd.setCursor(0, 1);
      lcd.print(" Vagas: ");

      if (vagasDisponiveis < 0) vagasDisponiveis = 0;
      if (vagasDisponiveis > MAX_VAGAS) vagasDisponiveis = MAX_VAGAS;

      lcd.print(vagasDisponiveis);

      int posicaoInicialEspacos = 8 + String(vagasDisponiveis).length();
      for (int i = posicaoInicialEspacos; i < 16; i++) {
        lcd.print(" ");
      }
    }
  }
}

// void atualizarDisplayLCD() {
//   if (estadoAtual == REPOUSO) {  
//     lcd.clear();                 
//     lcd.setCursor(0, 0);
//     lcd.print(" Bem-vindo!");  
//     lcd.setCursor(0, 1);
//     lcd.print(" Vagas: ");                                           
//     if (vagasDisponiveis < 0) vagasDisponiveis = 0;                  
//     if (vagasDisponiveis > MAX_VAGAS) vagasDisponiveis = MAX_VAGAS;  
//     lcd.print(vagasDisponiveis);
//     for (int i = 8 + String(vagasDisponiveis).length(); i < 16; i++) {
//       lcd.print(" ");
//     }
//   }
// }