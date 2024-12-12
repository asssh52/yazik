#include "../hpp/front.hpp"
#include "../hpp/dump.hpp"

#define MEOW fprintf(stderr, PNK  "MEOW\n" RESET);

const char*   DFLT_INPT_FILE = "./meow.txt";
const char*   DFLT_HTML_FILE = "htmldump.html";
const char*   DFLT_DOT_FILE  = "./bin/dot.dot";

enum errors_t{

    OK  = 0,
    ERR = 1,
    NAN = -1,

};

enum syntaxExits{

    S_TRMNL   = 1,
    S_NUM     = 2,
    S_BRCKT   = 3

};

enum nodeOperations{

    O_ADD = 43,
    O_SUB = 45,
    O_MUL = 42,
    O_DIV = 47,
    O_SIN = 115,
    O_COS = 99,
    O_TAN = 116,
    O_OBR = 40,
    O_CBR = 41,
    O_TRM = 36,

};

const opName_t opList[] = {

    {"+",   O_ADD, 1},
    {"-",   O_SUB, 1},
    {"*",   O_MUL, 1},
    {"/",   O_DIV, 1},
    {"sin", O_SIN, 3},
    {"cos", O_COS, 3},
    {"tan", O_TAN, 3},
    {"(",   O_OBR, 1},
    {")",   O_CBR, 1},
    {"$",   O_TRM, 1}

};

static node_t*  GetNum          (line_t* line);
static node_t*  GetG            (line_t* line);
static node_t*  GetE            (line_t* line);
static node_t*  GetT            (line_t* line);
static node_t*  GetP            (line_t* line);
static node_t*  GetId           (line_t* line);
static int      SyntaxError     (line_t* line, int param);

static int      FillTokensId    (line_t* line);

static int      FindOp          (char* word, int len);
static double   ProcessWord     (char* word, int len, int* type);
static int      SkipSpaces      (line_t* line);

static int      ProccessName    (line_t* line, int len);
static int      FindId          (line_t* line, char* word, int len);
static int      CreateId        (line_t* line, char* word, int len);

static node_t*  NewNode         (tree_t* tree, int data, int type, node_t* left, node_t* right);

/*========================================================================*/

int LineCtor(line_t* line){
    line->buffer = (char*)    calloc(MAX_BUFF, sizeof(char));
    line->tokens = (node_t*)  calloc(MAX_TKNS, sizeof(*line->tokens));
    line->tree   = (tree_t*)  calloc(1,        sizeof(*line->tree));
    line->id     = (names_t*) calloc(MAX_IDS,  sizeof(*line->id));

    if (!line->files.input) line->files.input = fopen(line->files.inputName, "r");
    if (!line->files.input) line->files.input = fopen(DFLT_INPT_FILE , "r");

    line->files.html = fopen(line->files.htmlName, "w");
    if (!line->files.html) line->files.html = fopen(DFLT_HTML_FILE, "w");

    line->files.dotName = DFLT_DOT_FILE;

    FillTokensId(line);

    return OK;
}

/*========================================================================*/

static int FillTokensId(line_t* line){
    for (int i = 0; i < MAX_TKNS; i++){
        line->tokens[i].id = i;
    }

    return OK;
}

/*========================================================================*/

int LineRead(line_t* line){
    fread(line->buffer, sizeof(char), MAX_BUFF, line->files.input);

    return OK;
}

/*========================================================================*/

int LineProcess(line_t* line){
    line->tree->root = GetG(line);

    TokensDump(line);
    DumpIds(line, stdout);

    TreeDump(line);

    return OK;
}

/*========================================================================*/

