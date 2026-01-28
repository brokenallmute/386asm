#include <string.h>
#include "../include/asm386.h"

/* Forward declarations for instruction handlers */
static void assemble_mov(char *operands);
static void assemble_push(char *operands);
static void assemble_pop(char *operands);
static void assemble_add(char *operands);
static void assemble_sub(char *operands);
static void assemble_xor(char *operands);
static void assemble_cmp(char *operands);
static void assemble_jmp(char *operands);
static void assemble_conditional_jump(uint8_t opcode, char *operands);
static void assemble_call(char *operands);
static void assemble_int(char *operands);
static void assemble_inc(char *operands);
static void assemble_dec(char *operands);
static void assemble_mul(char *operands);
static void assemble_imul(char *operands);
static void assemble_div(char *operands);
static void assemble_idiv(char *operands);
static void assemble_lea(char *operands);
static void assemble_test(char *operands);
static void assemble_neg(char *operands);
static void assemble_not(char *operands);
static void assemble_shl(char *operands);
static void assemble_shr(char *operands);

/* Main instruction dispatcher */
void
assemble_instruction(char *mnemonic, char *operands)
{
	/* Simple instructions (no operands) */
	if (strcmp(mnemonic, "nop") == 0) {
		emit_byte(0x90);
	} else if (strcmp(mnemonic, "ret") == 0) {
		emit_byte(0xc3);
	} else if (strcmp(mnemonic, "hlt") == 0) {
		emit_byte(0xf4);
	} else if (strcmp(mnemonic, "cli") == 0) {
		emit_byte(0xfa);
	} else if (strcmp(mnemonic, "sti") == 0) {
		emit_byte(0xfb);
	} else if (strcmp(mnemonic, "cld") == 0) {
		emit_byte(0xfc);
	} else if (strcmp(mnemonic, "std") == 0) {
		emit_byte(0xfd);
	}
	
	/* Data movement */
	else if (strcmp(mnemonic, "mov") == 0) {
		assemble_mov(operands);
	} else if (strcmp(mnemonic, "push") == 0) {
		assemble_push(operands);
	} else if (strcmp(mnemonic, "pop") == 0) {
		assemble_pop(operands);
	} else if (strcmp(mnemonic, "lea") == 0) {
		assemble_lea(operands);
	}
	
	/* Arithmetic */
	else if (strcmp(mnemonic, "add") == 0) {
		assemble_add(operands);
	} else if (strcmp(mnemonic, "sub") == 0) {
		assemble_sub(operands);
	} else if (strcmp(mnemonic, "inc") == 0) {
		assemble_inc(operands);
	} else if (strcmp(mnemonic, "dec") == 0) {
		assemble_dec(operands);
	} else if (strcmp(mnemonic, "mul") == 0) {
		assemble_mul(operands);
	} else if (strcmp(mnemonic, "imul") == 0) {
		assemble_imul(operands);
	} else if (strcmp(mnemonic, "div") == 0) {
		assemble_div(operands);
	} else if (strcmp(mnemonic, "idiv") == 0) {
		assemble_idiv(operands);
	} else if (strcmp(mnemonic, "neg") == 0) {
		assemble_neg(operands);
	}
	
	/* Logic */
	else if (strcmp(mnemonic, "xor") == 0) {
		assemble_xor(operands);
	} else if (strcmp(mnemonic, "cmp") == 0) {
		assemble_cmp(operands);
	} else if (strcmp(mnemonic, "test") == 0) {
		assemble_test(operands);
	} else if (strcmp(mnemonic, "not") == 0) {
		assemble_not(operands);
	}
	
	/* Shifts */
	else if (strcmp(mnemonic, "shl") == 0) {
		assemble_shl(operands);
	} else if (strcmp(mnemonic, "shr") == 0) {
		assemble_shr(operands);
	}
	
	/* Jumps */
	else if (strcmp(mnemonic, "jmp") == 0) {
		assemble_jmp(operands);
	} else if (strcmp(mnemonic, "je") == 0 || strcmp(mnemonic, "jz") == 0) {
		assemble_conditional_jump(0x74, operands);
	} else if (strcmp(mnemonic, "jne") == 0 || strcmp(mnemonic, "jnz") == 0) {
		assemble_conditional_jump(0x75, operands);
	} else if (strcmp(mnemonic, "jl") == 0) {
		assemble_conditional_jump(0x7c, operands);
	} else if (strcmp(mnemonic, "jg") == 0) {
		assemble_conditional_jump(0x7f, operands);
	} else if (strcmp(mnemonic, "jle") == 0) {
		assemble_conditional_jump(0x7e, operands);
	} else if (strcmp(mnemonic, "jge") == 0) {
		assemble_conditional_jump(0x7d, operands);
	} else if (strcmp(mnemonic, "ja") == 0) {
		assemble_conditional_jump(0x77, operands);
	} else if (strcmp(mnemonic, "jb") == 0) {
		assemble_conditional_jump(0x72, operands);
	}
	
	/* Calls */
	else if (strcmp(mnemonic, "call") == 0) {
		assemble_call(operands);
	} else if (strcmp(mnemonic, "int") == 0) {
		assemble_int(operands);
	}
}

