.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
a: .word 8
.text
read:
  li $v0, 4
  la $a0, _prompt
  syscall
  li $v0, 5
  syscall
  jr $ra

write:
  li $v0, 1
  syscall
  li $v0, 4
  la $a0, _ret
  syscall
  move $v0, $0
  jr $ra

func:
  li $t1, 4
  mul $t0, $0, $t1
  la $t2, a
  add $t3, $t2, $t0
  li $t4, 1
  sw $t4, 0($t3)
  li $t6, 1
  li $t7, 4
  mul $t5, $t6, $t7
  la $s0, a
  add $s1, $s0, $t5
  li $s2, 2
  sw $s2, 0($s1)
  li $s4, 4
  mul $s3, $a0, $s4
  la $s5, a
  add $s6, $s5, $s3
  li $t8, 4
  mul $s7, $a1, $t8
  la $t9, a
  add $t1, $t9, $s7
  lw $t4, 0($s6)
  lw $t6, 0($t1)
  add $t7, $t4, $t6
  move $v0, $t7
  jr $ra

main:
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  addi $sp, $sp, -72
  sw $t0, 0($sp)
  sw $t1, 4($sp)
  sw $t2, 8($sp)
  sw $t3, 12($sp)
  sw $t4, 16($sp)
  sw $t5, 20($sp)
  sw $t6, 24($sp)
  sw $t7, 28($sp)
  sw $s0, 32($sp)
  sw $s1, 36($sp)
  sw $s2, 40($sp)
  sw $s3, 44($sp)
  sw $s4, 48($sp)
  sw $s5, 52($sp)
  sw $s6, 56($sp)
  sw $s7, 60($sp)
  sw $t8, 64($sp)
  sw $t9, 68($sp)
  li $t1, 1
  move $a0, $t1
  move $a1, $0
  jal func
  lw $t0, 0($sp)
  lw $t1, 4($sp)
  lw $t2, 8($sp)
  lw $t3, 12($sp)
  lw $t4, 16($sp)
  lw $t5, 20($sp)
  lw $t6, 24($sp)
  lw $t7, 28($sp)
  lw $s0, 32($sp)
  lw $s1, 36($sp)
  lw $s2, 40($sp)
  lw $s3, 44($sp)
  lw $s4, 48($sp)
  lw $s5, 52($sp)
  lw $s6, 56($sp)
  lw $s7, 60($sp)
  lw $t8, 64($sp)
  lw $t9, 68($sp)
  addi $sp, $sp, 72
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  move $t0, $v0
  move $t2, $t0
  move $a0, $t2
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal write
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  move $v0, $0
  jr $ra
