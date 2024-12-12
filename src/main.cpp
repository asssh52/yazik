#include "../hpp/front.hpp"
#include "../hpp/dump.hpp"

int main(){
    line_t line = {};


    LineCtor(&line);
    LineRead(&line);
    HTMLGenerateHead(&line);

    AnalyseInput(&line);

    LineProcess(&line);
    HTMLDumpGenerate(&line);


    return 0;
}
