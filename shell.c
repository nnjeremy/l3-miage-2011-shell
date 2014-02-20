#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include "fonctionShell.h"


int main(int argc, char *argv[], char *arge[]){

	monShell(arge);
	return EXIT_SUCCESS;
}
