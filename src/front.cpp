#include "../hpp/front.hpp"
#include "../hpp/dump.hpp"

#define MEOW fprintf(stderr, PNK  "MEOW\n" RESET);

const char*   DFLT_INPT_FILE = "./meow.txt";
const char*   DFLT_HTML_FILE = "htmldump.html";
const char*   DFLT_SAVE_FILE = "save.txt";
const char*   DFLT_DOT_FILE  = "./bin/dot.dot";

enum errors_t{

    OK  = 0,
    ERR = 1,
    NAN = -1,

};

enum syntaxExits{

    S_TRMNL   = 1,
    S_NUM     = 2,
    S_BRCKT   = 3,
    S_ID      = 4,
    S_EQL     = 5,
    S_SEP     = 6,
    S_OP      = 7,
    S_IFC     = 8

};

static node_t*  GetNum          (line_t* line);
static node_t*  GetG            (line_t* line);
static node_t*  GetOp           (line_t* line);
static node_t*  GetE            (line_t* line);
static node_t*  GetP            (line_t* line);
static node_t*  GetId           (line_t* line);
static node_t*  GetIf           (line_t* line);
static node_t*  GetPrint        (line_t* line);
static int      SyntaxError     (line_t* line, int param);

static int      NodeSave        (line_t* line, node_t* node, int depth, FILE* file);

static int      FillTokensId    (line_t* line);

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

int LineSave(line_t* line){
    line->files.save = fopen(line->files.saveName, "w");
    if (!line->files.save) line->files.save = fopen(DFLT_SAVE_FILE, "w");

    NodeSave(line, line->tree->root, 0, line->files.save);

    return OK;
}

static int NodeSave(line_t* line, node_t* node, int depth, FILE* file){
    if (!node) return OK;
    if (depth >= line->tree->numElem) return ERR;

    fprintf(file, "{");


    if (node->type == T_OPR){
        int opNum = FindOpByNum(node->data.op);

        fprintf(file, "OP:\"%s\"", opList[opNum].stdname);
    }

    if (node->type == T_ID){
        char buffer[MAX_BUFF] = {};
        strncpy(buffer, line->id[node->data.id].name, line->id[node->data.id].len);
        fprintf(file, "ID:\"%s\"",   buffer);
    }

    if (node->type == T_NUM) fprintf(file, "NUM:\"%0.0lf\"", node->data.num);



    if(node->left || node->right) fprintf(file, "\n");

    if (node->left){
        for (int i = 0; i < depth + 1; i++) fprintf(file, "\t");

        NodeSave(line, node->left, depth + 1, file);
    }

    if (node->right){
        for (int i = 0; i < depth + 1; i++) fprintf(file, "\t");

        NodeSave(line, node->right, depth + 1, file);
    }

    if(node->left || node->right) for (int i = 0; i < depth; i++) fprintf(file, "\t");
    fprintf(file, "}\n");

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
//LEXICAL
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



/*========================================================================*/
//DEPRECATED

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

        case S_ID:
            printf(PNK "expected 'ID' at " CYN "<%llu>\n" RESET, line->tptr);
            break;

        case S_SEP:
            printf(PNK "expected 'SEP' at " CYN "<%llu>\n" RESET, line->tptr);
            break;

        case S_OP:
            printf(PNK "unknown operator at " CYN "<%llu>\n" RESET, line->tptr);
            break;

        case S_IFC:
            printf(PNK "expected close operator at " CYN "<%llu>\n" RESET, line->tptr);
            break;
    }

    line->err = true;

    return 0;
}

/*========================================================================*/
//GRAMMAR FUNCS

static node_t* GetG(line_t* line){
    if (line->err == true) return 0;

    node_t* node_left = GetOp(line);
    if (!node_left) return 0;

    if (line->tokens[line->tptr].type != T_OPR || line->tokens[line->tptr].data.op != O_SEP) SyntaxError(line, S_SEP);
    node_t* nodeSep = line->tokens + line->tptr;
    line->tptr += 1;
    line->tree->numElem += 1;

    nodeSep->left     = node_left;
    node_left->parent = nodeSep;

    node_t* nodeSepReturn = nodeSep;

    while (!(line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op == O_TRM) || line->tokens[line->tptr].type == 0){
        node_t* node_left = GetOp(line);
        if (!node_left) return 0;

        if (line->tokens[line->tptr].type != T_OPR || line->tokens[line->tptr].data.op != O_SEP) SyntaxError(line, S_SEP);
        nodeSep->right  = line->tokens + line->tptr;
        line->tokens[line->tptr].parent = nodeSep;

        nodeSep = line->tokens + line->tptr;
        line->tptr += 1;
        line->tree->numElem += 1;

        nodeSep->left     = node_left;
        node_left->parent = nodeSep;
    }

    if (line->tokens[line->tptr].type != T_OPR || line->tokens[line->tptr].data.op != O_TRM) SyntaxError(line, S_TRMNL);

    return nodeSepReturn;
}


