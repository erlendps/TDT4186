#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#define MAXIMUM_CONCURRENT_ALARMS 10 // Cannot be larger than 10.

// Function declarations
char getCommand();
void schedule();
int list();
void cancel();
bool insertAlarm(time_t target);
void runAlarm(time_t target);
void makeSound();
int getAlarmNumberFromInput();
void resetAlarm(int alarmNumber);
time_t getTimeFromInput();
void pressEnterToContinue();


// struct declarations
typedef struct {
    time_t target;
    pid_t PID;
} Alarm;


// Variable declarations
char command;
char inputLine[100];
Alarm alarms[MAXIMUM_CONCURRENT_ALARMS];

void pressEnterToContinue() {
    printf("[↵] ");
    fgets(inputLine, 100, stdin);
}


int main() {

    for (int i = 0; i < MAXIMUM_CONCURRENT_ALARMS; i++) {
        resetAlarm(i);
    }

    while (1) {
        command = getCommand();
        int status;
        waitpid(-1, &status, WNOHANG);

        switch (command) {
            case 's':
                schedule();
                pressEnterToContinue();
                break;

            case 'l':
                list();
                pressEnterToContinue();
                break;

            case 'c':
                cancel();
                pressEnterToContinue();
                break;

            case 'x':  
                printf("Goodbye!\n");
                return 0;

            default:
                printf("(✗) Not a recognized command - please try again.\n");
                pressEnterToContinue();
                break;
        }
    }
}

char getCommand() {
    char answer = '0';
    char buffer[26];
    time_t timeNow = time(NULL);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", localtime(&timeNow));
    printf("\n#######################\n");
    printf("Welcome to the alarm clock! It is currently %s\n", buffer);
    printf("Please enter \"s\" (schedule), \"l\" (list), \"c\" (cancel) or \"x\" (exit)\n");
    printf("#######################\n");
    printf("> ");
    // Gets three characters from the input buffer
    // First character is the one we actually want for input
    // Second character is string termination character(??)
    // Third character is newline
    fgets(inputLine, 100, stdin);
    answer = inputLine[0];
    return answer;
}

time_t getTimeFromInput() {
    printf("> ");
    fgets(inputLine, 100, stdin);

    // Remove the last character, which is the newline entered by the user to input their string.
    char timestamp[17];
    memcpy(timestamp, &inputLine[0], 17);
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
        printf("(✗) Not a valid format - Did not set an alarm.\n");

    } else {
        insertAlarm(alarmTime);
        char buffer[26];
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", localtime(&alarmTime));
        printf("(✓) Alarm scheduled for %s\n", buffer);
    }
}

int list() {
    int count = 0;
    for (int i = 0; i < MAXIMUM_CONCURRENT_ALARMS; i++) {
        if (alarms[i].target != -1) {
            count++;
            char buffer[26];
            strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", localtime(&alarms[i].target));
            printf("* Alarm %d at ", i);
            printf("%s\n", buffer);
        }
    }
    printf("(!) You have %d alarm(s).\n", count);
    return count;
}

int getAlarmNumberFromInput() {
    printf("> ");
    fgets(inputLine, 3, stdin);
    int inputNumber = (int) inputLine[0] - 48;
    if (0 <= inputNumber && inputNumber < MAXIMUM_CONCURRENT_ALARMS) {
        return inputNumber;
    } else {
        return -1;
    }
}

void resetAlarm(int alarmNumber) {
    alarms[alarmNumber].PID = -1;
    alarms[alarmNumber].target = -1;
}

void cancel() {
    if (list() == 0) { return; }
    printf("\n(?) Which alarm would you like to cancel?\n");
    int alarmNumber = getAlarmNumberFromInput();
    if (alarmNumber != -1 && alarms[alarmNumber].PID != -1) {
        kill(alarms[alarmNumber].PID, SIGKILL);
        int status; // not used, just for storing result
        waitpid(alarms[alarmNumber].PID, &status, 0);
        resetAlarm(alarmNumber);
    } else {
        printf("(✗) You don't have an alarm with this id.\n");
    }
}

bool insertAlarm(time_t target) {
    if (target < time(NULL)) {
        //return false;
        // TODO: Remove comment
    }
    int counter = 0;
    while (alarms[counter].PID != -1 && counter < MAXIMUM_CONCURRENT_ALARMS) {
        counter++;
    }
    if (counter == MAXIMUM_CONCURRENT_ALARMS) {
        return false;
    }
    pid_t alarmPID = fork();
    if (alarmPID == 0) { // child process
        runAlarm(target);
    } else { // parent process
        alarms[counter].PID = alarmPID;
        alarms[counter].target = target;
    }
    return true;
}

void runAlarm(time_t target) {
    int timeToSleep = target - time(NULL);
    if (timeToSleep > 0) {
        sleep(timeToSleep);
    }
    makeSound();
    exit(0);
}

void makeSound() {
    char *programName = "screen";
    char *args[] = {programName, "-d", "-m", "mpg123", "../alarm_sound.mp3", "&&", "exit", NULL};
    
    int alarm = fork();

    if (alarm == 0) {
        execvp(programName, args);
        exit(0);
    }
}
