#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "additional_function.h"
#include "token.h"
#include "lex_parser.h"

static char * number(char *, char *);
static char * real_number(char *, char *);
static char * operator(char *, char *);
static char * error(char *);
static char * single_line_comment(char *, char*);
static char * multi_line_comment(char *, char*);
static char * string_single_quote(char * cur, char * start);
static char * string_double_quote(char * cur, char * start);
static char * identifier(char * cur, char * start);
static int is_valid_symbol(char c);
static char * regex(char * cur, char * start);
static char * regex_flag(char * cur, char * start);
static char * escape_sequence(char * cur, char * start);

static const char * literals[] = {
    "true",     "false",    "{}",   "Infinity", "NaN", "null", "undefined"
};

#define RESERVED_WORDS_LENGTH 32
static const char* reserved_words[] = {
    "this",         "new",      "function",  "var",
    "for",          "if",       "else",     "true", 
    "false",        "return",   "undefined","null",
    "instanceof",   "in",       "do",       "default",
    "void",         "while",    "break",    "case", 
    "typeof",       "continue", "Infinity", "NaN",
    "delete",       "switch",   "try",      "catch",        
    "throw",        "finally",  "with",     "debugger",
};


#define FUTURE_RESERVED_WORDS_LENGTH 7
static const char * future_reserved_words[] = {
    "class",    "const",    "enum",     "export",
    "extends",  "import",   "super"
};


static const char * reserved_symbols = "{}.+!=<>;-~(),*&|%&[]?^/:\\";

#define PUNCTUATORS_LENGTH 49
static const  char * punctuators[] = {
    "{",    "}",    ".",    ">=",   "+",
    "<<",   "!",    "=",    ">>=",  ";", "/",
    "==",   "-",    ">>",   "~",    "+=", "/=", 
    ">>>=", "(",    ",",    "!=",   "*",
    ">>>",  "&&",   "-=",   "&=",   ")",
    "<",    "===",  "%",    "&",    "||",
    "*=",   "|=",   "[",    ">",    "!==",
    "++",   "|",    "?",    "%=",   "^=", "\\",
    "--",   "^",    ":",    "<<=",  "]",    "<="
};

#define ML_OFF 0
#define ML_COMMENT  1
#define ML_STRING 2
static int multiline_switch = ML_OFF;

static int current_line = (int) NULL;


static TokenList * tklst;

/**
      Этот метод принимает строку, перебирает ее символы
     и извлекает его токены, определяя, есть ли в нем лексические ошибки.
*/
int lexical_analisys(char * line, TokenList * tokenList, int line_number) {
    char * c;
    c = line; //начальная позиция

    tklst = tokenList;

    current_line = line_number;

    while(1) {
        if(multiline_switch) {                                  
            switch(multiline_switch){
                case ML_COMMENT:
                	if(LOG) printf("Line numbr: %d\n", current_line);
                    c = multi_line_comment(c+1, c);
                    break;
                case ML_STRING:
                    error(c);
                    return 1;
                default:
                    error(c);
                    return 1;
            }
        }

        if(isdigit(*c)){                                        //число
            c = number(c+1, c);
            if(c == NULL) {
                return 1;
            }
        } else if (isalpha(*c) || *c == '$' || *c == '_') {     //буква
            c = identifier(c+1, c);
            
            
        } else if(is_valid_symbol(*c)){                         //символ
            c = operator(c+1, c);
            if(c == NULL){ 
                return 1;
            } else if (*c == '\n' || *c == '\r') {
            	return 0;
            }
        } else {
            switch(*c){
            case '\"':
                c = string_double_quote(c+1, c); // разбираем строки в двойных кавычках
                break;
            case '\'':
                c = string_single_quote(c+1, c); //разбираем строки в одинарных кавычках
                break;
            case ' ': case '\t':
                c = c+1;
                break;
            case '\n': case '\r':
                if(LOG) printf("== New line character ==\n");
                if(LOG) printf("Finished parsing line\n\n");

                Token * token = createToken(c, c, 0, 0, NEW_LINE);
                appendToken(tklst, token);
                return 0;
            case '\0': // учитывает EOF, когда новая строка не указана.
                if(LOG) printf("Finished parsing line\n\n");
                return 0;
            default:
                printf("unrecognized char! %c\n", *c);
                error(c);
                return 1;
            }
        }
    }
}

