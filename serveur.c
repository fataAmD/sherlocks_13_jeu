#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define NB_JOUEURS 4
#define NB_SUSPECTS 13
#define BUFFER_SIZE 1024

// --- STRUCTURES ET DONNÉES ---
typedef struct {
    char nom[30];
    int symboles[8]; 
} Suspect;

typedef struct {
    int socket; 
    int id;
    char nom[50];
    int cartes[3];
    int nb_cartes;
} Joueur;

Suspect catalogue[NB_SUSPECTS] = {
    {"Sebastian Moran",    {0,0,1,0,0,0,0,1}}, {"Irene Adler",        {0,1,0,0,0,1,0,1}},
    {"Inspector Lestrade", {0,0,0,1,1,0,1,0}}, {"Inspector Gregson",  {0,0,1,1,1,0,0,0}},
    {"Inspector Baynes",   {0,1,0,1,0,0,0,0}}, {"Inspector Bradstreet",{0,0,1,1,0,0,0,0}},
    {"Inspector Hopkins",  {1,0,0,1,0,0,1,0}}, {"Sherlock Holmes",    {1,1,1,0,0,0,0,0}},
    {"John Watson",        {1,0,1,0,0,0,1,0}}, {"Mycroft Holmes",     {1,1,0,0,1,0,0,0}},
    {"Mrs. Hudson",        {1,0,0,0,0,1,0,0}}, {"Mary Morstan",       {0,0,0,0,1,1,0,0}},
    {"James Moriarty",     {0,1,0,0,0,0,0,1}}
};

char* noms_symboles[] = {"Pipe", "Ampoule", "Poing", "Couronne", "Livre", "Collier", "Oeil", "Crane"};
Joueur joueurs[NB_JOUEURS];
int coupable_id;
int tour_actuel = 0; // Le joueur 0 commence
pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;

// --- FONCTIONS SYSTÈME DE JEU ---

void diffuser(char* message) {
    printf("[BROADCAST] %s", message);
    for(int i=0; i<NB_JOUEURS; i++) {
        if(joueurs[i].socket != -1) {
            send(joueurs[i].socket, message, strlen(message), 0);
        }
    }
}

void traiter_action(int joueur_id, char* commande) {
    char msg[BUFFER_SIZE];
    int cible, symb;

    // Commande ASK : "ASK <id_cible> <id_symbole>"
    if (sscanf(commande, "ASK %d %d", &cible, &symb) == 2) {
        if (cible < 0 || cible >= NB_JOUEURS || symb < 0 || symb >= 8) {
            sprintf(msg, "Commande invalide.\n");
        } else {
            // Compter combien de fois la cible a ce symbole
            int count = 0;
            for(int c=0; c<3; c++) {
                if(catalogue[joueurs[cible].cartes[c]].symboles[symb] == 1) count++;
            }
            sprintf(msg, "REPONSE: %s demande a %s: Possedes-tu le symbole %s ? Reponse: %d\n", 
                    joueurs[joueur_id].nom, joueurs[cible].nom, noms_symboles[symb], count);
            diffuser(msg);
            
            // Passer au tour suivant
            tour_actuel = (tour_actuel + 1) % NB_JOUEURS;
            sprintf(msg, "C'est au tour de %s !\n", joueurs[tour_actuel].nom);
            diffuser(msg);
            return;
        }
    } 
    // Commande GUESS : "GUESS <id_suspect>"
    else if (sscanf(commande, "GUESS %d", &cible) == 1) {
        if (cible == coupable_id) {
            sprintf(msg, "VICTOIRE ! %s a trouve le coupable: %s !\n", joueurs[joueur_id].nom, catalogue[cible].nom);
            diffuser(msg);
            exit(0);
        } else {
            sprintf(msg, "ECHEC: %s s'est trompe d'accusation.\n", joueurs[joueur_id].nom);
            diffuser(msg);
            tour_actuel = (tour_actuel + 1) % NB_JOUEURS;
        }
    } else {
        sprintf(msg, "Format: ASK <cible> <symbole> OU GUESS <id_suspect>\n");
    }

    if(joueurs[joueur_id].socket != -1) send(joueurs[joueur_id].socket, msg, strlen(msg), 0);
}

