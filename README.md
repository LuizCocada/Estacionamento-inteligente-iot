# Sistema de Estacionamento Automático IoT com Arduino

Este projeto implementa um sistema de controle de estacionamento automatizado utilizando um Arduino, sensores infravermelhos, um servo motor para a cancela e um display LCD I2C para feedback ao usuário.

## Descrição

O sistema gerencia a entrada e saída de veículos de um estacionamento com um número limitado de vagas. Ele utiliza dois sensores infravermelhos para detectar a passagem de veículos: um posicionado antes da entrada e outro antes da saída da cancela (que é única para entrada e saída). Um servo motor controla a abertura e o fechamento da cancela. O display LCD informa o status do estacionamento, como mensagens de boas-vindas, número de vagas disponíveis ou se o estacionamento está lotado.

## Funcionalidades Principais

* **Controle Automático da Cancela:** Abre e fecha a cancela automaticamente com base na detecção dos sensores.
* **Contagem de Vagas:** Mantém o controle do número de vagas disponíveis.
* **Display de Status:** Exibe informações relevantes no LCD (Boas-vindas, Vagas Disponíveis, Estacionamento Lotado).
* **Lógica de Timeout:** Se um veículo iniciar a entrada/saída mas não completar a passagem em um tempo determinado (10 segundos), a cancela fecha e a contagem de vagas é ajustada/mantida.
* **Tratamento de Saída com Estacionamento Cheio:** Permite a saída de veículos mesmo que o sistema acredite estar com todas as vagas ocupadas, sem incrementar o contador de vagas além do máximo.
* **Detecção de Borda dos Sensores:** Reage apenas ao momento inicial da ativação do sensor, evitando múltiplos acionamentos.

## Componentes Necessários (Hardware)

* 1x Placa Arduino (Uno, Nano, Mega, etc.)
* 2x Sensores de Obstáculo Infravermelho (IR)
* 1x Servo Motor (ex: SG90, MG995)
* 1x Display LCD 16x2 com módulo I2C
* Jumpers (fios de conexão)
* Protoboard (opcional, para montagem)
* Fonte de alimentação para o Arduino e, possivelmente, para o servo (dependendo do consumo).

## Bibliotecas Necessárias (Software)

* `Wire.h` (padrão da IDE Arduino, para comunicação I2C)
* `LiquidCrystal_I2C.h` (para controle do display LCD via I2C - pode ser necessário instalar via Gerenciador de Bibliotecas da IDE Arduino)
* `Servo.h` (padrão da IDE Arduino, para controle do servo motor)

## Conexões (Pinos)

As conexões padrão definidas no código são:

* **Servo Motor:** `PINO_SERVO` (Pino Digital 4)
* **Sensor de Entrada (externo):** `PINO_SENSOR_ENTRADA` (Pino Digital 2)
* **Sensor de Saída (interno):** `PINO_SENSOR_SAIDA` (Pino Digital 3)
* **Display LCD I2C:**
    * SDA: Pino A4 (Uno/Nano) ou pino dedicado SDA
    * SCL: Pino A5 (Uno/Nano) ou pino dedicado SCL
    * Endereço I2C padrão no código: `0x27`

*Nota: Os sensores IR são configurados com `INPUT_PULLUP`, esperando um sinal `LOW` quando um obstáculo é detectado.*

## Lógica de Funcionamento

O sistema opera com base em uma máquina de estados:

1.  **`REPOUSO`**:
    * A cancela está fechada.
    * O display LCD mostra "Bem-vindo!" e o número de vagas, ou "Estacionamento Lotado!" se não houver vagas.
    * **Para Entrada:** Se o sensor de entrada é ativado e há vagas, a cancela abre, `vagasDisponiveis` é decrementada, e o estado muda para `AGUARDANDO_CONFIRMACAO_ENTRADA`. Se lotado, exibe mensagem no LCD.
    * **Para Saída:** Se o sensor de saída é ativado, a cancela abre, e o estado muda para `AGUARDANDO_CONFIRMACAO_SAIDA`. A contagem de vagas não muda neste momento.

2.  **`AGUARDANDO_CONFIRMACAO_ENTRADA`**:
    * A cancela está aberta, aguardando o veículo passar pelo sensor de saída.
    * Se o sensor de saída é ativado, a entrada é confirmada, a cancela fecha após um breve delay, e o estado volta para `REPOUSO`.
    * Se o tempo (`TIMEOUT_PASSAGEM_MS`) expirar antes do sensor de saída ser ativado, a cancela fecha, `vagasDisponiveis` é incrementada (restaurando a vaga), e o estado volta para `REPOUSO`.

3.  **`AGUARDANDO_CONFIRMACAO_SAIDA`**:
    * A cancela está aberta, aguardando o veículo passar pelo sensor de entrada.
    * Se o sensor de entrada é ativado, a saída é confirmada. Se `vagasDisponiveis < MAX_VAGAS`, o contador é incrementado. A cancela fecha após um breve delay, e o estado volta para `REPOUSO`.
    * Se o tempo (`TIMEOUT_PASSAGEM_MS`) expirar antes do sensor de entrada ser ativado, a cancela fecha, e o estado volta para `REPOUSO` (nenhuma alteração nas vagas é feita, pois não foi alterada inicialmente).

## Configurações Personalizáveis no Código

Você pode ajustar as seguintes constantes no início do arquivo `.ino`:

* `PINO_SERVO`, `PINO_SENSOR_ENTRADA`, `PINO_SENSOR_SAIDA`: Pinos de conexão dos componentes.
* `ANGULO_SERVO_ABERTO`, `ANGULO_SERVO_FECHADO`: Ângulos do servo para as posições da cancela.
* `MAX_VAGAS`: Número total de vagas do estacionamento.
* `TIMEOUT_PASSAGEM_MS`: Tempo em milissegundos para o timeout da cancela.
* Endereço I2C do LCD: `LiquidCrystal_I2C lcd(0x27, 16, 2);` - ajuste `0x27` se o seu display tiver um endereço diferente.

## Como Usar

1.  Conecte os componentes conforme especificado na seção "Conexões".
2.  Instale as bibliotecas necessárias na IDE Arduino.
3.  Abra o arquivo `.ino` na IDE Arduino.
4.  Selecione a placa Arduino correta e a porta COM correspondente em `Ferramentas`.
5.  Carregue o código para a placa Arduino.
6.  Abra o Monitor Serial (Ctrl+Shift+M) com baud rate de `9600` para mensagens de depuração.

## Possíveis Melhorias Futuras

* Implementar debounce de software para os sensores IR se houver problemas com ruído.
* Armazenar a contagem de vagas na EEPROM para persistir após desligamentos.
* Adicionar um sistema de LEDs para indicar status (livre/lotado).
* Integração com plataformas IoT para monitoramento remoto.