static char * escape_sequence(char * cur, char * start) {
	//hex escape sequence: '\x64' (hex char(2 digits))
	//unicode escape: '\u1234' (unicode char (4 digits))
	switch(*cur) {
	case '\'':
		cur++;
		//создать токен для одиночной кавычки
		break;
	case '\"':
		cur++;
		//создаем токен для двойной кавычки
		break;
	case '\\':
		cur++;
		//создать токен для обратной
		break;
	case 'b':
		cur++;
		break;
	case 'f': //Для канала
		cur++;
		break;
	case 'n': //перевод строки
		cur++;
		break;
	case 'r': //return
		cur++;
		break;
	case 't': //tab
		cur++;

		break;
	case 'v': //вертикальный tab
		cur++;
		break;
	case 'x':
		cur = cur+3; 
		break;
	case 'u':

		cur = cur+5; 
		break;
	case '/': // регулярное выражение
		cur++;
		if(LOG) printf("Escaped Bar");
		break;
	default:
		break;
	}
	return cur;
}

static char * regex(char * cur, char * start) {

	if(*cur == '\\') {
		cur = escape_sequence(cur+1, cur); 
	}

	if( *cur == '/') {
		return regex_flag(++cur, start);
	} else { 
		return regex(++cur, start);
	}
}

static char * regex_flag(char * cur, char * start) {
	if(isalpha(*cur)) { 
		return regex_flag(++cur, start);
	} else {
		Token * token = createToken(start, cur-1, 0, 0, REGEX);
		appendToken(tklst, token);

		if(LOG) printf("regex: %s\n", get_string(start, cur-1));

		return cur;
	}
}

static char * operator(char * cur, char * start){
	if(*start == '/' && isgraph(*cur)) {
		if(*cur == '/'){
			return single_line_comment(cur+1, start);
		}
		if(*cur == '*'){
			return multi_line_comment(cur+1, start);
		}
		else {
			
			return regex(cur, start);
		}
	}else if(is_valid_symbol(*cur) ) {
		//продолжаем искать оператор
        return operator(cur+1, start);
    } else {
        int string_index;
        if((string_index = is_string_in_array(get_string(start, cur-1), punctuators, PUNCTUATORS_LENGTH))) { 
            if(LOG) printf("punctuator: %s - punctuator index: %d, string_index: %d\n", get_string(start, cur-1), PUNCTUATOR, string_index);

            Token * token = createToken(start, cur-1, 0, 0, PUNCTUATOR + string_index);
            appendToken(tklst, token);
            return cur;
        } else { 
            /*Эта ветвь else означает, что некоторые составные знаки препинания были идентифицированы, но не распознаны
                 Они должны быть отделены друг от друга. например "() {" потребуется для расширения до токенов "(", ")" и "{" соответственно
            */
            if(LOG) printf("possible error on punctuator: %s\n", get_string(start, cur-1));

            char * c = cur-1; 
            //уменьшаем c, пока не будет найден правильный пунктуатор. следующий символ c возвращается
            while(!(string_index = is_string_in_array(get_string(start, --c), punctuators, PUNCTUATORS_LENGTH)));  
                
            if(LOG) printf("trying to fix punctuator: %s\n", get_string(start, c));
            
            Token * token = createToken(start, c, 0, 0, PUNCTUATOR + string_index);
            appendToken(tklst, token);
            
            return c+1;
        }
    }
}

