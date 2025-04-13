#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define PWD_SIZE 800
#define PATH_SIZE 1000
#define PATH2_SIZE 1500
#define LINE_SIZE 256
#define FILE_READ 2000
//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))
//((((((((((((((((((((((((((((((((((((((((((((((  ADD TREASURE ))))))))))))))))))))))))))))))))))))))))))))))
//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))

/////////////////////////////////////
//returns current time formated nicely
char* get_time(){

    static char current_time[100];
    time_t t = time(NULL);
    struct tm *tm_info=localtime(&t);
    strftime(current_time,sizeof(current_time), "%Y-%m-%d %H:%M:%S", tm_info);
    return current_time;

}

//////////////////////////////////////
//return path of hunt or null
DIR* search_hunt(const char *hunt_id, const char *base_dir) {
    struct dirent *entry;
    DIR *dir=opendir(base_dir);

    if (dir==NULL) {
        perror("opendir");
        return NULL;
    }

    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name,".")==0 ||strcmp(entry->d_name, "..")== 0) //skip parent and current
            continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", base_dir, entry->d_name);

        struct stat statbuf;
        if(stat(path,&statbuf)==-1) { //struct exists
            perror("stat");
            continue;
        }

        if(strcmp(entry->d_name,hunt_id)==0 &&S_ISDIR(statbuf.st_mode)) { //dir is found
            closedir(dir);
            return opendir(path);  
        }

    }

    closedir(dir);
    return NULL;
}


