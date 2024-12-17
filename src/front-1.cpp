#include "../hpp/back.hpp"

#define MEOW fprintf(stderr, PNK  "MEOW\n" RESET);

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
    fprintf(line->files.out, "%s", opList[trmnl].name);

    return OK;
}

static int GenerateNode(line_t* line, node_t* node){



    if (node->type == T_OPR){

        if (node->data.op == O_IFB){
            int opIFB = FindOpByNum(O_IFB);
            fprintf(line->files.out, "\n%s ", opList[opIFB].name);

            GenerateNode(line, node->right);

            int opIFC = FindOpByNum(O_IFC);
            fprintf(line->files.out, "\t%s ", opList[opIFC].name);

            GenerateNode(line, node->left);
        }

        else if (node->data.op == O_WHB){
            int opWHB = FindOpByNum(O_WHB);
            fprintf(line->files.out, "\n%s ", opList[opWHB].name);

            GenerateNode(line, node->right);

            int opWHC = FindOpByNum(O_WHC);
            fprintf(line->files.out, "\t%s ", opList[opWHC].name);

            GenerateNode(line, node->left);
        }

        else if (node->data.op == O_PNT){
            int opNum = FindOpByNum(node->data.op);
            fprintf(line->files.out, "%s ", opList[opNum].name);

            if (node->left) GenerateNode(line, node->left);
        }

        else{

            if (node->left) GenerateNode(line, node->left);

            int opNum = FindOpByNum(node->data.op);
            fprintf(line->files.out, "%s ", opList[opNum].name);

            if (node->data.op == O_SEP){
                fprintf(line->files.out, "\n");
            }

            if (node->right) GenerateNode(line, node->right);
        }
    }


    if (node->parent && (node->parent->right == node || node->parent->data.op == O_PNT)){

        if (node->type == T_ID){
            char buffer[BUFF_LEN] = {};
            snprintf(buffer, BUFF_LEN, "%s", line->id[node->data.id].name);
            fprintf(line->files.out, "%s", buffer);
        }

        if (node->type == T_NUM){
            fprintf(line->files.out, "%0.0lf", node->data.num);
        }

    }

    else{

        if (node->type == T_ID){
            char buffer[BUFF_LEN] = {};
            snprintf(buffer, BUFF_LEN, "%s", line->id[node->data.id].name);
            fprintf(line->files.out, "%s ", buffer);
        }

        if (node->type == T_NUM){
            fprintf(line->files.out, "%0.0lf ", node->data.num);
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
