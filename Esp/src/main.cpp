#include <SoftwareSerial.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <string>

// Variables globales pour la configuration Wi-Fi et MySQL
String MY_IP;
String MY_SSID;
String MY_PWD;

void connect_AP(const char *ssid, const char *password);
void insertMessageIntoDB(String message, String direction);
String checkUserInDB(String userID);
String checkCodeSecretInDB(String userID, String codeSaisi); // Vérification de l'UID et du code secret

#define HC12_RX D5
#define HC12_TX D6
#define HC12_SET D7

SoftwareSerial HC12(HC12_RX, HC12_TX);

// Configuration de MySQL
IPAddress server_ip(192, 168, 246, 89);
char user[] = "esp_user";
char pass[] = "password123";
char database[] = "Mini_Projet";

WiFiClient client;
MySQL_Connection conn((Client *)&client);

void setup()
{
  Serial.begin(9600);
  connect_AP("FreeWifi", "Motdepasse");

  unsigned long startMillis = millis();
  bool connected = false;
  while (millis() - startMillis < 10000)
  {
    if (conn.connect(server_ip, 3306, user, pass))
    {
      Serial.println("Connexion à MySQL réussie!");
      MySQL_Cursor *cursor = new MySQL_Cursor(&conn);
      cursor->execute("USE Mini_Projet;");
      delete cursor;
      connected = true;
      break;
    }
    else
    {
      Serial.println("Échec de la connexion à MySQL. Tentative...");
      delay(500);
    }
  }

  if (!connected)
  {
    Serial.println("Échec de la connexion à MySQL après 10 secondes.");
    while (true)
      ;
  }

  HC12.begin(9600);
  pinMode(HC12_SET, OUTPUT);
  digitalWrite(HC12_SET, LOW);
  delay(50);
  HC12.print(F("AT+DEFAULT"));
  delay(100);
  HC12.print(F("AT+C010"));
  delay(100);
  while (HC12.available())
  {
    Serial.print((char)HC12.read());
    delay(2);
  }
  digitalWrite(HC12_SET, HIGH);
  Serial.println("\nConfiguration HC-12 terminée. ESP8266 prêt !");
}

void loop()
{
  if (HC12.available())
  {
    String messageRecu = HC12.readStringUntil('\n');
    
    if (messageRecu.startsWith("SCPG:01:"))
    {
      // Gestion de SCPG:01: uniquement avec l'UID
      String userID = messageRecu.substring(8); // Extraire l'UID après "SCPG:01:"
      Serial.println("Code reçu de la Mega : " + userID);
      String result = checkUserInDB(userID); // Vérifier si l'utilisateur existe
      Serial.println("Résultat de la recherche dans la DB : " + result);

      if (result.indexOf("Utilisateur trouvé") >= 0)
      {
        HC12.println("SCPG:02:VALIDE");
      }
      else
      {
        HC12.println("SCPG:02:NON-VALIDE");
      }
    }

    if (messageRecu.startsWith("SCPG:03:"))
    {
      // Gestion de SCPG:03: avec UID et code secret
      int separatorIndex = messageRecu.indexOf(':', 8); // Rechercher le séparateur entre UID et code secret
      if (separatorIndex != -1)
      {
        String userID = messageRecu.substring(8, separatorIndex);     // Extraire l'UID
        String codeSaisi = messageRecu.substring(separatorIndex + 1); // Extraire le code secret
        Serial.println("UID reçu : " + userID);
        Serial.println("Code secret reçu : " + codeSaisi);

        String result = checkCodeSecretInDB(userID, codeSaisi); // Vérifier UID et code secret
        Serial.println("Résultat de la vérification : " + result);

        if (result.indexOf("Code valide") >= 0)
        {
          HC12.println("SCPG:04:VALIDE");
        }
        else
        {
          HC12.println("SCPG:04:NON-VALIDE");
        }
      }
      else
      {
        Serial.println("Trame SCPG:03: mal formée.");
        HC12.println("SCPG:04:ERREUR");
      }
    }

    if (messageRecu.startsWith("SCPG:05:")) {
    // Supprimer le préfixe "SCPG:05:"
    Serial.println(messageRecu);
    String etats = messageRecu.substring(8);

    // Diviser les états des outils en fonction des ":"
    int firstSeparator = etats.indexOf(':');
    int secondSeparator = etats.indexOf(':', firstSeparator + 1);

    // Extraire les états individuels
    String LeMarteau = etats.substring(0, firstSeparator);
    String Cle = etats.substring(firstSeparator + 1, secondSeparator);
    String Pince = etats.substring(secondSeparator + 1);

    delay(50);
    Serial.println("L'état du marteau est :" + LeMarteau);
    delay(50);
    Serial.println("L'état du la cle est:" + Cle);
    delay(50);
    Serial.println("L'état de la pince est :" + Pince);
    delay(50);

    // Construire la requête SQL pour insérer les états dans la base
    String query = "INSERT INTO `outils-test` (Temps, Marteau, Cle, Pince) VALUES (NOW(), '";
    query += LeMarteau + "', '" + Cle + "', '" + Pince + "');";

    // Afficher la requête SQL pour débogage
    //Serial.println("Requête SQL : " + query);

    // Exécuter la requête
    MySQL_Cursor *cursor = new MySQL_Cursor(&conn);
    cursor->execute(query.c_str());
    delete cursor;

    Serial.println("Les états des outils ont été insérés dans la base de données.");

    LeMarteau = "";
    Cle = "";
    Pince = "";
    }
}
}