/* MOV instruction - handles all addressing modes */
static void
assemble_mov(char *operands)
{
	char dst[64], src[64];
	operands = parse_token(operands, dst, sizeof(dst));
	operands = parse_token(operands, src, sizeof(src));

	operand_t dst_op, src_op;
	parse_operand(dst, &dst_op);
	parse_operand(src, &src_op);

	/* mov segreg, reg16 */
	if (is_segment_register(dst) && src_op.type == OPERAND_REG) {
		emit_byte(0x8e);
		emit_byte(0xc0 + (get_segment_register_code(dst) << 3) + src_op.reg);
		return;
	}

	/* mov reg16, segreg */
	if (dst_op.type == OPERAND_REG && is_segment_register(src)) {
		emit_byte(0x8c);
		emit_byte(0xc0 + (get_segment_register_code(src) << 3) + dst_op.reg);
		return;
	}

	/* mov reg, imm */
	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_IMM) {
		if (dst_op.size == 32) {
			emit_byte(0x66);  /* 32-bit prefix */
			emit_byte(0xb8 + dst_op.reg);
			emit_dword(src_op.imm);
		} else if (dst_op.size == 16) {
			emit_byte(0xb8 + dst_op.reg);
			emit_word(src_op.imm);
		} else {
			emit_byte(0xb0 + dst_op.reg);
			emit_byte(src_op.imm);
		}
		return;
	}

	/* mov reg, reg */
	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_REG) {
		if (dst_op.size == 32 || src_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0x89);
		} else if (dst_op.size == 16 || src_op.size == 16) {
			emit_byte(0x89);
		} else {
			emit_byte(0x88);
		}
		emit_byte(0xc0 + (src_op.reg << 3) + dst_op.reg);
		return;
	}

	/* mov reg, [mem] */
	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_MEM) {
		if (dst_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0x8b);
		} else if (dst_op.size == 16) {
			emit_byte(0x8b);
		} else {
			emit_byte(0x8a);
		}
		emit_memory_operand(dst_op.reg, &src_op);
		return;
	}

	/* mov [mem], reg */
	if (dst_op.type == OPERAND_MEM && src_op.type == OPERAND_REG) {
		if (src_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0x89);
		} else if (src_op.size == 16) {
			emit_byte(0x89);
		} else {
			emit_byte(0x88);
		}
		emit_memory_operand(src_op.reg, &dst_op);
		return;
	}

	/* mov [mem], imm */
	if (dst_op.type == OPERAND_MEM && src_op.type == OPERAND_IMM) {
		if (dst_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0xc7);
			emit_memory_operand(0, &dst_op);
			emit_dword(src_op.imm);
		} else if (dst_op.size == 16) {
			emit_byte(0xc7);
			emit_memory_operand(0, &dst_op);
			emit_word(src_op.imm);
		} else {
			emit_byte(0xc6);
			emit_memory_operand(0, &dst_op);
			emit_byte(src_op.imm);
		}
		return;
	}
}

