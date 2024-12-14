#include "../hpp/dump.hpp"

#define MEOW fprintf(stderr, PNK  "MEOW\n" RESET);

const int64_t MAXDOT_BUFF     = 8;
const int64_t MAXLEN_COMMAND  = 64;
const int64_t MAX_HTML_PRNT   = 1024;

const int64_t MAX_READER_BUFF = 256;

static int FindOpByNum      (int num);
static int NodeDump         (line_t* line, node_t* node, int depth, int param);
static int StartLineDump    (line_t* line);
static int EndLineDump      (line_t* line);
static int DoDot            (line_t* line);
static int HTMLGenerateBody (line_t* line);
static int TokenDump        (line_t* line, node_t* node);
static int HTMLPrint        (line_t* line, char* text);

static int      LoadNode    (tree_t* tree, node_t* node, int param, FILE* file);
static node_t*  NewNode     (tree_t* tree, int data, int type, node_t* left, node_t* right);

static int      FindOpStd   (char* word);
static int      FindType    (char* word);

const int NAN = -1;

enum errors_dump{

    OK  = 0,
    ERR = 1

};

enum dump_params{

    SIMPLE   = 52,
    DETAILED = -52

};


/*========================================================================*/

int TreeDump(line_t* line){

    StartLineDump(line);
    NodeDump(line, line->tree->root, 0, SIMPLE);
    EndLineDump(line);
    DoDot(line);
    HTMLGenerateBody(line);

    return OK;
}

/*========================================================================*/

int TokensDump(line_t* line){

    StartLineDump(line);

    for (int i = 0; i < MAX_TKNS_DMP; i++){
        TokenDump(line, line->tokens + i);
    }

    EndLineDump(line);
    DoDot(line);
    HTMLGenerateBody(line);

    char* text = (char*) calloc(MAX_HTML_PRNT, sizeof(*text));

    for (int i = 0; i < line->numId; i++){
        snprintf(text, MAX_HTML_PRNT, "%s%d. name:\"", text, i);

        for (int j = 0; j < line->id[i].len; j++){
            snprintf(text, MAX_HTML_PRNT, "%s%c", text, line->id[i].name[j]);
        }

        snprintf(text, MAX_HTML_PRNT, "%s\"\t len:%llu <br>", text, line->id[i].len);
    }

    HTMLPrint(line, text);

    return OK;
}

/*========================================================================*/

int DumpIds(line_t* line, FILE* file){
    fprintf(file, ORG "\n-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n" RESET);
    fprintf(file, "Nametable dump:\n\n");

    for (int i = 0; i < line->numId; i++){
        fprintf(file, "%d. name:", i);
        for (int j = 0; j < line->id[i].len; j++){
            fprintf(file, "%c", line->id[i].name[j]);
        }

        fprintf(file, "\t len:%llu\n", line->id[i].len);
    }
    fprintf(file, ORG "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n" RESET);

    return OK;
}

/*========================================================================*/

//READER
//READER
//READER

int LoadTree(line_t* line){

    LoadNode(line->tree, line->tree->root, ROOT, line->files.save);

    TreeDump(line);
    return OK;
}

