#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define LINE_SIZE 512
#define PATH_SIZE 512
#define MAX_USERS 100

typedef struct {
    char username[64];
    int total_score;
} UserScore;

UserScore users[MAX_USERS];
int user_count = 0;

///////////////////////////////////////////////////////////
//add to user
void add_score(const char *username, int value) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            users[i].total_score += value;
            return;
        }
    }

    if (user_count < MAX_USERS) {
        strncpy(users[user_count].username, username, sizeof(users[user_count].username)-1);
        users[user_count].total_score = value;
        user_count++;
    }
}


//get one data line
void parse_line(char *line) {
    line[strcspn(line, "\n")] = 0; 

    char *token = strtok(line, ","); 
    if (!token) return;

    token = strtok(NULL, ","); 
    if (!token) return;
    char username[64];
    strncpy(username, token, sizeof(username)-1);

    //skip some
    for (int i = 0; i < 3; i++) {
        token = strtok(NULL, ",");
        if (!token) return;
    }

    token = strtok(NULL, ","); 
    if (!token) return;
    int value = atoi(token);

    add_score(username, value);
}

int main(int argc, char *argv[]) {//get hunt and parse it
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_id>\n", argv[0]);
        return 1;
    }

    char *hunt_id = argv[1];
    char path[PATH_SIZE];
    snprintf(path, sizeof(path), "./hunt/%s/treasure_%s.dat", hunt_id, hunt_id);

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Could not open file: %s\n", path);
        return 2;
    }

    char line[LINE_SIZE];

    fgets(line, sizeof(line), f);

    while (fgets(line, sizeof(line), f)) {
        parse_line(line);
    }

    fclose(f);

    //results 2 pipe
    for (int i = 0; i < user_count; i++) {
        printf("%s: %d\n", users[i].username, users[i].total_score);
    }

    return 0;
}
