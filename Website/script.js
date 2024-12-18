// Gestion du Login
function login(event) {
    event.preventDefault();

    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;

    // Appel à l'API pour vérifier les identifiants
    fetch('http://localhost:3000/api/login', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ username, password })
    })
        .then(response => {
            if (!response.ok) {
                throw new Error('Nom d\'utilisateur ou mot de passe incorrect');
            }
            return response.json();
        })
        .then(data => {
            if (data.success) {
                // Cache la page de login et affiche la page principale
                document.getElementById('loginPage').style.display = "none";
                document.getElementById('mainPage').style.display = "block";
                
                // Vérifier le rôle et gérer l'affichage
                if (data.user.role === 'user') {
                    // Si l'utilisateur est un "user", cache les sections d'admin
                    document.getElementById('adminSection').style.display = "none";
                }

                // Gérer l'affichage du lien Admin
                const adminLink = document.querySelector('nav a[href="admin.html"]');
                if (data.user.role === 'user' && adminLink) {
                    adminLink.style.display = "none"; // Cache le lien vers la page admin
                }

                // Charger les outils après connexion
                loadTools();
            }
        })
        .catch(error => {
            document.getElementById('loginError').textContent = error.message;
        });
}

// Fonction pour se déconnecter
function logout(event) {
    event.preventDefault();
    document.getElementById('mainPage').style.display = "none";
    document.getElementById('loginPage').style.display = "block";
    document.getElementById('loginForm').reset();
    document.getElementById('loginError').textContent = "";
}

function loadToolStatus() {
    fetch('http://localhost:3000/api/getLatestToolStatus') // Endpoint pour récupérer la dernière ligne
        .then(response => response.json())
        .then(data => {
            // Nettoyer les valeurs avant de mettre à jour le tableau
            const marteauStatus = data.Marteau.trim();
            const pinceStatus = data.Pince.trim();
            const cleStatus = data.Cle.trim();

            updateToolStatus("hammer", marteauStatus);
            updateToolStatus("pliers", pinceStatus);
            updateToolStatus("wrench", cleStatus);
        })
        .catch(error => {
            console.error("Erreur lors du chargement des statuts des outils :", error);
        });
}

function updateToolStatus(tool, status) {

    const statusElement = document.getElementById(`${tool}Status`);
    const availabilityElement = document.getElementById(`${tool}Availability`);

    if (status === "OK") {
        statusElement.textContent = "Détecté";
        availabilityElement.textContent = "Disponible";
        statusElement.style.color = "green";
        availabilityElement.style.color = "green";
    } else if (status === "PAS OK") {
        statusElement.textContent = "Non détecté";
        availabilityElement.textContent = "Indisponible";
        statusElement.style.color = "red";
        availabilityElement.style.color = "red";
    } else {
        statusElement.textContent = "En attente...";
        availabilityElement.textContent = "En attente...";
        statusElement.style.color = "black";
        availabilityElement.style.color = "black";
    }
}

// Effacer les données série
function clearSerialData() {
    document.getElementById("serialData").textContent = "";
}

// Charger les statuts des outils dès le chargement de la page
document.addEventListener("DOMContentLoaded", loadToolStatus);

// Mise à jour automatique des statuts toutes les 5 secondes
setInterval(loadToolStatus, 5000);


// Fonction pour modifier les informations d'un outil
function editTool(toolName) {
    alert(`Modification de l'outil : ${toolName}`);
}

// Gestion WebSocket
const socket = new WebSocket('ws://localhost:3000');
const connectionStatus = document.getElementById('connectionStatus');
const serialDataElement = document.getElementById('serialData');

socket.onopen = function () {
    connectionStatus.textContent = 'Connexion WebSocket : Connectée';
    connectionStatus.style.color = 'green';
};

socket.onmessage = function (event) {
    try {
        const data = JSON.parse(event.data);
        serialDataElement.textContent += `Poids : ${data.weight} kg\n`;
        updateToolStatus(data.detectionStatus);
    } catch (error) {
        console.error('Erreur de traitement des données WebSocket :', error);
    }
};

socket.onclose = function () {
    connectionStatus.textContent = 'Connexion WebSocket : Déconnectée';
    connectionStatus.style.color = 'red';
};

socket.onerror = function (error) {
    connectionStatus.textContent = 'Connexion WebSocket : Erreur';
    connectionStatus.style.color = 'orange';
};

// Fonction pour créer un compte
function createAccount(event) {
    event.preventDefault();

    const username = document.getElementById('newUsername').value;
    const password = document.getElementById('newPassword').value;
    const role = document.getElementById('newRole').value;
    const rfidUid = document.getElementById('rfidUid').value; // Récupérer l'UID du badge RFID

    // Vérifier que tous les champs sont remplis
    if (!username || !password || !role || !rfidUid) {
        document.getElementById('createAccountError').textContent = 'Tous les champs sont obligatoires.';
        return;
    }

    // Appel à l'API pour créer un nouvel utilisateur avec un UID RFID
    fetch('http://localhost:3000/api/createAccount', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            username,
            password,
            role,
            rfid_uid: rfidUid // Passer l'UID RFID
        })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            alert('Compte créé avec succès !');
            // Réinitialiser le formulaire après création
            document.getElementById('createAccountForm').reset();
        } else {
            document.getElementById('createAccountError').textContent = data.error;
        }
    })
    .catch(error => {
        document.getElementById('createAccountError').textContent = 'Erreur lors de la création du compte.';
        console.error('Erreur:', error);
    });
}

// Appel initial pour charger les outils
loadTools();