// положительные отрицательные чистла и показатели степени
static char * number(char * cur, char * start) {
    switch(*cur) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      
        return number(cur+1, start);
    case '.':
        return real_number(cur+1, start); 
    case 'e': case 'E':
    default:
        if(LOG) printf("integer: %s\n", get_string(start, cur-1));

        Token * token = createToken(start, cur-1, 0, 0, LITERAL_NUMBER);
        appendToken(tklst, token);
        return cur;
    }
}
// индефикатор
static char * identifier(char * cur, char * start) {
    
    
    if(isalnum(*cur) || *cur == '_' || *cur == '$') {
        return identifier(++cur, start);
    }
    
    char * str = get_string(start, cur-1);  
    int string_index;
    if((string_index = is_string_in_array(str, reserved_words, RESERVED_WORDS_LENGTH))) {
        if(LOG) printf("reserved word: %s - kw_index = %d - token index: %d\n", str, string_index, KEYWORD + string_index );
        
        Token * token = createToken(start, cur-1, 0, 0, KEYWORD + string_index);
        appendToken(tklst, token);
        
    } else if ((string_index = is_string_in_array(str, future_reserved_words, FUTURE_RESERVED_WORDS_LENGTH))) {
        if(LOG) printf("future reserved word: %s - kw_index = %d - token index: %d\n", str, string_index, KEYWORD + string_index );
        
        Token * token = createToken(start, cur-1, 0, 0, KEYWORD + string_index);
        appendToken(tklst, token);
        
    } else {
        if(LOG) printf("identifier: %s\n", str);
        Token * token = createToken(start, cur-1, 0, 0, IDENTIFIER);
        appendToken(tklst, token);
        
    }
    return cur;
}

/*
Учитываем 0 слева. 0005 не должен быть действительным числом.
0 valid, 30 valid, 0.3 valid, 03 not valid
*/
static char * real_number(char * cur, char * start) {
    switch(*cur) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        return real_number(cur+1, start);
    default:
        if(LOG) printf("float: %s\n", get_string(start, cur-1));
        Token * token = createToken(start, cur-1, 0, 0, LITERAL_NUMBER);
        appendToken(tklst, token);
        return cur;
    }
}

/*
* Разбираем строки в двойных кавычках.
*/
static char * string_double_quote(char * cur, char * start) {
    switch(*cur){
        case '\"':
            if(*(cur-1) != '\\') {
                if(LOG) printf("string: %s\n", get_string(start, cur));
                Token * token = createToken(start, cur, 0, 0, LITERAL_STRING);
                appendToken(tklst, token);
                return ++cur;
            }
            return string_double_quote(cur+1, start);
        default:
            //printf(" %c, %x", *cur, (unsigned int) cur);
            return string_double_quote(cur+1, start);

    }
}
/*
*Разбираем строки в одинарных кавычках.
*/
static char * string_single_quote(char * cur, char * start) {
    switch(*cur){
        case '\'':
            if(*(cur-1) != '\\') {
                if(LOG) printf("string: %s\n", get_string(start, cur));
                Token * token = createToken(start, cur, 0, 0, LITERAL_STRING);
                appendToken(tklst, token);
                return ++cur;
            }
            return string_single_quote(cur+1, start);
        default:
            return string_single_quote(cur+1, start);

    }
}

/*
* Однострочные коментарии "//"
*/
static char * single_line_comment(char * cur, char * start) {
    switch(*cur){
        case '\n':  case '\r':
            if(LOG) printf("SLC: %s\n", get_string(start, cur-1));
            Token * token = createToken(start, cur-1, 0, 0, SINGLE_LINE_COMMENT);
            appendToken(tklst, token);
            return cur;
        default:
            return single_line_comment(cur+1, start);
    }
}
// многострочный комментарий
static char * multi_line_comment(char * cur, char * start) {
	if(LOG) printf("%c, %d - %x: detecto comment ml!\n", *cur, *cur, (unsigned int)*cur);
    switch(*cur){
        case '\n': case '\r':
            multiline_switch = ML_COMMENT;
            return cur;
        case '/':
            
            if(*(cur-1) == '*'){
                if(LOG) printf("MLC: %s\n", get_string(start, cur));

                Token * token = createToken(start, cur, 0, 0, MULTI_LINE_COMMENT);
                appendToken(tklst, token);

             
                if(multiline_switch) multiline_switch = ML_OFF;
                return cur+1;
            }
        default:
            return multi_line_comment(cur+1, start);
    }
}



static char * error(char *  cur) {
    printf("Error found near %c. Stopping parser\n", *cur);
    return cur;
}


static int is_valid_symbol(char c){
    if(c == '\0') return 0;
    
    return is_char_in_array(c, reserved_symbols);
}



