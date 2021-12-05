/**
 * Tafsir Mbodj NDIOUR alias tmn-dev
 * Copyright (c) 2021-05-12
 * 
 * chat.c
 * Le rôle de ce programme est de se connecter au serveur.
 * Une fois connecté au serveur, il peut: 
 * - soit démarrer un nouveau chat avec un autre client connecté en envoyant d'abord 
 *   au serveur la commande 'l' pour obtenir la liste des utilisateurs connectés
 * - soit accepter une demande de connexion d'un autre client.
 * 
 * Ce programme utilise le multithreading pour la gestion de la lecture en boucle et 
 * de l'écriture en boucle dans la socket pour le chat. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/wait.h>
#include <assert.h>

#define MAXMSGSIZE 1024
#define ADDRESS_SIZE 64
#define NB_CLIENT 5
#define PORT 9600

char listClient[NB_CLIENT][ADDRESS_SIZE];
int taille_tab;

/**
 * Affiche une message d'erreur et arrête l'exécution du programme
 */
void error(char* message);
/**
 * Affiche la liste des utilisateurs connectés
 */
void print_clients();
/**
 * Menu permettant au client de choisir le rôle de serveur ou de client
 */
int menu1();
/**
 * Menu pour choisir l'utilisateur à contacter
 */
int menu2();
/**
 * Chargée de la lecture en boucle dans la socket
 */
void* reader(void *arg);
/**
 * Chargée de l'écriture en boucle dans la socket
 */
void* writer(void *arg);
/**
 * Efface l'écran du console
 */
void clear_screen();
/**
 * Sélectionne un utilisateur parmi ceux connectés pour le contacter ou
 * d'attendre une connexion entrante
 */
int selectUser(int fd);
/**
 * Démarre un chat en jouant le role de client
 */
void* start_chat(void* arg);
/**
 * Joue le rôle de serveur et donc accepte une connexion entrante
 */
void* accept_chat_request(void* arg);

/**
 * 3 arguments en ligne de commande sont attendus par ce programme:
 * - le nom de l'exécutable qui sera ./chat
 * - l'adresse IP du serveur
 * - l'adresse IP du pc qui lance ce programme.
 * Exemple: ./chat 10.188.163.16 10.188.163.88
 */
int main(int argc, char* argv[]){
    assert(argc == 3);
    int sock;
    struct sockaddr_in server_addr;
    char host_addr[ADDRESS_SIZE] = "";
    pthread_t th;
    int len, userSelected;

    sock = socket(AF_INET, SOCK_STREAM, 0); //Création de la socket
    if(sock == -1)
        error("Erreur de création de la socket\n");
    // Initialisation du contexte d'adressage du serveur à contacter
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(PORT);
    len = sizeof(struct sockaddr_in);
    if(connect(sock, (struct sockaddr*) &server_addr, len) < 0)
        error("Erreur de connexion\n");

    userSelected = selectUser(sock);
    if(userSelected == -1){
        printf("En attente d'une nouvelle connexion...\n");
        // Création du thread chargé de l'acceptation d'une nouvelle demande de chat
        pthread_create(&th, NULL, accept_chat_request, argv[2]);
        pthread_join(th, NULL);
    } else{
        if(userSelected == -2)
            printf("Pas d'utilisateur connecté\n");
        else{
            strcpy(host_addr, listClient[userSelected]);
            // Création du thread chargé de démarrer un nouveau chat
            pthread_create(&th, NULL, start_chat, host_addr);
            pthread_join(th, NULL);
        }
    }
    close(sock);

    return 0;
}

void error(char* message){
    perror(message);
    exit(EXIT_FAILURE);
}

void print_clients(){
    for(int i = 0; i < taille_tab; i++)
        printf("%d -> %s\n", i, listClient[i]);
}

