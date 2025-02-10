
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
		//code pour stocker tous les tubes dans tab_tube et les commandes dans tab_tokens
		int max_tokens=10; 
		int tab_tube[max_tokens][2];
		char ***tab_tokens = (char ***) malloc(max_tokens * sizeof(char **)); 
		if(tab_tokens==NULL){
			perror("Erruer malloc pour tab_tokens"); 
			return(1); 
		}
		char **tokens_cmd; 
		int ind_tab_tube=0; 
		while(1){
			tokens_cmd = trouve_tube(tokens, "|");
			if(tokens_cmd ==NULL){break;}

			if(pipe(tab_tube[ind_tab_tube])==-1){
					fprintf(stderr, "Erreur de creation du tube\n");
					exit(1); 
			}
			tab_tokens[ind_tab_tube]= tokens; 
			
			int i=0; 
			while(tokens_cmd[i]!=NULL){
				tokens[i]= tokens_cmd[i]; 
				i++; 
			}
			tokens[i]= tokens_cmd[i]; 
			ind_tab_tube++; 	
		}
		tab_tokens[ind_tab_tube]= tokens; //pour insérer la dernière commande
		close(tab_tube[ind_tab_tube][0]);  close(tab_tube[ind_tab_tube][1]); //il y a un pipe en plus
		 
		int j = 0;
		while( tab_tokens[0][j]!=NULL){
			printf("%s \n", tokens[j]);
			printf("%s \n", tab_tokens[0][j]);

			j++;
		} 
/* 
		if(tab_tokens[0]==NULL){ //pas de pipe on exécute la comande
				pid_t pid1 = fork();
				if(pid1==0){
					execvp(tokens[0], tokens);
					perror("execvp");
					exit(0);

				}
				break;
			}

		int parcours_tube=0; 
		while (parcours_tube <= ind_tab_tube){ 
			if( ==0){ //si on se trouve avant le premier pipe
				printf("22 \n"); 
				if(pipe(tab_tube[cmp])==-1){
					fprintf(stderr, "Erreur de creation du tube\n");
					exit(1); 
				}
				pid_t pid1 = fork();
				if(pid1==0){ //on écrit dans le premier tube si c'est la première commande
					close(tab_tube[cmp][0]);
					dup2(tab_tube[cmp][1], 1);
					execvp(tokens[0], tokens);
					perror("execvp");
					exit(0);
				}
			}
			else if(tokens_cmd2!=NULL && tokens_cmd3==NULL ){ //si la commande se trouve entre deux pipe
				printf("33 \n"); 
				if(pipe(tab_tube[cmp])==-1){
					fprintf(stderr, "Erreur de creation du tube\n");
					exit(1); 
				}
				pid_t pid1 = fork();
				if(pid1==0){ //on écrit dans le premier tube si c'est la première commande
					close(tab_tube[cmp-1][1]);
					dup2(tab_tube[cmp-1][0], 0);

					close(tab_tube[cmp][0]);
					dup2(tab_tube[cmp][1], 1);
					execvp(tokens[0], tokens);
					perror("execvp");
					exit(0);
				}

			} 
			else if(tokens_cmd2!=NULL){//quand on est à la fin
				printf("fin x\n"); 
				pid_t pid2 = fork();
				if(pid2==0){
					close(tab_tube[cmp-1][1]);
					printf("fils \n");
					dup2(tab_tube[cmp-1][0], 0);
					execvp(tokens[0], tokens);
					printf("fils 2 \n"); 
					perror("execvp");
					exit(0);
				}
				fin -= 1;
				break;  
			}
			cmp+=1; 
			int ind=0; 
			while(tokens_cmd2[ind]!=NULL){
				tokens[ind] = tokens_cmd2[ind];
				ind++;
			}
			tokens[ind]=NULL;
			// TEST
			//int j = 0;
			while(tokens[j]!=NULL){
				printf("%s ", tokens[j]);
				j++;
			}
			printf("\n fin while\n"); 

			//tokens_cmd2= trouve_tube(tokens, "|");
			//if(tokens_cmd2!=NULL){  tokens_cmd3= trouve_tube(tokens_cmd2, "|");} 
		

		
		}
		close(tab_tube[cmp][0]); 
		close(tab_tube[cmp][1]);   
		
	*/


		/*int i =0; 
		while (i <=cmp){  //pour close les pipe
			close(tab_tube[i][0]); 
			close(tab_tube[i][1]); 
		} */
			/*
					if (tokens_cmd2!=NULL){
						int tube[2];
						if(pipe(tube)==-1){
							fprintf(stderr, "Erreur de creation du tube\n");
							exit(1);
						}
						pid_t pid1 = fork();
						if(pid1==0){
							close(tube[0]);
							dup2(tube[1], 1);
							execvp(tokens[0], tokens);
							perror("execvp");
							exit(0);
						}

						pid_t pid2 = fork();
						if(pid2==0){
							close(tube[1]);
							dup2(tube[0], 0);
							execvp(tokens_cmd2[0], tokens_cmd2);
							perror("execvp");
							exit(0);
						}
						close(tube[1]);
						close(tube[0]);
						}





					*/
			//_________________4.3____début
			/*
			char *file_out = trouve_redirection(tokens, ">");
			char *file_in = trouve_redirection(tokens, "<");

			pid_t pid = fork();
			if (pid == 0) {
				if(file_out!=NULL){
					int fd_out;
					if ((fd_out = open(file_out, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) < 0) {
						perror("open");
						exit(1);
					}
					dup2(fd_out, 1);
				}
				if(file_in!=NULL){
					int fd_in;
					if ((fd_in = open(file_in, O_RDONLY, S_IRUSR | S_IWUSR)) < 0) {
						perror("open");
						exit(1);
					}
					dup2(fd_in, 0);
				}

				execvp(tokens[0], tokens);
				perror("execvp");
				exit(0);
			} */
			free(tab_tokens); 
			wait(NULL);
	
	}
	/* On ne devrait jamais arriver là */

	perror("execvp");
	exit(1);
	
}
