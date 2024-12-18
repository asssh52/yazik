#include "../hpp/back.hpp"

#define MEOW fprintf(stderr, PNK  "MEOW\n" RESET);

#define OUT(...) fprintf(line->files.out, __VA_ARGS__)\

const char*   DFLT_HTML_FILE = "htmldump.html";
const char*   DFLT_SAVE_FILE = "save.txt";
const char*   DFLT_OUT_FILE  = "autoformatted.txt";
const char*   DFLT_DOT_FILE  = "./bin/dot.dot";

const int64_t MAX_IDS  = 16;
const int64_t BUFF_LEN = 16;

enum errors{

    OK  = 0,
    ERR = 1,

};

static int FormaterCtor     (line_t* line);
static int GenerateCode     (line_t* line);
static int GenerateNode     (line_t* line, node_t* node);
static int FindOpByStd      (char* word);
static int CountTabs        (line_t* line, node_t* node);
static int PlaceTabs        (line_t* line, node_t* node);

int main(){
    line_t* line = (line_t*)calloc(1, sizeof(*line));

    FormaterCtor(line);

    LoadTree(line);
    DumpIds(line, stdout);

    GenerateCode(line);

    return 0;
}


static int FormaterCtor(line_t* line){
    line->tree = (tree_t*)  calloc(1, sizeof(*line->tree));
    line->id   = (names_t*) calloc(MAX_IDS,  sizeof(*line->id));

    line->files.html = fopen(line->files.htmlName, "w");
    if (!line->files.html) line->files.html = fopen(DFLT_HTML_FILE, "w");

    line->files.save = fopen(line->files.saveName, "r");
    if (!line->files.save) line->files.save = fopen(DFLT_SAVE_FILE, "r");

    line->files.out = fopen(line->files.outName, "w");
    if (!line->files.out) line->files.out = fopen(DFLT_OUT_FILE, "w");

    line->files.dotName = DFLT_DOT_FILE;

    return OK;
}

static int GenerateCode(line_t* line){
    GenerateNode(line, line->tree->root);

    char terminal[9] = "terminal";
    int trmnl = FindOpByStd(terminal);

    printf("%d\n", trmnl);
    OUT("%s", opList[trmnl].name);

    return OK;
}

static int GenerateNode(line_t* line, node_t* node){

    if (node->parent && node->parent->data.op == O_SEP) PlaceTabs(line, node);

    if (node->type == T_OPR){

        //IF
        if (node->data.op == O_IFB){
            int opIFB = FindOpByNum(O_IFB);
            OUT("\n%s ", opList[opIFB].name);

            GenerateNode(line, node->right);

            int opIFC = FindOpByNum(O_IFC);
            OUT("\t%s ", opList[opIFC].name);

            GenerateNode(line, node->left);
        }

        //WHILE
        else if (node->data.op == O_WHB){
            int opWHB = FindOpByNum(O_WHB);
            OUT("\n%s ", opList[opWHB].name);

            GenerateNode(line, node->right);

            int opWHC = FindOpByNum(O_WHC);
            OUT("\t%s ", opList[opWHC].name);

            GenerateNode(line, node->left);
        }

        //PRINT
        else if (node->data.op == O_PNT){
            int opNum = FindOpByNum(node->data.op);
            OUT("%s ", opList[opNum].name);

            if (node->left) GenerateNode(line, node->left);
        }

        //RETURN
        else if (node->data.op == O_RET){
            int opNum = FindOpByNum(node->data.op);
            OUT("%s ", opList[opNum].name);

            if (node->left) GenerateNode(line, node->left);
        }

        //DEF FUNC
        else if (node->data.op == O_DEF){
            int opNum = FindOpByNum(node->data.op);
            OUT("%s ", opList[opNum].name);

            GenerateNode(line, node->right);

            GenerateNode(line, node->left);
        }

        //FUNC NAME
        else if (node->data.op == O_DFS){
            int opNum = FindOpByNum(node->data.op);
            OUT("%s ", opList[opNum].name);

            GenerateNode(line, node->left);

            GenerateNode(line, node->right);
        }

        //FUNC SEP
        else if (node->data.op == O_CMA){
            int opNum = FindOpByNum(node->data.op);
            OUT("%s ", opList[opNum].name);

            GenerateNode(line, node->left);

            if (node->right) GenerateNode(line, node->right);
        }

        //FUNC CALL
        else if (node->data.op == O_CAL){

            OUT("(");
            int opNum = FindOpByNum(node->data.op);
            OUT("%s ", opList[opNum].name);

            int opCSP = FindOpByNum(O_CSP);
            OUT("%s ", opList[opCSP].name);

            GenerateNode(line, node->left->left);

            if (node->left->right) GenerateNode(line, node->left->right);
            OUT(")");
        }

        //OTHERS
        else{

            if (node->data.op != O_SEP && node->data.op != O_EQL){
                OUT("(");
            }

            if (node->left) GenerateNode(line, node->left);

            int opNum = FindOpByNum(node->data.op);
            OUT("%s ", opList[opNum].name);

            if (node->data.op == O_SEP){
                OUT("\n\n");
            }

            if (node->right) GenerateNode(line, node->right);

            if (node->data.op != O_SEP && node->data.op != O_EQL){
                OUT(")");
            }
        }
    }


    if (node->parent && (node->parent->right == node || node->parent->data.op == O_PNT || node->parent->data.op == O_RET)){

        if (node->type == T_ID){
            char buffer[BUFF_LEN] = {};
            snprintf(buffer, BUFF_LEN, "%s", line->id[node->data.id].name);
            OUT("%s", buffer);
        }

        if (node->type == T_NUM){
            OUT("%0.0lf", node->data.num);
        }

    }

    else{

        if (node->type == T_ID){
            char buffer[BUFF_LEN] = {};
            snprintf(buffer, BUFF_LEN, "%s", line->id[node->data.id].name);
            OUT("%s ", buffer);
        }

        if (node->type == T_NUM){
            OUT("%0.0lf ", node->data.num);
        }

    }


    return OK;
}

static int FindOpByStd(char* word){
    for (int i = 0; i < sizeof(opList) / sizeof(opList[0]); i++){
        if (!strncmp(word, opList[i].stdname, 2)){
            return i;
        }
    }

    return -1;
}

static int CountTabs (line_t* line, node_t* node){
    int count = 0;

    if (node->type == T_OPR){
        if (node->data.op == O_DEF || node->data.op == O_IFB || node->data.op == O_WHB){
            count += 1;
        }
    }

    if (node->type == T_OPR && node->data.op == O_SEP) return count;

    if (node->parent) count += CountTabs(line, node->parent);

    return count;
}

static int PlaceTabs (line_t* line, node_t* node){
    int tabCount = CountTabs(line, node);

    for (int i = 0; i < tabCount; i++){
        OUT("\t");
    }

    return OK;
}
