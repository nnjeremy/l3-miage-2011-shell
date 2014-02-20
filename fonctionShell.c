#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "fonctionShell.h"

void monShell(char *arge[]) {
    //Fonction principale
    char cmd[100] = "";
    char *com[10];
    char * caractereSpe;
    char prompt[100];
    //signal(SIGINT,SIG_IGN);
    printf("Voici mon shell, taper Q pour sortir\n");
    getcwd(prompt, 100); //On récupère notre repèrtoire courant
    printf("%s@[%s]--> ", getlogin(), getcwd(NULL, 0)); //Prompt = nomUtilisateur@[repertoireCourant]
    while (strcmp(cmd, "Q") != 0) {
        gets(cmd); // On récupère une commande de l'utilisateur
        lireCommande(cmd, com); //On coupe la commande et on la range proprement dans un tableau com
        caractereSpe = rechercheCaractereSpecial(com); //On recherche l'existance d'un caractere spécial
        if (caractereSpe != NULL && (strcmp(caractereSpe, "|") == 0)) //Si on retrouve l'existence de tube(s), on appelle la fonction adéquate
            executePipe(com, arge);
        else if (caractereSpe != NULL && (strcmp(caractereSpe, ";") == 0)) //Si on retrouve l'existence de ";"
            executeCommandeSequentielle(com, arge);
        else if (caractereSpe != NULL && (strcmp(caractereSpe, "<") == 0 || strcmp(caractereSpe, ">") == 0))
            modifEntreeSortie(com, arge); //Si on a un caractere spécial, on modifie l'E/S
        else { //Sinon, on doit executer la commande normalement
            if (testCommandeInterne(com[0])) { //On test si la commande est une commande interne au Shell
                execCommandeInterne(com, arge); //On execute la commande
            } else {
                execCommandeExterne(com);
            }
        }
        getcwd(prompt, 100);
        printf("%s@[%s]--> ", getlogin(), prompt); //Prompt = nomUtilisateur@[repertoireCourant]
    }
}

int testCommandeInterne(char * nomCommande) {
    //Retourne 1 ssi cmd est une commande interne au shell 0 sinon
    if (nomCommande != NULL && (strcmp(nomCommande, "cd") == 0 || strcmp(nomCommande, "echo") == 0 ||
            strcmp(nomCommande, "pwd") == 0 || strcmp(nomCommande, "getenv") == 0 ||
            strcmp(nomCommande, "exit") == 0))
        return 1;
    else
        return 0;
}

