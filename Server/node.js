const express = require("express");
const mysql = require("mysql");
const cors = require("cors");
const session = require("express-session");

const app = express();

// ----- MIDDLEWARES -----
app.use(cors());
app.use(express.json()); // Analyse des requêtes JSON

// Configuration des sessions
app.use(session({
    secret: 'votre_secret',   // Remplacez par une clé secrète sécurisée
    resave: false,
    saveUninitialized: true,
    cookie: { secure: false } // secure: true en production avec HTTPS
}));

// ----- CONFIGURATION DE LA BASE DE DONNÉES -----
const db = mysql.createConnection({
    host: "192.168.67.89",    // Adresse de votre base de données
    user: "esp_user",         // Nom d'utilisateur MySQL
    password: "password123",         // Mot de passe
    database: "Mini_Projet" // Nom de la base
});

// Connexion à la base de données
db.connect(err => {
    if (err) {
        console.error("Erreur de connexion à la base de données :", err);
        process.exit(1);
    }
    console.log("Connexion à la base de données réussie !");
});

// ----- ROUTES -----

// 1. Route pour la connexion des utilisateurs
app.post("/api/login", (req, res) => {
    const { username, password } = req.body;

    if (!username || !password) {
        return res.status(400).json({ error: "Nom d'utilisateur et mot de passe requis" });
    }

    const query = "SELECT * FROM users WHERE username = ?";
    db.query(query, [username], (err, results) => {
        if (err) {
            console.error("Erreur lors de la vérification des identifiants :", err);
            return res.status(500).json({ error: "Erreur interne du serveur" });
        }

        if (results.length === 0) {
            return res.status(401).json({ success: false, message: "Utilisateur introuvable" });
        }

        const user = results[0];

        if (password === user.password) {
            req.session.user = {
                id: user.id,
                username: user.username,
                role: user.role
            };

            res.json({
                success: true,
                message: "Connexion réussie",
                user: req.session.user
            });
        } else {
            res.status(401).json({ success: false, message: "Mot de passe incorrect" });
        }
    });
});

// 2. Route pour récupérer les outils avec leur statut
app.get("/api/tools", (req, res) => {
    const query = "SELECT name, status, available FROM tools";

    db.query(query, (err, results) => {
        if (err) {
            console.error("Erreur lors de la récupération des outils :", err);
            return res.status(500).json({ error: "Erreur interne du serveur" });
        }

        res.json({ success: true, tools: results });
    });
});

// Route pour récupérer la dernière ligne des statuts des outils
app.get("/api/getLatestToolStatus", (req, res) => {
    const query = "SELECT * FROM `outils-test` ORDER BY `Temps` DESC LIMIT 1";

    db.query(query, (err, results) => {
        if (err) {
            console.error("Erreur lors de la récupération des statuts :", err);
            return res.status(500).json({ error: "Erreur serveur" });
        }
        if (results.length > 0) {
            res.json(results[0]); // Envoie la dernière ligne
        } else {
            res.status(404).json({ error: "Aucune donnée trouvée" });
        }
    });
});


// 4. Route pour déconnecter l'utilisateur
app.post("/api/logout", (req, res) => {
    req.session.destroy(err => {
        if (err) {
            console.error("Erreur lors de la déconnexion :", err);
            return res.status(500).json({ error: "Erreur serveur" });
        }
        res.json({ success: true, message: "Déconnexion réussie" });
    });
});

// Route par défaut pour tester si le serveur fonctionne
app.get("/", (req, res) => {
    res.send("Bienvenue sur l'API de gestion des outils !");
});

// ----- DÉMARRAGE DU SERVEUR -----
const PORT = 3000;
app.listen(PORT, () => {
    console.log(`Serveur démarré sur le port ${PORT}`);
});
