/**
 * Tafsir Mbodj NDIOUR alias tmn-dev
 * Copyright (c) 2021-05-12
 * 
 * server.c
 * Le role de ce programme est d'accepter de nouvelles connexions entrantes (5 maximum)
 * et de fournir la liste des utilisateurs connectés (adresse ip).
 * Un client connecté au serveur peut envoyer la commande 'l' et ainsi recevoir la 
 * liste des utilisateurs connectés y compris lui même.
 * 
 * Ce programme utilise le multithreading afin de gérer plusieurs clients en même temps.
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
#include <assert.h>

#define MAXMSGSIZE 1024
#define ADDRESS_SIZE 64
#define PORT 9600
#define NB_CLIENT 5

char listClient[NB_CLIENT][ADDRESS_SIZE];
int taille_tab;

/**
 * Affiche une message d'erreur et arrête l'exécution du programme
 */
void error(char* msgError);
/**
 * Envoie la liste des utilisateurs connectés
 */
void send_list_user(int fd);
/**
 * Gère une nouvelle connexion entrante
 */
void* handle_client(void* arg);
/**
 * Affiche la liste des clients
 */
void print_clients();
/**
 * Gère les commandes envoyés par un client
 */
void handleCommand(char c, int fd);

int main(int argc, char* argv[]){
    int s_sock, new_sock;
    struct sockaddr_in s_addr, c_addr;
    int len;
    int i = 0;
    pthread_t clients[NB_CLIENT]; //Tableau de thread pour la gestion des client

    taille_tab = 0;
    len = sizeof(struct sockaddr_in);
    s_sock = socket(AF_INET, SOCK_STREAM, 0); //Création de la socket
    if(s_sock == -1)
        error("Erreur lors de la création de la socket");

    // Initialisation du contexte d'adressage de la socket serveur
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(PORT);
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // Attachement de la socket à son contexte d'adressage
    if(bind(s_sock, (struct sockaddr*) &s_addr, len) < 0)
        error("Erreur d'attachement de la socket");
    if(listen(s_sock, 5) < 0) //Ecoute de nouvelle connexion sur la socket
        error("Erreur d'écoute sur la socket");

    while(i < NB_CLIENT){
        new_sock = accept(s_sock, (struct sockaddr*) &c_addr, (socklen_t*) &len);
        if(new_sock < 0)
            error("Erreur accept");
        strcpy(listClient[taille_tab], inet_ntoa(c_addr.sin_addr));
        taille_tab++;
        print_clients();
        // Création d'un nouveau thread pour gérer le nouveau client connecté
        pthread_create(&clients[i], NULL, handle_client, &new_sock);
        i++;
    }
    printf("Nombre maximum de connexion atteint.\n");
    i = 0;
    // Attente de la terminaison des threads en cours d'exécution
    while(i < NB_CLIENT){
        pthread_join(clients[i], NULL);
        i++;
    }
    close(s_sock);

    return 0;
}

void error(char* msgError){
    perror(msgError);
    exit(EXIT_FAILURE);
}

void send_list_user(int fd){
    int nbOctets;
    // Envoi d'abord de lu nombre de clients connectés
    nbOctets = write(fd, &taille_tab, sizeof(int));
    if(nbOctets < 0)
        error("Erreur d'écriture sur la socket");
    // Envoi des adresses IP des clients
    for(int i = 0; i < taille_tab; i++){
        nbOctets = write(fd, listClient[i], strlen(listClient[i])+1);
        if(nbOctets < 0)
            error("Erreur d'écriture sur la socket");
        usleep(500);
    }
}

void* handle_client(void* arg){
    int sock;
    int nbOctet;
    char commande;
    sock = *((int*) arg);
    nbOctet = read(sock, &commande, sizeof(char));
    if(nbOctet < 0)
        error("Erreur de lecture sur la socket");
    // Traitement des commandes envoyés par le client
    handleCommand(commande, sock);
    close(sock);
    pthread_exit(NULL);
}

void print_clients(){
    printf("Liste des clients:\n");
    for(int i = 0; i < taille_tab; i++)
        printf("- %s\n", listClient[i]);
}

void handleCommand(char c, int fd){
    switch(c){
        case 'l':
            send_list_user(fd);
            break;
        default: ;
    }
}