static int LoadNode(tree_t* tree, node_t* node, int param, FILE* file){
    char value_type_char[MAX_READER_BUFF] = {};
    char value_char[MAX_READER_BUFF] = {};

    char* strtodRet = 0;

    int     value_type = 0;
    double  value = 0;

    int offset_len = 0;

    node_t* newNode = nullptr;
    char buffer[MAX_READER_BUFF] = {};
    char bracket = 0;

    //printf("node:%p\n", node);

    if (param == ROOT){
        fscanf(file, "%[^{}]", buffer);
        bracket = getc(file);

        if (bracket == '{'){
            fscanf(file, "%[^:]:\"%[^\"]\"%n", value_type_char, value_char, &offset_len);
            value_type = FindType(value_type_char);

            if (value_type == T_NUM) value = strtod(value_char, &strtodRet);
            if (value_type == T_ID)  value = 1;
            if (value_type == T_OPR)  value = FindOpStd(value_char);

            tree->root = NewNode(tree, value, value_type, nullptr, nullptr);
        }
        else return 0;


        fscanf(file, "%[^{}]", buffer);
        bracket = getc(file);

        if (bracket == '{'){
            ungetc(bracket, file);
            LoadNode(tree, tree->root, LEFT, file);
        }

        LoadNode(tree, tree->root, RIGHT, file);
        return OK;
    }

    else if (param == LEFT){
        fscanf(file, "%[^{}]", buffer);
        bracket = getc(file);

        if (bracket == '{'){
            fscanf(file, "%[^:]:\"%[^\"]\"%n", value_type_char, value_char, &offset_len);

            printf("type:%s\n", value_type_char);

            value_type = FindType(value_type_char);

            printf("type:%d\n", value_type);

            if (value_type == T_NUM)  value = strtod(value_char, &strtodRet);
            if (value_type == T_ID)   value = 1;
            if (value_type == T_OPR)  value = FindOpStd(value_char);
            newNode = NewNode(tree, value, value_type, nullptr, nullptr);

            node->left = newNode;
            newNode->parent = node;
        }
        else return 0;



        fscanf(file, "%[^{}]", buffer);
        bracket = getc(file);

        if (bracket == '{'){
            ungetc(bracket, file);
            LoadNode(tree, newNode, LEFT, file);
        }

        LoadNode(tree, node, RIGHT, file);

        return OK;
    }

    else if (param == RIGHT){
        fscanf(file, "%[^{}]", buffer);
        bracket = getc(file);

        if (bracket == '{'){
            fscanf(file, "%[^:]:\"%[^\"]\"%n", value_type_char, value_char, &offset_len);

            printf("type:%s\n", value_type_char);

            value_type = FindType(value_type_char);

            printf("type:%d\n", value_type);

            if (value_type == T_NUM)  value = strtod(value_char, &strtodRet);
            if (value_type == T_ID)   value = 1;
            if (value_type == T_OPR)  value = FindOpStd(value_char);
            newNode = NewNode(tree, value, value_type, nullptr, nullptr);

            node->right = newNode;
            newNode->parent = node;
        }
        else return OK;



        fscanf(file, "%[^{}]", buffer);
        bracket = getc(file);

        if (bracket == '{'){
            ungetc(bracket, file);
            LoadNode(tree, newNode, LEFT, file);
        }

        LoadNode(tree, newNode, RIGHT, file);
        return OK;
    }
}

int FindOp(char* word, int length){
    for (int i = 0; i < sizeof(opList) / sizeof(opList[0]); i++){
        if (opList[i].len == length && !strncmp(word, opList[i].name, length)){
            return opList[i].opNum;
        }
    }

    return NAN;
}

static int FindOpStd(char* word){
    for (int i = 0; i < sizeof(opList) / sizeof(opList[0]); i++){
        if (!strncmp(word, opList[i].stdname, 2)){
            return opList[i].opNum;
        }
    }

    return NAN;
}

static int FindType(char* word){
    for (int i = 0; i < sizeof(typeList) / sizeof(typeList[0]); i++){
        if (!strncmp(word, typeList[i].name, 2)){
            return typeList[i].opNum;
        }
    }

    return NAN;
}

static node_t* NewNode(tree_t* tree, int data, int type, node_t* left, node_t* right){
    node_t* newNode = (node_t*)calloc(1, sizeof(*newNode));

    if (type == T_NUM) newNode->data.num = data;
    if (type == T_OPR) newNode->data.op  = data;
    if (type == T_ID)  newNode->data.id  = data;
    newNode->type    = type;

    newNode->left   = left;
    newNode->right  = right;

    if (left)  left->parent  = newNode;
    if (right) right->parent = newNode;

    newNode->id = tree->numElem;
    tree->numElem += 1;

    return newNode;
}

