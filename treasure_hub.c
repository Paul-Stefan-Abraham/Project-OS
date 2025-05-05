#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <bits/sigaction.h>
#include <asm-generic/signal-defs.h>
#include <dirent.h>
#include <sys/stat.h>

//For no compatibility problems
//gcc -Wall treasure_hub.c -o hub.exe

//our central data keeping file, easiest version.
#define CMD_FILE ".hub_command"
#define PATH_SIZE 1000


pid_t monitor_pid = -1;
int monitor_active = 0;
int waiting_for_monitor = 0;


//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))
//((((((((((((((((((((((((((((((((((((((((((((((  LIST ALL HUNTs ))))))))))))))))))))))))))))))))))))))))))))))
//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))

void read_symlinked_files(){

    DIR *dir = opendir("./logs");
    if (!dir) {
        perror("Failed to open directory");
        return;
    }

    struct dirent *entry;
    char symlink_path[PATH_SIZE];
    char target_path[PATH_SIZE];
    ssize_t len;

    printf("All hunts and their contained treasures...\n");
    while((entry=readdir(dir))!=NULL) {
        
        snprintf(symlink_path,sizeof(symlink_path),"%s/%s","./logs",entry->d_name);
        
        struct stat st;
        if (lstat(symlink_path, &st)==-1) {
            perror("lstat failed");
            continue;
        }

        if(!S_ISLNK(st.st_mode)){//not simlink skip
            continue;
        }


        len=readlink(symlink_path,target_path, sizeof(target_path)-1);

        if (len==-1) {
            perror("failed to read symlink");
            continue;
        }
        target_path[len]='\0';

        FILE *fp = fopen(target_path,"r");
        if (!fp) {
            perror("failed to open target file\n");
            continue;
        }

        char line[PATH_SIZE];
        fgets(line, sizeof(line), fp);
        fgets(line, sizeof(line), fp);
        char *huntnb=strtok(line," "); 
        printf("%s ",huntnb);
        fgets(line, sizeof(line), fp);
        fgets(line, sizeof(line), fp);
        fgets(line, sizeof(line), fp);
        fgets(line, sizeof(line), fp);

        int tr_count=0;
        while (fgets(line, sizeof(line), fp)) {
            
            char *command=strtok(line," ");
           
            if(strcmp(command,"added")==0){
                tr_count++;
                
            }
            else if(strcmp(command,"removed")==0){
                tr_count--;
            }
        }
        printf("%d treasures\n",tr_count);

        fclose(fp);
    }

    closedir(dir);
}



//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))
//((((((((((((((((((((((((((((((((((((((((((((((  MONITOR  ))))))))))))))))))))))))))))))))))))))))))))))
//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))

void handle_usr1(int sig) {//upgrade handle to actualy handle displaing
    char cmd[256];
    FILE *f=fopen(CMD_FILE, "r");
    if (!f) {
        perror("monitor: fopen");
        return;
    }
    if (fgets(cmd, sizeof(cmd),f)==NULL) {
        perror("monitor: fgets");
        fclose(f);
        return;
    }
    fclose(f);

    cmd[strcspn(cmd,"\n")]=0; // get rid newline

    if (strncmp(cmd,"list_hunts",10)==0){
        read_symlinked_files();
    } 

    else if(strstr(cmd, "list_treasures")!=NULL){

        char full_cmd[512];
        strtok(cmd, " "); 
        char *args=strtok(NULL,""); 

        if(args==NULL){
            fprintf(stderr, "monitor: missing arguments for view_treasure\n");
            return;
        }
        snprintf(full_cmd, sizeof(full_cmd),"./hunt.exe list %s", args);
        system(full_cmd);

    }

    else if(strstr(cmd,"view_treasure")!=NULL){
    char full_cmd[512];
    strtok(cmd," "); 
    char *args=strtok(NULL, ""); 
    if (args==NULL) {
        fprintf(stderr, "monitor: missing arguments for view_treasure\n");
        return;
    }
    snprintf(full_cmd,sizeof(full_cmd), "./hunt.exe view %s", args);
    system(full_cmd);
    }

    else {
        fprintf(stderr, "monitor: unknown command: %s\n", cmd);
    }
}