// --- THREADS ET MAIN ---

void* thread_client(void* arg) {
    Joueur* j = (Joueur*)arg;
    char buffer[BUFFER_SIZE];
    int n;
    
    while((n = recv(j->socket, buffer, BUFFER_SIZE-1, 0)) > 0) {
        buffer[n] = '\0';
        pthread_mutex_lock(&verrou);
        if (tour_actuel == j->id) {
            traiter_action(j->id, buffer);
        } else {
            char* refused = "Ce n'est pas votre tour !\n";
            send(j->socket, refused, strlen(refused), 0);
        }
        pthread_mutex_unlock(&verrou);
    }
    return NULL;
}

void melanger_et_distribuer(){
    int deck[NB_SUSPECTS];
    int i, j, temp;

    // 1. Initialiser le paquet avec les IDs de 0 à 12
    for(i = 0; i < NB_SUSPECTS; i++) {
        deck[i] = i;
    }

    // 2. Mélanger le paquet (Algorithme Fisher-Yates)
    srand(time(NULL)); 
    for (i = NB_SUSPECTS - 1; i > 0; i--) {
        j = rand() % (i + 1);
        temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }

    // 3. La première carte du paquet mélangé est le coupable
    coupable_id = deck[0];
    printf("\n[SERVEUR] Le coupable est choisi (ID: %d). On distribue le reste...\n", coupable_id);

    // 4. Distribuer les cartes suivantes sans jamais réutiliser le même index
    int deck_idx = 1; // On commence à 1 car deck[0] est le coupable
    for(i = 0; i < NB_JOUEURS; i++) {
        for(int c = 0; c < 3; c++) {
            joueurs[i].cartes[c] = deck[deck_idx];
            deck_idx++; // On passe à la carte suivante du paquet
        }

        // Envoyer les cartes aux clients réseaux
        if(joueurs[i].socket != -1) {
            char msg[BUFFER_SIZE];
            sprintf(msg, "CARDS %d %d %d\n", 
                    joueurs[i].cartes[0], 
                    joueurs[i].cartes[1], 
                    joueurs[i].cartes[2]);
            send(joueurs[i].socket, msg, strlen(msg), 0);
        }
    }
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    if (argc < 2) { printf("Usage: ./serveur <port>\n"); exit(1); }

    joueurs[0].socket = -1; 
    joueurs[0].id = 0;
    strcpy(joueurs[0].nom, "Hôte");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    listen(sockfd, 5);

    printf("Attente de 3 joueurs...\n");
    int connectes = 1;
    while(connectes < NB_JOUEURS) {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        joueurs[connectes].socket = newsockfd;
        joueurs[connectes].id = connectes;
        sprintf(joueurs[connectes].nom, "Client_%d", connectes);
        pthread_t tid;
        pthread_create(&tid, NULL, thread_client, &joueurs[connectes]);
        connectes++;
    }

    melanger_et_distribuer();
    diffuser("--- LA PARTIE COMMENCE ---\n");
    printf("Vos cartes (Hôte): %s, %s, %s\n", catalogue[joueurs[0].cartes[0]].nom, catalogue[joueurs[0].cartes[1]].nom, catalogue[joueurs[0].cartes[2]].nom);

    char cmd[BUFFER_SIZE];
    while(1) {
        if (tour_actuel == 0) {
            printf("\n(Hôte) Votre tour: ");
            fgets(cmd, BUFFER_SIZE, stdin);
            pthread_mutex_lock(&verrou);
            traiter_action(0, cmd);
            pthread_mutex_unlock(&verrou);
        }
        sleep(1);
    }
    return 0;
}