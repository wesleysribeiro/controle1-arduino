/*
  Programa desenvolvido para a disciplina de Sistemas de Controle I
*/

const int INPUT_PIN = A3;
const int OUTPUT_PIN = 11;

const int INPUT_BUFFER_SIZE = 11;

double current_value = 0;
unsigned long previousMillis = 0;
unsigned long counter = 1;


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
            //Serial.print("Signal read:");
            //Serial.println(signalRead);
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

// Dados falsos para teste
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
  }
  else if (current_signal == UpRamp) {
    current_value += 0.001;

    if(current_value >= output_value)
    {
      current_value = 0;
    }
  }
  else if (current_signal == DownRamp) {
    current_value -= 0.001;

    if(current_value <= output_value)
    {
      current_value = 0;
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
}

void loop() {
  // Ler instrucao (entrada)
  readInstructions();

  unsigned long currentMillis = millis();
  // Envia os dados de amostragem para o computador
  if ((double) (currentMillis - previousMillis) / 1000.0 >= interval) {
    previousMillis = currentMillis;
    sendSamplingData();
  }
  
  // Executa instrucao (saida)
  executeInstructions();
}