void* start_chat(void* arg){
    char ipaddr[ADDRESS_SIZE] = "";
    int s_sock;
    int len;
    struct sockaddr_in s_addr;
    pthread_t thReader, thWriter;

    sprintf(ipaddr, "%s", (char*) arg);
    s_sock = socket(AF_INET, SOCK_STREAM, 0);
    len = sizeof(struct sockaddr_in);
    if(s_sock == -1)
        error("Erreur lors de la création de la socket");

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(PORT);
    s_addr.sin_addr.s_addr = inet_addr(ipaddr);
    if(connect(s_sock, (struct sockaddr*) &s_addr, len) < 0)
        error("Erreur de connexion\n");
    clear_screen();
    usleep(100);
    printf("Début de la conversation avec %s\n\n", ipaddr);
    pthread_create(&thWriter, NULL, writer, &s_sock);
    pthread_create(&thReader, NULL, reader, &s_sock);
    pthread_join(thWriter, NULL);
    pthread_join(thReader, NULL);
    close(s_sock);

    pthread_exit(NULL);
}

void* accept_chat_request(void* arg){
    char ipaddr[ADDRESS_SIZE] = "";
    int s_sock, new_sock;
    struct sockaddr_in s_addr, c_addr;
    int len;
    pthread_t thReader, thWriter;
    sprintf(ipaddr, "%s", (char*) arg);
    s_sock = socket(AF_INET, SOCK_STREAM, 0);
    len = sizeof(struct sockaddr_in);
    if(s_sock == -1)
        error("Erreur lors de la création de la socket");

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(PORT);
    s_addr.sin_addr.s_addr = inet_addr(ipaddr);
    if(bind(s_sock, (struct sockaddr*) &s_addr, len) < 0)
        error("Erreur d'attachement de la socket");
    if(listen(s_sock, 1) < 0)
        error("Erreur d'écoute sur la socket");
    new_sock = accept(s_sock, (struct sockaddr*) &c_addr, (socklen_t*) &len);
    if(new_sock < 0)
        error("Erreur accept");
    clear_screen();
    usleep(100);
    printf("Début de la conversation avec %s\n\n", inet_ntoa(c_addr.sin_addr));
    pthread_create(&thWriter, NULL, writer, &new_sock);
    pthread_create(&thReader, NULL, reader, &new_sock);
    pthread_join(thWriter, NULL);
    pthread_join(thReader, NULL);
    close(s_sock);
    close(new_sock);

    pthread_exit(NULL);
}

void* reader(void *arg){
    int fd = *((int*) arg);
    char msg[MAXMSGSIZE] = "";
    while(1){
        read(fd, msg, MAXMSGSIZE);
        printf("\t\t\t\t%s\n", msg);
        bzero((char*) msg, MAXMSGSIZE);
    }
    pthread_exit(NULL);
}

void* writer(void* arg){
    int fd = *((int*) arg);
    char buffer[MAXMSGSIZE] = "";
    int len;
    while(1){
        fgets(buffer, MAXMSGSIZE, stdin);
        len = strlen(buffer);
        buffer[len - 1] = '\0';
        write(fd, buffer, len + 1);
        bzero((char*)buffer, MAXMSGSIZE);
    }
    pthread_exit(NULL);
}

int selectUser(int fd){
    char commande = 'l';
    int nb_octets, reponse;
    taille_tab = 0;
    reponse = menu1();
    if(reponse == 2)
        return -1;
    nb_octets = write(fd, &commande, sizeof(char));
    if(nb_octets < 0)
        error("Erreur d'écriture sur la socket");
    nb_octets = read(fd, &taille_tab, sizeof(int));
    if(nb_octets < 0)
        error("Erreur de lecture sur la socket\n");
    for(int i = 0; i < taille_tab; i++){
        nb_octets = read(fd, listClient[i], ADDRESS_SIZE);
        if(nb_octets < 0)
            error("Erreur de lecture sur la socket\n");
    }
    reponse = menu2();
    return reponse;
}

int menu1(){
    int reponse;
    do{
        printf("Veuillez choisir un nombre parmi les options ci-dessous:\n");
        printf("1) Démarrer un chat\n");
        printf("2) Accepter une demande de connexion\n");
        scanf("%d", &reponse);
    } while(reponse != 1 && reponse != 2);
    return reponse;
}

int menu2(){
    int reponse;
    if(taille_tab == 0)
        return -2;
    do{
        printf("Veuillez choisir un nombre parmi les utilisateurs ci-dessous:\n");
        print_clients();
        scanf("%d", &reponse);
    } while(reponse < 0 || reponse >= taille_tab);
    return reponse;
}

void clear_screen(){
    if(!fork())
        execlp("clear", "clear", NULL);
    wait(NULL);
}