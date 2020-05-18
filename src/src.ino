/*
  Programa desenvolvido para a disciplina de Sistemas de Controle I
*/

const int INPUT_PIN = A3;
const int OUTPUT_PIN = 11;

const int POS_PIN = 9;
const int NEG_PIN = 10;

const int IS_POS = 7;
const int IS_NEG = 6;

const int INPUT_BUFFER_SIZE = 11;

double current_value = 0;
unsigned long previousMillis = 0;
unsigned long counter = 1;
float pwm_pos =0;
float pwm_neg =0;


enum Signal {Step = 0, UpRamp, DownRamp};

// Inicializacao padrao
Signal current_signal = UpRamp;
// Frequencia de amostragem
unsigned long current_frequency = 10;
double interval = ((double) 1 / (double)current_frequency);
// Valor maximo a ser atingido
double output_value = 10;

// Le as instrucoes via comunicacao serial enviadas pelo computador
// e preenche:
// current_signal = (Sinal escolhido pelo usuario)
// output_value = (Amplitude maxima escolhida pelo usuario)
//

/*
   Instrucao enviada pelo computador ao Arduino
   Formato:
*/

//    ----------------------------------------------------------
//    | "TIPO de sinal, frequencia de amostragem, valor (em V) |
//    ----------------------------------------------------------
/*
          TIPO de sinal:
            (0) -> Degrau
            (1) -> Rampa crescente
            (2) -> Rampa descrescente

          Frequencia de amostragem:
            (0) -> Maxima
            (1) -> Media
            (2) -> Mínima
*/


void readInstructions() {
  static char buff[INPUT_BUFFER_SIZE];
  static unsigned int pos = 0;

  Signal signalRead = current_signal;
  unsigned long sampling_frequency = current_frequency;
  double outputValueRead = output_value;

  if(Serial.available())
  //if(1==0)
  {
    byte value = Serial.read();
    buff[pos] = value;
    //Serial.print("Read: ");
    //Serial.println(value);
    if(buff[pos] == '\n')
    {
      //Serial.println("Delimiter found!");
      buff[++pos] = '\0';

      //Serial.print("Buffer: ");
      //Serial.println(buff);
      pos = -1;
      int substringCounter = 0;

      char *token;

      token = strtok(buff, ",");

      while(token != NULL)
      {
        int currentValue = atoi(token);

        //Serial.print("Current token: ");
        //Serial.println(token);

        switch(substringCounter) {
          case 0:
            signalRead = currentValue;
            Serial.print("Signal read:");
            Serial.println(signalRead);
            break;
          case 1:
            sampling_frequency = currentValue;
            break;
          case 2:
            outputValueRead = currentValue;
            break;
        }
        substringCounter++;
        token = strtok(NULL, ",");
      }
    }
    pos++;
  }


 //Signal signalRead = UpRamp;
 //unsigned long sampling_frequency = 35;
 //double outputValueRead = 9;

  // Sinal de saida mudou
  if ((current_signal != signalRead) || (output_value != outputValueRead)) {
    current_signal = signalRead;
    output_value = outputValueRead;
    current_value = current_signal == Step ? -10 : 0;
  }

  // Frequencia de amostragem mudou
  if (sampling_frequency != current_frequency) {
    current_frequency = sampling_frequency;
    interval = ((double) 1 / (double)sampling_frequency);
  }
}

void executeInstructions() {
  if (current_signal == Step) {
    current_value = output_value;
    if(output_value < 0){

      digitalWrite(IS_NEG, LOW);
      digitalWrite(IS_POS, HIGH);

      pwm_pos = 0;
      analogWrite(POS_PIN, 0);

      pwm_neg =(int) -output_value * 255 / 10;
      analogWrite(NEG_PIN, (int) pwm_neg);

    }
    else{

      digitalWrite(IS_NEG, HIGH);
      digitalWrite(IS_POS, LOW);

      pwm_neg = 0;
      analogWrite(NEG_PIN, 0);

      pwm_pos =(int) output_value * 255 / 10;
      analogWrite(POS_PIN, (int) pwm_pos);

    }
  }
  else if (current_signal == UpRamp) {
    analogWrite(NEG_PIN, 0);
    pwm_neg = 0;

    digitalWrite(IS_NEG, HIGH);
    digitalWrite(IS_POS, LOW);

    current_value += 0.01;
    pwm_pos += 0.25;
    analogWrite(POS_PIN, (int) pwm_pos);
    if(current_value >= output_value)
    {
      current_value = 0;
      pwm_pos = 0;
    }
  }
  else if (current_signal == DownRamp) {
    analogWrite(POS_PIN, 0);
    pwm_pos = 0;

    digitalWrite(IS_NEG, LOW);
    digitalWrite(IS_POS, HIGH);

    current_value -= 0.01;
    pwm_neg += 0.25;
    analogWrite(NEG_PIN, (int) pwm_neg);
    if(current_value <= output_value)
    {
      current_value = 0;
      pwm_neg = 0;
    }
  }
}

// Envia os dados para o computador via serial no formato:
// -----------------------------------------------------------------------------------
// | tempo decorrido em segundos, Valor de tensão (V), Frequencia de amostragem (Hz) |
// -----------------------------------------------------------------------------------
void sendSamplingData() {
  Serial.print(counter * ((double) 1 / (double)current_frequency), 10);
  Serial.print(',');
  Serial.print(current_value, 10);
  Serial.print(',');
  Serial.println(current_frequency);
  counter++;
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(POS_PIN, OUTPUT);
  pinMode(NEG_PIN, OUTPUT);
  pinMode(IS_POS,OUTPUT);
  pinMode(IS_NEG,OUTPUT);
  digitalWrite(IS_POS, HIGH);
  digitalWrite(IS_NEG, HIGH);
}

void loop() {
  // Ler instrucao (entrada)
  readInstructions();

  delay(10);

  unsigned long currentMillis = millis();
  // Envia os dados de amostragem para o computador
  if ((double) (currentMillis - previousMillis) / 1000.0 >= interval) {
    previousMillis = currentMillis;
    sendSamplingData();
  }

  // Executa instrucao (saida)
  executeInstructions();
}