// Vérification si l'UID existe dans la DB
String checkUserInDB(String userID)
{
  if (!conn.connected())
  {
    Serial.println("Connexion à la base de données perdue.");
    return "Erreur : connexion perdue.";
  }

  MySQL_Cursor *cursor = new MySQL_Cursor(&conn);

  String query = "SELECT ID FROM ID_acces;";
  cursor->execute(query.c_str());

  column_names *cols = cursor->get_columns();
  if (cols == NULL)
  {
    Serial.println("Erreur lors de la lecture des colonnes.");
    delete cursor;
    return "Erreur lors de la lecture des colonnes.";
  }

  row_values *row;
  String result = "Utilisateur non trouvé";

  Serial.println("Affichage des IDs récupérés :");

  while ((row = cursor->get_next_row()) != NULL)
  {
    if (row->values[0] != NULL) // Vérifie si la valeur est non nulle
    {
      String idInDB = row->values[0];
      idInDB.trim(); // Élimine les espaces en début/fin
      userID.trim(); // Élimine les espaces en début/fin

      Serial.println("ID dans la DB : " + idInDB);
      if (idInDB.equals(userID))
      {
        result = "Utilisateur trouvé : " + idInDB;
        break;
      }
    }
    else
    {
      Serial.println("Valeur NULL rencontrée dans les résultats.");
    }
  }

  delete cursor; // Libère les ressources
  return result;
}

String cleanInput(String input)
{
  // Supprimer les espaces, les caractères invisibles et les caractères non imprimables
  input.trim();            // Enlever les espaces au début et à la fin
  input.replace("\n", ""); // Supprimer les retours à la ligne
  input.replace("\r", ""); // Supprimer les retours chariot
  input.replace("\t", ""); // Supprimer les tabulations
  return input;
}

String checkCodeSecretInDB(String userID, String codeSaisi)
{
  // Vérifie si la connexion est active
  if (!conn.connected())
  {
    Serial.println("Connexion à la base de données perdue.");
    return "Erreur : connexion perdue.";
  }

  MySQL_Cursor *cursor = new MySQL_Cursor(&conn);

  // Debug : Afficher l'ID
  Serial.println("ID du badge : " + userID);

  // Construire la requête SQL en validant l'entrée utilisateur
  userID.trim();
  String query = "SELECT Code FROM ID_acces WHERE ID = '" + userID + "';";

  // Debug : Afficher la requête SQL
  Serial.println("Requête SQL : " + query);

  // Exécution de la requête
  cursor->execute(query.c_str());

  column_names *cols = cursor->get_columns();
  if (cols == NULL)
  {
    Serial.println("Erreur lors de la lecture des colonnes.");
    delete cursor;
    return "Erreur lors de la lecture des colonnes.";
  }

  row_values *row;
  String codeSecretDB = "";

  // Récupérer le code secret depuis la base
  while ((row = cursor->get_next_row()) != NULL)
  {
    if (row->values[0] != NULL) // Vérifie si la valeur est non nulle
    {
      codeSecretDB = row->values[0];
      codeSecretDB.trim(); // Nettoie les espaces autour
    }
    break; // On récupère uniquement la première ligne
  }

  delete cursor; // Libérer les ressources

  // Vérifier si le code a été trouvé dans la base
  if (codeSecretDB == "")
  {
    Serial.println("Utilisateur non trouvé ou code absent dans la base.");
    return "Utilisateur non trouvé ou code absent dans la base.";
  }

  // Nettoyer et comparer les codes
  codeSaisi.trim();
  codeSecretDB.trim();

  Serial.println("Code de la base (avant comparaison) : [" + codeSecretDB + "]");
  Serial.println("Code saisi (avant comparaison) : [" + codeSaisi + "]");

  if (codeSaisi.equals(codeSecretDB))
  {
    return "Code valide : Accès autorisé.";
  }
  else
  {
    return "Code invalide : Accès refusé.";
  }
}


// Connexion au réseau Wi-Fi
void connect_AP(const char *ssid, const char *password)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connexion au réseau Wi-Fi : ");
  Serial.println(ssid);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 20)
    {
      Serial.println("\nImpossible de se connecter.");
      return;
    }
  }
  Serial.println("\nConnexion réussie !");
  MY_IP = WiFi.localIP().toString();
  Serial.print("Adresse IP : ");
  Serial.println(MY_IP);
}

void insertToolStatus(String marteau, String cle, String pince) {
  MySQL_Cursor *cursor = new MySQL_Cursor(&conn);

  // Construire la requête SQL
  String query = "INSERT INTO `outils-test` (Temps, Marteau, Cle, Pince) VALUES (NOW(), '";
  query += marteau + "', '" + cle + "', '" + pince + "');";

  // Afficher la requête pour débogage
  Serial.println("Requête SQL : " + query);

  // Exécuter la requête
  cursor->execute(query.c_str());

  delete cursor;
  Serial.println("Les états des outils ont été insérés dans la base de données.");
}

