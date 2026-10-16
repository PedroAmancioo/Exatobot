// ============================================================
// Seguidor de linha com Algoritmo Genético ONLINE - versão feira de ciências
// Arduino Mega | pista fechada (volta completa)
//
// - Treino CONTÍNUO: os indivíduos se sucedem sem o robô parar
// - Se o robô perder a linha, ele procura sozinho (sem reposicionar)
// - Reta: acelera | Curva aberta: reduz | Curva fechada: reduz mais e gira
// - O AG evolui velocidades e o ajuste fino dos 4 motores
// - Botão: clique = alterna TREINO / APRESENTAÇÃO | segurar 2 s = recomeçar do zero
// - Serial em 115200 baud
// ============================================================
#include <EEPROM.h>

// ---------- PINOS ----------
const byte AFsentido1 = 40, AFsentido2 = 41, AFenable = 12;
const byte BFsentido1 = 10, BFsentido2 = 11, BFenable = 13;
const byte ATsentido1 = 37, ATsentido2 = 35, ATenable = 3;
const byte BTsentido1 = 33, BTsentido2 = 31, BTenable = 2;
const byte sensorEsq = 4, sensorDir = 5;   // LOW = branco, HIGH = preto
const byte botao = 22;                     // botão entre o pino 22 e o GND (INPUT_PULLUP)
const byte ledModo = 23;                   // LED opcional: aceso = APRESENTAÇÃO

// ---------- CONFIGURAÇÕES (ajuste aqui) ----------
const bool TESTE_HARDWARE = false;  // true = só testa sensores e sentido dos motores
const bool PLOTTER        = false;  // true = Serial Plotter mostra o gráfico de aprendizado
const bool USAR_EEPROM    = true;   // false = sempre começa do zero
const unsigned long TEMPO_AVAL = 20000; // duração de cada indivíduo (ms) - de preferência >= 1 volta
const unsigned long T_FECHADA  = 180;   // sensor preto por mais que isso (ms) = curva fechada
const unsigned long T_PERDIDO  = 2000;  // branco/branco por mais que isso (ms) = linha perdida
const unsigned long T_BUSCA    = 900;   // a cada quanto tempo a busca inverte o lado (ms)
const unsigned int  GERACOES_TREINO = 15; // após N gerações vai sozinho p/ APRESENTAÇÃO (0 = nunca)
const int V_MIN     = 60;   // velocidade mínima da rampa na reta
const int PWM_BUSCA = 170;  // PWM do giro de busca quando perde a linha

// ---------- GENES ----------
enum {
  G_VRETA,     // velocidade máxima na reta
  G_RAMPA,     // aceleração na reta (PWM por segundo)
  G_RETOMA,    // % da velocidade mantida ao sair de curva aberta
  G_VABERTA,   // PWM dos motores dominantes na curva aberta
  G_FABERTA,   // % de PWM dos motores internos na curva aberta
  G_VFECHADA,  // PWM na curva fechada
  G_FFECHADA,  // % dos motores internos na curva fechada (100 = gira no eixo)
  G_TAF, G_TBF, G_TAT, G_TBT,  // ajuste fino de cada motor
  N_GENES
};
const int16_t GMIN[N_GENES] = { 80,  20,  0,  90, 20, 100,  60, -25, -25, -25, -25};
const int16_t GMAX[N_GENES] = {220, 200, 90, 230, 90, 230, 100,  25,  25,  25,  25};

// ---------- POPULAÇÃO ----------
const byte N_POP = 6;
int16_t pop[N_POP][N_GENES];
float   fit[N_POP];
byte    ind = 0;
unsigned int geracao = 0;          // total (inclui o que veio da EEPROM)
unsigned int geracoesSessao = 0;   // só desta ligação
float   melhorFit = -999;
int16_t melhorGenes[N_GENES];

struct Salvo { byte magic; unsigned int geracao; float fit; int16_t genes[N_GENES]; };
const byte MAGIC = 0xA7;

// ---------- ESTADO ----------
enum Modo { TREINO, APRESENTACAO };
Modo modo = TREINO;
int16_t g[N_GENES];                // genes em uso agora (zerado = sem ajuste, usado no teste)
unsigned long tInicioAval, tUlt;
float fitness = 0;
byte  erros = 0;
float vAtual = V_MIN;
byte  estadoAnt = 0;               // 0 = reta, 1 = esq preto, 2 = dir preto, 3 = ambos
byte  ultimoLado = 2;
bool  fechada = false;
bool  perdido = false;
unsigned long tEntrada, tPerdido, tUltimoPreto;
float vEntrada = V_MIN;

