#include "dberror.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *RC_message;

/* print a message to standard out describing the error */
void 
printError (RC error)
{
    const char *msg;
    switch (error) {
        case RC_OK: msg = "RC_OK"; break;
        case RC_FILE_NOT_FOUND: msg = "RC_FILE_NOT_FOUND"; break;
        case RC_FILE_HANDLE_NOT_INIT: msg = "RC_FILE_HANDLE_NOT_INIT"; break;
        case RC_WRITE_FAILED: msg = "RC_WRITE_FAILED"; break;
        case RC_READ_NON_EXISTING_PAGE: msg = "RC_READ_NON_EXISTING_PAGE"; break;
        case RC_SEEK_FAILED: msg = "RC_SEEK_FAILED"; break;
        case RC_CLOSE_FAIL: msg = "RC_CLOSE_FAIL"; break;
        case RC_FILE_REMOVE_FAILED: msg = "RC_FILE_REMOVE_FAILED"; break;
        case RC_INSUFFICIENT_MEMORY: msg = "RC_INSUFFICIENT_MEMORY"; break;
        case RC_READ_FAILED: msg = "RC_READ_FAILED"; break;
        default: msg = "Unknown RC"; break;
    }
    
    if (RC_message != NULL) {
        printf("EC (%i) %s - \"%s\"\n", error, msg, RC_message);
    } else {
        printf("EC (%i) %s\n", error, msg);
    }
}

char *
errorMessage (RC error)
{
    char *message;
    if (RC_message != NULL)
    {
        message = (char *) malloc(strlen(RC_message) + 30);
        sprintf(message, "EC (%i), \"%s\"\n", error, RC_message);
    }
    else
    {
        message = (char *) malloc(30);
        sprintf(message, "EC (%i)\n", error);
    }

    return message;
}