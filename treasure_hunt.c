#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char **argv){

    if(argc!=3 && argc!=4){
        printf("Wrong imput at the start 3 or 4 args req\n");
        exit(0);
    }
    

    //////////////  Add a new treasure to the specified hunt (game session). Each hunt is stored in a 
    // separate directory.

    if(strcmp(argv[1],"add")==0){

        if(argc!=3){
            printf("wrong input for add\n");
            exit(1);
        }

        //add_treasure(argv[2]);//take id then take data from stdin and add to existing hunt or create new one

    }

    //////////////  List all treasures in the specified hunt. First print the hunt name, the (total) file
    // size and last modification time of its treasure file(s), then list the treasures.

    else if(strcmp(argv[1],"list")==0){

        if(argc!=3){
            printf("wrong input: list <hunt_id>\n");
            exit(2);
        }

        //list_hunt(argv[2]);//take huntid find it and parse if exists

    }

    /////////////////////////////////// View details of a specific treasure
    else if(strcmp(argv[1],"view")==0){
        
        if(argc!=4){
            printf("wrong input: view <hunt_id> <id>\n");
            exit(2);
        }

        //list_view(argv[2],argv[3])// takes hunt and treasure and prints treaas data
    }

    ///////////////////////////////// remove entire treasure
    else if(strcmp(argv[1],"remove_treasure")==0){

        if(argc!=4){
            printf("wrong input: remove_treasure <hunt_id> <id>\n");
            exit(2);
        }

        //remove_treasure(argv[2],argv[3]);//search if exists then do
    }

    ////////////////////////////////////remove entire hunt
    else if(strcmp(argv[1],"remove_hunt")==0){
        
        if(argc!=4){
            printf("wrong input: remove_hunt <hunt_id>\n");
            exit(2);
        }

        //remove_hunt(argv[2],argv[3]);//search if exists then do
    }

    else{
        printf("\nWrong instruction set \n");
        exit(3);
    }




}