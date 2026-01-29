#include <stdio.h>
#include <string.h>
#include <ctype.h>
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
		} else {
			fprintf(stderr, "error: invalid .org address\n");
		}
	}
	
	/* .db - define byte(s) */
	else if (strcmp(directive, ".db") == 0) {
		char *p = operands;
		while (*p) {
			/* Skip whitespace */
			while (*p && isspace(*p))
				p++;
			if (!*p)
				break;

			/* String literal - handle specially */
			if (*p == '"' || *p == '\'') {
				char quote = *p++;
				while (*p && *p != quote) {
					emit_byte(*p++);
				}
				if (*p == quote)
					p++;
			}
			/* Numeric value */
			else {
				char token[64];
				int i = 0;
				while (*p && !isspace(*p) && *p != ',' && i < 63) {
					token[i++] = *p++;
				}
				token[i] = '\0';
				
				uint32_t value;
				if (parse_number(token, &value)) {
					emit_byte(value);
				}
			}
			
			/* Skip comma */
			while (*p && (isspace(*p) || *p == ','))
				p++;
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
	
	/* .pad - pad to address */
	else if (strcmp(directive, ".pad") == 0) {
		uint32_t target;
		if (parse_number(operands, &target)) {
			/* Pad to target address */
			uint32_t current = asm_ctx.origin + asm_ctx.code_pos;
			if (target > current) {
				uint32_t pad_bytes = target - current;
				for (uint32_t i = 0; i < pad_bytes; i++) {
					emit_byte(0x00);
				}
			}
		}
	}
}
