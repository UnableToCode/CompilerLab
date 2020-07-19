#ifndef _ASSEMBLY_H_
#define _ASSEMBLY_H_

#include "inter.h"

#define REG_NUM 32

typedef struct _register* pRegister;
typedef struct _varible* pVarible;
typedef struct _registers* pRegisters;
typedef struct _varList* pVarList;
typedef struct _varTable* pVarTable;

typedef struct _register {
    boolean isFree;
    char* name;
} Register;

typedef struct _varible {
    int regNo;
    pOperand op;
    pVarible next;
} Varible;

typedef struct _registers {
    pRegister regLsit[REG_NUM];
    int lastChangedNo;
} Registers;

typedef struct _varList {
    pVarible head;
    pVarible cur;
} VaribleList;

typedef struct _varTable {
    pVarList varListReg;  // 寄存器中的变量表
    pVarList varListMem;  // 内存中的变量表
    boolean inFunc;
    char* curFuncName;
} VarTable;

typedef enum _regNo {
    ZERO,
    AT,
    V0,
    V1,
    A0,
    A1,
    A2,
    A3,
    T0,
    T1,
    T2,
    T3,
    T4,
    T5,
    T6,
    T7,
    S0,
    S1,
    S2,
    S3,
    S4,
    S5,
    S6,
    S7,
    T8,
    T9,
    K0,
    K1,
    GP,
    SP,
    FP,
    RA,
} RegNo;

extern pRegisters registers;
extern pVarTable varTable;

pRegisters initRegisters();
void resetRegisters(pRegisters registers);
void deleteRegisters(pRegisters registers);

pVarTable newVarTable();
pVarList newVarList();
void printVarList(pVarList varList);
void addVarible(pVarList varList, int regNo, pOperand op);
void delVarible(pVarList varList, pVarible var);
void clearVarList(pVarList varList);
int checkVarible(FILE* fp, pVarTable varTable, pRegisters registers,
                 pOperand op);

int allocReg(pRegisters registers, pVarTable varTable, pOperand op);

pRegister newRegister(const char* regName);
pVarible newVarible(int regNo, pOperand op);

void genAssemblyCode(FILE* fp);
void initCode(FILE* fp);
void interToAssem(FILE* fp, pInterCodes interCodes);
void pusha(FILE* fp);
void popa(FILE* fp);

#endif