// =====================================================================
//  ROBÔ SEGUIDOR DE LINHA  +  MODO TESTE (via Serial Monitor)
//  Serial Monitor: 115200 baud, "Nenhum final de linha" (No line ending)
//
//  Como usar:
//    - Envie  T  -> entra no MODO TESTE (robô para)
//    - Envie  X  -> sai do modo teste e volta a seguir a linha
//    - Dentro do modo teste, envie  ?  para ver o menu de comandos
// =====================================================================

const byte AFsentido1 = 40, AFsentido2 = 41, AFenable = 12, BFsentido1 = 10, BFsentido2 = 11, BFenable = 13;
const byte ATsentido1 = 37, ATsentido2 = 35, ATenable = 3,  BTsentido1 = 33, BTsentido2 = 31, BTenable = 2;
const byte sensorEsq = 4, sensorDir = 5;
byte esq, dir, i = 0;

const byte velo = 75;

const byte S0  = 49;
const byte S1  = 51;
const byte S2  = 53;
const byte S3  = 48;
const byte OUT = 50;

unsigned long red;
unsigned long green;
unsigned long blue;

const byte trigger = 45;
const byte echo = 47;
float duration, distance;

// ---------- MODO TESTE ----------
bool modoTeste = false;
const unsigned int TEMPO_TESTE = 1000;   // quanto tempo cada movimento roda (ms)
const byte PWM_TESTE_MOTOR = 120;        // PWM usado no teste de motor individual

void setup() {
  // Sensor de cor
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  // Serial (apenas UMA vez, senão o baud muda no meio)
  Serial.begin(115200);

  // Ultrassom
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);

  // Infravermelhos
  pinMode(sensorEsq, INPUT);
  pinMode(sensorDir, INPUT);

  // Motores da frente
  pinMode(AFsentido1, OUTPUT);
  pinMode(AFsentido2, OUTPUT);
  pinMode(BFsentido1, OUTPUT);
  pinMode(BFsentido2, OUTPUT);
  pinMode(AFenable, OUTPUT);
  pinMode(BFenable, OUTPUT);

  // Motores de trás
  pinMode(ATsentido1, OUTPUT);
  pinMode(ATsentido2, OUTPUT);
  pinMode(BTsentido1, OUTPUT);
  pinMode(BTsentido2, OUTPUT);
  pinMode(ATenable, OUTPUT);
  pinMode(BTenable, OUTPUT);

  parar();
  Serial.println("Robo iniciado! Envie T para entrar no modo teste.");
}

// =====================================================================
//  SENSORES
// =====================================================================
void medirCor() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  delay(20);
  red = pulseIn(OUT, LOW);
  Serial.print("RED = ");
  Serial.print(red);
  Serial.print(" | ");

  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  delay(20);
  green = pulseIn(OUT, LOW);
  Serial.print("GREEN = ");
  Serial.print(green);
  Serial.print(" | ");

  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  delay(20);
  blue = pulseIn(OUT, LOW);
  Serial.print("BLUE = ");
  Serial.println(blue);
}

// Só lê a distância (sem executar a manobra de desvio)
float lerDistancia() {
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  duration = pulseIn(echo, HIGH, 30000);   // timeout de 30 ms
  return (duration * .0343) / 2;
}

// =====================================================================
//  MOVIMENTOS (originais)
// =====================================================================
void frente() {
  digitalWrite(BFsentido1, LOW);
  digitalWrite(BFsentido2, HIGH);
  analogWrite(BFenable, velo);

  digitalWrite(AFsentido1, HIGH);
  digitalWrite(AFsentido2, LOW);
  analogWrite(AFenable, velo);

  digitalWrite(BTsentido1, LOW);
  digitalWrite(BTsentido2, HIGH);
  analogWrite(BTenable, velo);

  digitalWrite(ATsentido1, HIGH);
  digitalWrite(ATsentido2, LOW);
  analogWrite(ATenable, velo);
}

void re() {
  digitalWrite(BFsentido1, LOW);
  digitalWrite(BFsentido2, HIGH);
  analogWrite(BFenable, 100);

  digitalWrite(AFsentido1, HIGH);
  digitalWrite(AFsentido2, LOW);
  analogWrite(AFenable, 90);

  digitalWrite(BTsentido1, HIGH);
  digitalWrite(BTsentido2, LOW);
  analogWrite(BTenable, 100);

  digitalWrite(ATsentido1, LOW);
  digitalWrite(ATsentido2, HIGH);
  analogWrite(ATenable, 100);
}