// ============================================================
// MOTORES (mantém a polaridade original de cada motor)
// pwm > 0 = frente, pwm < 0 = ré
// ============================================================
void motorB(byte s1, byte s2, byte en, int pwm) {
  bool fw = pwm >= 0;
  digitalWrite(s1, fw ? LOW : HIGH);
  digitalWrite(s2, fw ? HIGH : LOW);
  analogWrite(en, constrain(abs(pwm), 0, 255));
}
void motorA(byte s1, byte s2, byte en, int pwm) {
  bool fw = pwm >= 0;
  digitalWrite(s1, fw ? HIGH : LOW);
  digitalWrite(s2, fw ? LOW : HIGH);
  analogWrite(en, constrain(abs(pwm), 0, 255));
}
int comTrim(int v, int trim) {
  if (v == 0) return 0;
  int m = constrain(abs(v) + trim, 0, 255);
  return v > 0 ? m : -m;
}
void mover(int af, int bf, int at, int bt) {
  motorA(AFsentido1, AFsentido2, AFenable, comTrim(af, g[G_TAF]));
  motorB(BFsentido1, BFsentido2, BFenable, comTrim(bf, g[G_TBF]));
  motorA(ATsentido1, ATsentido2, ATenable, comTrim(at, g[G_TAT]));
  motorB(BTsentido1, BTsentido2, BTenable, comTrim(bt, g[G_TBT]));
}
void parar() {
  analogWrite(AFenable, 0); analogWrite(BFenable, 0);
  analogWrite(ATenable, 0); analogWrite(BTenable, 0);
}
void frente(int v) { mover(v, v, v, v); }

// Mesmo mapeamento das suas funções esquerda()/girarE() e direita()/girarD()
void curvaEsq(int vDom, int fInt) {   // BF e AT em ré, BT e AF para frente
  int vi = vDom * fInt / 100;
  mover(vi, -vDom, -vDom, vi);
}
void curvaDir(int vDom, int fInt) {   // AF e BT em ré, BF e AT para frente
  int vi = vDom * fInt / 100;
  mover(-vDom, vi, vi, -vDom);
}

// ============================================================
// TESTE DE HARDWARE (TESTE_HARDWARE = true)
// ============================================================
void esperaLendo(unsigned long ms) {
  unsigned long t0 = millis();
  while (millis() - t0 < ms) {
    Serial.print("  esq="); Serial.print(digitalRead(sensorEsq));
    Serial.print(" dir="); Serial.println(digitalRead(sensorDir));
    delay(200);
  }
}
void testeHardware() {
  Serial.println("TESTE: ponha o robo com as rodas no ar ou sobre a pista.");
  Serial.println("Sensores: 0 = branco, 1 = preto. Passe um papel preto sob cada sensor.");
  while (true) {
    Serial.println("FRENTE (as 4 rodas devem ir para frente)");
    frente(100); esperaLendo(1500); parar(); delay(500);
    Serial.println("CURVA ESQUERDA aberta (o robo deve virar para a esquerda)");
    curvaEsq(150, 50); esperaLendo(1500); parar(); delay(500);
    Serial.println("CURVA DIREITA aberta (o robo deve virar para a direita)");
    curvaDir(150, 50); esperaLendo(1500); parar(); delay(500);
    Serial.println("GIRO NO EIXO ESQUERDA");
    curvaEsq(150, 100); esperaLendo(1500); parar(); delay(500);
    Serial.println("GIRO NO EIXO DIREITA");
    curvaDir(150, 100); esperaLendo(1500); parar(); delay(1000);
  }
}

// ============================================================
// ALGORITMO GENÉTICO
// ============================================================
void genesAleatorios(int16_t *x) {
  for (byte k = 0; k < N_GENES; k++) x[k] = random(GMIN[k], GMAX[k] + 1);
}

void mutar(int16_t *x, float taxa, byte prob) {
  for (byte k = 0; k < N_GENES; k++) {
    if (random(100) < prob) {
      float r = random(-1000, 1001) / 1000.0;
      x[k] = constrain((int)(x[k] + r * (GMAX[k] - GMIN[k]) * taxa), GMIN[k], GMAX[k]);
    }
  }
}

byte torneio() {
  byte melhor = random(N_POP);
  for (byte i = 0; i < 2; i++) {
    byte c = random(N_POP);
    if (fit[c] > fit[melhor]) melhor = c;
  }
  return melhor;
}

void salvarMelhor() {
  Salvo s;
  s.magic = MAGIC; s.geracao = geracao; s.fit = melhorFit;
  for (byte k = 0; k < N_GENES; k++) s.genes[k] = melhorGenes[k];
  EEPROM.put(0, s);
}

bool carregarMelhor() {
  Salvo s;
  EEPROM.get(0, s);
  if (s.magic != MAGIC) return false;
  for (byte k = 0; k < N_GENES; k++) melhorGenes[k] = constrain(s.genes[k], GMIN[k], GMAX[k]);
  geracao = s.geracao;
  melhorFit = s.fit;
  return true;
}

