#include <ESP32Servo.h>
#include <Arduino.h>

Servo queue;
Servo gueule;  

#define PWM_freq 20000
#define PWM_res 8

#define CH_A 0
#define CH_B 1

#define IN1   4
#define IN2   5
#define IN3   6
#define IN4   7
#define EEP   15

#define LED_PIN 12

// Capteurs infrarouge (0 = noir détecté, 1 = blanc)
#define GG 18  // Gauche extrême
#define G  8   // Gauche
#define C  3   // Centre
#define D  46  // Droite
#define DD 9   // Droite extrême

// Vitesses
#define VITESSE_BASE 210
#define DELTA_LEGER  20
#define DELTA_FORT   45

void setup() 
{
  Serial.begin(115200);
  delay(50);

  queue.setPeriodHertz(50);
  gueule.setPeriodHertz(50);
  queue.attach(16, 500, 2500);
  gueule.attach(17, 500, 2500);
  queue.write(60);
  gueule.write(70);
  
  ledcAttach(IN1, PWM_freq, PWM_res);
  ledcAttach(IN2, PWM_freq, PWM_res);
  ledcAttach(IN3, PWM_freq, PWM_res);
  ledcAttach(IN4, PWM_freq, PWM_res);

  stopMotors();
  pinMode(EEP, OUTPUT);

  ledcAttach(CH_A, PWM_freq, PWM_res);
  ledcAttach(CH_B, PWM_freq, PWM_res);
  digitalWrite(EEP, HIGH);

  pinMode(GG, INPUT);
  pinMode(G, INPUT);
  pinMode(C, INPUT);
  pinMode(D, INPUT);
  pinMode(DD, INPUT);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() 
{
  delay(2500); // Temps pour placer le robot

  // Suivre la ligne jusqu'au premier cercle rouge
  suivreLigne(2500);
  avancer(215);
  delay(1500);

  pivoterGauche(200);
  delay(300);

  avancer(235);
  delay(300);

  stopMotors();
  drop_poulet();

  reculer(210);
  delay(200);

  pivoterGauche(200);
  delay(1000);

  stopMotors();
  delay(1000);

  led_clignotement(3);
  delay(500);

  pivoterDroite(200);
  delay(550);

  // Suivre la ligne jusqu'au deuxième cercle
  suivreLigne(4000);
  avancer(210);
  delay(2500);

  stopMotors();
  pivoterGauche(200);
  delay(1250);
  stopMotors();

  battement(3);
  delay(500);

  stopMotors();
  delay(5000);

  pivoterDroite(200);
  delay(550);

  // Suivre la ligne jusqu'au troisième cercle (BONUS)
  suivreLigne(1750);
  avancer(210);
  delay(2000);
  stopMotors();
  delay(1000);

  pivoterDroite(255);
  delay(1250);
  stopMotors();

  while(true); // Fin
}

//===========================================
// FONCTION DE SUIVI DE LIGNE
//===========================================
void suivreLigne(unsigned long dureeMax)
{
  unsigned long debut = millis();
  int correctionPrecedente = 0;

  while (millis() - debut < dureeMax)
  {
    bool gg = !digitalRead(GG);
    bool g  = !digitalRead(G);
    bool c  = !digitalRead(C);
    bool d  = !digitalRead(D);
    bool dd = !digitalRead(DD);

    Serial.printf("GG:%d G:%d C:%d D:%d DD:%d\n", gg, g, c, d, dd);

    // Calcul d'une erreur pondérée
    // Négatif = robot trop à droite → virer gauche
    // Positif = robot trop à gauche → virer droite
    int erreur = 0;
    if (dd)              erreur = -2;
    else if (d && !c)    erreur = -2;
    else if (d && c && !g) erreur = -1;
    else if (c && !g && !d) erreur = 0;  // Centré
    else if (g && c && !d) erreur =  1;
    else if (g && !c)    erreur =  2;
    else if (gg)         erreur =  2;
    else if (c)          erreur =  0;
    else                 erreur = correctionPrecedente; // Aucun capteur → garde la dernière direction

    correctionPrecedente = erreur;

    // Dosage de la correction selon l'erreur
    int delta = 0;
    if      (abs(erreur) == 1) delta = DELTA_LEGER;
    else if (abs(erreur) == 2) delta = DELTA_FORT;

    int vitG = VITESSE_BASE;
    int vitD = VITESSE_BASE;

    if (erreur < 0) { vitG += delta; vitD -= delta; } // Trop à droite → virer gauche
    if (erreur > 0) { vitG -= delta; vitD += delta; } // Trop à gauche → virer droite

    vitG = constrain(vitG, 0, 255);
    vitD = constrain(vitD, 0, 255);

    ledcWrite(IN1, vitG);
    ledcWrite(IN2, 0);
    ledcWrite(IN3, vitD);
    ledcWrite(IN4, 0);

    delay(10);
  }
}

//===========================================
// FONCTIONS DE MOUVEMENT
// Convention : (vitesse_moteur_gauche, vitesse_moteur_droit)
//===========================================
void tournerGauche(int vitesseMoteurGauche, int vitesseMoteurDroit)
{
  ledcWrite(IN1, vitesseMoteurGauche);
  ledcWrite(IN2, 0);
  ledcWrite(IN3, vitesseMoteurDroit);
  ledcWrite(IN4, 0);
}

void tournerDroite(int vitesseMoteurGauche, int vitesseMoteurDroit)
{
  ledcWrite(IN1, vitesseMoteurGauche);
  ledcWrite(IN2, 0);
  ledcWrite(IN3, vitesseMoteurDroit);
  ledcWrite(IN4, 0);
}

void stopMotors()
{
  ledcWrite(IN1, 0);
  ledcWrite(IN2, 0);
  ledcWrite(IN3, 0);
  ledcWrite(IN4, 0);
}

void avancer(int vitesse)
{
  ledcWrite(IN1, vitesse);
  ledcWrite(IN2, 0);
  ledcWrite(IN3, vitesse);
  ledcWrite(IN4, 0);
}

void reculer(int vitesse)
{
  ledcWrite(IN1, 0);
  ledcWrite(IN2, vitesse);
  ledcWrite(IN3, 0);
  ledcWrite(IN4, vitesse);
}

void pivoterGauche(int vitesse)
{
  ledcWrite(IN1, 0);
  ledcWrite(IN2, vitesse);
  ledcWrite(IN3, vitesse);
  ledcWrite(IN4, 0);
}

void pivoterDroite(int vitesse)
{
  ledcWrite(IN1, vitesse);
  ledcWrite(IN2, 0);
  ledcWrite(IN3, 0);
  ledcWrite(IN4, vitesse);
}

//===========================================
// FONCTIONS EXISTANTES
//===========================================
void battement(int nb) {
  for (int i = 0; i < nb; i++) {
    queue.write(0);
    delay(200);
    queue.write(60);
    delay(200);
    queue.write(120);
    delay(200);
    queue.write(60);
    delay(400);
  }
}

void led_clignotement(int nb)
{
  for (int i = 0; i < nb; i++)
  {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
}

void drop_poulet()
{
  gueule.write(60);
  delay(200);
  gueule.write(50);
  delay(200);
  gueule.write(45);
  delay(500);
  gueule.write(30);
  delay(500);
  gueule.write(25);
  delay(1000);
  gueule.write(15);
  delay(200);
  gueule.write(70);
}