void execCommandeInterne(char * commande[], char * arge[]) {
    //Execution des différentes commandes internes
    if (strcmp(commande[0], "cd") == 0) {
        if (commande[1] == NULL)
            chdir(getenv("HOME"));
        else
            if (chdir(commande[1])) perror(commande[1]);
    } else if (strcmp(commande[0], "pwd") == 0)
        printf("%s\n", getcwd(NULL, 0));
    else if (strcmp(commande[0], "getenv") == 0) {
        if (commande[1] == NULL)
            printf("Veuillez donnez un argument pour la commande getenv\n");
        else
            if (getenv(commande[1]) == NULL)
            printf("Variable d'environnement non trouvable\n");
        else
            printf("%s\n", getenv(commande[1]));
    } else if (strcmp(commande[0], "echo") == 0) {
        if (*commande[1] == '$') {
            del_char(commande[1], '$');
            char * pPath;
            pPath = getenv(commande[1]);
            if (pPath != NULL)
                printf("%s\n", pPath);
        } else {
            int i = 0;
            int j = 1;
            while (i < nbElementsCommande(commande) && commande[i] != NULL) {
                if (commande[j] != NULL)
                    printf("%s ", commande[j]);
                i++;
                j++;
            }
            printf("\n");
        }
    } else if (strcmp(commande[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(commande[0], "getenv") == 0) {
        char * pPath;
        pPath = getenv(commande[1]);
        if (pPath != NULL)
            printf("%s\n", pPath);
    } else if (strcmp(commande[0], "setenv") == 0) {
        if (commande[1] != NULL && commande[2] != NULL)
            setenv(commande[1], commande[2], strlen(commande[2]));
        else if (commande[1] != NULL)
            setenv(commande[1], " ", 1);
        else {
            int setenvcpt = 0;
            while (arge[setenvcpt] != NULL) {
                printf("%s\n", arge[setenvcpt]);
                setenvcpt++;
            }
        }
    }
}

void del_char(char* str, char c)
//Enleve tous les c de str
{
    int id_read, id_write;
    id_read = 0;
    id_write = 0;

    while (str[id_read] != '\0') {
        if (str[id_read] != c) {
            str[id_write] = str[id_read];
            id_write++;
        }
        id_read++;
    }
    str[id_write] = '\0';
}

void execCommandeExterne(char * commande[]) {
    //Execute toute commande externe au Shell
    char * commande_new[10];
    int i;
    for (i = 0; i <= nbElementsCommande(commande) - 2; i++)
        commande_new[i] = commande[i];

    int pid = fork();
    switch (pid) {
        case -1:
            perror("Erreur fork");
            exit(-1);
            break;
        case 0:
            if (commande[nbElementsCommande(commande) - 1] != NULL && commande[nbElementsCommande(commande) - 1][0] == '&')
                execvp(commande_new[0], commande_new);
            else
                execvp(commande[0], commande);
            exit(-1);
            break;
        default:
            if (commande[nbElementsCommande(commande) - 1] != NULL && commande[nbElementsCommande(commande) - 1][0] != '&')
                waitpid(pid, NULL, 0);
    }
}

void lireCommande(char * ligneCommande, char * nomCommande[]) {
    //Découpe ligneCommande et range proprement la commande et ses arguments dans nomCommande
    char *result = NULL;
    char delimiteur[] = " ";
    result = strtok(ligneCommande, delimiteur);
    int i = 0;
    while (result != NULL) {
        nomCommande[i] = result;
        result = strtok(NULL, delimiteur);
        i++;
    }
    nomCommande[i] = (char*) NULL; //Nécessaire pour la bonne éxecution de execvp

}

int nbElementsCommande(char * commande[]) {
    //Retourne le nombre d'éléments présents dans commande
    int i = 0;
    while (commande[i] != NULL)
        i++;
    return i;
}

void modifEntreeSortie(char * commande[], char * arge[]) {
    //Execute commande dans le cas d'une redirection d'E/S
    int redirectEntree = 0; //Position de la dernière occurrence de '<' dans commande
    int redirectSortie = 0; //Position de la dernière occurrence de '>' dans commande
    int descripteurFicEntree, descripteurFicSortie;
    char * commandeAExecuter[10]; //On déclare un tableau avec la commande a executer sans les caractères spéciaux
    rechercheCaractereEntreeSortie(commande, &redirectEntree, &redirectSortie); //On récupère les positions des caractères
    switch (fork()) {
        case -1:
            printf("Erreur fork\n");
            break;
        case 0:
            if (redirectEntree && redirectSortie) {
                //On redirige l'entrée et la sortie
                if ((descripteurFicEntree = open(commande[redirectEntree + 1], O_RDONLY)) == -1) { //Le fichier sur lequel on va lire doit exister
                    printf("Le fichier %s n'existe pas\n", commande[redirectEntree + 1]);
                    exit(0);
                }
                if ((descripteurFicSortie = open(commande[redirectSortie + 1], O_WRONLY | O_TRUNC)) == -1) //Si le fichier sur lequel on va ecrire n'existe pas, il faut le créer
                    descripteurFicSortie = open(commande[redirectSortie + 1], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                //On dispose donc maintenant des descripteurs des fichiers d'Entrée et de Sortie
                close(0); //On ferme l'entrée standard
                dup(descripteurFicEntree); //On remplace cette entrée standard par notre fichier
                close(1); //on ferme la sortie standard
                dup(descripteurFicSortie); //On remplace cette sortie standard par notre fichier
                //On récupère la commande à executer
                int i;
                int positionFinalCar;
                //La commande et ses arguments doivent se trouver avant le premier signe d'E/S
                if (redirectEntree < redirectSortie)
                    positionFinalCar = redirectEntree;
                else
                    positionFinalCar = redirectSortie;
                for (i = 0; i < positionFinalCar; i++)
                    commandeAExecuter[i] = commande[i];
                commandeAExecuter[positionFinalCar] = (char*) NULL;
            } else if (redirectEntree && !redirectSortie) {
                if ((descripteurFicEntree = open(commande[redirectEntree + 1], O_RDONLY)) == -1)
                    printf("Le fichier %s n'existe pas\n", commande[redirectEntree + 1]);
                else {
                    close(0); //On libère la place
                    dup(descripteurFicEntree); //On place notre fichier à la place de stdin
                    int i = 0;
                    while (i < nbElementsCommande(commande) && (strcmp(commande[i], "<") != 0)) {
                        commandeAExecuter[i] = commande[i];
                        i++;
                    }
                    commandeAExecuter[i] = (char*) NULL;
                }
            } else if (!redirectEntree && redirectSortie) {
                //On redirige la sortie mais pas l'entrée
                if ((descripteurFicSortie = open(commande[redirectSortie + 1], O_WRONLY | O_TRUNC)) == -1)
                    descripteurFicSortie = open(commande[redirectSortie + 1], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                close(1); //On ferme la sortie standard
                dup(descripteurFicSortie); //On place notre fichier en position 1
                int i = 0;
                while (i < nbElementsCommande(commande) && (strcmp(commande[i], ">") != 0)) {
                    commandeAExecuter[i] = commande[i];
                    i++;
                }
                commandeAExecuter[i] = (char*) NULL;
            }
            if (testCommandeInterne(commandeAExecuter[0]))//On teste la nature de la commande
                execCommandeInterne(commandeAExecuter, arge);
            else
                execCommandeExterne(commandeAExecuter);
            exit(0);
            break;
        default:
            wait(NULL);
    }
}

int nbPipeCommande(char * commande[]) {
    //Retourne le nombre de tubes contenus dans commande
    int nbPipe = 0;
    int i;
    for (i = 0; i < nbElementsCommande(commande); i++) {
        if (strcmp(commande[i], "|") == 0)
            nbPipe++;
    }
    return nbPipe;
}

int nbCommSeq(char * commande[]) {
    //Retourne le nombre de commandes contenues dans commande
    int nbCom = 0;
    int i;
    for (i = 0; i < nbElementsCommande(commande); i++) {
        if (strcmp(commande[i], ";") == 0)
            nbCom++;
    }
    return nbCom + 1;
}

void nbCommandesCaractereSpeciaux(char * commande[], int tabNbCommande[]) {
    int i = 0, j = 0;
    int nbCommande = 0;
    for (i = 0; i < nbElementsCommande(commande); i++) {
        if (strcmp(commande[i], "|") != 0 || strcmp(commande[i], ";") != 0) {
            tabNbCommande[j] = ++nbCommande;
        } else {
            j++;
            nbCommande = 0;
        }
    }
}

void executePipe(char * commande[], char * arge[]) {
    //On crée un tableau de pipe -> Chaque case est destinée à recevoir un tube
    int nbPipe = nbPipeCommande(commande);
    int tabNbCommande[nbPipe + 1];
    nbCommandesCaractereSpeciaux(commande, tabNbCommande);
    int ** tabPipe = malloc(nbPipe * sizeof (int*));
    int i; //Compteur servant pour allouer la zone des pipes
    for (i = 0; i < nbPipe; i++) {
        tabPipe[i] = (int*) malloc(2 * sizeof (int)); //Un tube est un tableau de 2 éléments (0 pour la lecture, 1 pour l'écriture)
        pipe(tabPipe[i]);
    }

    //On crée notre tableau de commande sans tube
    int nbElements = nbElementsCommande(commande) - nbPipe;
    int j, k = 0, m = 0;
    char *** tabTouteCommande = (char***) malloc((nbElements + 1) * sizeof (char**));
    for (j = 0; j <= nbElements; j++) { //On crée notre tableau destiné à recevoir l'ensemble de nos commandes
        tabTouteCommande[j] = (char**) malloc((tabNbCommande[j] + 1) * sizeof (char*)); //On alloue une case supplémentaire pour le (char*)NULL à la fin
        while (k < (nbElements + nbPipe) && strcmp(commande[k], "|") != 0) { //Tant qu'on ne tombe pas sur "|"
            tabTouteCommande[j][m] = commande[k]; //On récupère la commande (ou ses arguments)
            m++;
            k++;
        }
        tabTouteCommande[j][m] = (char*) NULL; //Pour la bonne execution de la commande
        if (k < (nbElements + nbPipe) && strcmp(commande[k], "|") == 0)
            k++; //On saut le tube pour pouvoir revenir dans la boucle
        m = 0;
    }
    int cptCom = 0; //Compteur de commande
    int cptPipe = -1; //Compteur de pipe
    int nbCom = nbPipe + 1;
    for (cptCom = 0; cptCom < nbCom; cptCom++) {
        cptPipe++;
        switch (fork()) {
            case -1:
                //Aucun nouveau processus n'a pu être créé
                printf("Erreur Fork\n");
                break;
            case 0:
                if (cptCom == 0) {
                    //On utilise le 1er Pipe sur lequel on va écrire
                    close(tabPipe[cptPipe][0]); //On ferme le côté lecture du pipe
                    dup2(tabPipe[cptPipe][1], 1); //On remplace la sortie standard par la sortie du pipe
                    close(tabPipe[cptPipe][1]); //On peut désormais fermer la sortie du pipe
                } else if (cptCom > 0 && tabTouteCommande[cptCom + 1][0] != NULL) {//On est au milieu
                    //On doit lire sur le pipe-1 et ecrire sur le prochain pipe
                    close(tabPipe[cptPipe - 1][1]); //On ferme la sortie du pipe-1
                    dup2(tabPipe[cptPipe - 1][0], 0); //On remplace l'entrée standard par l'entrée du pipe-1
                    close(tabPipe[cptPipe - 1][0]); //On peut fermer l'entrée du pipe-1
                    //Pour la suite, on redirige la sortie standard sur la sortie du pipe
                    close(tabPipe[cptPipe][0]);
                    dup2(tabPipe[cptPipe][1], 1);
                    close(tabPipe[cptPipe][1]);
                } else if (tabTouteCommande[cptCom + 1][0] == NULL) {
                    //On est à la fin. Donc on lit sur le pipe-1 et on ne redirige pas la sortie standard
                    close(tabPipe[cptPipe - 1][1]);
                    dup2(tabPipe[cptPipe - 1][0], 0);
                    close(tabPipe[cptPipe - 1][0]);
                }
                if (testCommandeInterne(tabTouteCommande[cptCom][0]))
                    execCommandeInterne(tabTouteCommande[cptCom], arge);
                else
                    execvp(tabTouteCommande[cptCom][0], tabTouteCommande[cptCom]);
                exit(0);
                break;
            default:
                /*Le père doit fermer les pipes que plus personne n'utilise en veillant
                                 à ne pas les fermer trop tôt*/
                if (cptCom >= 1) {
                    close(tabPipe[cptCom - 1][0]);
                    close(tabPipe[cptCom - 1][1]);
                }
        }
    }
    for (cptCom = 0; cptCom < nbCom; cptCom++) {
        //On attend TOUS nos fils
        free(tabTouteCommande[cptCom]);
        wait(NULL);
    }

    free(tabPipe); //On libère le tableau de pipe
    free(tabTouteCommande); //On libère le tableau de commandes
}

void executeCommandeSequentielle(char * commande[], char * arge[]) {
    //On crée notre tableau de commande sans tube
    int nbElements = nbElementsCommande(commande); //Total d'éléments de notre commande (en comptant les ";")
    int nbCommande = nbCommSeq(commande); //Nombre de commandes sans compter les ";"
    int tabNbCommande[nbCommande];
    nbCommandesCaractereSpeciaux(commande, tabNbCommande);
    int j, k = 0, m = 0;
    char *** tabTouteCommande = (char***) malloc((nbCommande + 1) * sizeof (char**));
    for (j = 0; j <= nbCommande; j++) { //On crée notre tableau destiné à recevoir l'ensemble de nos commandes
        tabTouteCommande[j] = (char**) malloc((tabNbCommande[j] + 1) * sizeof (char*)); //On alloue une case supplémentaire pour le (char*)NULL à la fin
        while (k < (nbElements) && strcmp(commande[k], ";") != 0) { //Tant qu'on ne tombe pas sur ";"
            tabTouteCommande[j][m] = commande[k]; //On récupère la commande (ou ses arguments)
            m++;
            k++;
        }
        tabTouteCommande[j][m] = (char*) NULL; //Pour la bonne execution de la commande
        if (k < (nbElements) && strcmp(commande[k], ";") == 0)
            k++; //On saut le tube pour pouvoir revenir dans la boucle
        m = 0;
    }
    int i;
    for (i = 0; i < nbCommande; i++) { //On peut désormais executer nos commandes
        switch (fork()) {
            case -1:
                perror("Erreur fork\n");
                break;
            case 0:
                if (testCommandeInterne(tabTouteCommande[i][0]) != 0) //On test si il s'agit d'une commande interne
                    execCommandeInterne(tabTouteCommande[i], arge);
                else
                    //execCommandeInterne(tabTouteCommande[i]);
                    execvp(tabTouteCommande[i][0], tabTouteCommande[i]);
                break;
            default:
                wait(NULL);
        }
    }
    free(tabTouteCommande);
}

char * rechercheCaractereSpecial(char * commande[]) {
    //Renvoie le caractere spécial (< > |) ou NULL si on en trouve pas
    char * caractereSpecial = NULL;
    int nbElement = nbElementsCommande(commande);
    int i = 0;
    while (i < nbElement && (!(strcmp(commande[i], ">") == 0 || strcmp(commande[i], "<") == 0
            || strcmp(commande[i], "|") == 0 || strcmp(commande[i], ";") == 0))) {
        i++;
    }
    if (i < nbElement) {
        caractereSpecial = commande[i];
    }
    return caractereSpecial;
}

int nbCaracteresSpeciaux(char * commande[]) {
    //Retourne le nombre de caractères spéciaux
    int nbCar = 0;
    int i;
    for (i = 0; i < nbElementsCommande(commande); i++) {
        if (strcmp(commande[i], "<") == 0 || strcmp(commande[i], ">") == 0 || strcmp(commande[i], "|") == 0)
            nbCar++;
    }
    return nbCar;
}

void rechercheCaractereEntreeSortie(char * com[], int * redirectEntree, int * redirectSortie) {
    int positCarEntree = 0;
    int positCarSortie = 0;
    int i = 0;
    for (i = 0; i < nbElementsCommande(com); i++) {
        if (strcmp(com[i], "<") == 0)
            positCarEntree = i;
        if (strcmp(com[i], ">") == 0)
            positCarSortie = i;
    }
    *redirectEntree = positCarEntree;
    *redirectSortie = positCarSortie;
}

