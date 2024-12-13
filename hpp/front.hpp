#pragma once

#include <ctype.h>

#include "../hpp/structs.hpp"
#include "../hpp/colors.hpp"
#include "../hpp/ops.hpp"


const int64_t MAX_BUFF = 256;
const int64_t MAX_TKNS = 64;
const int64_t MAX_IDS  = 16;


int AnalyseInput    (line_t* line);

int LineCtor        (line_t* line);
int LineRead        (line_t* line);
int LineProcess     (line_t* line);
int LineSave        (line_t* line);