void novoIndividuo() {
  for (byte k = 0; k < N_GENES; k++) g[k] = pop[ind][k];
  fitness = 0; erros = 0;
  tInicioAval = millis();
}

void evoluir() {
  byte ordem[N_POP];
  for (byte i = 0; i < N_POP; i++) ordem[i] = i;
  for (byte i = 0; i < N_POP - 1; i++)
    for (byte j = 0; j < N_POP - 1 - i; j++)
      if (fit[ordem[j]] < fit[ordem[j + 1]]) { byte t = ordem[j]; ordem[j] = ordem[j + 1]; ordem[j + 1] = t; }

  float media = 0;
  for (byte i = 0; i < N_POP; i++) media += fit[i];
  media /= N_POP;

  // o recorde "esquece" um pouco a cada geração, para uma avaliação com sorte não ficar para sempre
  if (melhorFit > -998) melhorFit -= 0.03 * fabs(melhorFit);
  if (fit[ordem[0]] > melhorFit) {
    melhorFit = fit[ordem[0]];
    for (byte k = 0; k < N_GENES; k++) melhorGenes[k] = pop[ordem[0]][k];
    salvarMelhor();
  }

  if (PLOTTER) {
    Serial.print("melhor:"); Serial.print(fit[ordem[0]], 2);
    Serial.print(",media:"); Serial.println(media, 2);
  } else {
    Serial.print("=== GERACAO "); Serial.print(geracao);
    Serial.print(" | melhor: "); Serial.print(fit[ordem[0]], 2);
    Serial.print(" | media: "); Serial.print(media, 2);
    Serial.print(" | recorde: "); Serial.println(melhorFit, 2);
  }

  // mutação diminui com as gerações: explora no começo, refina no fim
  float taxa = 0.25;
  for (unsigned int i = 0; i < geracao && taxa > 0.04; i++) taxa *= 0.92;

  int16_t nova[N_POP][N_GENES];
  for (byte e = 0; e < 2; e++)                       // elitismo: 2 melhores passam direto
    for (byte k = 0; k < N_GENES; k++) nova[e][k] = pop[ordem[e]][k];

  for (byte i = 2; i < N_POP; i++) {                 // torneio + cruzamento + mutação
    byte a = torneio(), b = torneio();
    for (byte k = 0; k < N_GENES; k++) {
      float alpha = random(-20, 121) / 100.0;
      int v = pop[a][k] + (int)((pop[b][k] - pop[a][k]) * alpha);
      nova[i][k] = constrain(v, GMIN[k], GMAX[k]);
    }
    mutar(nova[i], taxa, 30);
  }
  for (byte i = 0; i < N_POP; i++)
    for (byte k = 0; k < N_GENES; k++) pop[i][k] = nova[i][k];

  geracao++;
  geracoesSessao++;
  ind = 0;
}

// ============================================================
// MODOS
// ============================================================
void entrarTreino() {
  modo = TREINO;
  digitalWrite(ledModo, LOW);
  novoIndividuo();
  Serial.println(">>> MODO TREINO");
}

void entrarApresentacao() {
  modo = APRESENTACAO;
  digitalWrite(ledModo, HIGH);
  if (melhorFit > -998) for (byte k = 0; k < N_GENES; k++) g[k] = melhorGenes[k];
  Serial.println(">>> MODO APRESENTACAO (melhor individuo)");
}

void reiniciarDoZero() {
  EEPROM.update(0, 0);                 // apaga o melhor salvo
  for (byte i = 0; i < N_POP; i++) genesAleatorios(pop[i]);
  geracao = 0; geracoesSessao = 0; melhorFit = -999; ind = 0;
  Serial.println(">>> RECOMECANDO DO ZERO");
  entrarTreino();
}

void terminarIndividuo() {
  fit[ind] = fitness;
  if (!PLOTTER) {
    Serial.print("Gen "); Serial.print(geracao);
    Serial.print(" Ind "); Serial.print(ind);
    Serial.print(" fit="); Serial.print(fitness, 2);
    Serial.print(" erros="); Serial.println(erros);
  }
  ind++;
  if (ind >= N_POP) {
    evoluir();
    if (GERACOES_TREINO > 0 && geracoesSessao >= GERACOES_TREINO) {
      entrarApresentacao();
      return;
    }
  }
  novoIndividuo();   // sem parar: o próximo indivíduo assume de onde o robô está
}