void esquerda() {
  digitalWrite(BFsentido1, HIGH);
  digitalWrite(BFsentido2, LOW);
  analogWrite(BFenable, 240);

  digitalWrite(BTsentido1, LOW);
  digitalWrite(BTsentido2, HIGH);
  analogWrite(BTenable, 120);

  digitalWrite(AFsentido1, HIGH);
  digitalWrite(AFsentido2, LOW);
  analogWrite(AFenable, 110);

  digitalWrite(ATsentido1, LOW);
  digitalWrite(ATsentido2, HIGH);
  analogWrite(ATenable, 240);

  delay(100);
}

void girarE() {
  digitalWrite(BFsentido1, HIGH);
  digitalWrite(BFsentido2, LOW);
  analogWrite(BFenable, 200);

  digitalWrite(BTsentido1, LOW);
  digitalWrite(BTsentido2, HIGH);
  analogWrite(BTenable, 220);

  digitalWrite(AFsentido1, LOW);
  digitalWrite(AFsentido2, HIGH);
  analogWrite(AFenable, 220);

  digitalWrite(ATsentido1, LOW);
  digitalWrite(ATsentido2, HIGH);
  analogWrite(ATenable, 200);

  delay(100);
}

void direita() {
  digitalWrite(AFsentido1, LOW);
  digitalWrite(AFsentido2, HIGH);

  digitalWrite(ATsentido1, HIGH);
  digitalWrite(ATsentido2, LOW);

  analogWrite(AFenable, 240);
  analogWrite(ATenable, 110);

  digitalWrite(BFsentido1, LOW);
  digitalWrite(BFsentido2, HIGH);
  analogWrite(BFenable, 110);

  digitalWrite(BTsentido1, HIGH);
  digitalWrite(BTsentido2, LOW);
  analogWrite(BTenable, 255);

  delay(100);
}

void girarD() {
  digitalWrite(AFsentido1, HIGH);
  digitalWrite(AFsentido2, LOW);

  digitalWrite(ATsentido1, HIGH);
  digitalWrite(ATsentido2, LOW);

  analogWrite(AFenable, 200);
  analogWrite(ATenable, 200);

  digitalWrite(BFsentido1, LOW);
  digitalWrite(BFsentido2, HIGH);
  analogWrite(BFenable, 200);

  digitalWrite(BTsentido1, HIGH);
  digitalWrite(BTsentido2, LOW);
  analogWrite(BTenable, 200);

  delay(100);
}

void parar() {
  analogWrite(ATenable, 0);
  analogWrite(AFenable, 0);
  analogWrite(BTenable, 0);
  analogWrite(BFenable, 0);
}

void tudo() {
  digitalWrite(BFsentido1, HIGH);
  digitalWrite(BFsentido2, LOW);

  digitalWrite(AFsentido1, LOW);
  digitalWrite(AFsentido2, HIGH);

  digitalWrite(BTsentido1, LOW);
  digitalWrite(BTsentido2, HIGH);

  digitalWrite(ATsentido1, HIGH);
  digitalWrite(ATsentido2, LOW);

  analogWrite(BFenable, 255);
  analogWrite(AFenable, 255);
  analogWrite(BTenable, 255);
  analogWrite(ATenable, 255);
}

void som() {
  distance = lerDistancia();
  delay(100);

  if (distance < 15) {
    parar();
    delay(20);
    re();
    delay(00);
    esquerda();
    delay(900);
    parar();
    delay(20);

    frente();
    delay(1500);

    parar();
    delay(20);

    re();
    delay(20);
    direita();
    delay(800);
    parar();

    delay(1000);

    frente();
    delay(1850);
    parar();
    delay(20);

    re();
    delay(20);
    direita();
    delay(700);
    parar();

    frente();
    delay(1350);
    parar();
    delay(20);

    re();
    delay(20);
    esquerda();
    delay(950);
    parar();
    delay(2000);
  }
}

// =====================================================================
//  MODO TESTE
// =====================================================================

