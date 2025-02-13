
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


			//_________________4.3____début
//fonction qui va rediriger l'entrée standard s'il y a un < et la sortie standard s'il y a un >
// prend un argument un tokens, et le modifie avec l'appel à trouve_redirection si il y a une rédirection
void rediriger(char **tokens){  
	char *file_out = trouve_redirection(tokens, ">");
	char *file_in = trouve_redirection(tokens, "<");
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
}

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
		
		
		char **tokens_cmd = trouve_tube(tokens, "|");
		int parcours =0; 
		int first=0;
		int max_pipe = 300;   //nombre maximal de pipe
		int tab_tube[max_pipe][2];
		while (1 ){ 
			if(first==0 && tokens_cmd==NULL){ //pas de pipe on exécute la comande
				pid_t pid1 = fork();
				if(pid1==0){
					rediriger(tokens); //pour vérifier s'il n'y a pas de redirection
					execvp(tokens[0], tokens);
					perror("execvp");
					exit(0);

				}
				break;
			}
			first++; 

			if(parcours ==0){ //si on se trouve avant le premier pipe
				if(pipe(tab_tube[parcours])==-1){
					fprintf(stderr, "Erreur de creation du tube\n");
					exit(1); 
				}
				pid_t pid = fork();
				if(pid==0){ //on écrit dans le premier tube si c'est la première commande
					close(tab_tube[parcours][0]);
					dup2(tab_tube[parcours][1], 1);
					execvp(tokens[0], tokens);
					perror("execvp");
					exit(0);
				}
			}

			else if(tokens_cmd!=NULL && parcours!=0 ){ //si la commande se trouve entre deux pipe
				if(pipe(tab_tube[parcours])==-1){
					fprintf(stderr, "Erreur de creation du tube\n");
					exit(1); 
				}
				close(tab_tube[parcours-1][1]); // on fermer l'écriture du pipe précédant car on va plus écrire dedans, 
												//afin que le pipe qui lit puisse savoir qu'on écrit plus, comme ça il restera pas bloquer
				pid_t pid = fork();
				if(pid==0){ 
					close(tab_tube[parcours-1][1]);
					dup2(tab_tube[parcours-1][0], 0);

					close(tab_tube[parcours][0]);
					dup2(tab_tube[parcours][1], 1);

					execvp(tokens[0], tokens);
					perror("execvp");
					exit(0);
				}
				close(tab_tube[parcours - 1][0]); //on ferme la lecture du pipe précédant car on va plus lire dedans
			} 
			else if(tokens_cmd==NULL){//quand on est à la fin
				close(tab_tube[parcours -1][1]) ;  //on ferme l'écriture du tube précédent avant le fork pour s'assurer que le processus d'avant a fini d'écrire
				pid_t pid_fin = fork();
				if(pid_fin==0){
					close(tab_tube[parcours-1][1]);
					dup2(tab_tube[parcours-1][0], 0);

					rediriger(tokens); //pour vérifier s'il n'y a pas de redirection
					execvp(tokens[0], tokens);
					printf("fils 2 \n"); 
					perror("execvp");
					exit(0);
				}
					
				close(tab_tube[parcours-1][0]);   //on ferme la lecture car on va plus lire depuis ce tube
				break;  
			}


			parcours+=1; 
			int ind=0; 
			while(tokens_cmd[ind]!=NULL){
				tokens[ind] = strdup(tokens_cmd[ind]);
				ind++;
			}
			tokens[ind]=NULL;
			tokens_cmd = trouve_tube(tokens, "|");
			
		}
		int status; 
		while(waitpid(-1, &status, 0)> 0) ;  //pour attendre tous les processus

	
	}
	/* On ne devrait jamais arriver là */

	perror("execvp");
	exit(1);
	
}