/////////////////////////////////////
//searches/ creates hunt dir and creates treasurefile /& adds treasure
void add_treasure(const char* hunt_id, const char* treasure_id) {

    //create hunt path
    char pwd_path[PWD_SIZE];
    getcwd(pwd_path, PWD_SIZE);
    char base_path[PATH_SIZE];
    snprintf(base_path,sizeof(base_path),"%s/hunt",pwd_path);
    mkdir(base_path, 0777);
    DIR *hunt_dir = search_hunt(hunt_id, base_path);
    FILE *log;
    
    if (hunt_dir == NULL) {

        //create dir
        char hunt_path[PATH2_SIZE];
        snprintf(hunt_path, sizeof(hunt_path), "%s/%s", base_path, hunt_id);

        if (mkdir(hunt_path, 0777) != 0) {
            perror("could not create hunt dir");
            return;
        }

        hunt_dir = opendir(hunt_path);
        if (hunt_dir == NULL) {
            perror("opendir on newly created hunt dir failed");
            return;
        }

        printf("Created and opened new hunt dir: %s\n", hunt_path);


        //create hunt log 
        FILE *hunt_id_log;
        char log_path[PATH2_SIZE+100];

        snprintf(log_path, sizeof(log_path), "%s/%s_logs.txt", hunt_path, hunt_id);
        printf("hunt log: %s\n",log_path);
        if((hunt_id_log=fopen(log_path,"w"))==NULL){
             perror("could not create hunt_log");
            return;
        }
        fputs("///////////////////////////////////////////////\n",hunt_id_log);
        fputs(hunt_id,hunt_id_log);
        fputs(" log created at: ",hunt_id_log);
        fputs(get_time(),hunt_id_log);
        fputs("\n///////////////////////////////////////////////\n",hunt_id_log);

        //create final_log and symlink
        
        //FILE *final_hunt_id_log;
        char final_log_path[PATH_SIZE];

        snprintf(final_log_path, sizeof(final_log_path), "%s/logs/final_%s_logs.txt", pwd_path, hunt_id);
        //printf("final hunt log: %s",final_log_path);

        
    //creates logs if they dont exist
       char logs_path[PATH_SIZE];
        snprintf(logs_path, sizeof(logs_path), "%s/logs", pwd_path);
        mkdir(logs_path, 0777);

        //create symlink
         if (symlink(log_path, final_log_path) == 0) {
            printf("Symlink created: %s ---> %s\n", final_log_path, log_path);
        } else {
            perror("error creating symlink\n");
            return;
        }
        log=hunt_id_log;
        

        } else {
            printf("Hunt found!\n");

            char log_path[2000];
            char log_name[40];

            snprintf(log_name,sizeof(log_name),"%s_logs.txt",hunt_id);
            snprintf(log_path,sizeof(log_path),"%s/%s/%s",base_path,hunt_id,log_name);
             if((log=fopen(log_path,"a"))==NULL){
                printf("error opening treasure.dat file\n");
                exit(4);
             }
        }

    //create /open treasure file
    FILE *t_file;
    char t_file_path[PATH2_SIZE];
    char fname[LINE_SIZE];
    snprintf(fname,sizeof(fname),"treasure_%s.dat",hunt_id);
    snprintf(t_file_path,sizeof(t_file_path),"%s/%s/%s",base_path,hunt_id,fname);
    printf("%s\n",t_file_path);

    if((t_file=fopen(t_file_path,"a+"))==NULL){
        printf("error opening treasure.dat file\n");
        exit(4);
    }

    //case of old or new treasure file
    fseek(t_file, 0, SEEK_END);     
    long size = ftell(t_file);       
    if (size == 0) {
        
        fwrite("data-->id,user_id,lat,lon,clue,value\n",37*(sizeof(char)),1,t_file);
        fputs("created new ",log);
        fputs(fname,log);
        fputs(" ",log);
        fputs(get_time(),log);
        fputs("\n///////////////////////////////////////////////\n",log);
        fputs("///////////////begining of logged data////////////\n",log);

    }
    else{
        fseek(log,0,SEEK_CUR);
    }

    //getting data
    printf("Now insert the treasure data\n");
    printf("user_id: ");
    char user_id[30];
    scanf("%30s",user_id);

    printf("latitude,longitude: ");
    float latitude ,longitude;
    scanf("%30f %30f",&latitude,&longitude);

    printf("clue:");
    char clue[100];
    getchar(); 
    fgets(clue, sizeof(clue), stdin);
    clue[strcspn(clue, "\n")] = 0;

    printf("value:");
    int value;
    scanf("%d",&value);

    //adding data
    char treasure_data[LINE_SIZE];
    snprintf(treasure_data,sizeof(treasure_data),"%s,%s,%f,%f,%s,%d",treasure_id,user_id,latitude,longitude,clue,value);
    fwrite(treasure_data, sizeof(char), strlen(treasure_data), t_file);
    fputc('\n', t_file);

    fputs("added ",log);
    fputs(treasure_id,log);
    fputs(" ",log);
    fputs(get_time(),log);
    fputs("\n",log);

    fclose(log);
    fclose(t_file);
}


//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))
//((((((((((((((((((((((((((((((((((((((((((((((  LIST HUNT ))))))))))))))))))))))))))))))))))))))))))))))
//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))

void print_treasure(char buf[]){

            buf[strcspn(buf,"\n")]=0;
            char *item = strtok(buf,",");
            printf("Treasure_id: %s",item);

            item=strtok(NULL,",");
            printf(" found by %s",item);

            item=strtok(NULL,",");
            printf(" at location lat:%s",item);

            item=strtok(NULL,",");
            printf(" long:%s",item);

            item=strtok(NULL,",");
            printf(" with clue:%s",item);

            item=strtok(NULL,",");
            printf(" and value:%s\n",item);
}

