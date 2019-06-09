#include <stdio.h>
#include <malloc.h>
#include "token.h"
#include "lex_parser.h"
#include "filee.h"


int main(int argc, char *argv[]){
    FILE * f;
    TokenList * tokenList;
    f_i fi;
    

    
    if(argc == 1){
        f = fopen("src.js", "r");

    } else {
        f = fopen(argv[1], "r");
    }

    if(f == NULL){
        printf(">>>>>ERROR: File does not exist.\n>>>>>Exiting...\n");
        return 1;
    }

    fi = (f_i) malloc(sizeof(struct FILE_INFO));
    fi = get_file_info(f, &fi);

    rewind(f);

    tokenList = initializeTokenList(tokenList);

    printf("LoC: %d\n", fi->lines);
    printf("Max line length: %d\n", fi->max_line_length);
    
    char * line_buff = (char*) malloc(sizeof(char) * fi->max_line_length);
    
    int line_num = 0;

    while(get_line(line_buff, fi->max_line_length, f) != NULL){

        if(LOG) printf(">>>>> Parsing Line: %s\n", line_buff);
        //синтаксический анализ 
        if(lexical_analisys(line_buff, tokenList, ++line_num) == 1) {
            fclose(f);
            return 1;
        }
    }
    
    printf("\n====== Finished parsing file ======\n");
    // дерево
    printTokenList(tokenList);
    
    fclose(f);
    return 0;
}