//READER_END
//READER_END
//READER_END

/*========================================================================*/

static int FindOpByNum(int opNum){
    for (int i = 0; i < sizeof(opList) / sizeof(opList[0]); i++){
        if (opNum == opList[i].opNum){
            return i;
        }
    }

    return -1;
}

/*========================================================================*/

static int TokenDump(line_t* line, node_t* node){
    char outBuff[MAXDOT_BUFF] = {};

/*---------DETAILED---------*/

    if (node->type == T_NUM){
        fprintf(line->files.dot,
            "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6f2ff\";label = \" {{ %0.3llu } | { p: %p } | { num: %0.2lf } | {left: %p} | {right: %p} | {parent: %p}}\"];\n",
            node->id, node->id, node, node->data.num, node->left, node->right, node->parent);
    }

    else if (node->type == T_OPR){
        snprintf(outBuff, MAXDOT_BUFF, "%c", node->data.op);

        fprintf(line->files.dot,
            "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#fff3e6\";label = \" {{ %0.3llu } | { p: %p } | { oper: %s } | {left: %p} | {right: %p} | {parent: %p}}\"];\n",
            node->id, node->id, node, outBuff, node->left, node->right, node->parent);
    }

    else if (node->type == T_ID){
        fprintf(line->files.dot,
            "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6ffe6\";label = \" {{ %0.3llu } | { p: %p } | { id: %d }| {left: %p} | {right: %p} | {parent: %p}}\"];\n",
            node->id, node->id, node, node->data.id, node->left, node->right, node->parent);
    }

    else {
        fprintf(line->files.dot,
            "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#ffe6fe\";label = \" {{ %0.3llu } | { p: %p } | { unknown %0.0lf }| {left: %p} | {right: %p} | {parent: %p}}\"];\n",
            node->id, node->id, node, node->data.num, node->left, node->right, node->parent);
    }

    return OK;
}

/*========================================================================*/

static int NodeDump(line_t* line, node_t* node, int depth, int param){
    if (!node) return OK;

    char outBuff[MAXDOT_BUFF] = {};
/*---------SIMPLE---------*/
    if (param == SIMPLE){
        if (node->type == T_NUM){
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6f2ff\";label = \" { %0.3llu } | { num: %0.2lf }\"];\n",
                node->id, node->id, node->data.num);
        }

        else if (node->type == T_OPR){
            int opNum = FindOpByNum(node->data.op);

            snprintf(outBuff, MAXDOT_BUFF, "%s", opList[opNum].stdname);

            char color[8] = {};

            if (node->data.op == O_SEP) strcpy(color, "#eee6ff");
            else                        strcpy(color, "#fff3e6");

            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"%s\";label = \" { %0.3llu } | { oper: %s }\"];\n",
                node->id, color, node->id, outBuff);
        }

        else if (node->type == T_ID){
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6ffe6\";label = \" { %0.3llu } | { id: %d }\"];\n",
                node->id, node->id, node->data.id);
        }

        else {
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#ffe6fe\";label = \" { %0.3llu } | { unknown %0.0lf }\"];\n",
                node->id, node->id, node->data.num);
        }
    }
/*---------SIMPLE---------*/

/*---------DETAILED---------*/
    else if (param == DETAILED){
        if (node->type == T_NUM){
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6f2ff\";label = \" {{ %0.3llu } | { p: %p } | { num: %0.2lf } | {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, node->data.num, node->left, node->right, node->parent);
        }

        else if (node->type == T_OPR){
            snprintf(outBuff, MAXDOT_BUFF, "%c", node->data.op);

            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#fff3e6\";label = \" {{ %0.3llu } | { p: %p } | { oper: %s } | {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, outBuff, node->left, node->right, node->parent);
        }

        else if (node->type == T_ID){
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6ffe6\";label = \" {{ %0.3llu } | { p: %p } | { var: %c }| {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, node->data.id, node->left, node->right, node->parent);
        }

        else {
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#ffe6fe\";label = \" {{ %0.3llu } | { p: %p } | { unknown %0.0lf }| {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, node->data.num, node->left, node->right, node->parent);
        }
    }
