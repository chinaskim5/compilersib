#ifndef FILEE
#define FILEE
//о файле lines - количество строк , max_line_length - максимальное количество символов в строке.
    struct FILE_INFO{
        int lines;
        int max_line_length;
    };

    typedef struct FILE_INFO *f_i;

    f_i get_file_info(FILE *fp, f_i * fi);
    char * get_line(char * line, int max_line, FILE * f);

#endif
