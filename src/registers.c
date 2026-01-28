#include <stddef.h>
#include <strings.h>
#include "../include/asm386.h"

/* Check if token is a segment register */
int
is_segment_register(const char *token)
{
	const char *sregs[] = {"cs", "ds", "es", "fs", "gs", "ss", NULL};
	
	for (int i = 0; sregs[i]; i++) {
		if (strcasecmp(token, sregs[i]) == 0)
			return 1;
	}
	return 0;
}

/* Get segment register code (0-5) */
int
get_segment_register_code(const char *token)
{
	if (strcasecmp(token, "es") == 0) return 0;
	if (strcasecmp(token, "cs") == 0) return 1;
	if (strcasecmp(token, "ss") == 0) return 2;
	if (strcasecmp(token, "ds") == 0) return 3;
	if (strcasecmp(token, "fs") == 0) return 4;
	if (strcasecmp(token, "gs") == 0) return 5;
	return 0;
}

/* Check if token is any register */
int
is_register(const char *token)
{
	const char *regs[] = {
		"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
		"ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
		"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
		"cs", "ds", "es", "fs", "gs", "ss",
		NULL
	};

	for (int i = 0; regs[i]; i++) {
		if (strcasecmp(token, regs[i]) == 0)
			return 1;
	}
	return 0;
}

/* Get register code (0-7) for encoding */
int
get_register_code(const char *token)
{
	const char *regs8[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
	const char *regs16[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
	const char *regs32[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};

	for (int i = 0; i < 8; i++) {
		if (strcasecmp(token, regs8[i]) == 0)
			return i;
		if (strcasecmp(token, regs16[i]) == 0)
			return i;
		if (strcasecmp(token, regs32[i]) == 0)
			return i;
	}
	return 0;
}

/* Get register size in bits (8, 16, or 32) */
int
get_register_size(const char *token)
{
	if (token[0] == 'e')
		return 32;
	if (token[1] == 'x' || token[1] == 'p' || token[1] == 'i')
		return 16;
	return 8;
}
