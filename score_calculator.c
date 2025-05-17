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

// Add or update a user's score
void add_score(const char *username, int value) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            users[i].total_score += value;
            return;
        }
    }

    // New user
    if (user_count < MAX_USERS) {
        strncpy(users[user_count].username, username, sizeof(users[user_count].username)-1);
        users[user_count].total_score = value;
        user_count++;
    }
}

// Parse a single treasure line and extract user and value
void parse_line(char *line) {
    line[strcspn(line, "\n")] = 0;  // Remove newline

    char *token = strtok(line, ","); // ID
    if (!token) return;

    token = strtok(NULL, ","); // Username
    if (!token) return;
    char username[64];
    strncpy(username, token, sizeof(username)-1);

    // Skip lat, long, clue
    for (int i = 0; i < 3; i++) {
        token = strtok(NULL, ",");
        if (!token) return;
    }

    token = strtok(NULL, ","); // Value
    if (!token) return;
    int value = atoi(token);

    add_score(username, value);
}

int main(int argc, char *argv[]) {
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

    // Skip header if present
    fgets(line, sizeof(line), f);

    while (fgets(line, sizeof(line), f)) {
        parse_line(line);
    }

    fclose(f);

    // Print results for pipe to read
    for (int i = 0; i < user_count; i++) {
        printf("%s: %d\n", users[i].username, users[i].total_score);
    }

    return 0;
}
