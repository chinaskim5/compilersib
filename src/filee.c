#include <stdio.h>
#include <malloc.h>
#include "filee.h"

//Функция, которая возвращает структуру, содержащую файл(fp) "количество строк" и "длина самой длинной строки"
f_i get_file_info(FILE *fp, f_i * fi){
    char c;
    int char_count = 0;
    int lines = 0;
    int max = 0;

    *fi = (f_i) malloc(sizeof(struct FILE_INFO));
    while((c = getc(fp)) != EOF){
        if(c == '\n'){
            ++lines;
            ++char_count; //учитываем символ новой строки.
            ++char_count; //учет самой длинной строки, которая заканчивается символом.
            if(char_count > max){
                max = char_count;
            }
            char_count = 0;
        } else {
            char_count++;
        }
    }
    (*fi)->max_line_length = max;
    (*fi)->lines = ++lines;
    return *fi;
}
//Получаем нужную строку из файла.
char * get_line(char * line, int max_line, FILE * f) {
    register int c; // хранение данных в регистре "кэш памяти"
    register char * str;

    str = line;
    while(--max_line > 0 && (c = getc(f)) != EOF) {
        *str++ = c;
        if(c == '\n')
            break;
    }
    *str = '\0';
    return (c == EOF && str == line) ? NULL : line;
}