// ============================================================
// CONTROLE DO ROBÔ (roda o tempo todo, nunca para)
// ============================================================
void rodarRobo(float dt, unsigned long agora, bool contaFit) {
  byte esq = digitalRead(sensorEsq);
  byte dir = digitalRead(sensorDir);
  byte estado = (esq == LOW && dir == LOW) ? 0 : (esq == HIGH && dir == LOW) ? 1 : (esq == LOW && dir == HIGH) ? 2 : 3;

  if (estado == 0) {
    // ----- RETA (ou linha perdida) -----
    if (estadoAnt != 0) {   // acabou de sair de uma curva
      if (fechada) vAtual = V_MIN;
      else vAtual = max((float)V_MIN, vEntrada * g[G_RETOMA] / 100.0);
    }
    if (agora - tUltimoPreto > T_PERDIDO) {
      // reta longa demais para a pista: provavelmente perdeu a linha -> procura girando
      if (!perdido) {
        perdido = true; tPerdido = agora; vAtual = V_MIN;
        if (contaFit) { fitness -= 2.0; erros++; }
      }
      byte lado = ultimoLado;
      if (((agora - tPerdido) / T_BUSCA) % 2) lado = (lado == 1) ? 2 : 1;
      if (lado == 1) curvaEsq(PWM_BUSCA, 100); else curvaDir(PWM_BUSCA, 100);
    } else {
      vAtual += g[G_RAMPA] * dt;
      if (vAtual > g[G_VRETA]) vAtual = g[G_VRETA];
      frente((int)vAtual);
      if (contaFit) fitness += (vAtual / 255.0) * dt;
    }
  } else {
    // ----- CURVA -----
    tUltimoPreto = agora;
    perdido = false;
    if (estadoAnt == 0) {            // acabou de entrar na curva
      tEntrada = agora; vEntrada = vAtual; fechada = false;
      if (contaFit) { erros++; fitness -= 0.3; }
    }
    if (!fechada && (estado == 3 || agora - tEntrada > T_FECHADA)) {
      fechada = true;
      if (contaFit) fitness -= (estado == 3) ? 1.0 : 0.6;
    }
    byte lado = (estado == 3) ? ultimoLado : estado;
    if (estado != 3) ultimoLado = estado;

    int v = fechada ? g[G_VFECHADA] : g[G_VABERTA];
    int f = fechada ? g[G_FFECHADA] : g[G_FABERTA];
    if (lado == 1) curvaEsq(v, f); else curvaDir(v, f);
    if (contaFit) fitness += (v / 255.0) * dt * (fechada ? 0.2 : 0.5);
  }
  estadoAnt = estado;
}

// ============================================================
// BOTÃO: clique = alterna modo | segurar 2 s = recomeçar do zero
// ============================================================
void lerBotao(unsigned long agora) {
  static bool antes = false, longoFeito = false;
  static unsigned long tPress = 0;
  bool press = (digitalRead(botao) == LOW);
  if (press && !antes) { tPress = agora; longoFeito = false; }
  if (press && !longoFeito && agora - tPress > 2000) { longoFeito = true; reiniciarDoZero(); }
  if (!press && antes && !longoFeito && agora - tPress > 40) {
    if (modo == TREINO) entrarApresentacao(); else entrarTreino();
  }
  antes = press;
}

// ============================================================
void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(A0));

  pinMode(sensorEsq, INPUT);
  pinMode(sensorDir, INPUT);
  pinMode(botao, INPUT_PULLUP);
  pinMode(ledModo, OUTPUT);
  byte pinos[] = {AFsentido1, AFsentido2, AFenable, BFsentido1, BFsentido2, BFenable,
                  ATsentido1, ATsentido2, ATenable, BTsentido1, BTsentido2, BTenable};
  for (byte i = 0; i < sizeof(pinos); i++) pinMode(pinos[i], OUTPUT);
  parar();

  if (TESTE_HARDWARE) testeHardware();   // nunca sai daqui

  bool tem = USAR_EEPROM && carregarMelhor();
  for (byte i = 0; i < N_POP; i++) {
    if (tem) {
      for (byte k = 0; k < N_GENES; k++) pop[i][k] = melhorGenes[k];
      if (i > 0) mutar(pop[i], 0.12, 40);
    } else {
      genesAleatorios(pop[i]);
    }
  }
  if (!tem) { geracao = 0; melhorFit = -999; }
  Serial.println(tem ? "Melhor carregado da EEPROM" : "Populacao aleatoria");

  delay(3000);                       // tempo para colocar o robô na pista
  tUlt = millis();
  tUltimoPreto = tUlt;
  entrarTreino();
}

void loop() {
  unsigned long agora = millis();
  float dt = (agora - tUlt) / 1000.0;
  tUlt = agora;

  lerBotao(agora);
  rodarRobo(dt, agora, modo == TREINO);
  if (modo == TREINO && agora - tInicioAval >= TEMPO_AVAL) terminarIndividuo();
}
