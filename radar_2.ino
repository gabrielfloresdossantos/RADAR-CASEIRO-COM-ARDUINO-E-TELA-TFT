#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <Servo.h>

MCUFRIEND_kbv tft;
Servo meuServo;

const int servoPin = 22;
const int trigPin = 24;
const int echoPin = 26;

#define PRETO    0x0000
#define VERDE    0x07E0
#define VERMELHO 0xF800

// --- PARÂMETRO DE VELOCIDADE ---
// 15 a 30 é o ideal para dar estabilidade mecânica ao servo
int velocidadeRadar = 15; 
// -------------------------------

int xCenter = 160;
int yCenter = 240;
int radius = 200;

// Memória voltou para 180 posições (1 grau = 1 posição)
int distancias[180]; 

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  meuServo.attach(servoPin);

  tft.reset();
  tft.begin(0x9341); 
  tft.setRotation(1);
  tft.fillScreen(PRETO);
  
  for(int i = 0; i < 180; i++) {
    distancias[i] = 0;
  }
  
  desenharArcosBase();
}

void desenharArcosBase() {
  tft.drawCircle(xCenter, yCenter, 50, VERDE);
  tft.drawCircle(xCenter, yCenter, 100, VERDE);
  tft.drawCircle(xCenter, yCenter, 150, VERDE);
  tft.drawCircle(xCenter, yCenter, 200, VERDE);
}

void loop() {
  // Varre de 15 a 165 graus de 1 em 1
  for(int i = 15; i <= 165; i++) {
    processarAngulo(i);
  }
  desenharArcosBase(); 
  
  for(int i = 165; i >= 15; i--) {
    processarAngulo(i);
  }
  desenharArcosBase();
}

void processarAngulo(int angulo) {
  // Manda o motor para o grau exato, de forma firme
  meuServo.write(angulo);
  
  float anguloRad = angulo * PI / 180.0;
  int xMax = xCenter + radius * cos(anguloRad);
  int yMax = yCenter - radius * sin(anguloRad);

  // 1. Apaga ponto antigo
  int distAntiga = distancias[angulo];
  if (distAntiga > 0 && distAntiga < 40) {
    int distTelaAntiga = map(distAntiga, 0, 40, 0, radius);
    int xAntigo = xCenter + distTelaAntiga * cos(anguloRad);
    int yAntigo = yCenter - distTelaAntiga * sin(anguloRad);
    tft.fillCircle(xAntigo, yAntigo, 5, PRETO); 
  }

  // 2. Lê nova distância
  int distancia = calcularDistancia();
  if (distancia > 0 && distancia < 40) {
    distancias[angulo] = distancia;
  } else {
    distancias[angulo] = 0;
  }

  // 3. Desenha feixe de varredura (Linha Verde)
  tft.drawLine(xCenter, yCenter, xMax, yMax, VERDE);

  // 4. Desenha o novo ponto vermelho se encontrar algo
  bool temObjeto = (distancias[angulo] > 0);
  int xObj = 0, yObj = 0;
  if(temObjeto) {
    int distTela = map(distancias[angulo], 0, 40, 0, radius);
    xObj = xCenter + distTela * cos(anguloRad);
    yObj = yCenter - distTela * sin(anguloRad);
    tft.fillCircle(xObj, yObj, 5, VERMELHO);
  }

  // Pausa para estabilizar o movimento físico
  if (velocidadeRadar > 0) {
    delay(velocidadeRadar); 
  }

  // 5. Apaga o feixe (Linha Preta)
  tft.drawLine(xCenter, yCenter, xMax, yMax, PRETO);

  // 6. Remenda os buracos dos arcos de fundo
  for (int r = 50; r <= 200; r += 50) {
    int xRemendo = xCenter + r * cos(anguloRad);
    int yRemendo = yCenter - r * sin(anguloRad);
    tft.fillRect(xRemendo - 1, yRemendo - 1, 3, 3, VERDE); 
  }

  // 7. Mantém o ponto vermelho por cima do apagão
  if(temObjeto) {
    tft.fillCircle(xObj, yObj, 5, VERMELHO);
  }
}

int calcularDistancia() {
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duracao = pulseIn(echoPin, HIGH, 5000); 
  if (duracao == 0) return 0;
  
  return duracao * 0.034 / 2;
}
