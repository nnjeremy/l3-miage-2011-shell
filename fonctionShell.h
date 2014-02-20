#ifndef FONCTION_SHELL
#define FONCTION_SHELL


//Fonction principale
void monShell(char *argv[]);

//Stocke une ligne de commande dans ligneCommande
void lireCommande(char * ligneCommande, char * nomCommande[]);

//Teste si nomCommande est une commande interne au Shell
int testCommandeInterne(char * nomCommande);

//Execute la commande interne correspondant a commande[]
void execCommandeInterne(char * commande[], char * arge[]);

//Enleve tous les c presents dans str
void del_char(char* str, char c);

//Execute toute commande externe au Shell
void execCommandeExterne(char * commande[]);

//Retourne le nombre d'éléments dans commande
int nbElementsCommande(char * commande[]);

//Retourne le nombre de caractères spéciaux
int nbCaracteresSpeciaux(char * commande[]);

//Retourne le nombre de pipe(s) saisi(s) dans la commande
int nbPipeCommande(char * commande[]);

//Execute commande dans le cas d'une redirection d'E/S
void modifEntreeSortie(char * commande[], char * arge[]);

void nbCommandesCaractereSpeciaux(char * commande[], int tabNbCommande[]);

//Execute commande dans le cas ou les commandes sont chainées entres elles par des tubes
void executePipe(char * commande[], char * arge[]);

//Retourne le nombre de commandes à executer séquentiellement
int nbCommSeq(char * commande[]);

//Execution des commandes de manières sequentielle (séparée par ;)
void executeCommandeSequentielle(char * commande[], char * arge[]);

//Recherche dans com[] la dernière occurrence des caractères d'E/S
void rechercheCaractereEntreeSortie(char * com[], int * redirectEntree, int * redirectSortie);

//Renvoie le caractere spécial (< > |) ou NULL si on en trouve pas
char * rechercheCaractereSpecial(char * com[]);


#endif
