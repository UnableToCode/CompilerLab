#include "assembly.h"

const char* REG_NAME[REG_NUM] = {
    "$0",  "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2",
    "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5",
    "$s6", "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};

pRegisters registers;
pVarTable varTable;

pRegisters initRegisters() {
    pRegisters p = (pRegisters)malloc(sizeof(Registers));
    assert(p != NULL);
    for (int i = 0; i < REG_NUM; i++) {
        p->regLsit[i] = newRegister(REG_NAME[i]);
    }
    p->regLsit[0]->isFree = FALSE;  // $0不允许使用
    p->lastChangedNo = 0;
    return p;
}

void resetRegisters(pRegisters registers) {
    for (int i = 1; i < REG_NUM; i++) {
        registers->regLsit[i]->isFree = TRUE;
    }
}

void deleteRegisters(pRegisters registers) {
    assert(registers != NULL);
    for (int i = 0; i < REG_NUM; i++) {
        free(registers->regLsit[i]);
    }
    free(registers);
}

pVarTable newVarTable() {
    pVarTable p = (pVarTable)malloc(sizeof(VarTable));
    assert(p != NULL);
    p->varListReg = newVarList();
    p->varListMem = newVarList();
    p->inFunc = FALSE;
    p->curFuncName = NULL;
    return p;
}

void deleteVarTable(pVarTable varTable) {
    assert(varTable != NULL);
    clearVarList(varTable->varListReg);
    clearVarList(varTable->varListMem);
    free(varTable->varListReg);
    free(varTable->varListMem);
    free(varTable);
}

pVarList newVarList() {
    pVarList p = (pVarList)malloc(sizeof(VaribleList));
    assert(p != NULL);
    p->head = NULL;
    p->cur = NULL;
    return p;
}

void printVarList(pVarList varList) {
    printf("----------VarList------------\n");
    pVarible temp = varList->head;
    while (temp) {
        if (temp->op->kind == OP_CONSTANT)
            printf("reg: %s, value: %d\n", REG_NAME[temp->regNo],
                   temp->op->u.value);
        else
            printf("reg: %s, varName: %s\n", REG_NAME[temp->regNo],
                   temp->op->u.name);
        temp = temp->next;
    }
    printf("------------end--------------\n");
}

void addVarible(pVarList varList, int regNo, pOperand op) {
    pVarible newVar = newVarible(regNo, op);
    // reg_list[reg_num].is_free=0;
    if (varList->head == NULL) {
        varList->head = newVar;
        varList->cur = newVar;
    } else {
        varList->cur->next = newVar;
        varList->cur = newVar;
    }
}

void delVarible(pVarList varList, pVarible var) {
    if (var == varList->head) {
        varList->head = varList->head->next;
    } else {
        pVarible temp = varList->head;
        while (temp) {
            if (temp->next == var) break;
            temp = temp->next;
        }
        if (varList->cur == var) varList->cur = temp;
        temp->next = var->next;
        var->next = NULL;
        free(var);
    }
}

void clearVarList(pVarList varList) {
    assert(varList != NULL);
    pVarible temp = varList->head;
    while (temp) {
        pVarible p = temp;
        temp = temp->next;
        free(p);
    }
    varList->head = NULL;
    varList->cur = NULL;
}

int checkVarible(FILE* fp, pVarTable varTable, pRegisters registers,
                 pOperand op) {
    if (op->kind != OP_CONSTANT) {
        // 若为变量，先看变量表里有没有，有就返回，没有新分配一个
        pVarible temp = varTable->varListReg->head;
        while (temp) {
            if (temp->op->kind != OP_CONSTANT &&
                !strcmp(temp->op->u.name, op->u.name))
                return temp->regNo;
            temp = temp->next;
        }
        int regNo = allocReg(registers, varTable, op);
        return regNo;
    } else {
        // 立即数则找个寄存器放进去, 如果为0直接返回0号寄存器
        if (op->u.value == 0) return ZERO;
        int regNo = allocReg(registers, varTable, op);
        fprintf(fp, "  li %s, %d\n", registers->regLsit[regNo]->name,
                op->u.value);
        return regNo;
    }
}

int allocReg(pRegisters registers, pVarTable varTable, pOperand op) {
    // 先看有无空闲，有就直接放
    // printf("allocNewReg\n");
    for (int i = T0; i <= T9; i++) {
        if (registers->regLsit[i]->isFree) {
            registers->regLsit[i]->isFree = 0;
            addVarible(varTable->varListReg, i, op);
            return i;
        }
    }
    // printf("nofree\n");
    // 无空闲了，需要找一个寄存器释放掉
    // 可以先释放掉最近没用过的立即数
    pVarible temp = varTable->varListReg->head;
    while (temp) {
        if (temp->op->kind == OP_CONSTANT &&
            temp->regNo != registers->lastChangedNo) {
            int regNo = temp->regNo;
            registers->lastChangedNo = regNo;
            delVarible(varTable->varListReg, temp);
            addVarible(varTable->varListReg, regNo, op);
            return regNo;
        }
        temp = temp->next;
    }

    // 如果没有立即数，就找一个最近没用过的临时变量的释放掉
    temp = varTable->varListReg->head;
    while (temp) {
        if (temp->op->kind != OP_CONSTANT) {
            if (temp->op->u.name[0] == 't' &&
                temp->regNo != registers->lastChangedNo) {
                int regNo = temp->regNo;
                registers->lastChangedNo = regNo;
                delVarible(varTable->varListReg, temp);
                addVarible(varTable->varListReg, regNo, op);
                return regNo;
            }
        }
        temp = temp->next;
    }
}

pRegister newRegister(const char* regName) {
    pRegister p = (pRegister)malloc(sizeof(Register));
    assert(p != NULL);
    p->isFree = TRUE;
    p->name = regName;
}

pVarible newVarible(int regNo, pOperand op) {
    pVarible p = (pVarible)malloc(sizeof(Varible));
    assert(p != NULL);
    p->regNo = regNo;
    p->op = op;
    p->next = NULL;
}

void genAssemblyCode(FILE* fp) {
    registers = initRegisters();
    varTable = newVarTable();
    initCode(fp);
    printf("init success\n");
    pInterCodes temp = interCodeList->head;
    while (temp) {
        interToAssem(fp, temp);
        // printVarList(varTable->varListReg);
        temp = temp->next;
    }
    printf("gen end\n");
    deleteRegisters(registers);
    deleteVarTable(varTable);
    registers = NULL;
    varTable = NULL;
}

void initCode(FILE* fp) {
    fprintf(fp, ".data\n");
    fprintf(fp, "_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(fp, "_ret: .asciiz \"\\n\"\n");
    fprintf(fp, ".globl main\n");

    // 无重复变量名，直接把数组当全局变量声明了
    pInterCodes temp = interCodeList->head;
    while (temp) {
        if (temp->code->kind == IR_DEC)
            fprintf(fp, "%s: .word %d\n", temp->code->u.dec.op->u.name,
                    temp->code->u.dec.size);
        temp = temp->next;
    }

    // read function
    fprintf(fp, ".text\n");
    fprintf(fp, "read:\n");
    fprintf(fp, "  li $v0, 4\n");
    fprintf(fp, "  la $a0, _prompt\n");
    fprintf(fp, "  syscall\n");
    fprintf(fp, "  li $v0, 5\n");
    fprintf(fp, "  syscall\n");
    fprintf(fp, "  jr $ra\n\n");

    // write function
    fprintf(fp, "write:\n");
    fprintf(fp, "  li $v0, 1\n");
    fprintf(fp, "  syscall\n");
    fprintf(fp, "  li $v0, 4\n");
    fprintf(fp, "  la $a0, _ret\n");
    fprintf(fp, "  syscall\n");
    fprintf(fp, "  move $v0, $0\n");
    fprintf(fp, "  jr $ra\n");
}

void printinter(pInterCodes cur) {
    FILE* fp = NULL;
    switch (cur->code->kind) {
        case IR_LABEL:
            printf("LABEL ");
            printOp(fp, cur->code->u.oneOp.op);
            printf(" :");
            break;
        case IR_FUNCTION:
            printf("FUNCTION ");
            printOp(fp, cur->code->u.oneOp.op);
            printf(" :");
            break;
        case IR_ASSIGN:
            printOp(fp, cur->code->u.assign.left);
            printf(" := ");
            printOp(fp, cur->code->u.assign.right);
            break;
        case IR_ADD:
            printOp(fp, cur->code->u.binOp.result);
            printf(" := ");
            printOp(fp, cur->code->u.binOp.op1);
            printf(" + ");
            printOp(fp, cur->code->u.binOp.op2);
            break;
        case IR_SUB:
            printOp(fp, cur->code->u.binOp.result);
            printf(" := ");
            printOp(fp, cur->code->u.binOp.op1);
            printf(" - ");
            printOp(fp, cur->code->u.binOp.op2);
            break;
        case IR_MUL:
            printOp(fp, cur->code->u.binOp.result);
            printf(" := ");
            printOp(fp, cur->code->u.binOp.op1);
            printf(" * ");
            printOp(fp, cur->code->u.binOp.op2);
            break;
        case IR_DIV:
            printOp(fp, cur->code->u.binOp.result);
            printf(" := ");
            printOp(fp, cur->code->u.binOp.op1);
            printf(" / ");
            printOp(fp, cur->code->u.binOp.op2);
            break;
        case IR_GET_ADDR:
            printOp(fp, cur->code->u.assign.left);
            printf(" := &");
            printOp(fp, cur->code->u.assign.right);
            break;
        case IR_READ_ADDR:
            printOp(fp, cur->code->u.assign.left);
            printf(" := *");
            printOp(fp, cur->code->u.assign.right);
            break;
        case IR_WRITE_ADDR:
            printf("*");
            printOp(fp, cur->code->u.assign.left);
            printf(" := ");
            printOp(fp, cur->code->u.assign.right);
            break;
        case IR_GOTO:
            printf("GOTO ");
            printOp(fp, cur->code->u.oneOp.op);
            break;
        case IR_IF_GOTO:
            printf("IF ");
            printOp(fp, cur->code->u.ifGoto.x);
            printf(" ");
            printOp(fp, cur->code->u.ifGoto.relop);
            printf(" ");
            printOp(fp, cur->code->u.ifGoto.y);
            printf(" GOTO ");
            printOp(fp, cur->code->u.ifGoto.z);
            break;
        case IR_RETURN:
            printf("RETURN ");
            printOp(fp, cur->code->u.oneOp.op);
            break;
        case IR_DEC:
            printf("DEC ");
            printOp(fp, cur->code->u.dec.op);
            printf(" ");
            printf("%d", cur->code->u.dec.size);
            break;
        case IR_ARG:
            printf("ARG ");
            printOp(fp, cur->code->u.oneOp.op);
            break;
        case IR_CALL:
            printOp(fp, cur->code->u.assign.left);
            printf(" := CALL ");
            printOp(fp, cur->code->u.assign.right);
            break;
        case IR_PARAM:
            printf("PARAM ");
            printOp(fp, cur->code->u.oneOp.op);
            break;
        case IR_READ:
            printf("READ ");
            printOp(fp, cur->code->u.oneOp.op);
            break;
        case IR_WRITE:
            printf("WRITE ");
            printOp(fp, cur->code->u.oneOp.op);
            break;
    }
    printf("\n");
}

void interToAssem(FILE* fp, pInterCodes interCodes) {
    // printf("gen one\n");
    // printinter(interCodes);
    pInterCode interCode = interCodes->code;
    int kind = interCode->kind;

    // switch里不能声明新变量，改用if else逻辑处理
    // assert(kind >= 0 && kind < 19);
    // switch (kind) {
    //     case IR_LABEL:
    //         fprintf(fp, "%s:\n", interCode->u.oneOp.op->u.name);
    //         break;
    //     case IR_FUNCTION:
    //         fprintf(fp, "\n%s:\n", interCode->u.oneOp.op->u.name);

    //         //
    //         新函数，寄存器重新变为可用，并清空变量表（因为假定没有全局变量）
    //         resetRegisters(registers);
    //         clearVarList(varTable->varListReg);
    //         clearVarList(varTable->varListMem);

    //         // main函数单独处理一下，在main里调用函数不算函数嵌套调用
    //         if (!strcmp(interCode->u.oneOp.op->u.name, "main")) {
    //             varTable->inFunc = FALSE;
    //             varTable->curFuncName = NULL;
    //         } else {
    //             varTable->inFunc = TRUE;
    //             varTable->curFuncName = interCode->u.oneOp.op->u.name;

    //             // 处理形参 IR_PARAM
    //             pItem item =
    //                 searchTableItem(table, interCode->u.oneOp.op->u.name);
    //             int argc = 0;
    //             pInterCodes temp = interCodes->next;
    //             while (temp && temp->code->kind == IR_PARAM) {
    //                 // 前4个参数存到a0 到a3中
    //                 if (argc < 4) {
    //                     addVarible(varTable->varListReg, A0 + argc,
    //                                temp->code->u.oneOp.op);
    //                 } else {
    //                     // 剩下的要用栈存
    //                     int regNo = checkVarible(fp, varTable, registers,
    //                                              temp->code->u.oneOp.op);
    //                     fprintf(
    //                         fp, "  lw %s, %d($fp)\n",
    //                         registers->regLsit[regNo]->name,
    //                         (item->field->type->u.function.argc - 1 - argc) *
    //                             4);
    //                 }
    //                 argc++;
    //                 temp = temp->next;
    //             }
    //         }
    //         break;
    //     case IR_GOTO:
    //         fprintf(fp, "  j %s\n", interCode->u.oneOp.op->u.name);
    //         break;
    //     case IR_RETURN:
    //         // return 0可以用$0寄存器中的常数0
    //         if (interCode->u.oneOp.op->kind == OP_CONSTANT &&
    //             interCode->u.oneOp.op->u.value == 0)
    //             fprintf(fp, "  move $v0, $0\n");
    //         else {
    //             int RegNo = checkVarible(fp, varTable, registers,
    //                                      interCode->u.oneOp.op);
    //             fprintf(fp, "  move $v0, %s\n",
    //                     registers->regLsit[RegNo]->name);
    //         }
    //         fprintf(fp, "  jr $ra\n");
    //         break;
    //     case IR_ARG:
    //         // 需要在call里处理
    //         break;
    //     case IR_PARAM:
    //         // 需要在function里处理
    //         break;
    //     case IR_READ:
    //         fprintf(fp, "  addi $sp, $sp, -4\n");
    //         fprintf(fp, "  sw $ra, 0($sp)\n");
    //         fprintf(fp, "  jal read\n");
    //         fprintf(fp, "  lw $ra, 0($sp)\n");
    //         fprintf(fp, "  addi $sp, $sp, 4\n");
    //         int RegNo =
    //             checkVarible(fp, varTable, registers, interCode->u.oneOp.op);
    //         fprintf(fp, "  move %s, $v0\n", registers->regLsit[RegNo]->name);
    //         break;
    //     case IR_WRITE:
    //         int RegNo =
    //             checkVarible(fp, varTable, registers, interCode->u.oneOp.op);
    //         if (varTable->inFunc == FALSE) {
    //             fprintf(fp, "  move $a0, %s\n",
    //                     registers->regLsit[RegNo]->name);
    //             fprintf(fp, "  addi $sp, $sp, -4\n");
    //             fprintf(fp, "  sw $ra, 0($sp)\n");
    //             fprintf(fp, "  jal write\n");
    //             fprintf(fp, "  lw $ra, 0($sp)\n");
    //             fprintf(fp, "  addi $sp, $sp, 4\n");
    //         } else {
    //             // 函数嵌套调用，先将a0压栈 调用结束以后需要恢复a0
    //             fprintf(fp, "  addi $sp, $sp, -8\n");
    //             fprintf(fp, "  sw $a0, 0($sp)\n");
    //             fprintf(fp, "  sw $ra, 4($sp)\n");
    //             fprintf(fp, "  move $a0, %s\n",
    //                     registers->regLsit[RegNo]->name);
    //             fprintf(fp, "  jal write\n");
    //             fprintf(fp, "  lw $a0, 0($sp)\n");
    //             fprintf(fp, "  lw $ra, 4($sp)\n");
    //             fprintf(fp, "  addi $sp, $sp, 8\n");
    //         }
    //     case IR_ASSIGN:
    //         int leftRegNo =
    //             checkVarible(fp, varTable, registers,
    //             interCode->u.assign.left);
    //         // 右值为立即数，直接放左值寄存器里
    //         if (interCode->u.assign.right->kind == OP_CONSTANT) {
    //             fprintf(fp, "  li %s, %d\n",
    //                     registers->regLsit[leftRegNo]->name,
    //                     interCode->u.assign.right->u.value);
    //         }
    //         // 右值为变量，先check再move赋值寄存器
    //         else {
    //             int rightRegNo = checkVarible(fp, varTable, registers,
    //                                           interCode->u.assign.right);
    //             fprintf(fp, "  move %s, %s\n",
    //                     registers->regLsit[leftRegNo]->name,
    //                     registers->regLsit[rightRegNo]->name);
    //         }
    //         break;
    //     case IR_GET_ADDR:
    //         int leftRegNo =
    //             checkVarible(fp, varTable, registers,
    //             interCode->u.assign.left);
    //         fprintf(fp, "  la %s, %s\n", registers->regLsit[leftRegNo]->name,
    //                 interCode->u.assign.right->u.name);
    //         break;
    //     case IR_READ_ADDR:
    //         int leftRegNo =
    //             checkVarible(fp, varTable, registers,
    //             interCode->u.assign.left);
    //         int rightRegNo = checkVarible(fp, varTable, registers,
    //                                       interCode->u.assign.right);
    //         fprintf(fp, "  lw %s, 0(%s)\n",
    //         registers->regLsit[leftRegNo]->name,
    //                 registers->regLsit[rightRegNo]->name);
    //         break;
    //     case IR_WRITE_ADDR:
    //         int leftRegNo =
    //             checkVarible(fp, varTable, registers,
    //             interCode->u.assign.left);
    //         int rightRegNo = checkVarible(fp, varTable, registers,
    //                                       interCode->u.assign.right);
    //         fprintf(fp, "  sw %s, 0(%s)\n",
    //                 registers->regLsit[rightRegNo]->name,
    //                 registers->regLsit[leftRegNo]->name);
    //         break;
    //     case IR_CALL:
    //         break;
    //     case IR_ADD:
    //         break;
    //     case IR_SUB:
    //         break;
    //     case IR_MUL:
    //         break;
    //     case IR_DIV:
    //         break;
    //     case IR_DEC:
    //         break;
    //     case IR_IF_GOTO:
    //         break;
    // }
    if (kind == IR_LABEL) {
        fprintf(fp, "%s:\n", interCode->u.oneOp.op->u.name);
    } else if (kind == IR_FUNCTION) {
        fprintf(fp, "\n%s:\n", interCode->u.oneOp.op->u.name);

        // 新函数，寄存器重新变为可用，并清空变量表（因为假定没有全局变量）
        resetRegisters(registers);
        clearVarList(varTable->varListReg);
        clearVarList(varTable->varListMem);

        // main函数单独处理一下，在main里调用函数不算函数嵌套调用
        if (!strcmp(interCode->u.oneOp.op->u.name, "main")) {
            varTable->inFunc = FALSE;
            varTable->curFuncName = NULL;
        } else {
            varTable->inFunc = TRUE;
            varTable->curFuncName = interCode->u.oneOp.op->u.name;

            // 处理形参 IR_PARAM
            pItem item = searchTableItem(table, interCode->u.oneOp.op->u.name);
            int argc = 0;
            pInterCodes temp = interCodes->next;
            while (temp && temp->code->kind == IR_PARAM) {
                // 前4个参数存到a0 到a3中
                if (argc < 4) {
                    addVarible(varTable->varListReg, A0 + argc,
                               temp->code->u.oneOp.op);
                } else {
                    // 剩下的要用栈存
                    int regNo = checkVarible(fp, varTable, registers,
                                             temp->code->u.oneOp.op);
                    fprintf(
                        fp, "  lw %s, %d($fp)\n",
                        registers->regLsit[regNo]->name,
                        (item->field->type->u.function.argc - 1 - argc) * 4);
                }
                argc++;
                temp = temp->next;
            }
        }
    } else if (kind == IR_GOTO) {
        fprintf(fp, "  j %s\n", interCode->u.oneOp.op->u.name);
    } else if (kind == IR_RETURN) {
        int RegNo =
            checkVarible(fp, varTable, registers, interCode->u.oneOp.op);
        fprintf(fp, "  move $v0, %s\n", registers->regLsit[RegNo]->name);
        fprintf(fp, "  jr $ra\n");
    } else if (kind == IR_ARG) {
        // 需要在call里处理
    } else if (kind == IR_PARAM) {
        // 需要在function里处理
    } else if (kind == IR_READ) {
        fprintf(fp, "  addi $sp, $sp, -4\n");
        fprintf(fp, "  sw $ra, 0($sp)\n");
        fprintf(fp, "  jal read\n");
        fprintf(fp, "  lw $ra, 0($sp)\n");
        fprintf(fp, "  addi $sp, $sp, 4\n");
        int RegNo =
            checkVarible(fp, varTable, registers, interCode->u.oneOp.op);
        fprintf(fp, "  move %s, $v0\n", registers->regLsit[RegNo]->name);
    } else if (kind == IR_WRITE) {
        int RegNo =
            checkVarible(fp, varTable, registers, interCode->u.oneOp.op);
        if (varTable->inFunc == FALSE) {
            fprintf(fp, "  move $a0, %s\n", registers->regLsit[RegNo]->name);
            fprintf(fp, "  addi $sp, $sp, -4\n");
            fprintf(fp, "  sw $ra, 0($sp)\n");
            fprintf(fp, "  jal write\n");
            fprintf(fp, "  lw $ra, 0($sp)\n");
            fprintf(fp, "  addi $sp, $sp, 4\n");
        } else {
            // 函数嵌套调用，先将a0压栈 调用结束以后需要恢复a0
            fprintf(fp, "  addi $sp, $sp, -8\n");
            fprintf(fp, "  sw $a0, 0($sp)\n");
            fprintf(fp, "  sw $ra, 4($sp)\n");
            fprintf(fp, "  move $a0, %s\n", registers->regLsit[RegNo]->name);
            fprintf(fp, "  jal write\n");
            fprintf(fp, "  lw $a0, 0($sp)\n");
            fprintf(fp, "  lw $ra, 4($sp)\n");
            fprintf(fp, "  addi $sp, $sp, 8\n");
        }
    } else if (kind == IR_ASSIGN) {
        int leftRegNo =
            checkVarible(fp, varTable, registers, interCode->u.assign.left);
        // 右值为立即数，直接放左值寄存器里
        if (interCode->u.assign.right->kind == OP_CONSTANT) {
            fprintf(fp, "  li %s, %d\n", registers->regLsit[leftRegNo]->name,
                    interCode->u.assign.right->u.value);
        }
        // 右值为变量，先check再move赋值寄存器
        else {
            int rightRegNo = checkVarible(fp, varTable, registers,
                                          interCode->u.assign.right);
            fprintf(fp, "  move %s, %s\n", registers->regLsit[leftRegNo]->name,
                    registers->regLsit[rightRegNo]->name);
        }
    } else if (kind == IR_GET_ADDR) {
        int leftRegNo =
            checkVarible(fp, varTable, registers, interCode->u.assign.left);
        // int rightRegNo =
        //     checkVarible(fp, varTable, registers, interCode->u.assign.right);
        fprintf(fp, "  la %s, %s\n", registers->regLsit[leftRegNo]->name,
                interCode->u.assign.right->u.name);
        // fprintf(fp, "  move %s, %s\n", registers->regLsit[leftRegNo]->name,
        //         registers->regLsit[rightRegNo]->name);
    } else if (kind == IR_READ_ADDR) {
        int leftRegNo =
            checkVarible(fp, varTable, registers, interCode->u.assign.left);
        int rightRegNo =
            checkVarible(fp, varTable, registers, interCode->u.assign.right);
        fprintf(fp, "  lw %s, 0(%s)\n", registers->regLsit[leftRegNo]->name,
                registers->regLsit[rightRegNo]->name);
    } else if (kind == IR_WRITE_ADDR) {
        int leftRegNo =
            checkVarible(fp, varTable, registers, interCode->u.assign.left);
        int rightRegNo =
            checkVarible(fp, varTable, registers, interCode->u.assign.right);
        fprintf(fp, "  sw %s, 0(%s)\n", registers->regLsit[rightRegNo]->name,
                registers->regLsit[leftRegNo]->name);
    } else if (kind == IR_CALL) {
        pItem calledFunc =
            searchTableItem(table, interCode->u.assign.right->u.name);
        int leftRegNo =
            checkVarible(fp, varTable, registers, interCode->u.assign.left);
        // 函数调用前的准备
        fprintf(fp, "  addi $sp, $sp, -4\n");
        fprintf(fp, "  sw $ra, 0($sp)\n");
        pusha(fp);

        // 如果是函数嵌套调用，把前形参存到内存，腾出a0-a3寄存器给新调用使用
        if (varTable->inFunc) {
            fprintf(fp, "  addi $sp, $sp, -%d\n",
                    calledFunc->field->type->u.function.argc * 4);
            pItem curFunc = searchTableItem(table, varTable->curFuncName);
            for (int i = 0; i < curFunc->field->type->u.function.argc; i++) {
                if (i > calledFunc->field->type->u.function.argc) break;
                if (i < 4) {
                    fprintf(fp, "  sw %s, %d($sp)\n",
                            registers->regLsit[A0 + i]->name, i * 4);
                    pVarible var = varTable->varListReg->head;
                    while (var && var->regNo != A0 + i) {
                        var = var->next;
                    }
                    delVarible(varTable->varListReg, var);
                    addVarible(varTable->varListMem, -1, var->op);
                    int regNo = checkVarible(fp, varTable, registers, var->op);
                    fprintf(fp, "  move %s, %s\n",
                            registers->regLsit[regNo]->name,
                            registers->regLsit[A0 + i]->name);
                }
            }
        }

        // 处理实参 IR_ARG
        pInterCodes arg = interCodes->prev;
        int argc = 0;
        while (arg && argc < calledFunc->field->type->u.function.argc) {
            if (arg->code->kind == IR_ARG) {
                int argRegNo = checkVarible(fp, varTable, registers,
                                            arg->code->u.oneOp.op);
                // 前4个参数直接用寄存器存
                if (argc < 4) {
                    fprintf(fp, "  move %s, %s\n",
                            registers->regLsit[A0 + argc]->name,
                            registers->regLsit[argRegNo]->name);
                    argc++;
                }
                // 4个以后的参数压栈
                else {
                    fprintf(fp, "  addi $sp, $sp, -4\n");
                    fprintf(fp, "  sw %s, 0($sp)\n",
                            registers->regLsit[argRegNo]->name);
                    fprintf(fp, "  move $fp, $sp\n");
                    argc++;
                }
            }
            arg = arg->prev;
        }

        // 函数调用
        fprintf(fp, "  jal %s\n", interCode->u.assign.right->u.name);

        // 调用完后恢复栈指针、形参，然后恢复之前保存入栈的寄存器信息
        if (argc > 4) fprintf(fp, "  addi $sp, $sp, %d\n", 4 * argc);
        if (varTable->inFunc) {
            pItem curFunc = searchTableItem(table, varTable->curFuncName);
            for (int i = 0; i < curFunc->field->type->u.function.argc; i++) {
                if (i > calledFunc->field->type->u.function.argc) break;
                if (i < 4) {
                    fprintf(fp, "  lw %s, %d($sp)\n",
                            registers->regLsit[A0 + i]->name, i * 4);
                    pVarible var = varTable->varListReg->head;
                    while (var) {
                        if (var->op->kind != OP_CONSTANT &&
                            !strcmp(varTable->varListMem->head->op->u.name,
                                    var->op->u.name))
                            break;
                        var = var->next;
                    }
                    if (var) {
                        registers->regLsit[var->regNo]->isFree = TRUE;
                        var->regNo = A0 + i;
                    } else {
                        addVarible(varTable->varListReg, A0 + i,
                                   varTable->varListMem->head->op);
                    }
                    delVarible(varTable->varListMem,
                               varTable->varListMem->head);
                }
            }
            fprintf(fp, "  addi $sp, $sp, %d\n",
                    calledFunc->field->type->u.function.argc * 4);
        }
        popa(fp);
        fprintf(fp, "  lw $ra, 0($sp)\n");
        fprintf(fp, "  addi $sp, $sp, 4\n");
        fprintf(fp, "  move %s, $v0\n", registers->regLsit[leftRegNo]->name);
    } else if (kind == IR_ADD) {
        int resultRegNo =
            checkVarible(fp, varTable, registers, interCode->u.binOp.result);
        // 常数 常数
        if (interCode->u.binOp.op1->kind == OP_CONSTANT &&
            interCode->u.binOp.op2->kind == OP_CONSTANT) {
            fprintf(fp, "  li %s, %d\n", registers->regLsit[resultRegNo]->name,
                    interCode->u.binOp.op1->u.value +
                        interCode->u.binOp.op2->u.value);
        }
        // 变量 常数
        else if (interCode->u.binOp.op1->kind != OP_CONSTANT &&
                 interCode->u.binOp.op2->kind == OP_CONSTANT) {
            int op1RegNo =
                checkVarible(fp, varTable, registers, interCode->u.binOp.op1);
            fprintf(fp, "  addi %s, %s, %d\n",
                    registers->regLsit[resultRegNo]->name,
                    registers->regLsit[op1RegNo]->name,
                    interCode->u.binOp.op2->u.value);
        }
        // 变量 变量
        else {
            int op1RegNo =
                checkVarible(fp, varTable, registers, interCode->u.binOp.op1);
            int op2RegNo =
                checkVarible(fp, varTable, registers, interCode->u.binOp.op2);
            fprintf(fp, "  add %s, %s, %s\n",
                    registers->regLsit[resultRegNo]->name,
                    registers->regLsit[op1RegNo]->name,
                    registers->regLsit[op2RegNo]->name);
        }
    } else if (kind == IR_SUB) {
        int resultRegNo =
            checkVarible(fp, varTable, registers, interCode->u.binOp.result);
        // 常数 常数
        if (interCode->u.binOp.op1->kind == OP_CONSTANT &&
            interCode->u.binOp.op2->kind == OP_CONSTANT) {
            fprintf(fp, "  li %s, %d\n", registers->regLsit[resultRegNo]->name,
                    interCode->u.binOp.op1->u.value -
                        interCode->u.binOp.op2->u.value);
        }
        // 变量 常数
        else if (interCode->u.binOp.op1->kind != OP_CONSTANT &&
                 interCode->u.binOp.op2->kind == OP_CONSTANT) {
            int op1RegNo =
                checkVarible(fp, varTable, registers, interCode->u.binOp.op1);
            fprintf(fp, "  addi %s, %s, %d\n",
                    registers->regLsit[resultRegNo]->name,
                    registers->regLsit[op1RegNo]->name,
                    -interCode->u.binOp.op2->u.value);
        }
        // 变量 变量
        else {
            int op1RegNo =
                checkVarible(fp, varTable, registers, interCode->u.binOp.op1);
            int op2RegNo =
                checkVarible(fp, varTable, registers, interCode->u.binOp.op2);
            fprintf(fp, "  sub %s, %s, %s\n",
                    registers->regLsit[resultRegNo]->name,
                    registers->regLsit[op1RegNo]->name,
                    registers->regLsit[op2RegNo]->name);
        }
    } else if (kind == IR_MUL) {
        int resultRegNo =
            checkVarible(fp, varTable, registers, interCode->u.binOp.result);
        int op1RegNo =
            checkVarible(fp, varTable, registers, interCode->u.binOp.op1);
        int op2RegNo =
            checkVarible(fp, varTable, registers, interCode->u.binOp.op2);
        fprintf(fp, "  mul %s, %s, %s\n", registers->regLsit[resultRegNo]->name,
                registers->regLsit[op1RegNo]->name,
                registers->regLsit[op2RegNo]->name);
    } else if (kind == IR_DIV) {
        int resultRegNo =
            checkVarible(fp, varTable, registers, interCode->u.binOp.result);
        int op1RegNo =
            checkVarible(fp, varTable, registers, interCode->u.binOp.op1);
        int op2RegNo =
            checkVarible(fp, varTable, registers, interCode->u.binOp.op2);
        fprintf(fp, "  div %s, %s\n", registers->regLsit[op1RegNo]->name,
                registers->regLsit[op2RegNo]->name);
        fprintf(fp, "  mflo %s\n", registers->regLsit[resultRegNo]->name);
    } else if (kind == IR_DEC) {
        // init 时候坐到全局变量里了
        // fprintf(fp, "  %s: .word %d\n", interCode->u.dec.op->u.name,
        //         interCode->u.dec.size);

    } else if (kind == IR_IF_GOTO) {
        char* relopName = interCode->u.ifGoto.relop->u.name;
        int xRegNo =
            checkVarible(fp, varTable, registers, interCode->u.ifGoto.x);
        int yRegNo =
            checkVarible(fp, varTable, registers, interCode->u.ifGoto.y);
        if (!strcmp(relopName, "=="))
            fprintf(fp, "  beq %s, %s, %s\n", registers->regLsit[xRegNo]->name,
                    registers->regLsit[yRegNo]->name,
                    interCode->u.ifGoto.z->u.name);
        else if (!strcmp(relopName, "!="))
            fprintf(fp, "  bne %s, %s, %s\n", registers->regLsit[xRegNo]->name,
                    registers->regLsit[yRegNo]->name,
                    interCode->u.ifGoto.z->u.name);
        else if (!strcmp(relopName, ">"))
            fprintf(fp, "  bgt %s, %s, %s\n", registers->regLsit[xRegNo]->name,
                    registers->regLsit[yRegNo]->name,
                    interCode->u.ifGoto.z->u.name);
        else if (!strcmp(relopName, "<"))
            fprintf(fp, "  blt %s, %s, %s\n", registers->regLsit[xRegNo]->name,
                    registers->regLsit[yRegNo]->name,
                    interCode->u.ifGoto.z->u.name);
        else if (!!strcmp(relopName, ">="))
            fprintf(fp, "  bge %s, %s, %s\n", registers->regLsit[xRegNo]->name,
                    registers->regLsit[yRegNo]->name,
                    interCode->u.ifGoto.z->u.name);
        else if (strcmp(relopName, "<="))
            fprintf(fp, "  ble %s, %s, %s\n", registers->regLsit[xRegNo]->name,
                    registers->regLsit[yRegNo]->name,
                    interCode->u.ifGoto.z->u.name);
    }
}

void pusha(FILE* fp) {
    fprintf(fp, "  addi $sp, $sp, -72\n");
    for (int i = T0; i <= T9; i++) {
        fprintf(fp, "  sw %s, %d($sp)\n", registers->regLsit[i]->name,
                (i - T0) * 4);
    }
}

void popa(FILE* fp) {
    for (int i = T0; i <= T9; i++) {
        fprintf(fp, "  lw %s, %d($sp)\n", registers->regLsit[i]->name,
                (i - T0) * 4);
    }
    fprintf(fp, "  addi $sp, $sp, 72\n");
}