static node_t* GetOp(line_t* line){
    if (line->err == true) return 0;

    node_t* retNode = nullptr;

    if (line->tokens[line->tptr].type == T_ID){

        node_t* nodeId = GetId(line);
        if (!nodeId) return 0;

        if (line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op == O_EQL){
            node_t* nodeEq = line->tokens + line->tptr;
            line->tptr += 1;
            line->tree->numElem += 1;

            node_t* nodeE = GetE(line);
            if (!nodeE) return 0;

            nodeEq->left   = nodeId;
            nodeEq->right  = nodeE;
            nodeE->parent  = nodeEq;
            nodeId->parent = nodeEq;

            retNode = nodeEq;
        }

        else if (line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op == O_OBR){
            line->tptr += 1;

            node_t* nodeE = GetE(line);
            if (!nodeE) return 0;

            if (line->tokens[line->tptr].type != T_OPR || line->tokens[line->tptr].data.op != O_CBR) SyntaxError(line, S_BRCKT);
            line->tptr += 1;

            nodeId->left  = nodeE;
            nodeE->parent = nodeId;

            retNode = nodeId;
        }

        else SyntaxError(line, S_OP);

    }

    else if (line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op == O_IFB){

        retNode = GetIf(line);
        if (!retNode) return 0;

    }

    else{

        retNode = GetPrint(line);
        if (!retNode) return 0;
    }

    return retNode;
}

static node_t* GetIf(line_t* line){
    node_t* retNode = nullptr;

    if (line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op == O_IFB){
        retNode = line->tokens + line->tptr;
        line->tptr += 1;
        line->tree->numElem += 1;

        node_t* node_left = GetOp(line);
        if (!node_left) return 0;

        if (line->tokens[line->tptr].type != T_OPR || line->tokens[line->tptr].data.op != O_SEP) SyntaxError(line, S_SEP);
        node_t* nodeSep = line->tokens + line->tptr;
        line->tptr += 1;
        line->tree->numElem += 1;

        nodeSep->left     = node_left;
        node_left->parent = nodeSep;

        node_t* nodeSepReturn = nodeSep;

        while (!(line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op == O_IFC)){
            node_left = GetOp(line);
            if (!node_left) return 0;

            if (line->tokens[line->tptr].type != T_OPR || line->tokens[line->tptr].data.op != O_SEP) SyntaxError(line, S_SEP);
            nodeSep->right  = line->tokens + line->tptr;
            line->tokens[line->tptr].parent = nodeSep;

            nodeSep = line->tokens + line->tptr;
            line->tptr += 1;
            line->tree->numElem += 1;

            nodeSep->left     = node_left;
            node_left->parent = nodeSep;
        }

        if (line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op == O_IFC){
            line->tptr += 1;

            node_t* nodeCondition = GetE(line);
            if (!nodeCondition) return 0;

            retNode->left         = nodeCondition;
            nodeCondition->parent = retNode;

            retNode->right = nodeSepReturn;
            nodeSepReturn->parent = retNode;
        }

        else SyntaxError(line, S_IFC);

    }

    else SyntaxError(line, S_OP);

    return retNode;
}

static node_t* GetPrint(line_t* line){
    node_t* retNode = nullptr;

    if (line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op == O_PNT){
        retNode = line->tokens + line->tptr;
        line->tptr += 1;
        line->tree->numElem += 1;

        node_t* nodeE = GetE(line);
        if (!nodeE) return 0;

        retNode->left = nodeE;
        nodeE->parent = retNode;
    }

    return retNode;
}

static node_t* GetE(line_t* line){
    if (line->err == true) return 0;

    node_t* left = GetP(line);
    if (!left) return 0;

    int op = line->tokens[line->tptr].data.op;
    while (line->tokens[line->tptr].type == T_OPR && (op == O_ADD || op == O_SUB || op == O_MUL || op == O_DIV)){
        node_t* opNode  = line->tokens + line->tptr;
        line->tptr++;
        line->tree->numElem += 1;

        node_t* right = GetP(line);
        if (!right) return 0;

        opNode->left  = left;
        opNode->right = right;
        left->parent  = opNode;
        right->parent = opNode;

        left = opNode;

        op = line->tokens[line->tptr].data.op;
    }

    return left;
}

static node_t* GetP(line_t* line){
    if (line->err == true) return 0;

    if (line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op == O_OBR){
        line->tptr++;

        node_t* node = GetE(line);
        if (!node) return 0;

        //printf("tptr:%d, data:%d\n", line->tptr, line->tokens[line->tptr].data.op);
        if (line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op != O_CBR) SyntaxError(line, S_BRCKT);

        line->tptr++;

        return node;
    }

    else if (line->tokens[line->tptr].type == T_ID && line->tokens[line->tptr + 1].type == T_OPR && line->tokens[line->tptr + 1].data.op == O_OBR){
        node_t* nodeId = GetId(line);
        if (!nodeId) return 0;

        line->tptr++;

        node_t* nodeE = GetE(line);
        if (!nodeE) return 0;

        if (line->tokens[line->tptr].type == T_OPR && line->tokens[line->tptr].data.op != O_CBR) SyntaxError(line, S_BRCKT);

        line->tptr += 1;

        nodeId->right = nodeE;

        return nodeId;
    }

    else if (line->tokens[line->tptr].type == T_ID){
        node_t* nodeId = GetId(line);
        if (!nodeId) return 0;

        return nodeId;
    }

    else {
        return GetNum(line);
    }
}

static node_t* GetId(line_t* line){
    if (line->err == true) return 0;

    node_t* node = line->tokens + line->tptr;

    if (node->type != T_ID) SyntaxError(line, S_ID);
    else{
        line->tptr += 1;
        line->tree->numElem += 1;
    }

    return node;
}

static node_t* GetNum(line_t* line){
    if (line->err == true) return 0;

    node_t* node = line->tokens + line->tptr;

    if (node->type == T_NUM){
        line->tptr += 1;
        line->tree->numElem += 1;
    }

    else {
        SyntaxError(line, S_NUM);
    }

    return node;
}