void list_hunt(const char hunt_id[]){
    char pwd_path[PWD_SIZE];
    getcwd(pwd_path, PWD_SIZE);
    char base_path[PATH_SIZE];
    snprintf(base_path,sizeof(base_path),"%s/hunt",pwd_path);
    DIR *hunt_dir = search_hunt(hunt_id, base_path);
    
    
    if (hunt_dir == NULL){
        printf("Hunt_id not found among directories\n");
        exit(5);
    }
    else{

        FILE *treasure_file;
        FILE *log_file;

        char tr_path[PATH2_SIZE];
        char log_path[PATH2_SIZE];
        
        snprintf(tr_path,sizeof(tr_path),"%s/%s/treasure_%s.dat",base_path,hunt_id,hunt_id);
        snprintf(log_path,sizeof(log_path),"%s/%s/%s_logs.txt",base_path,hunt_id,hunt_id);

        if((treasure_file=fopen(tr_path,"r"))==NULL){
            printf("Failed opening .dat file\n");
            exit(5);
        }

        if((log_file=fopen(log_path,"a"))==NULL){
            printf("Failed opening log file\n");
            exit(5);
        }

        //log action to log file
        fseek(log_file,0,SEEK_END);
        fputs("Treasures viewed at: ",log_file);
        fputs(get_time(),log_file);
        fputc('\n',log_file);
        fclose(log_file);


        //parse file and print each treasure
        char buf[LINE_SIZE];
        printf("All data contained in treasure_%s.dat file...\n",hunt_id);
        fgets(buf,LINE_SIZE,treasure_file);
        //printf("%s",buf);

        while(fgets(buf,LINE_SIZE,treasure_file)){
            
            print_treasure(buf);
        }
        
        fclose(treasure_file);
        
    }
}


//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))
//((((((((((((((((((((((((((((((((((((((((((((((  VIEW TREASURE ))))))))))))))))))))))))))))))))))))))))))))))
//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))



void view_treasure(const char hunt_id[],const char tr_id[]){

    char pwd_path[PWD_SIZE];
    getcwd(pwd_path, PWD_SIZE);
    char base_path[PATH_SIZE];
    snprintf(base_path,sizeof(base_path),"%s/hunt",pwd_path);
    DIR *hunt_dir = search_hunt(hunt_id, base_path);
    
    
    if (hunt_dir == NULL){
        printf("Hunt_id not found among directories\n");
        exit(5);
    }
    else{

        FILE *treasure_file;
        FILE *log_file;

        char tr_path[PATH2_SIZE];
        char log_path[PATH2_SIZE];
        
        snprintf(tr_path,sizeof(tr_path),"%s/%s/treasure_%s.dat",base_path,hunt_id,hunt_id);
        snprintf(log_path,sizeof(log_path),"%s/%s/%s_logs.txt",base_path,hunt_id,hunt_id);

        if((treasure_file=fopen(tr_path,"r"))==NULL){
            printf("Failed opening .dat file\n");
            exit(5);
        }

        if((log_file=fopen(log_path,"a"))==NULL){
            printf("Failed opening log file\n");
            exit(5);
        }

        //parse file and print searched treasure
        char buf[LINE_SIZE];
        fgets(buf,LINE_SIZE,treasure_file);
        int found=0;
   
        while(fgets(buf,LINE_SIZE,treasure_file)){
            
            char buf_cpy[100];
            strcpy(buf_cpy,buf);
            char *tr_cand;
            tr_cand=strtok(buf_cpy,",");

            if(strcmp(tr_cand,tr_id)==0){
                printf("Our treasure:\n");
                print_treasure(buf);

            //log action to log file
            fseek(log_file,0,SEEK_END);
            fputs("Treasure found and viewed at: ",log_file);
            fputs(get_time(),log_file);
            fputc('\n',log_file);
            found=1;
            }
        }

        if(!found){
            printf("Treasure %s is not in hunt %s\n",tr_id,hunt_id);
        }
        
        fclose(treasure_file);
        fclose(log_file);
    }
}


//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))
//((((((((((((((((((((((((((((((((((((((((((((((  REMOVE TREASURE ))))))))))))))))))))))))))))))))))))))))))))))
//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))

