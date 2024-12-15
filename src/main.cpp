#include "../hpp/front.hpp"
#include "../hpp/dump.hpp"
#include "../hpp/back.hpp"

#define MEOW fprintf(stderr, PNK  "MEOW\n" RESET);

int main(){
    line_t line = {};


    LineCtor(&line);
    LineRead(&line);

    HTMLGenerateHead(&line);

    AnalyseInput(&line);

    LineProcess(&line);
    LineSave(&line);


    FILE* file = fopen("save.txt", "r");
    line.files.save = file;

    // LoadTree(&line);
    HTMLDumpGenerate(&line);

    return 0;
}