int AnalyseInput(line_t* line){
    int   idLen   = 0;
    char* numEnd    = nullptr;
    bool  startId = 0;

    while (line->ptr < MAX_BUFF){
        double value  = 0;
        int    nodeOp = 0;
        int    nodeId = 0;

        bool canBeId        = isalpha(line->buffer[line->ptr])     || isdigit(line->buffer[line->ptr])     || line->buffer[line->ptr]     == '_';
        bool canNextBeId    = isalpha(line->buffer[line->ptr + 1]) || isdigit(line->buffer[line->ptr + 1]) || line->buffer[line->ptr + 1] == '_';

        //ONE LETTER VARIABLES
        if (isalpha(line->buffer[line->ptr]) && !canNextBeId && startId == 0){

            ProccessName(line, 0);

            line->ptr += 1;
        }
        //START OF ID
        else if (isalpha(line->buffer[line->ptr]) && startId == 0){
            idLen  += 1;
            startId = 1;

            line->ptr += 1;
        }
        //MIDDLE OF ID
        else if (startId && canBeId && canNextBeId){
            idLen  += 1;

            line->ptr += 1;
        }

        //END OF ID
        else if (startId && canBeId && !canNextBeId){
            // semantic

            ProccessName(line, idLen);

            // semantic
            idLen = 0;
            startId = 0;

            line->ptr += 1;
        }

        //NUMBERS
        else if (isdigit(line->buffer[line->ptr])){
            value = strtod(line->buffer + line->ptr, &numEnd);
            // semantic

            line->tokens[line->tptr].type       = T_NUM;
            line->tokens[line->tptr].data.num   = value;

            line->tptr += 1;

            // semantic
            printf(MAG "%lf\n" RESET, value);
            line->ptr += numEnd - (line->buffer + line->ptr);
        }

        //UNAR OPERATORS
        else if (!SkipSpaces(line)){
            if (line->buffer[line->ptr] != 0){
                // semantic

                ProccessName(line, 0);

                // semantic
            }

            line->ptr += 1;
        }
    }

    TokensDump(line);
    DumpIds(line, stdout);

    line->tptr = 0;

    return OK;
}

/*========================================================================*/

static int ProccessName(line_t* line, int idLen){

    int nodeOp = FindOp(line->buffer + line->ptr - idLen, idLen + 1);
    printf("nodeOp:%d\n", nodeOp);
    if (nodeOp == NAN){
        int nodeId = FindId(line, line->buffer + line->ptr - idLen, idLen + 1);

        if (nodeId == NAN){
            nodeId = CreateId(line, line->buffer + line->ptr - idLen, idLen + 1);
        }

        line->tokens[line->tptr].type    = T_ID;
        line->tokens[line->tptr].data.id = nodeId;

        line->tptr += 1;

    }

    else{

        line->tokens[line->tptr].type    = T_OPR;
        line->tokens[line->tptr].data.op = nodeOp;

        line->tptr += 1;

    }

    return OK;
}

/*========================================================================*/

static int FindId(line_t* line, char* word, int len){
    for (int i = 0; i < line->numId; i++){
        if (line->id[i].len == len && !strncmp(word, line->id[i].name, len)){
            return i;
        }
    }

    return NAN;
}

/*========================================================================*/

static int CreateId(line_t* line, char* word, int len){
    line->id[line->numId].name = word;
    line->id[line->numId].len  = len;

    line->numId += 1;

    return line->numId - 1;
}

/*========================================================================*/

static int SkipSpaces(line_t* line){
    int skipped = 0;

    while (line->buffer[line->ptr] == ' ' || line->buffer[line->ptr] == '\n'){
        line->ptr++;

        skipped += 1;
    }

    return skipped;
}

/*========================================================================*/
//DEPRECATED
static double ProcessWord(char* word, int length, int* type){
    double value = 0;

    return value;
}

/*========================================================================*/

static int FindOp(char* word, int length){
    for (int i = 0; i < sizeof(opList) / sizeof(opList[0]); i++){
        if (opList[i].len == length && !strncmp(word, opList[i].name, length)){
            return opList[i].opNum;
        }
    }

    return NAN;
}

/*========================================================================*/
//DEPRECATED
static node_t* NewNode(tree_t* tree, int data, int type, node_t* left, node_t* right){
    node_t* newNode = (node_t*)calloc(1, sizeof(*newNode));

    newNode->data.num = data;
    newNode->type    = type;

    newNode->left   = left;
    newNode->right  = right;

    if (left)  left->parent  = newNode;
    if (right) right->parent = newNode;

    newNode->id = tree->numElem;
    tree->numElem += 1;

    return newNode;
}

