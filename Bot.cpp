#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <algorithm>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <cctype>

const int PORT = 4242; // Port IRC standard
const char* SERVER_IP = "127.0.0.1"; // Mettez l'adresse IP du serveur ici
const int BUFFER_SIZE = 1024;

// Fonction pour vérifier si la chaîne contient des insultes
bool containsBadWords(const std::string& message, const std::set<std::string>& badWords) {
    // chaine en lower ==> prends en comptes les majuscules etc
    std::string lowercaseMessage = message;
    std::transform(lowercaseMessage.begin(), lowercaseMessage.end(), lowercaseMessage.begin(), ::tolower);

    // Vérifier si la chaîne contient une insulte
    for (std::set<std::string>::iterator it = badWords.begin(); it != badWords.end(); it++) {
        if (lowercaseMessage.find(*it) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::set<std::string> loadBadWords(const std::string& filename) {
    std::set<std::string> badWords;
    std::ifstream file(filename.c_str());
    std::string word;
    while (std::getline(file, word)) {
        // Convertir le mot en minuscules (pas très utile normalement mais au cas ou)
        std::transform(word.begin(), word.end(), word.begin(), tolower);
        badWords.insert(word);
    }
    return badWords;
}

void sendReactionMessage(int clientSocket, std::string channel) {
    std::string reactionMessage = "PRIVMSG " + channel +  " :\033[1;31mLangage !\x03\r\n"; // Message en rouge (ca rends le terminal du serveur rouge du coup c'est fun)
    send(clientSocket, reactionMessage.c_str(), reactionMessage.length(), 0);
}

int main() {
    // Charger les insultes depuis le fichier
    std::set<std::string> badWords = loadBadWords("badwords.txt");

    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];

    // Création du socket
    if ((clientSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Erreur lors de la création du socket." << std::endl;
        return EXIT_FAILURE;
    }

    // Configuration de l'adresse du serveur
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connexion au serveur
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Impossible de se connecter au serveur." << std::endl;
        close(clientSocket);
        return EXIT_FAILURE;
    }

    std::cout << "Connecté au serveur." << std::endl;
    send(clientSocket, "PASS pass\r\n", 11, 0);
    send(clientSocket, "NICK bot\r\n", 10, 0);
    // send(clientSocket, "JOIN T\r\n", 8, 0);
    // Boucle de traitement des messages
    while (true) {
    // Réception du message du serveur IRC
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead < 0) {
            std::cerr << "Erreur lors de la réception des données." << std::endl;
            close(clientSocket);
            return EXIT_FAILURE;
        }

    // Terminaison de la chaîne de caractères
        buffer[bytesRead] = '\0';

        // Vérification du message pour les mots indésirables
        std::string message(buffer);

        //if message start by "INVITE" then join the channel, should receive ":localhost 341 " + client->get_nickname() + " " + nick + " " + channel + "\r\n"
        if (message.find(":localhost 341") != std::string::npos) {
            // get the channel name from the message that should be the last word
            std::cout << "Joining channel :" << message.substr(message.find_last_of(" ") + 1) << std::endl;
            std::cout << "Message reçu : " << buffer << std::endl;
            std::string channel = message.substr(message.find_last_of(" ") + 1);
            // remove the possible \r\n at the end of the channel name
            if (channel.find("\n") != std::string::npos)
                channel.erase(channel.length() - 1);
            if (channel.find("\r") != std::string::npos)
                channel.erase(channel.length() - 1);
            std::string joinMessage = "JOIN " + channel + "\r\n";
            send(clientSocket, joinMessage.c_str(), joinMessage.length(), 0);
        }


        if (containsBadWords(message, badWords)) {
            // Envoyer une réaction automatique
            // get the channel name (should receive :nickname PRIVMSG #channel :message\r\n)
            std::string channel = message.substr(message.find("PRIVMSG") + 8, message.find(":", message.find("PRIVMSG")) - message.find("PRIVMSG") - 9);
            sendReactionMessage(clientSocket, channel);
            std::cout << "Message reçu : " << buffer << std::endl;
        }

        // Affichage du message
        }
    // Fermeture du socket
    close(clientSocket);
    return EXIT_SUCCESS;
}