/*---------DETAILED---------*/

    //edges
    if (node->left){
        fprintf(line->files.dot,
                "\tnode%0.3llu -> node%0.3llu [ fontname=\"SF Pro\"; weight=1; color=\"#04BF00\"; style=\"bold\"];\n\n",
                node->id, node->left->id);

        NodeDump(line, node->left, depth + 1, param);
    }

    if (node->right){
        fprintf(line->files.dot,
                "\tnode%0.3llu -> node%0.3llu [ fontname=\"SF Pro\"; weight=1; color=\"#fd4381\"; style=\"bold\"];\n\n",
                node->id, node->right->id);

        NodeDump(line, node->right, depth + 1, param);
    }
    //edges

    return OK;
}

/*========================================================================*/

static int StartLineDump(line_t* line){

    line->files.dot = fopen("./bin/dot.dot", "w");

    fprintf(line->files.dot, "digraph G{\n");

    fprintf(line->files.dot, "\trankdir=TB;\n");
    fprintf(line->files.dot, "\tbgcolor=\"#f8fff8\";\n");

    return OK;
}

/*========================================================================*/

static int EndLineDump(line_t* line){

    fprintf(line->files.dot, "}\n");

    fclose(line->files.dot);

    return OK;
}

/*========================================================================*/

static int DoDot(line_t* line){
    char command[MAXLEN_COMMAND]   = {};
    char out[MAXLEN_COMMAND]       = {};

    const char* startOut= "./bin/png/output";
    const char* endOut  = ".png";

    snprintf(out,     MAXLEN_COMMAND, "%s%llu%s", startOut, line->tree->numDump, endOut);
    snprintf(command, MAXLEN_COMMAND, "dot -Tpng %s > %s", line->files.dotName, out);
    system(command);

    line->tree->numDump++;
    return OK;
}

/*========================================================================*/

int HTMLGenerateHead(line_t* line){
    fprintf(line->files.html, "<html>\n");

    fprintf(line->files.html, "<head>\n");
    fprintf(line->files.html, "</head>\n");

    fprintf(line->files.html, "<body style=\"background-color:#f8fff8;\">\n");

    return OK;
}

/*========================================================================*/

static int HTMLGenerateBody(line_t* line){
    fprintf(line->files.html, "<div style=\"text-align: center;\">\n");

    fprintf(line->files.html, "\t<h2 style=\"font-family: 'Haas Grot Text R Web', 'Helvetica Neue', Helvetica, Arial, sans-serif;'\"> Dump: %llu</h2>\n", line->tree->numDump);
    fprintf(line->files.html, "\t<h3 style=\"font-family: 'Haas Grot Text R Web', 'Helvetica Neue', Helvetica, Arial, sans-serif;'\"> Num elems: %llu</h3>\n", line->tree->numElem);

    fprintf(line->files.html, "\t<img src=\"./bin/png/output%llu.png\">\n\t<br>\n\t<br>\n\t<br>\n", line->tree->numDump - 1);

    fprintf(line->files.html, "</div>\n");

    return OK;
}

/*========================================================================*/

static int HTMLPrint(line_t* line, char* text){
    fprintf(line->files.html, "<div style=\"text-align: left;\">\n");

    fprintf(line->files.html, "\t<h4 style=\"font-family: 'Haas Grot Text R Web', 'Helvetica Neue', Helvetica, Arial, sans-serif;'\"> %s </h3>\n", text);

    fprintf(line->files.html, "</div>\n");

    return OK;
}

/*========================================================================*/

int HTMLDumpGenerate(line_t* line){

    fprintf(line->files.html, "</body>\n");
    fprintf(line->files.html, "</html>\n");

    return OK;
}