/* PUSH instruction */
static void
assemble_push(char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	if (is_register(token)) {
		int reg = get_register_code(token);
		if (token[0] == 'e') {
			emit_byte(0x66);  /* 32-bit prefix */
		}
		emit_byte(0x50 + reg);
	} else {
		uint32_t imm;
		if (parse_number(token, &imm)) {
			emit_byte(0x68);
			emit_dword(imm);
		}
	}
}

/* POP instruction */
static void
assemble_pop(char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	if (is_register(token)) {
		int reg = get_register_code(token);
		if (token[0] == 'e') {
			emit_byte(0x66);  /* 32-bit prefix */
		}
		emit_byte(0x58 + reg);
	}
}

/* ADD instruction */
static void
assemble_add(char *operands)
{
	char dst[64], src[64];
	operands = parse_token(operands, dst, sizeof(dst));
	operands = parse_token(operands, src, sizeof(src));

	operand_t dst_op, src_op;
	parse_operand(dst, &dst_op);
	parse_operand(src, &src_op);

	/* add reg, imm */
	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_IMM) {
		if (strcasecmp(dst, "eax") == 0) {
			emit_byte(0x66);
			emit_byte(0x05);
			emit_dword(src_op.imm);
		} else if (strcasecmp(dst, "ax") == 0) {
			emit_byte(0x05);
			emit_word(src_op.imm);
		} else if (strcasecmp(dst, "al") == 0) {
			emit_byte(0x04);
			emit_byte(src_op.imm);
		} else {
			if (dst_op.size == 32) {
				emit_byte(0x66);
				emit_byte(0x81);
				emit_byte(0xc0 + dst_op.reg);
				emit_dword(src_op.imm);
			} else if (dst_op.size == 16) {
				emit_byte(0x81);
				emit_byte(0xc0 + dst_op.reg);
				emit_word(src_op.imm);
			} else {
				emit_byte(0x80);
				emit_byte(0xc0 + dst_op.reg);
				emit_byte(src_op.imm);
			}
		}
		return;
	}

	/* add reg, reg */
	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_REG) {
		if (dst_op.size == 32 || src_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0x01);
		} else if (dst_op.size == 16 || src_op.size == 16) {
			emit_byte(0x01);
		} else {
			emit_byte(0x00);
		}
		emit_byte(0xc0 + (src_op.reg << 3) + dst_op.reg);
		return;
	}

	/* add reg, [mem] */
	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_MEM) {
		if (dst_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0x03);
		} else if (dst_op.size == 16) {
			emit_byte(0x03);
		} else {
			emit_byte(0x02);
		}
		emit_memory_operand(dst_op.reg, &src_op);
	}
}

/* SUB instruction */
static void
assemble_sub(char *operands)
{
	char dst[64], src[64];
	operands = parse_token(operands, dst, sizeof(dst));
	operands = parse_token(operands, src, sizeof(src));

	operand_t dst_op, src_op;
	parse_operand(dst, &dst_op);
	parse_operand(src, &src_op);

	/* sub reg, imm */
	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_IMM) {
		if (strcasecmp(dst, "eax") == 0) {
			emit_byte(0x66);
			emit_byte(0x2d);
			emit_dword(src_op.imm);
		} else if (strcasecmp(dst, "ax") == 0) {
			emit_byte(0x2d);
			emit_word(src_op.imm);
		} else if (strcasecmp(dst, "al") == 0) {
			emit_byte(0x2c);
			emit_byte(src_op.imm);
		} else {
			if (dst_op.size == 32) {
				emit_byte(0x66);
				emit_byte(0x81);
				emit_byte(0xe8 + dst_op.reg);
				emit_dword(src_op.imm);
			} else if (dst_op.size == 16) {
				emit_byte(0x81);
				emit_byte(0xe8 + dst_op.reg);
				emit_word(src_op.imm);
			} else {
				emit_byte(0x80);
				emit_byte(0xe8 + dst_op.reg);
				emit_byte(src_op.imm);
			}
		}
		return;
	}

	/* sub reg, reg */
	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_REG) {
		if (dst_op.size == 32 || src_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0x29);
		} else if (dst_op.size == 16 || src_op.size == 16) {
			emit_byte(0x29);
		} else {
			emit_byte(0x28);
		}
		emit_byte(0xc0 + (src_op.reg << 3) + dst_op.reg);
		return;
	}

	/* sub reg, [mem] */
	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_MEM) {
		if (dst_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0x2b);
		} else if (dst_op.size == 16) {
			emit_byte(0x2b);
		} else {
			emit_byte(0x2a);
		}
		emit_memory_operand(dst_op.reg, &src_op);
	}
}

