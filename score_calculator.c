#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <linux/limits.h>

#define LINE_SIZE 1024
#define USERNAME_SIZE 128
#define HUNT_DIR "hunt"

typedef struct UserScore
{
    char username[USERNAME_SIZE];
    int score;
    struct UserScore *next;
} UserScore;

void update_score(UserScore **head, const char *username, int value)
{

    UserScore *curr = *head;
    while (curr != NULL)
    {
        if (strcmp(curr->username, username) == 0)
        {
            curr->score += value;
            return;
        }
        curr = curr->next;
    }

    UserScore *new_user = malloc(sizeof(UserScore));
    strncpy(new_user->username, username, USERNAME_SIZE - 1);
    new_user->username[USERNAME_SIZE - 1] = '\0';
    new_user->score = value;
    new_user->next = *head;
    *head = new_user;
}

// check if path is dir
int is_directory(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
    {
        return 1;
    }
    return 0;
}

// parse line get score
void parse_line(UserScore **head, char *line)
{

    line[strcspn(line, "\n")] = 0;

    char *token = strtok(line, ","); // treasure_id

    if (!token)
    {
        return;
    }

    token = strtok(NULL, ","); // username
    if (!token)
    {
        return;
    }

    char username[USERNAME_SIZE];

    strncpy(username, token, USERNAME_SIZE - 1);
    username[USERNAME_SIZE - 1] = '\0';

    strtok(NULL, ","); // latitude
    strtok(NULL, ","); // longitude
    strtok(NULL, ","); // clue

    token = strtok(NULL, ","); // value
    if (!token)
        return;
    int value = atoi(token);

    update_score(head, username, value);
}

void process_hunt(const char *hunt_path, const char *hunt_id)
{
    char treasure_file_path[PATH_MAX];
    snprintf(treasure_file_path, sizeof(treasure_file_path), "%s/treasure_%s.dat", hunt_path, hunt_id);

    FILE *fp = fopen(treasure_file_path, "r");
    if (!fp)
    {
        fprintf(stderr, "Failed to open %s\n", treasure_file_path);
        return;
    }

    UserScore *scores = NULL;

    char line[LINE_SIZE];
    fgets(line, LINE_SIZE, fp); // header
    while (fgets(line, LINE_SIZE, fp))
    {
        parse_line(&scores, line);
    }

    fclose(fp);

    printf("Scores for hunt: %s\n", hunt_id);
    UserScore *curr = scores;
    while (curr != NULL)
    {
        printf("  %s: %d\n", curr->username, curr->score);
        UserScore *tmp = curr;
        curr = curr->next;
        free(tmp);
    }
    printf("\n");
}

int cmpstr(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

int main()
{
    char cwd[PATH_MAX / 3];
    if (!getcwd(cwd, sizeof(cwd)))
    {
        perror("getcwd");
        return 1;
    }

    char hunt_path[PATH_MAX / 2];
    snprintf(hunt_path, sizeof(hunt_path), "%s/%s", cwd, HUNT_DIR);

    DIR *dir = opendir(hunt_path);
    if (!dir)
    {
        perror("opendir");
        return 2;
    }

    char *hunt_dirs[100000];
    int hunt_count = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)))
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char hunt_subdir[PATH_MAX];
        snprintf(hunt_subdir, sizeof(hunt_subdir), "%s/%s", hunt_path, entry->d_name);

        if (is_directory(hunt_subdir))
        {
            hunt_dirs[hunt_count] = strdup(entry->d_name);
            hunt_count++;
        }
    }
    closedir(dir);

    qsort(hunt_dirs, hunt_count, sizeof(char *), cmpstr);

    for (int i = 0; i < hunt_count; ++i)
    {
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", hunt_path, hunt_dirs[i]);
        process_hunt(full_path, hunt_dirs[i]);
        free(hunt_dirs[i]);
    }

    return 0;
}
