#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

// --- CATALOGUES POUR L'AFFICHAGE ---
char* noms_suspects[] = {
    "Sebastian Moran", "Irene Adler", "Inspector Lestrade",
    "Inspector Gregson", "Inspector Baynes", "Inspector Bradstreet",
    "Inspector Hopkins", "Sherlock Holmes", "John Watson",
    "Mycroft Holmes", "Mrs. Hudson", "Mary Morstan", "James Moriarty"
};

char* noms_symboles[] = {
    "0:Pipe", "1:Ampoule", "2:Poing", "3:Couronne", 
    "4:Livre", "5:Collier", "6:Oeil", "7:Crane"
};

// --- LOGIQUE RÉSEAU ---

void* ecouter_serveur(void* arg) {
    int sock = *(int*)arg;
    char buffer[BUFFER_SIZE];
    int n;

    while ((n = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[n] = '\0';

        // Cas spécial : Réception des cartes
        if (strncmp(buffer, "CARDS", 5) == 0) {
            int id1, id2, id3;
            sscanf(buffer, "CARDS %d %d %d", &id1, &id2, &id3);
            printf("\n==============================");
            printf("\n[VOS CARTES] :");
            printf("\n1. %s", noms_suspects[id1]);
            printf("\n2. %s", noms_suspects[id2]);
            printf("\n3. %s", noms_suspects[id3]);
            printf("\n==============================\n");
            
            printf("\nSymboles disponibles pour ASK :\n");
            for(int i=0; i<8; i++) printf("%s ", noms_symboles[i]);
            printf("\n");
        } 
        else {
            // Message général (tour de jeu, réponse ASK, victoire)
            printf("\n[INFO] %s", buffer);
        }
        
        printf("\nVotre action (ASK <joueur> <symb> ou GUESS <id>) : ");
        fflush(stdout);
    }

    printf("\n[ERREUR] Connexion perdue avec le serveur.\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <IP_serveur> <port>\n", argv[0]);
        exit(1);
    }

    // Création de la socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    // Connexion
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur de connexion");
        exit(1);
    }

    printf("--- CONNECTÉ AU SHERLOCK 13 ---\n");
    printf("En attente des autres joueurs...\n");

    // Lancement du thread qui reçoit les messages (CARDS, tours, etc.)
    pthread_t tid;
    pthread_create(&tid, NULL, ecouter_serveur, &sockfd);

    // Boucle d'envoi des actions
    while (1) {
        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
            send(sockfd, buffer, strlen(buffer), 0);
        }
    }

    close(sockfd);
    return 0;
}