/* XOR instruction */
static void
assemble_xor(char *operands)
{
	char dst[64], src[64];
	operands = parse_token(operands, dst, sizeof(dst));
	operands = parse_token(operands, src, sizeof(src));

	operand_t dst_op, src_op;
	parse_operand(dst, &dst_op);
	parse_operand(src, &src_op);

	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_REG) {
		if (dst_op.size == 32 || src_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0x31);
		} else if (dst_op.size == 16 || src_op.size == 16) {
			emit_byte(0x31);
		} else {
			emit_byte(0x30);
		}
		emit_byte(0xc0 + (src_op.reg << 3) + dst_op.reg);
	}
}

/* CMP instruction */
static void
assemble_cmp(char *operands)
{
	char dst[64], src[64];
	operands = parse_token(operands, dst, sizeof(dst));
	operands = parse_token(operands, src, sizeof(src));

	operand_t dst_op, src_op;
	parse_operand(dst, &dst_op);
	parse_operand(src, &src_op);

	/* cmp reg, imm */
	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_IMM) {
		if (strcasecmp(dst, "eax") == 0) {
			emit_byte(0x66);
			emit_byte(0x3d);
			emit_dword(src_op.imm);
		} else if (strcasecmp(dst, "ax") == 0) {
			emit_byte(0x3d);
			emit_word(src_op.imm);
		} else if (strcasecmp(dst, "al") == 0) {
			emit_byte(0x3c);
			emit_byte(src_op.imm);
		} else {
			if (dst_op.size == 32) {
				emit_byte(0x66);
				emit_byte(0x81);
				emit_byte(0xf8 + dst_op.reg);
				emit_dword(src_op.imm);
			} else if (dst_op.size == 16) {
				emit_byte(0x81);
				emit_byte(0xf8 + dst_op.reg);
				emit_word(src_op.imm);
			} else {
				emit_byte(0x80);
				emit_byte(0xf8 + dst_op.reg);
				emit_byte(src_op.imm);
			}
		}
		return;
	}

	/* cmp reg, reg */
	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_REG) {
		if (dst_op.size == 32 || src_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0x39);
		} else if (dst_op.size == 16 || src_op.size == 16) {
			emit_byte(0x39);
		} else {
			emit_byte(0x38);
		}
		emit_byte(0xc0 + (src_op.reg << 3) + dst_op.reg);
		return;
	}

	/* cmp reg, [mem] */
	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_MEM) {
		if (dst_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0x3b);
		} else if (dst_op.size == 16) {
			emit_byte(0x3b);
		} else {
			emit_byte(0x3a);
		}
		emit_memory_operand(dst_op.reg, &src_op);
	}
}

/* JMP instruction */
static void
assemble_jmp(char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	uint32_t target;
	if (find_label(token, &target)) {
		int32_t offset = target - (asm_ctx.code_pos + 2);
		/* Short jump if possible */
		if (offset >= -128 && offset <= 127) {
			emit_byte(0xeb);
			emit_byte(offset);
		} else {
			/* Long jump */
			offset = target - (asm_ctx.code_pos + 5);
			emit_byte(0xe9);
			emit_dword(offset);
		}
	} else if (parse_number(token, &target)) {
		int32_t offset = target - (asm_ctx.code_pos + 5);
		emit_byte(0xe9);
		emit_dword(offset);
	} else {
		/* Forward reference - assume long jump */
		emit_byte(0xe9);
		emit_dword(0);
	}
}

