#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Wire.h>
#include "rgb_lcd.h"
#include <Servo.h>
#include <HX711.h>

#define RST_PIN 41
#define SS_PIN 53

#define Broche_Echo1 12 // Broche Echo du premier HC-SR04
#define Broche_Trigger1 13 // Broche Trigger du premier HC-SR04
#define Broche_Echo2 14 // Broche Echo du second HC-SR04
#define Broche_Trigger2 15 // Broche Trigger du second HC-SR04

// Definition des variables
int MesureMaxi = 300; // Distance maximale à mesurer (cm)
int MesureMini = 3; // Distance minimale à mesurer (cm)

String Cle = "";
String Pince = "";
String LeMarteau = "";
// Fonction pour mesurer la distance d'un capteur ultrason
long mesurerDistance(int brocheTrigger, int brocheEcho) {
    long duree, distance;

    // Début de la mesure avec un signal de 10 μs appliqué sur TRIG
    digitalWrite(brocheTrigger, LOW); // On efface l'état logique de TRIG
    delayMicroseconds(2);
    digitalWrite(brocheTrigger, HIGH); // On met la broche TRIG à "1" pendant 10 μs
    delayMicroseconds(10);
    digitalWrite(brocheTrigger, LOW); // On remet la broche TRIG à "0"

    // On mesure combien de temps le niveau logique haut est actif sur ECHO
    duree = pulseIn(brocheEcho, HIGH);

    // Calcul de la distance grâce au temps mesuré
    distance = duree * 0.034 / 2; // Conversion en cm

    return distance;
}

#define HC12 Serial1
const int setPin = 21;

MFRC522 rfid(SS_PIN, RST_PIN);
Servo servo;
int servoPin = A0 ;
// Angles pour ouvrir et fermer la porte
const int angleFerme = 10;   // Angle fermé
const int angleOuvert = 115; // Angle ouvert

rgb_lcd lcd;

#define L1 9
#define L2 8
#define L3 7
#define L4 6
#define C1 5
#define C2 4
#define C3 3
#define C4 2

const byte lignes = 4;
const byte colonnes = 4;

char code[lignes][colonnes] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte broches_lignes[lignes] = {L1, L2, L3, L4};
byte broches_colonnes[colonnes] = {C1, C2, C3, C4};
Keypad clavier = Keypad(makeKeymap(code), broches_lignes, broches_colonnes, lignes, colonnes);

String codeSaisi = "";
String codeCorrect = "1234";
int tentatives = 0;
bool badgeValide = false;
bool codeValide = false;
String uidLue = "";
String resultat = "";

HX711 marteau;
HX711 pince;
HX711 tournevis;

int calibration_marteau = -100000;
int calibration_pince = -120000;
int calibration_tournevis = -110000;

unsigned long dernierEnvoi = 0;  // Variable pour stocker le temps du dernier envoi
const unsigned long intervalle = 10000; // 2 minutes = 120000 millisecondes

void setup() {
  Serial.begin(9600);
  HC12.begin(9600);
  pinMode(setPin, OUTPUT);

  // Configuration des broches pour les deux capteurs
    pinMode(Broche_Trigger1, OUTPUT); // Broche Trigger du capteur 1 en sortie
    pinMode(Broche_Echo1, INPUT);    // Broche Echo du capteur 1 en entrée

    pinMode(Broche_Trigger2, OUTPUT); // Broche Trigger du capteur 2 en sortie
    pinMode(Broche_Echo2, INPUT);    // Broche Echo du capteur 2 en entrée

  /*servo.attach(servoPin);
  servo.write(angleFerme);
  servo.detach();*/
  Serial.println("Porte initialisée à la position fermée.");

  // Initialisation des capteurs HX711
  initialiserCapteur(marteau, 10, 11, calibration_marteau);
 /* initialiserCapteur(pince, 4, 5, calibration_pince);
  initialiserCapteur(tournevis, 6, 8, calibration_tournevis);*/

  // LCD
  lcd.begin(16, 2);
  lcd.setRGB(0, 213, 255);
  lcd.print("Scan badge...");
  
  // RFID
  SPI.begin();
  rfid.PCD_Init();
  
  // HC-12 configuration
  digitalWrite(setPin, LOW);
  HC12.print(F("AT+DEFAULT"));
  delay(100);
  HC12.print(F("AT+C010"));
  delay(100);
  while (HC12.available()) {
    Serial.print((char)HC12.read());
    delay(2);
  }
  digitalWrite(setPin, HIGH);
  Serial.println("\nConfiguration HC-12 terminée.");
}

