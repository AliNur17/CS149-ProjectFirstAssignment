#include <stdio.h>
#include <string.h>
#include "parser.h"

#define MAX_INPUT 100

int main() {
    	char input[MAX_INPUT];

    	while (1) {
        	printf("{'e' to Exit} > ");

        	/* read full line input */
        	if (fgets(input, MAX_INPUT, stdin) == NULL) break;

        	/* remove newline character */
        	input[strcspn(input, "\n")] = '\0';

        	/* exit condition */
        	if (strcmp(input, "e") == 0) {
            		printf("Exiting...\n");
            	break;
        	}

        	/* send to parser */
		parseInstruct(input);
	}

	return 0;
}