/* Conditional jump */
static void
assemble_conditional_jump(uint8_t opcode, char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	uint32_t target;
	if (find_label(token, &target)) {
		int32_t offset = target - (asm_ctx.code_pos + 2);
		/* Short jump if possible */
		if (offset >= -128 && offset <= 127) {
			emit_byte(opcode);
			emit_byte(offset);
		} else {
			/* Long jump (0x0f prefix) */
			offset = target - (asm_ctx.code_pos + 6);
			emit_byte(0x0f);
			emit_byte(opcode + 0x10);
			emit_dword(offset);
		}
	} else {
		/* Forward reference - assume short */
		emit_byte(opcode);
		emit_byte(0);
	}
}

/* CALL instruction */
static void
assemble_call(char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	uint32_t target;
	if (find_label(token, &target)) {
		int32_t offset = target - (asm_ctx.code_pos + 5);
		emit_byte(0xe8);
		emit_dword(offset);
	} else if (parse_number(token, &target)) {
		int32_t offset = target - (asm_ctx.code_pos + 5);
		emit_byte(0xe8);
		emit_dword(offset);
	} else {
		emit_byte(0xe8);
		emit_dword(0);
	}
}

/* INT instruction */
static void
assemble_int(char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	uint32_t vector;
	if (parse_number(token, &vector)) {
		emit_byte(0xcd);
		emit_byte(vector);
	}
}

/* INC instruction */
static void
assemble_inc(char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	if (is_register(token)) {
		int reg = get_register_code(token);
		if (token[0] == 'e') {
			emit_byte(0x66);
			emit_byte(0x40 + reg);
		} else if (token[1] == 'x' || token[1] == 'p' || token[1] == 'i') {
			emit_byte(0x40 + reg);
		} else {
			emit_byte(0xfe);
			emit_byte(0xc0 + reg);
		}
	}
}

/* DEC instruction */
static void
assemble_dec(char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	if (is_register(token)) {
		int reg = get_register_code(token);
		if (token[0] == 'e') {
			emit_byte(0x66);
			emit_byte(0x48 + reg);
		} else if (token[1] == 'x' || token[1] == 'p' || token[1] == 'i') {
			emit_byte(0x48 + reg);
		} else {
			emit_byte(0xfe);
			emit_byte(0xc8 + reg);
		}
	}
}

/* MUL instruction (unsigned multiply) */
static void
assemble_mul(char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	operand_t op;
	parse_operand(token, &op);

	if (op.type == OPERAND_REG) {
		if (op.size == 32) {
			emit_byte(0x66);
			emit_byte(0xf7);
			emit_byte(0xe0 + op.reg);
		} else if (op.size == 16) {
			emit_byte(0xf7);
			emit_byte(0xe0 + op.reg);
		} else {
			emit_byte(0xf6);
			emit_byte(0xe0 + op.reg);
		}
	}
}

/* IMUL instruction (signed multiply) */
static void
assemble_imul(char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	operand_t op;
	parse_operand(token, &op);

	if (op.type == OPERAND_REG) {
		if (op.size == 32) {
			emit_byte(0x66);
			emit_byte(0xf7);
			emit_byte(0xe8 + op.reg);
		} else if (op.size == 16) {
			emit_byte(0xf7);
			emit_byte(0xe8 + op.reg);
		} else {
			emit_byte(0xf6);
			emit_byte(0xe8 + op.reg);
		}
	}
}

/* DIV instruction (unsigned divide) */
static void
assemble_div(char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	operand_t op;
	parse_operand(token, &op);

	if (op.type == OPERAND_REG) {
		if (op.size == 32) {
			emit_byte(0x66);
			emit_byte(0xf7);
			emit_byte(0xf0 + op.reg);
		} else if (op.size == 16) {
			emit_byte(0xf7);
			emit_byte(0xf0 + op.reg);
		} else {
			emit_byte(0xf6);
			emit_byte(0xf0 + op.reg);
		}
	}
}

/* IDIV instruction (signed divide) */
static void
assemble_idiv(char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	operand_t op;
	parse_operand(token, &op);

	if (op.type == OPERAND_REG) {
		if (op.size == 32) {
			emit_byte(0x66);
			emit_byte(0xf7);
			emit_byte(0xf8 + op.reg);
		} else if (op.size == 16) {
			emit_byte(0xf7);
			emit_byte(0xf8 + op.reg);
		} else {
			emit_byte(0xf6);
			emit_byte(0xf8 + op.reg);
		}
	}
}

