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
    O_CBR = 41

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
    {")",   O_CBR, 1}

};



static node_t*  GetN            (line_t* line);
static node_t*  GetG            (line_t* line);
static node_t*  GetE            (line_t* line);
static node_t*  GetT            (line_t* line);
static node_t*  GetP            (line_t* line);
static int      GetF            (line_t* line);
static int      SyntaxError     (line_t* line, int param);

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

    printf("%lld\n", line->tree->numElem);
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

        if (isalpha(line->buffer[line->ptr]) && !canNextBeId && startId == 0){

            ProccessName(line, 0);

            line->ptr += 1;
        }

        else if (isalpha(line->buffer[line->ptr]) && startId == 0){
            idLen  += 1;
            startId = 1;

            line->ptr += 1;
        }

        else if (startId && canBeId && canNextBeId){
            idLen  += 1;

            line->ptr += 1;
        }

        //WORDS
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

            line->tokens[line->tptr].type = T_NUM;
            line->tokens[line->tptr].data = value;

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

        line->tokens[line->tptr].type = T_ID;
        line->tokens[line->tptr].data = nodeId;

        line->tptr += 1;

    }

    else{

        line->tokens[line->tptr].type = T_OPR;
        line->tokens[line->tptr].data = nodeOp;

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

static node_t* NewNode(tree_t* tree, int data, int type, node_t* left, node_t* right){
    node_t* newNode = (node_t*)calloc(1, sizeof(*newNode));

    newNode->data   = data;
    newNode->type   = type;

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
    printf(PNK "Syntax error in " ORG "\'%c\'\n" RESET, line->buffer[line->ptr]);

    switch (param){
        case S_TRMNL:
            printf(PNK "expected '$' at " CYN "<%llu> '%c'\n" RESET, line->ptr, line->buffer[line->ptr]);
            break;

        case S_NUM:
            printf(PNK "expected 'num' at " CYN "<%llu> '%c'\n" RESET, line->ptr, line->buffer[line->ptr]);
            break;

        case S_BRCKT:
            printf(PNK "expected ')' at " CYN "<%llu> '%c'\n" RESET, line->ptr, line->buffer[line->ptr]);
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
    if (line->buffer[line->ptr] != '$') SyntaxError(line, S_TRMNL);

    return node;
}

static node_t* GetN(line_t* line){
    int old_p = line->ptr;
    int val = 0;

    while('0' <= line->buffer[line->ptr] && line->buffer[line->ptr] <= '9'){
        val = val * 10 + line->buffer[line->ptr] - '0';
        line->ptr++;
    }

    if (old_p == line->ptr) SyntaxError(line, S_NUM);

    node_t* newNode = NewNode(line->tree, val, T_NUM, nullptr, nullptr);

    return newNode;
}

static node_t* GetE(line_t* line){
    node_t* left = GetT(line);

    while (line->buffer[line->ptr] == '+' || line->buffer[line->ptr] == '-'){
        int op = line->buffer[line->ptr];
        line->ptr++;

        node_t* right = GetT(line);

        if      (op == '+'){
            left = NewNode(line->tree, O_ADD, T_OPR, left, right);
        }

        else if (op == '-'){
            left = NewNode(line->tree, O_SUB, T_OPR, left, right);
        }
    }

    return left;
}

static node_t* GetT(line_t* line){
    node_t* left = GetP(line);

    while (line->buffer[line->ptr] == '*' || line->buffer[line->ptr] == '/'){
        int op = line->buffer[line->ptr];
        line->ptr++;

        node_t* right = GetP(line);

        if      (op == '*'){
            left = NewNode(line->tree, O_MUL, T_OPR, left, right);
        }

        else if (op == '/'){
            left = NewNode(line->tree, O_DIV, T_OPR, left, right);
        }
    }

    return left;
}

static node_t* GetP(line_t* line){
    if (line->buffer[line->ptr] == '('){
        line->ptr++;

        node_t* node = GetE(line);
        if (line->buffer[line->ptr] != ')') SyntaxError(line, S_BRCKT);

        line->ptr++;

        return node;
    }

    else if (line->buffer[line->ptr] == 'x' && !isalpha(line->buffer[line->ptr + 1])){
        line->ptr++;

        return NewNode(line->tree, 'x', T_ID, nullptr, nullptr);
    }

    else if (isdigit(line->buffer[line->ptr])){
        return GetN(line);
    }

    else{
        int op = GetF(line);

        if (line->buffer[line->ptr] == '('){
            line->ptr++;

            node_t* node = GetE(line);
            if (line->buffer[line->ptr] != ')') SyntaxError(line, S_BRCKT);

            line->ptr++;
            node_t* newNode = NewNode(line->tree, op, T_OPR, nullptr, node);

            return newNode;
        }
    }
}

static int GetF(line_t* line){
    int op = 0;

    if (!strncmp(line->buffer + line->ptr, "sin", 3)){
        line->ptr += 3;
        op = O_SIN;
    }

    else if (!strncmp(line->buffer + line->ptr, "cos", 3)){
        line->ptr += 3;
        op = O_COS;
    }

    else if (!strncmp(line->buffer + line->ptr, "tan", 3)){
        line->ptr += 3;
        op = O_TAN;
    }

    return op;
}
