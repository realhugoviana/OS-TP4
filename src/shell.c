
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>   //pour wait
#include "shell-utils.h"
#include <fcntl.h>
#include <sys/stat.h>

#define INPUT_BUFFER_SIZE 2048
#define MAX_TOKEN_NB 512

int main() {
	/* Une variable pour sotcker les caractères tapés au clavier */
	char line[INPUT_BUFFER_SIZE+1];

	/* Une variable qui pointera vers les données lues par fgets
	 * Sauf problème (erreur, plus de données, ...), elle pointera en
	 * fait vers la variable précédente (line) */
	char* data;

	/* Un tableau où les mots de l'utilisateur seront stockés
	 * indépendamment les uns des autres
	 * Note: un mot est une suite de lettres séparées par une ou plusieurs
	 * espaces des autres mots.  */
	char* tokens[MAX_TOKEN_NB+1];
	/* variables entières pour compter le nombre de token */
	int nb_tokens;

	while(1){
		printf("COMMANDE: "); 
		/* Récupération des données entrées au clavier */
		data=fgets(line, INPUT_BUFFER_SIZE, stdin);

		if (data==NULL) {
			/* Erreur ou fin de fichier : on quitte tout de suite */
			if (errno) {
				/* Une erreur: on affiche le message correspondant
				* et on quitte en indiquant une erreur */
				perror("fgets");
				exit(1);
			} else {
				/* Sinon ça doit être une fin de fichier.
				* On l'indique et on quitte normalement */
				fprintf(stderr, "EOF: exiting\n");
				exit(0);
			}
		}
		
		nb_tokens=split_tokens(tokens, data, MAX_TOKEN_NB);

		/* S'il y a trop de tokens, on abandonne */
		if (nb_tokens==MAX_TOKEN_NB) {
			fprintf(stderr, "Too many tokens: exiting\n");
			exit(1);
		}
		if (nb_tokens<=0) {
			fprintf(stderr, "No tokens found: exiting\n");
			exit(1);
		}
		
		if (strcmp(tokens[0], "exit") == 0) {
			printf("EOF: exiting\n");
			exit(0);
		}

		//_________________4.3____début
		int trouver = 0; //pour savoir s'il y a une rédirection
		char *file_out = trouve_redirection(tokens, ">");
		if(file_out!=NULL){
			trouver = 1; 
			pid_t pid = fork();
			if (pid == 0) { 
				int fd;
				if ((fd = open(file_out, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) < 0) {
					perror("open");
					exit(1);
				}
				dup2(fd, 1);
				execvp(tokens[0], tokens);
				perror("execvp");
				exit(0);
			}
			
		}
		
		file_out=trouve_redirection(tokens, "<");
		if(file_out!=NULL){
			trouver = 1; 
			pid_t pid = fork();
			if (pid == 0) { 
				int fd;
				if ((fd = open(file_out, O_RDONLY, S_IRUSR | S_IWUSR)) < 0) {
					perror("open");
					exit(1);
				}
				dup2(fd, 0);
				execvp(tokens[0], tokens);
				perror("execvp");
				exit(0);
			}
			

		} //_____4.3_fin
		
		if(trouver==0){ //on exécute la commande s'il n'y a pas de rédirection
		pid_t pid;
		pid = fork();
		if (pid == 0) { 
			execvp(tokens[0], tokens);
			perror("execvp");
			
			exit(0);
		}
		}
		wait(NULL);
	}

	/* On ne devrait jamais arriver là */

	perror("execvp");
	exit(1);
}