void remove_treasure(const char*hunt_id, const char* tr_id){

    char pwd_path[PWD_SIZE];
    getcwd(pwd_path, PWD_SIZE);
    char base_path[PATH_SIZE];
    snprintf(base_path,sizeof(base_path),"%s/hunt",pwd_path);
    DIR *hunt_dir = search_hunt(hunt_id, base_path);
    
    
    if (hunt_dir == NULL){
        printf("Hunt_id not found among directories\n");
        exit(5);
    }
    else{
        
        FILE* treasure_file;
        FILE* temp_tr_file;
        FILE* log_file;

        char tr_path[PATH2_SIZE];
        char temp_tr_path[PATH2_SIZE];
        char log_path[PATH2_SIZE];
        
        snprintf(tr_path,sizeof(tr_path),"%s/%s/treasure_%s.dat",base_path,hunt_id,hunt_id);
        snprintf(temp_tr_path,sizeof(temp_tr_path),"%s/%s/temp_treasure_%s.dat",base_path,hunt_id,hunt_id);
        snprintf(log_path,sizeof(log_path),"%s/%s/%s_logs.txt",base_path,hunt_id,hunt_id);

        if((treasure_file=fopen(tr_path,"r+"))==NULL){
            printf("Failed opening .dat file\n");
            exit(5);
        }
        if((temp_tr_file=fopen(temp_tr_path,"w+"))==NULL){
            printf("Failed opening temp.dat file\n");
            exit(5);
        }

        if((log_file=fopen(log_path,"a"))==NULL){
            printf("Failed opening log file\n");
            exit(5);
        }


        //parse file and print searched treasure
       int found=0;
        char buf[FILE_READ];

        while(fgets(buf,sizeof(buf),treasure_file)){

            char* potential_tr=strstr(buf,tr_id);
            
            if(potential_tr!=NULL){
                found=1;
                char *rest_buf=strchr(potential_tr,'\n')+1;
                buf[buf-potential_tr]='\0';
                fputs(buf,temp_tr_file);
                fputs(rest_buf,temp_tr_file);

            }else{
                fputs(buf,temp_tr_file);
            }
        }

        

        //log action to log file

        fseek(log_file,0,SEEK_END);
        fputs(tr_id,log_file);
        if(found){
            fputs(" Treasure found and removed at: ",log_file);

            //also replace the tr_file with temp
             if(remove(tr_path)!=0){
                printf("Could not delete tr_file\n");
                exit(6);
            }else{
                printf("tr_file removed!\n");
            }

             if(rename(temp_tr_path,tr_path)!=0){
                printf("Could not rename temp_tr_file\n");
                exit(6);
            }else{
                printf("tr_file renamed. exchange succesful!\n");
            }

        }else{
            fputs(" Treasure could not be removed at: ",log_file);
            printf("Treasure %s not found\n",tr_id);
            //just delete temp
             if(remove(temp_tr_path)!=0){
                printf("Could not delete temp_tr_file\n");
                exit(6);
            }else{
                printf("temp_tr_file removed!\n");
            }
        }
        fputs(get_time(),log_file);
        fputc('\n',log_file);
        fclose(log_file);

        fclose(treasure_file);
        
         
    }


}

//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))
//((((((((((((((((((((((((((((((((((((((((((((((  REMOVE HUNT ))))))))))))))))))))))))))))))))))))))))))))))
//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))