/* LEA instruction (load effective address) */
static void
assemble_lea(char *operands)
{
	char dst[64], src[64];
	operands = parse_token(operands, dst, sizeof(dst));
	operands = parse_token(operands, src, sizeof(src));

	operand_t dst_op, src_op;
	parse_operand(dst, &dst_op);
	parse_operand(src, &src_op);

	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_MEM) {
		if (dst_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0x8d);
		} else {
			emit_byte(0x8d);
		}
		emit_memory_operand(dst_op.reg, &src_op);
	}
}

/* TEST instruction */
static void
assemble_test(char *operands)
{
	char dst[64], src[64];
	operands = parse_token(operands, dst, sizeof(dst));
	operands = parse_token(operands, src, sizeof(src));

	operand_t dst_op, src_op;
	parse_operand(dst, &dst_op);
	parse_operand(src, &src_op);

	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_REG) {
		if (dst_op.size == 32 || src_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0x85);
		} else if (dst_op.size == 16 || src_op.size == 16) {
			emit_byte(0x85);
		} else {
			emit_byte(0x84);
		}
		emit_byte(0xc0 + (src_op.reg << 3) + dst_op.reg);
	}
}

/* NEG instruction (negate) */
static void
assemble_neg(char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	operand_t op;
	parse_operand(token, &op);

	if (op.type == OPERAND_REG) {
		if (op.size == 32) {
			emit_byte(0x66);
			emit_byte(0xf7);
			emit_byte(0xd8 + op.reg);
		} else if (op.size == 16) {
			emit_byte(0xf7);
			emit_byte(0xd8 + op.reg);
		} else {
			emit_byte(0xf6);
			emit_byte(0xd8 + op.reg);
		}
	}
}

/* NOT instruction (bitwise NOT) */
static void
assemble_not(char *operands)
{
	char token[64];
	parse_token(operands, token, sizeof(token));

	operand_t op;
	parse_operand(token, &op);

	if (op.type == OPERAND_REG) {
		if (op.size == 32) {
			emit_byte(0x66);
			emit_byte(0xf7);
			emit_byte(0xd0 + op.reg);
		} else if (op.size == 16) {
			emit_byte(0xf7);
			emit_byte(0xd0 + op.reg);
		} else {
			emit_byte(0xf6);
			emit_byte(0xd0 + op.reg);
		}
	}
}

/* SHL instruction (shift left) */
static void
assemble_shl(char *operands)
{
	char dst[64], src[64];
	operands = parse_token(operands, dst, sizeof(dst));
	operands = parse_token(operands, src, sizeof(src));

	operand_t dst_op, src_op;
	parse_operand(dst, &dst_op);
	parse_operand(src, &src_op);

	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_IMM) {
		if (dst_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0xc1);
			emit_byte(0xe0 + dst_op.reg);
			emit_byte(src_op.imm);
		} else if (dst_op.size == 16) {
			emit_byte(0xc1);
			emit_byte(0xe0 + dst_op.reg);
			emit_byte(src_op.imm);
		} else {
			emit_byte(0xc0);
			emit_byte(0xe0 + dst_op.reg);
			emit_byte(src_op.imm);
		}
	}
}

/* SHR instruction (shift right) */
static void
assemble_shr(char *operands)
{
	char dst[64], src[64];
	operands = parse_token(operands, dst, sizeof(dst));
	operands = parse_token(operands, src, sizeof(src));

	operand_t dst_op, src_op;
	parse_operand(dst, &dst_op);
	parse_operand(src, &src_op);

	if (dst_op.type == OPERAND_REG && src_op.type == OPERAND_IMM) {
		if (dst_op.size == 32) {
			emit_byte(0x66);
			emit_byte(0xc1);
			emit_byte(0xe8 + dst_op.reg);
			emit_byte(src_op.imm);
		} else if (dst_op.size == 16) {
			emit_byte(0xc1);
			emit_byte(0xe8 + dst_op.reg);
			emit_byte(src_op.imm);
		} else {
			emit_byte(0xc0);
			emit_byte(0xe8 + dst_op.reg);
			emit_byte(src_op.imm);
		}
	}
}
