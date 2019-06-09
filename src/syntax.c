#include <stdio.h>
#include <malloc.h>
#include "additional_function.h"
#include "token.h"
#include "lex_parser.h"
#include "filee.h"

static int parseReservedWord(Token * tkn) ;
    
    
void parse(TokenList * tkLst) {
    Token * tkn = getNextToken(tkLst);
    
    while(tkn != NULL) {
        
        if(tkn->type == KEYWORD) {
            parseReservedWord(tkn);
        } else if (tkn->type == PUNCTUATOR) {
            
        } else if (tkn->type == IDENTIFIER) {
            
        }
        
        tkn = getNextToken(tkLst);
    
        
    }
}

//разбираем слово
static int parseReservedWord(Token * tkn) {
    if(! strcmp(tkn->content, "this")) {
        return 0;
        
    }
}