/*========================================================================*/

static int SyntaxError(line_t* line, int param){
    printf(PNK "Syntax error in " ORG "\'%llu\'\n" RESET, line->tptr);

    switch (param){
        case S_TRMNL:
            printf(PNK "expected '$' at " CYN "<%llu>\n" RESET, line->tptr);
            break;

        case S_NUM:
            printf(PNK "expected 'num' at " CYN "<%llu>\n" RESET, line->tptr);
            break;

        case S_BRCKT:
            printf(PNK "expected ')' at " CYN "<%llu>\n" RESET, line->tptr);
            break;
    }

    return 0;
}

/*========================================================================*/
//LEXICAL



/*========================================================================*/
//GRAMMAR FUNCS

static node_t* GetG(line_t* line){
    node_t* node = GetE(line);

    if (line->tokens[line->tptr].type != T_OPR || line->tokens[line->tptr].data.op != O_TRM) SyntaxError(line, S_TRMNL);

    return node;
}

//DEPRECATED
// static node_t* GetId(line_t* line){
//     int old_p = line->ptr;
//     int val = 0;
//
//     while('0' <= line->buffer[line->ptr] && line->buffer[line->ptr] <= '9'){
//         val = val * 10 + line->buffer[line->ptr] - '0';
//         line->ptr++;
//     }
//
//     if (old_p == line->ptr) SyntaxError(line, S_NUM);
//
//     node_t* newNode = NewNode(line->tree, val, T_NUM, nullptr, nullptr);
//
//     return newNode;
// }

static node_t* GetE(line_t* line){
    node_t* left = GetT(line);

    int op = line->tokens[line->tptr].data.op;
    while (line->tokens[line->tptr].type == T_OPR && (op == O_ADD || op == O_SUB)){
        node_t* opNode  = line->tokens + line->tptr;
        line->tptr++;

        node_t* right = GetT(line);

        opNode->left  = left;
        opNode->right = right;

        left = opNode;

        op = line->tokens[line->tptr].data.op;
    }

    return left;
}

static node_t* GetT(line_t* line){
    node_t* left = GetP(line);

    int op = line->tokens[line->tptr].data.op;
    while (line->tokens[line->tptr].type == T_OPR && (op == O_MUL || op == O_DIV)){
        node_t* opNode  = line->tokens + line->tptr;
        line->tptr++;

        node_t* right = GetP(line);

        opNode->left  = left;
        opNode->right = right;

        left = opNode;

        op = line->tokens[line->tptr].data.op;
    }

    return left;
}

static node_t* GetP(line_t* line){
    printf("getp:tptr:%d\n", line->tptr);

    if (line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op == O_OBR){
        printf("first:%d\n", line->tptr);
        line->tptr++;
        node_t* node = GetE(line);

        //printf("tptr:%d, data:%d\n", line->tptr, line->tokens[line->tptr].data.op);
        if (line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op != O_CBR) SyntaxError(line, S_BRCKT);

        line->tptr++;

        return node;
    }

    else if (line->tokens[line->tptr].type == T_ID && line->tokens[line->tptr + 1].type == T_OPR && line->tokens[line->tptr + 1].data.op == O_OBR){
        node_t* nodeId = line->tokens + line->tptr;

        line->tptr++;
        line->tptr++;

        node_t* nodeE = GetE(line);

        if (line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op != O_CBR) SyntaxError(line, S_BRCKT);

        line->tptr++;

        nodeId->right = nodeE;

        return nodeId;
    }

    else if (line->tokens[line->tptr].type == T_ID){
        node_t* nodeId = line->tokens + line->tptr;

        line->tptr++;

        return nodeId;
    }

    else {
        return GetNum(line);
    }
}

static node_t* GetNum(line_t* line){
    node_t* node = line->tokens + line->tptr;

    if (node->type == T_NUM){
        line->tptr += 1;
    }

    else {
        SyntaxError(line, S_NUM);
    }

    return node;
}
