#include <stdio.h>
#include <stdlib.h>
#include "../include/asm386.h"

/* Emit single byte to code buffer */
void
emit_byte(uint8_t byte)
{
	if (asm_ctx.code_pos >= MAX_CODE) {
		fprintf(stderr, "error: code size exceeded\n");
		exit(1);
	}
	if (asm_ctx.pass == 2)
		asm_ctx.code[asm_ctx.code_pos] = byte;
	asm_ctx.code_pos++;
}

/* Emit 16-bit word (little-endian) */
void
emit_word(uint16_t word)
{
	emit_byte(word & 0xff);
	emit_byte((word >> 8) & 0xff);
}

/* Emit 32-bit dword (little-endian) */
void
emit_dword(uint32_t dword)
{
	emit_byte(dword & 0xff);
	emit_byte((dword >> 8) & 0xff);
	emit_byte((dword >> 16) & 0xff);
	emit_byte((dword >> 24) & 0xff);
}

/* Emit ModR/M byte */
void
emit_modrm(int mod, int reg, int rm)
{
	emit_byte((mod << 6) | ((reg & 7) << 3) | (rm & 7));
}

/* Emit SIB (Scale-Index-Base) byte */
void
emit_sib(int scale, int index, int base)
{
	int scale_bits = 0;
	
	/* Convert scale to 2-bit value */
	if (scale == 2) scale_bits = 1;
	else if (scale == 4) scale_bits = 2;
	else if (scale == 8) scale_bits = 3;

	emit_byte((scale_bits << 6) | ((index & 7) << 3) | (base & 7));
}

/* Emit memory operand with ModR/M and optional SIB/displacement */
void
emit_memory_operand(int reg, operand_t *mem)
{
	/* 16-bit addressing mode */
	if (mem->size == 16 || (mem->base >= 0 && mem->base <= 7 && mem->size != 32)) {
		int mod = 0;
		int rm = 0;

		/* 16-bit addressing combinations */
		if (mem->base == 3 && mem->index == 6) {  /* bx+si */
			rm = 0;
		} else if (mem->base == 3 && mem->index == 7) {  /* bx+di */
			rm = 1;
		} else if (mem->base == 5 && mem->index == 6) {  /* bp+si */
			rm = 2;
		} else if (mem->base == 5 && mem->index == 7) {  /* bp+di */
			rm = 3;
		} else if (mem->base == 6 && mem->index == -1) {  /* si */
			rm = 4;
		} else if (mem->base == 7 && mem->index == -1) {  /* di */
			rm = 5;
		} else if (mem->base == 5 && mem->index == -1) {  /* bp */
			rm = 6;
		} else if (mem->base == 3 && mem->index == -1) {  /* bx */
			rm = 7;
		} else {
			rm = 6;  /* default to [bp+disp] */
		}

		/* Determine mod based on displacement */
		if (mem->disp == 0 && rm != 6) {  /* bp always needs displacement */
			mod = 0;
		} else if (mem->disp >= -128 && mem->disp <= 127) {
			mod = 1;
		} else {
			mod = 2;
		}

		emit_modrm(mod, reg, rm);

		/* Emit displacement */
		if (mod == 1) {
			emit_byte(mem->disp);
		} else if (mod == 2) {
			emit_word(mem->disp);
		}
		return;
	}

	/* 32-bit addressing mode */
	int mod = 0;
	int rm = mem->base;

	/* Determine addressing mode based on displacement */
	if (mem->disp == 0 && mem->base != 5) {  /* ebp requires displacement */
		mod = 0;  /* no displacement */
	} else if (mem->disp >= -128 && mem->disp <= 127) {
		mod = 1;  /* 8-bit displacement */
	} else {
		mod = 2;  /* 32-bit displacement */
	}

	/* Check if SIB byte is needed */
	int need_sib = (mem->index != -1) || (mem->base == 4);  /* esp always needs SIB */

	if (need_sib) {
		emit_modrm(mod, reg, 4);  /* rm=4 signals SIB follows */
		emit_sib(mem->scale, 
			 mem->index != -1 ? mem->index : 4,  /* no index = 4 */
			 mem->base);
	} else {
		emit_modrm(mod, reg, rm);
	}

	/* Emit displacement if needed */
	if (mod == 1) {
		emit_byte(mem->disp);
	} else if (mod == 2) {
		emit_dword(mem->disp);
	}
}
