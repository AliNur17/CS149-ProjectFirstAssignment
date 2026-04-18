#include <stdio.h>
#include <string.h>
#include "commands.h"

void parseInstruct(char *input) {
    char *command;
    char *args;

    char *specifier;
    char *argument;

    //split command from rest
    char *space = strchr(input, ' ');

    if (space == NULL) {
        command = input;
        args = "";
    } else {
        *space = '\0';
        command = input;
        args = space + 1;
    }

    //split args into specifier and argument
    char *space2 = strchr(args, ' ');

    if (space2 == NULL) {
        specifier = args;
        argument = "";
    } else {
        *space2 = '\0';
        specifier = args;
        argument = space2 + 1;
    }

    //If specifier doesn't start with '-', treat it as an argument
    if (specifier[0] != '-')
    {
	    argument = specifier;
	    specifier = "";
    }

    //dispatch
    if (strcmp(command, "createF") == 0)
        createF(specifier, argument);
    else if (strcmp(command, "openF") == 0)
        openF(specifier, argument);
    else if (strcmp(command, "closeF") == 0)
        closeF(specifier, argument);
    else if (strcmp(command, "searchF") == 0)
        searchF(specifier, argument);
    else if (strcmp(command, "help") == 0)
        helpList(specifier, argument);
    else
        printf("Unknown command, please try again...\n");
}
