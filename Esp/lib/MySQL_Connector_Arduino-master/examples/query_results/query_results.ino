#include <SoftwareSerial.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

String MY_IP;
String MY_SSID;
String MY_PWD;

void connect_AP(const char *ssid, const char *password);

#define HC12_RX D5 // Broche RX du HC-12 connectée à D5 (GPIO14)
#define HC12_TX D6 // Broche TX du HC-12 connectée à D6 (GPIO12)
#define HC12_SET D7 // Broche SET du HC-12 connectée à D7 (GPIO13)

SoftwareSerial HC12(HC12_RX, HC12_TX); // Configuration SoftwareSerial pour HC-12

// === Paramètres pour la connexion MySQL ===
IPAddress server_ip(192, 168, 161, 89); // Adresse IP du serveur MySQL
char user[] = "esp_user";                 // Utilisateur MySQL
char pass[] = "password123";              // Mot de passe MySQL
char database[] = "Mini_Projet";          // Nom de la base de données

WiFiClient client;                       // Client Wi-Fi
MySQL_Connection conn((Client *)&client); // Objet MySQL_Connection

void setup() {
  Serial.begin(9600); // Moniteur série

  // Connexion Wi-Fi
  connect_AP("FreeWifi", "Motdepasse");

  // === Connexion à MySQL ===
  Serial.println("Tentative de connexion à MySQL...");
  if (conn.connect(server_ip, 3306, user, pass)) {
    Serial.println("Connexion à MySQL réussie !");
  } else {
    Serial.println("Échec de la connexion à MySQL. Vérifiez les paramètres.");
  }

  HC12.begin(9600);   // Communication avec le module HC-12
  pinMode(HC12_SET, OUTPUT);

  // Configuration du HC-12
  digitalWrite(HC12_SET, LOW); // Mode configuration
  delay(50);
  HC12.print(F("AT+DEFAULT")); delay(100); // Réinitialisation
  HC12.print(F("AT+C010")); delay(100);    // Canal 10
  while (HC12.available()) {
    Serial.print((char)HC12.read()); // Lire la réponse
    delay(2);
  }
  digitalWrite(HC12_SET, HIGH); // Retour au mode normal
  Serial.println("\nConfiguration HC-12 terminée. ESP8266 prêt !");
}

void loop() {
  // === Envoi de données depuis le moniteur série ESP8266 vers le HC-12
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    HC12.println(message); // Envoi du message au HC-12
    Serial.print("Envoyé : ");
    Serial.println(message);
  }

  // === Réception de données via HC-12
  if (HC12.available()) {
    String messageRecu = HC12.readStringUntil('\n');
    Serial.print("Reçu : ");
    Serial.println(messageRecu); // Affiche sur le moniteur série
  }

  // === Requête MySQL ===
  // Exécution de la requête SQL à chaque boucle
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  char query[] = "SELECT * FROM table_example LIMIT 5"; // Remplacez par votre table réelle

  cur_mem->execute(query);

  // Récupération des colonnes de la requête
  column_names *cols = cur_mem->get_columns();
  for (int f = 0; f < cols->num_fields; f++) {
    Serial.print(cols->fields[f]->name);
    if (f < cols->num_fields-1) {
      Serial.print(", ");
    }
  }
  Serial.println();

  // Récupération des lignes et affichage des résultats
  row_values *row = NULL;
  do {
    row = cur_mem->get_next_row();
    if (row != NULL) {
      for (int f = 0; f < cols->num_fields; f++) {
        Serial.print(row->values[f]);
        if (f < cols->num_fields-1) {
          Serial.print(", ");
        }
      }
      Serial.println();
    }
  } while (row != NULL);

  // Nettoyage après la requête
  delete cur_mem;

  delay(5000); // Attendre 5 secondes avant d'exécuter une autre requête
}

////////// GESTION DU SSID (SI MODE POINT D'ACCES)
void connect_AP(const char *ssid, const char *password) {
  Serial.begin(9600);
  delay(500);
  WiFi.mode(WIFI_STA); // Configure l'ESP en mode Station (client)
  WiFi.begin(ssid, password); // Connexion au réseau Wi-Fi

  Serial.print("Connexion au réseau Wi-Fi : ");
  Serial.println(ssid);

  // Attendre la connexion
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 20) { // Timeout après ~10 secondes
      Serial.println("\nImpossible de se connecter au réseau Wi-Fi.");
      return;
    }
  }

  // Une fois connecté
  Serial.println("\nConnexion réussie !");
  MY_IP = WiFi.localIP().toString(); // Adresse IP attribuée par le routeur
  MY_SSID = ssid;
  MY_PWD = password;

  Serial.print("Adresse IP de l'ESP8266 : ");
  Serial.println(MY_IP);
}