// Aciona UM motor sozinho. sentido: +1 = "pra frente", -1 = "pra trás", 0 = para
// (o "pra frente" de cada motor segue exatamente a mesma direção usada em frente())
void motorIndividual(byte s1, byte s2, byte en, bool invertido, int sentido, byte pwm) {
  if (sentido == 0) {
    analogWrite(en, 0);
    return;
  }
  bool horario = (sentido > 0) ? !invertido : invertido;
  digitalWrite(s1, horario ? HIGH : LOW);
  digitalWrite(s2, horario ? LOW : HIGH);
  analogWrite(en, pwm);
}

// Cada motor: "invertido" = true quando em frente() ele usa (LOW, HIGH)
void testeMotor(char nome1, char nome2, int sentido) {
  parar();
  if (nome1 == 'A' && nome2 == 'F') {
    Serial.print("Motor AF ");
    motorIndividual(AFsentido1, AFsentido2, AFenable, false, sentido, PWM_TESTE_MOTOR);
  } else if (nome1 == 'B' && nome2 == 'F') {
    Serial.print("Motor BF ");
    motorIndividual(BFsentido1, BFsentido2, BFenable, true, sentido, PWM_TESTE_MOTOR);
  } else if (nome1 == 'A' && nome2 == 'T') {
    Serial.print("Motor AT ");
    motorIndividual(ATsentido1, ATsentido2, ATenable, false, sentido, PWM_TESTE_MOTOR);
  } else if (nome1 == 'B' && nome2 == 'T') {
    Serial.print("Motor BT ");
    motorIndividual(BTsentido1, BTsentido2, BTenable, true, sentido, PWM_TESTE_MOTOR);
  }
  Serial.println(sentido > 0 ? "-> frente" : "-> tras");
  delay(TEMPO_TESTE);
  parar();
}

// Roda um movimento por TEMPO_TESTE ms e para.
// Usa repetição porque esquerda/direita/girar têm delay(100) dentro.
void executarMovimento(void (*mov)(), const char *nome) {
  Serial.print("Testando: ");
  Serial.println(nome);
  unsigned long ini = millis();
  while (millis() - ini < TEMPO_TESTE) {
    mov();
  }
  parar();
  Serial.println("ok - parado");
}

void menuTeste() {
  Serial.println();
  Serial.println("=========== MODO TESTE ===========");
  Serial.println(" MOVIMENTOS (cada um roda 1 s e para)");
  Serial.println("  f = frente        r = re");
  Serial.println("  e = esquerda      d = direita");
  Serial.println("  E = girar esq     D = girar dir");
  Serial.println("  t = tudo (255)    p = parar");
  Serial.println(" MOTOR INDIVIDUAL (1 = frente, 2 = tras)");
  Serial.println("  q/w = AF 1/2      a/s = BF 1/2");
  Serial.println("  z/x = AT 1/2      c/v = BT 1/2");
  Serial.println(" SENSORES");
  Serial.println("  i = infravermelhos (esq/dir)");
  Serial.println("  u = ultrassom (distancia)");
  Serial.println("  k = sensor de cor (RGB)");
  Serial.println("  m = monitor continuo de todos os sensores (qualquer tecla para sair)");
  Serial.println(" SEQUENCIAS");
  Serial.println("  g = sequencia automatica (todos os movimentos)");
  Serial.println("  o = manobra de desvio de obstaculo (som)");
  Serial.println(" OUTROS");
  Serial.println("  ? = menu     X = sair do modo teste");
  Serial.println("==================================");
}

void lerInfra() {
  Serial.print("IR esq: ");
  Serial.print(digitalRead(sensorEsq));
  Serial.print(" | IR dir: ");
  Serial.println(digitalRead(sensorDir));
}

void lerUltra() {
  float d = lerDistancia();
  Serial.print("Distancia: ");
  if (duration == 0) Serial.println("sem eco (fora de alcance)");
  else {
    Serial.print(d);
    Serial.println(" cm");
  }
}

void monitorSensores() {
  Serial.println("Monitor continuo... envie qualquer tecla para sair.");
  while (Serial.available()) Serial.read();   // limpa buffer
  while (!Serial.available()) {
    lerInfra();
    lerUltra();
    medirCor();
    Serial.println("-----");
    delay(300);
  }
  while (Serial.available()) Serial.read();
  Serial.println("Monitor encerrado.");
}

void sequenciaAutomatica() {
  Serial.println("Sequencia automatica iniciada...");
  executarMovimento(frente, "frente");   delay(500);
  executarMovimento(re, "re");           delay(500);
  executarMovimento(esquerda, "esquerda"); delay(500);
  executarMovimento(direita, "direita"); delay(500);
  executarMovimento(girarE, "girar esquerda"); delay(500);
  executarMovimento(girarD, "girar direita");  delay(500);
  Serial.println("Sequencia finalizada.");
}

