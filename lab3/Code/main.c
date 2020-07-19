#include "inter.h"
#include "syntax.tab.h"

extern pNode root;

extern int yylineno;
extern int yyparse();
extern void yyrestart(FILE*);

unsigned lexError = FALSE;
unsigned synError = FALSE;

int main(int argc, char** argv) {
    if (argc <= 1) {
        yyparse();
        return 1;
    }

    FILE* fr = fopen(argv[1], "r");
    if (!fr) {
        perror(argv[1]);
        return 1;
    }

    FILE* fw = fopen(argv[2], "wt+");
    if (!fw) {
        perror(argv[2]);
        return 1;
    }

    yyrestart(fr);
    yyparse();
    if (!lexError && !synError) {
        table = initTable();
        //printTreeInfo(root, 0);
        traverseTree(root);
        // printTable(table);
        interCodeList = newInterCodeList();
        genInterCodes(root);
        if (!interError) {
            //printInterCode(NULL, interCodeList);
            printInterCode(fw, interCodeList);
        }
        // deleteInterCodeList(interCodeList);
        deleteTable(table);
    }
    delNode(&root);
    return 0;
}