void loop() {
  // Communication HC-12 et moniteur série
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    HC12.println(message);
    //Serial.print("Envoyé : ");
    Serial.println(message);
  }

  if (HC12.available()) {
    String messageRecu = HC12.readStringUntil('\n');
    //Serial.print("Reçu : ");
    Serial.println(messageRecu);
  }

  delay(50);

  long distance1 = mesurerDistance(Broche_Trigger1, Broche_Echo1);
    if (distance1 >= MesureMaxi || distance1 <= MesureMini) {
        //Serial.println("Capteur 1 : OK");
        Cle = "OK";
    } else {
        //Serial.println("Capteur 1 : PAS OK");
        Cle = "PAS OK";
    }

    // Mesure de la distance pour le capteur 2
    long distance2 = mesurerDistance(Broche_Trigger2, Broche_Echo2);
    if (distance2 >= MesureMaxi || distance2 <= MesureMini) {
        //Serial.println("Capteur 2 : OK");
        Pince = "OK";
        
    } else {
        //Serial.println("Capteur 2 : PAS OK");
        Pince = "PAS OK";
    }

    // Lire les données des capteurs
    float poids_marteau = lireCapteur(marteau, 10); // Moyenne de 10 mesures

    // Envoyer les données uniquement si elles changent
    envoyerDonneesSiChangement("Marteau", poids_marteau);
    
  // Vérifier si 2 minutes se sont écoulées depuis le dernier envoi
  if (millis() - dernierEnvoi >= intervalle) {
    HC12.println("SCPG:05:" + LeMarteau + ":" + Cle + ":" + Pince);
    Serial.println("Trame envoyée : SCPG:05:" + LeMarteau + ":" + Cle + ":" + Pince);
    LeMarteau = "";
    Cle = "";
    Pince = "";
    dernierEnvoi = millis();  // Réinitialiser le compteur
  }


  lcd.setCursor(0, 0);
  lcd.setRGB(0, 213, 255);
  lcd.print("Scan badge...");

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    verifieBadge();  // Appeler la fonction pour vérifier le badge scanné
  }

  if (badgeValide && !codeValide) {
    verifieCodeSecret();  // Vérifier le code secret seulement si le badge est validé
  }

  if (badgeValide && codeValide) {
    // Effectuer une action ici lorsque les deux sont validés
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Acces autorise");
    Serial.println("Ouverture de la porte...");
    servo.attach(servoPin);
    servo.write(angleOuvert); // Angle pour ouvrir la porte
    delay(1500);              // Attendre 3 secondes
        servo.detach();
    Serial.println("Porte ouverte.");
    delay(5000);
    Serial.println("Fermeture de la porte...");
    servo.attach(servoPin);
    servo.write(angleFerme);  // Angle pour fermer la porte
    delay(1300);              // Attendre 3 secondes
        servo.detach();
    Serial.println("Porte fermée.");
    badgeValide = false;  // Réinitialiser la validation du badge
    codeValide = false;  // Réinitialiser la validation du cod
    
  }
    //Serial.println("1 loop de faite");

}

void verifieBadge() {
  uidLue = "";

  for (byte i = 0; i < rfid.uid.size; i++) {
    uidLue += String(rfid.uid.uidByte[i], HEX);
  }
  uidLue.toUpperCase();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UID: ");
  lcd.print(uidLue);
  delay(2000);

  Serial.print("SCPG:01:");
  Serial.println(uidLue);
  HC12.print("SCPG:01:");
  HC12.println(uidLue);

  // Attente et vérification de la réponse plusieurs fois si nécessaire
  int essaisMax = 3;
  String trameRecue = "";
  for (int essais = 0; essais < essaisMax; essais++) {
    // Attente de la réponse
    unsigned long timeout = millis() + 500; // 5 secondes d'attente
    while (HC12.available() == 0 && millis() < timeout) {
      // Attente de la réponse sans bloquer trop longtemps
    }

    if (HC12.available() > 0) {
      trameRecue = HC12.readStringUntil('\n');
      Serial.println(trameRecue);
      trameRecue.trim();
      break; // Si on a une réponse, on sort de la boucle
    }
  }

  // Vérification de la trame reçue
  if (trameRecue.startsWith("SCPG:02:")) {
    resultat = trameRecue.substring(8);
    Serial.println("Resultat:" + resultat);
    resultat.trim();

    if (resultat == "VALIDE") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Utilisateur valide");
      delay(2000);
      lcd.clear();
      lcd.print("Saisir code :");
      badgeValide = true;
    } else if (resultat == "NON-VALIDE") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Utilisateur inconnu");
      delay(2000);
      lcd.clear();
      lcd.print("Scan badge...");
    }
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Erreur de reponse");
    delay(2000);
    lcd.clear();
    lcd.print("Scan badge...");
  }
}