void loopTeste() {
  if (!Serial.available()) return;
  char cmd = Serial.read();
  if (cmd == '\n' || cmd == '\r' || cmd == ' ') return;

  switch (cmd) {
    case 'f': executarMovimento(frente, "frente"); break;
    case 'r': executarMovimento(re, "re"); break;
    case 'e': executarMovimento(esquerda, "esquerda"); break;
    case 'd': executarMovimento(direita, "direita"); break;
    case 'E': executarMovimento(girarE, "girar esquerda"); break;
    case 'D': executarMovimento(girarD, "girar direita"); break;
    case 't': executarMovimento(tudo, "tudo (PWM 255)"); break;
    case 'p': parar(); Serial.println("parado"); break;

    case 'q': testeMotor('A', 'F', +1); break;
    case 'w': testeMotor('A', 'F', -1); break;
    case 'a': testeMotor('B', 'F', +1); break;
    case 's': testeMotor('B', 'F', -1); break;
    case 'z': testeMotor('A', 'T', +1); break;
    case 'x': testeMotor('A', 'T', -1); break;
    case 'c': testeMotor('B', 'T', +1); break;
    case 'v': testeMotor('B', 'T', -1); break;

    case 'i': lerInfra(); break;
    case 'u': lerUltra(); break;
    case 'k': medirCor(); break;
    case 'm': monitorSensores(); break;

    case 'g': sequenciaAutomatica(); break;
    case 'o':
      Serial.println("Manobra de desvio (robo vai andar bastante!)...");
      // mesma sequência do som(), sem depender do sensor:
      parar(); delay(20);
      re(); esquerda(); delay(900); parar(); delay(20);
      frente(); delay(1500); parar(); delay(20);
      re(); delay(20); direita(); delay(800); parar(); delay(1000);
      frente(); delay(1850); parar(); delay(20);
      re(); delay(20); direita(); delay(700); parar();
      frente(); delay(1350); parar(); delay(20);
      re(); delay(20); esquerda(); delay(950); parar();
      Serial.println("Manobra concluida.");
      break;

    case '?': menuTeste(); break;

    case 'X': break;   // tratado logo abaixo
    default:
      Serial.print("Comando invalido: ");
      Serial.println(cmd);
      break;
  }

  // 'X' maiúsculo sai do modo teste
  if (cmd == 'X') {
    parar();
    modoTeste = false;
    Serial.println(">>> Saindo do modo teste. Seguindo a linha...");
  }
}

// =====================================================================
//  LOOP PRINCIPAL
// =====================================================================
void loop() {

  // Entrada no modo teste a qualquer momento
  if (!modoTeste && Serial.available()) {
    char c = Serial.read();
    if (c == 'T') {
      modoTeste = true;
      parar();
      Serial.println(">>> MODO TESTE ATIVADO <<<");
      menuTeste();
    }
  }

  if (modoTeste) {
    loopTeste();
    return;   // não executa o seguidor de linha enquanto testa
  }

  // ------------------ SEGUIDOR DE LINHA (original) ------------------
  esq = digitalRead(sensorEsq);
  dir = digitalRead(sensorDir);

  if (esq == LOW && dir == LOW) {            // BRANCO BRANCO | RETO
    frente();
    Serial.println("frente");
  } else if ((esq == LOW) && (dir == HIGH)) { // BRANCO PRETO | CURVA PRA DIREITA
    direita();
    Serial.println("direita");
  } else if (esq == HIGH && dir == LOW) {     // PRETO BRANCO | CURVA PRA ESQUERDA
    esquerda();
    Serial.println("esquerda");
  } else {
    parar();
    Serial.println("parar");
  }

  //som();
  medirCor();

  if (red > 40) {
    i++;
    if (i >= 3) {
      i = 0;
      parar();
      delay(800);
      frente();
      delay(300);
      parar();
      delay(20);
      esquerda();
      delay(800);
      parar();
      delay(20);
      frente();
      delay(300);
      Serial.println("ESQUERDAAAAAAAAAAAA SENSOR");
    }
  }

  Serial.print("esq: ");
  Serial.print(esq);
  Serial.print(" dir: ");
  Serial.println(dir);

  delay(10);
}
