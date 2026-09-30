const byte AFsentido1 = 40, AFsentido2 = 41, AFenable =  12, BFsentido1 = 10, BFsentido2 = 11, BFenable = 13;
const byte ATsentido1 = 37, ATsentido2 = 35 , ATenable =  3, BTsentido1 = 33, BTsentido2 = 31, BTenable = 2;
const byte sensorEsq = 4, sensorDir = 5; //adicionar numeros
byte esq, dir, i=0;

const byte S0  = 49;
const byte S1  = 51;
const byte S2  = 53;
const byte S3  = 48;
const byte OUT = 50;

// Variáveis das leituras
unsigned long red;
unsigned long green;
unsigned long blue;

const byte trigger = 45;
const byte echo = 47;
float duration, distance;

void setup() {
  ///////////////////////////
  //sensor cor
  // Pinos de controle do TCS3200
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  // Saída do sensor
  pinMode(OUT, INPUT);

  // Comunicação Serial
  Serial.begin(115200);

  // Configura a escala de frequência do TCS3200
  // S0 = HIGH
  // S1 = LOW
  // Frequência de saída = 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.println("TCS3200 iniciado!");
  Serial.println("-----------------------------");

  ///////////////////////////
  //ultrasom
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);


  Serial.begin(9600);
  ///////////////////////////
  //sensor infravermelhos
  pinMode(sensorEsq, INPUT);
  pinMode(sensorDir, INPUT);

  ///////////////////////////
  //motor da frente
  pinMode(AFsentido1, OUTPUT);
  pinMode(AFsentido2, OUTPUT);
  
  pinMode(BFsentido1, OUTPUT);
  pinMode(BFsentido2, OUTPUT);
  
  pinMode(AFenable, OUTPUT);
  
  
  pinMode(BFenable, OUTPUT);


  ////////////////////////////
  //motor de trás
  pinMode(ATsentido1, OUTPUT);
  pinMode(ATsentido2, OUTPUT);
  
  pinMode(BTsentido1, OUTPUT);
  pinMode(BTsentido2, OUTPUT);

  pinMode(ATenable, OUTPUT);
  
  pinMode(BTenable, OUTPUT);

}

void medirCor() {

  // ----------------------------------------
  // VERMELHO
  // S2 = LOW
  // S3 = LOW
  // ----------------------------------------

  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);

  delay(20);

  red = pulseIn(OUT, LOW);

  Serial.print("RED = ");
  Serial.print(red);
  Serial.print(" | ");


  // ----------------------------------------
  // VERDE
  // S2 = HIGH
  // S3 = HIGH
  // ----------------------------------------

  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);

  delay(20);

  green = pulseIn(OUT, LOW);

  Serial.print("GREEN = ");
  Serial.print(green);
  Serial.print(" | ");

  


  // ----------------------------------------
  // AZUL
  // S2 = LOW
  // S3 = HIGH
  // ----------------------------------------

  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);

  delay(20);

  blue = pulseIn(OUT, LOW);

  Serial.print("BLUE = ");
  Serial.println(blue);
}

void frente(){
  digitalWrite(BFsentido1, LOW);
  digitalWrite(BFsentido2, HIGH);
  analogWrite(BFenable, 110); //100

  digitalWrite(AFsentido1, HIGH);
  digitalWrite(AFsentido2, LOW);
  analogWrite(AFenable, 110); //90

  digitalWrite(BTsentido1, LOW);
  digitalWrite(BTsentido2, HIGH);
  analogWrite(BTenable, 110); //90

  digitalWrite(ATsentido1, HIGH);
  digitalWrite(ATsentido2, LOW);
  analogWrite(ATenable, 110); //110
}

void re(){
  digitalWrite(BFsentido1, LOW);
  digitalWrite(BFsentido2, HIGH);
  analogWrite(BFenable, 100);

  digitalWrite(AFsentido1, HIGH);
  digitalWrite(AFsentido2, LOW);
  analogWrite(AFenable, 90);

  digitalWrite(BTsentido1, HIGH);
  digitalWrite(BTsentido2, LOW);
  analogWrite(BTenable, 100); //ok

  digitalWrite(ATsentido1, LOW);
  digitalWrite(ATsentido2, HIGH);
  analogWrite(ATenable, 100); //OK
}

void esquerda(){

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

void girarE(){
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

void direita(){
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

void girarD(){
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

void parar(){
  analogWrite(ATenable, 0);
  analogWrite(AFenable, 0);
  analogWrite(BTenable, 0);
  analogWrite(BFenable, 0);
}

void tudo(){
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

void som(){
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  duration = pulseIn(echo, HIGH);
  distance = (duration*.0343)/2;
  delay(100);

  if(distance<15){
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

    //le o coiso e vira 90 graus esq anda 90° dir e cabo

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


void loop() {

  esq = digitalRead(sensorEsq);
  dir = digitalRead(sensorDir);

  if(esq == LOW && dir == LOW){ //BRANCO BRANCO | RETO
    frente();
    Serial.println("frente");
  }else if((esq == LOW)&&(dir == HIGH)){ // BRANCO PRETO | CURVA PRA DIREITA
    direita();
    Serial.println("direita");
  }else if(esq == HIGH && dir == LOW){ // PRETO BRANCO | CURVA PRA ESQUERDA
    esquerda();
    Serial.println("esquerda");
  }else{
    parar();
    Serial.println("parar");
  }

  som();
  medirCor();

  if(green < 100){
    i++;
    if(i>=5){
      i=0;
      esquerda();
      delay(200);
    }
  }
  


  Serial.print("esq: ");
  Serial.print(esq);
  Serial.print(" dir: ");
  Serial.println(dir);

  //Serial.print("Distancia: ");
  //Serial.println(distance);
  
  delay(100);
} 