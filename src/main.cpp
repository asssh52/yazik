#include "../hpp/front.hpp"
#include "../hpp/dump.hpp"

int main(){
    line_t line = {};


    LineCtor(&line);
    HTMLGenerateHead(&line);
    LineRead(&line);
    LineProcess(&line);
    HTMLDumpGenerate(&line);


    return 0;
}
