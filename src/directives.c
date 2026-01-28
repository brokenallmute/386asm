#include <string.h>
#include "../include/asm386.h"

/* Process assembler directive (.org, .db, .dw, etc) */
void
process_directive(char *directive, char *operands)
{
	/* .org - set origin address */
	if (strcmp(directive, ".org") == 0) {
		uint32_t addr;
		if (parse_number(operands, &addr)) {
			asm_ctx.origin = addr;
			asm_ctx.code_pos = addr;
		}
	}
	
	/* .db - define byte(s) */
	else if (strcmp(directive, ".db") == 0) {
		char token[64];
		while (*operands) {
			operands = parse_token(operands, token, sizeof(token));
			if (token[0] == '\0')
				break;

			/* String literal */
			if (token[0] == '"' || token[0] == '\'') {
				int len = strlen(token);
				for (int i = 1; i < len - 1; i++) {
					emit_byte(token[i]);
				}
			}
			/* Numeric value */
			else {
				uint32_t value;
				if (parse_number(token, &value)) {
					emit_byte(value);
				}
			}
		}
	}
	
	/* .dw - define word(s) (16-bit) */
	else if (strcmp(directive, ".dw") == 0) {
		char token[64];
		while (*operands) {
			operands = parse_token(operands, token, sizeof(token));
			if (token[0] == '\0')
				break;

			uint32_t value;
			if (parse_number(token, &value)) {
				emit_word(value);
			}
		}
	}
	
	/* .dd - define dword(s) (32-bit) */
	else if (strcmp(directive, ".dd") == 0) {
		char token[64];
		while (*operands) {
			operands = parse_token(operands, token, sizeof(token));
			if (token[0] == '\0')
				break;

			uint32_t value;
			if (parse_number(token, &value)) {
				emit_dword(value);
			}
		}
	}
	
	/* .align - align to boundary */
	else if (strcmp(directive, ".align") == 0) {
		uint32_t alignment;
		if (parse_number(operands, &alignment)) {
			/* Pad with zeros until aligned */
			while (asm_ctx.code_pos % alignment != 0) {
				emit_byte(0x00);
			}
		}
	}
	
	/* .times - repeat directive */
	else if (strcmp(directive, ".times") == 0) {
		char count_str[64], rest[512];
		operands = parse_token(operands, count_str, sizeof(count_str));
		
		/* Rest contains directive like ".db 0" */
		strncpy(rest, operands, sizeof(rest) - 1);
		rest[sizeof(rest) - 1] = '\0';
		
		uint32_t count;
		if (parse_number(count_str, &count)) {
			/* Repeat directive count times */
			for (uint32_t i = 0; i < count; i++) {
				char temp[512];
				strncpy(temp, rest, sizeof(temp));
				process_directive(".db", temp);
			}
		}
	}
}