void verifieCodeSecret() {
  String codeSaisiTemp = "";  // Variable temporaire pour le code saisi
  int essaisMax = 3;          // Nombre maximum de tentatives
  String trameRecue = "";     // Pour stocker la trame reçue

  for (int essais = 0; essais < essaisMax; essais++) {
    lcd.clear();
    lcd.print("Saisir code :");
    codeSaisiTemp = "";  // Réinitialisation du code saisi

    // Attente que l'utilisateur saisisse le code
    while (true) {
      char touche = clavier.getKey();
      if (touche != NO_KEY) {
        if (touche == '*') {
          if (codeSaisiTemp.length() > 0) {
            codeSaisiTemp.remove(codeSaisiTemp.length() - 1);
            lcd.setCursor(0, 1);
            lcd.print("                ");
            lcd.setCursor(0, 1);
            lcd.print(codeSaisiTemp);
          }
        } 
        else if (touche == '#') {
          Serial.println("Entrer systeme");
            HC12.print("SCPG:03:" + uidLue + ":");
            HC12.println(codeSaisiTemp);  // Envoi du code secret à vérifier

            // Attente et vérification de la réponse plusieurs fois si nécessaire
            int essaisMaxSerie = 3;
            for (int i = 0; i < essaisMaxSerie; i++) {
              unsigned long timeout = millis() + 5000;
              while (Serial.available() == 0 && millis() < timeout) { }

              if (HC12.available() > 0) {
                trameRecue = HC12.readStringUntil('\n');
                trameRecue.trim();
                break;
              }
            }

            if (trameRecue.startsWith("SCPG:04:")) {
              String resultat = trameRecue.substring(8);
              resultat.trim();

              if (resultat == "VALIDE") {
                lcd.clear();
                lcd.print("Code valide!");
                codeValide = true;  // Marquer le code comme valide
                break;
              } else if (resultat == "NON-VALIDE") {
                lcd.clear();
                lcd.print("Code incorrect");
                delay(2000);  // Attendre avant de permettre une nouvelle tentative
              }
              else {
                Serial.println("Rien Recu");
                
                }
            }
        }
        else
        {
        codeSaisiTemp += touche;
        lcd.setCursor(0, 1);
        lcd.print(codeSaisiTemp);
        
        }
      }
    }

    if (codeValide) break;
  }

  if (!codeValide) {
    lcd.clear();
    lcd.print("Trop de tentatives!");
    delay(2000);
    lcd.clear();
    lcd.print("Saisir code :");
    tentatives = 0;
    codeValide = false;
  }
}


void initialiserCapteur(HX711 &capteur, int pinDT, int pinSCK, int facteurCalibration) {
  capteur.begin(pinDT, pinSCK);
  capteur.set_scale(facteurCalibration);
  capteur.tare();
}

// Fonction pour lire les mesures d'un capteur HX711
float lireCapteur(HX711 &capteur, int nbMesures) {
  //Serial.println(capteur.get_units(nbMesures));
  return capteur.get_units(nbMesures);
  
  
}

void envoyerDonneesSiChangement(const char* outils, float poidsActuel) {
  // Affiche le poids actuel pour débogage
  //Serial.println(poidsActuel);

  // Vérifie si le poids est supérieur à un seuil (par exemple, 0.1 pour détecter un objet)
  if (poidsActuel > 0.1) {
    //Serial.println(String(outils) + " OK");
    HC12.println(String(outils) + " OK"); // Envoie le message via HC12
    LeMarteau = "OK";
  } else {
    //Serial.println(String(outils) + " Pas OK");
    HC12.println(String(outils) + " Pas OK"); // Envoie le message via HC12
    LeMarteau = "PAS OK";
  }
}


// Fonction pour encapsuler les données dans une trame identifiable
String encapsulerTrame(const char* outils, float donnees) {

  if (outils == "Marteau") {
    return String("SCPG:") + "05:" + String(donnees, 0); // Format SCPG:CodeFonction:Data
  }
  if (outils == "Pince") {
    return String("SCPG:") + "06:" + String(donnees, 0); // Format SCPG:CodeFonction:Data
  }
  if (outils == "Tournevis") {
    return String("SCPG:") + "07:" + String(donnees, 0); // Format SCPG:CodeFonction:Data
  }
}