void remove_hunt(const char* hunt_id){

    char pwd_path[PWD_SIZE];
    getcwd(pwd_path, PWD_SIZE);
    char base_path[PATH_SIZE];
    snprintf(base_path,sizeof(base_path),"%s/hunt",pwd_path);
    DIR *hunt_dir = search_hunt(hunt_id, base_path);
    
    
    if (hunt_dir == NULL){
        printf("Hunt_id not found among directories\n");
        exit(5);
    }
    else{
    
        char tr_path[PATH2_SIZE];
        char log_path[PATH2_SIZE];
        
        snprintf(tr_path,sizeof(tr_path),"%s/%s/treasure_%s.dat",base_path,hunt_id,hunt_id);
        snprintf(log_path,sizeof(log_path),"%s/%s/%s_logs.txt",base_path,hunt_id,hunt_id);

        if(remove(tr_path)!=0){
            printf("Could not delete tr_file\n");
            exit(6);
        }else{
            printf("tr_file removed!\n");
        }

        if(remove(log_path)!=0){
            printf("Cold not delete log_file\n");
            exit(6);
        }{
            printf("log_file removed!\n");
        }

        char hunt_path[PATH2_SIZE];
        snprintf(hunt_path, sizeof(hunt_path), "%s/%s", base_path, hunt_id);

        if(rmdir(hunt_path)!=0){
             printf("Cold not delete hunt_dir\n");
            exit(6);
        }{
            printf("hunt_id_dir removed!\n");
        }

        char link_path[PATH2_SIZE];
        snprintf(link_path, sizeof(link_path), "%s/logs/final_%s_logs.txt", pwd_path,hunt_id);

        if(unlink(link_path)!=0){
            {
            printf("Could not remove symlink\n");
            exit(6);
        }
        }{
            printf("symlink removed!\n");
        }
    }
}

//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))
//((((((((((((((((((((((((((((((((((((((((((((((  MAIN   ))))))))))))))))))))))))))))))))))))))))))))))
//(((((((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))))))))))))))

int main(int argc, char **argv){

    if(argc!=3 && argc!=4){
        printf("Wrong input. Corect usage and instruction set below...\n");
        printf("<.exe> add <hunt_id> <treasure_id>\n");
        printf("<.exe> list <hunt_id>\n");
        printf("<.exe> view <hunt_id> <treasure_id>\n");
        printf("<.exe> remove_treasure <hunt_id> <treasure_id\n");
        printf("<.exe> remove_hunt <hunt_id>\n");
        exit(0);
    }
    

    //////////////  Add a new treasure to the specified hunt (game session). Each hunt is stored in a 
    // separate directory.

    if(strcmp(argv[1],"add")==0){

        if(argc!=4){
            printf("wrong input for add\n");
            exit(1);
        }

        add_treasure(argv[2],argv[3]);//input data, store in right hunt/ create new one 

    }

    //////////////  List all treasures in the specified hunt. First print the hunt name, the (total) file
    // size and last modification time of its treasure file(s), then list the treasures.

    else if(strcmp(argv[1],"list")==0){

        if(argc!=3){
            printf("wrong input: list <hunt_id>\n");
            exit(2);
        }

        list_hunt(argv[2]);//take huntid find it and parse if exists

    }

    /////////////////////////////////// View details of a specific treasure
    else if(strcmp(argv[1],"view")==0){
        
        if(argc!=4){
            printf("wrong input: view <hunt_id> <id>\n");
            exit(2);
        }

        view_treasure(argv[2],argv[3]);// takes hunt and treasure and prints treasure data
    }

    ///////////////////////////////// remove entire treasure
    else if(strcmp(argv[1],"remove_treasure")==0){

        if(argc!=4){
            printf("wrong input: remove_treasure <hunt_id> <id>\n");
            exit(2);
        }

        remove_treasure(argv[2],argv[3]);//search if exists then do
    }

    ////////////////////////////////////remove entire hunt
    else if(strcmp(argv[1],"remove_hunt")==0){
        
        if(argc!=3){
            printf("wrong input: remove_hunt <hunt_id>\n");
            exit(2);
        }

        remove_hunt(argv[2]);//search if exists then do
    }

    else{
        printf("\nWrong instruction set. Corect usage below...\n");
        printf("<.exe> add <hunt_id> <treasure_id>\n");
        printf("<.exe> list <hunt_id>\n");
        printf("<.exe> view <hunt_id> <treasure_id>\n");
        printf("<.exe> remove_treasure <hunt_id> <treasure_id\n");
        printf("<.exe> remove_hunt <hunt_id>\n");
        exit(3);
    }




}