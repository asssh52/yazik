#pragma once

#include "../hpp/front.hpp"

int TreeDump            (line_t* line);
int TokensDump          (line_t* line);
int HTMLDumpGenerate    (line_t* line);
int HTMLGenerateHead    (line_t* line);

int DumpIds             (line_t* line, FILE* file);
