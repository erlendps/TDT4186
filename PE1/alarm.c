#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Function declarations
char getSelection();
void schedule();
void list();
void cancel();
void getcharnws(char*);
// Variable declarations
char selection;
char inputline[100];

int main() {
    

    while (1) {
        selection = getSelection();

        switch (selection) {
            case 's':
                schedule();
                break;

            case 'l':
                list();
                break;

            case 'c':
                cancel();
                break;

            case 'x':  
                printf("Goodbye!\n");
                exit(0);

            default:
                printf("Not a recognized selection - please try again.\n");
                printf("Press enter to continue...\n");

                // Termination character + newline
                fgets(inputline, 2, stdin);
                printf("\n\n");
                break;
        }
    }
}

char getSelection() {
    char answer = '0';
    printf("Welcome to the alarm clock! It is currently ...\n");
    printf("Please enter \"s\" (schedule), \"l\" (list), \"c\" (cancel) or \"x\" (exit)\n");
    printf("> ");

    // Gets three characters from the input buffer
    // First character is the one we actually want for input
    // Second character is string termination character(??)
    // Third character is newline
    fgets(inputline, 3, stdin);
    answer = inputline[0];
    return answer;
}

time_t getTimeFromInput() {
    char timeInput[18];
    printf("> ");
    fgets(timeInput, 18, stdin);

    // Remove the last character, which is the newline entered by the user to input their string.
    char timestamp[17];
    memcpy(timestamp, &timeInput[0], 17);
    timestamp[16] = '\0';
    
    struct tm timeStruct;
    memset(&timeStruct, 0, sizeof(struct tm));

    // Convert a timestamp in the form YYYY-mm-dd hh:mm
    strptime(timestamp, "%Y-%m-%d %R", &timeStruct);
    return mktime(&timeStruct);
}

void schedule() {
    printf("Schedule alarm at which date and time?\n");
    time_t alarmTime = getTimeFromInput();
    if (alarmTime == -1) {
        printf("Not a valid format - Did not set an alarm.\n");
        printf("Press enter to continue...\n");
        fgets(inputline, 2, stdin);
        printf("\n\n");

    } else {
        // Print timestamp to see if it works
        printf("%lld\n", (long long) alarmTime);
        // set alarm
    }
}

void list() {

}

void cancel() {

}