void handle_term(int sig) {
    usleep(1000000); // delay 1s
    exit(0);
}

void monitor_loop(){

    struct sigaction sa_usr1, sa_term;

    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags=SA_RESTART;
    sa_usr1.sa_handler=handle_usr1;
    sigaction(SIGUSR1,&sa_usr1, NULL);

    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags=SA_RESTART;
    sa_term.sa_handler=handle_term;
    sigaction(SIGTERM,&sa_term, NULL);

    while (1) {
        pause();
    }
}

//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))
//((((((((((((((((((((((((((((((((((((((((((((((  HUB ))))))))))))))))))))))))))))))))))))))))))))))
//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))


//keeping a record of commands
void write_command_file(const char *cmd) {
    FILE *f=fopen(CMD_FILE,"w");
    if (!f) {
        perror("hub: fopen");
        return;
    }
    fprintf(f,"%s\n",cmd);
    fclose(f);
}

//terminate a pid process
void handle_sigchld(int sig) {
    int status;
    pid_t pid=waitpid(monitor_pid,&status, WNOHANG);
    if (pid>0) {
        printf("Monitor terminated with status %d.\n", WEXITSTATUS(status));
        monitor_active=0;
        waiting_for_monitor=0;
    }
}

void start_monitor() {
    if (monitor_active) {
        printf("Monitor already running.\n");
        return;
    }

    monitor_pid=fork();
    if (monitor_pid==0) {
        monitor_loop();
        exit(0);
    } else if (monitor_pid>0) {
        monitor_active=1;
        printf("Monitor started with PID %d.\n", monitor_pid);
    } else {
        perror("fork");
    }
}

void stop_monitor() {
    if (!monitor_active) {
        printf("Monitor is not running.\n");
        return;
    }
    kill(monitor_pid, SIGTERM);
    waiting_for_monitor = 1;
    printf("Sent termination signal to monitor.\n");
}

//just send forward command
void send_command_to_monitor(const char *cmd) {
    if (!monitor_active) {
        printf("Monitor is not running.\n");
        return;
    }
    if (waiting_for_monitor) {
        printf("Monitor is shutting down. Please wait...\n");
        return;
    }
    write_command_file(cmd);
    kill(monitor_pid, SIGUSR1);
}



//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))
//((((((((((((((((((((((((((((((((((((((((((((((  MAIN   ))))))))))))))))))))))))))))))))))))))))))))))
//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))


int main() {

    struct sigaction sa_chld;
    sa_chld.sa_handler = handle_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa_chld, NULL);

    char input[256];
    printf("List of commands: start_monitor, stop_monitor, list_hunts, list_treasures <huntid>, view_treasure <huntid-treasureid>, help, exit\n");

    while (1) {
        printf("-> ");
        if(!fgets(input, sizeof(input), stdin)){
            break;
        }

        input[strcspn(input, "\n")]=0; 

        char *command=strtok(input, " ");
        char *args=strtok(NULL, ""); 

        if(command==NULL){
            continue;
        }

        if(strcmp(command, "start_monitor")==0){
            start_monitor();
        } 
        else if(strcmp(command, "stop_monitor")==0){
            stop_monitor();
        } 
        else if(strcmp(command, "list_hunts")==0){
            send_command_to_monitor("list_hunts");
        } 
        else if(strcmp(command, "list_treasures") == 0 && args != NULL){

            char full_cmd[256];
            snprintf(full_cmd, sizeof(full_cmd), "list_treasures %s",args);
            send_command_to_monitor(full_cmd);
        } 
        else if(strcmp(command,"view_treasure")==0 && args!=NULL){
            char full_cmd[256];
            
            snprintf(full_cmd, sizeof(full_cmd), "view_treasure %s", args);
            send_command_to_monitor(full_cmd);
        } 
        else if(strcmp(command, "help")==0){
            printf("List of commands: start_monitor, stop_monitor, list_hunts, list_treasures <huntid>, view_treasure <huntid-treasureid>, help, exit\n");
        } 
        else if(strcmp(command, "exit")==0){
            if(monitor_active){
                printf("Error: monitor is still running. Close it then exit...\n");
            } 
            else{
                break;
            }
        } 
        else{
            printf("Unknown command.\n");
        }
    }

    return 0;
}