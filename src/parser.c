#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/asm386.h"

/* Skip whitespace characters */
char *
skip_whitespace(char *str)
{
	while (*str && isspace(*str))
		str++;
	return str;
}

/* Parse next token from string */
char *
parse_token(char *str, char *token, int max_len)
{
	str = skip_whitespace(str);
	int i = 0;

	/* Extract token until whitespace or comma */
	while (*str && !isspace(*str) && *str != ',' && i < max_len - 1) {
		token[i++] = *str++;
	}
	token[i] = '\0';

	/* Skip comma if present */
	if (*str == ',')
		str++;

	return str;
}

/* Parse number (decimal, hex, or expression) */
int
parse_number(const char *str, uint32_t *value)
{
	char *endptr;

	/* Skip leading whitespace */
	while (*str && isspace(*str))
		str++;

	/* Character literal ('A' or 'x') */
	if (str[0] == '\'' && str[2] == '\'') {
		*value = (uint8_t)str[1];
		return 1;
	}

	/* Current position ($) */
	if (strcmp(str, "$") == 0) {
		*value = asm_ctx.code_pos + asm_ctx.origin;
		return 1;
	}

	/* Section start ($$) */
	if (strcmp(str, "$$") == 0) {
		*value = asm_ctx.origin;
		return 1;
	}

	/* Simple expressions like "510-($-$$)" */
	if (strchr(str, '-') || strchr(str, '+')) {
		char expr[256];
		strncpy(expr, str, sizeof(expr) - 1);
		expr[sizeof(expr) - 1] = '\0';

		/* Replace $ and $$ with actual values */
		char processed[256];
		int j = 0;
		for (int i = 0; expr[i] && j < (int)sizeof(processed) - 20; i++) {
			if (expr[i] == '$' && expr[i+1] == '$') {
				j += snprintf(processed + j, sizeof(processed) - j, 
					      "%u", asm_ctx.origin);
				i++;
			} else if (expr[i] == '$') {
				j += snprintf(processed + j, sizeof(processed) - j, 
					      "%u", asm_ctx.code_pos + asm_ctx.origin);
			} else {
				processed[j++] = expr[i];
			}
		}
		processed[j] = '\0';

		/* Evaluate expression (simple left-to-right) */
		int result = 0;
		int current = 0;
		char op = '+';
		char *p = processed;

		while (*p) {
			if (isdigit(*p) || (*p == '0' && (p[1] == 'x' || p[1] == 'X'))) {
				/* Parse number (hex or decimal) */
				if (*p == '0' && (p[1] == 'x' || p[1] == 'X')) {
					current = strtol(p, &p, 16);
				} else {
					current = strtol(p, &p, 10);
				}

				/* Apply operator */
				if (op == '+')
					result += current;
				else if (op == '-')
					result -= current;
			} else if (*p == '+' || *p == '-') {
				op = *p;
				p++;
			} else if (*p == '(' || *p == ')' || isspace(*p)) {
				p++;
			} else {
				return 0;
			}
		}

		*value = result;
		return 1;
	}

	/* Hexadecimal (0x1234) */
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
		*value = strtoul(str + 2, &endptr, 16);
		return endptr != str + 2;
	}

	/* Hexadecimal (1234h) */
	if (str[strlen(str) - 1] == 'h' || str[strlen(str) - 1] == 'H') {
		*value = strtoul(str, &endptr, 16);
		return endptr != str;
	}

	/* Decimal */
	*value = strtoul(str, &endptr, 10);
	return endptr != str;
}

/* Parse memory operand like [ebx+ecx*4+16] */
static int
parse_memory_operand(const char *str, operand_t *op)
{
	if (str[0] != '[')
		return 0;

	char buf[256];
	strncpy(buf, str + 1, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	/* Find closing bracket */
	char *end = strchr(buf, ']');
	if (!end)
		return 0;
	*end = '\0';

	op->type = OPERAND_MEM;
	op->base = -1;
	op->index = -1;
	op->scale = 1;
	op->disp = 0;

	/* Parse [reg] or [reg+offset] or [reg+reg*scale+offset] */
	char *p = buf;
	while (*p && isspace(*p))
		p++;

	/* Get base register */
	char reg[16];
	int i = 0;
	while (*p && isalpha(*p) && i < 15) {
		reg[i++] = *p++;
	}
	reg[i] = '\0';

	if (i > 0 && is_register(reg)) {
		op->base = get_register_code(reg);
		op->size = get_register_size(reg);
	}

	/* Parse rest: +offset, +reg*scale, etc */
	while (*p) {
		while (*p && isspace(*p))
			p++;

		if (*p == '+' || *p == '-') {
			int sign = (*p == '+') ? 1 : -1;
			p++;
			while (*p && isspace(*p))
				p++;

			/* Check if register or number */
			if (isalpha(*p)) {
				/* Index register */
				i = 0;
				while (*p && isalpha(*p) && i < 15) {
					reg[i++] = *p++;
				}
				reg[i] = '\0';

				if (is_register(reg)) {
					op->index = get_register_code(reg);

					/* Check for scale (*2, *4, *8) */
					while (*p && isspace(*p))
						p++;
					if (*p == '*') {
						p++;
						op->scale = strtol(p, &p, 10);
					}
				}
			} else if (isdigit(*p) || *p == '0') {
				/* Displacement */
				uint32_t val;
				if (parse_number(p, &val)) {
					op->disp += sign * (int32_t)val;
					while (*p && (isdigit(*p) || *p == 'x' || *p == 'X' || 
						      isxdigit(*p)))
						p++;
				}
			}
		} else {
			p++;
		}
	}

	return 1;
}

/* Parse operand (register, immediate, or memory) */
int
parse_operand(const char *str, operand_t *op)
{
	memset(op, 0, sizeof(*op));

	if (!str || !*str) {
		op->type = OPERAND_NONE;
		return 1;
	}

	/* Memory operand [...]  */
	if (str[0] == '[') {
		return parse_memory_operand(str, op);
	}

	/* Register operand */
	if (is_register(str)) {
		op->type = OPERAND_REG;
		op->reg = get_register_code(str);
		op->size = get_register_size(str);
		return 1;
	}

	/* Immediate operand (number) */
	if (parse_number(str, &op->imm)) {
		op->type = OPERAND_IMM;
		return 1;
	}

	/* Label (try to resolve) */
	uint32_t addr;
	if (find_label(str, &addr)) {
		op->type = OPERAND_IMM;
		op->imm = addr;  /* Already absolute */
		return 1;
	}

	/* Forward reference - treat as immediate with value 0 for pass 1 */
	if (asm_ctx.pass == 1) {
		op->type = OPERAND_IMM;
		op->imm = 0;
		return 1;
	}

	/* Pass 2: unresolved reference is an error */
	fprintf(stderr, "error: undefined symbol '%s'\n", str);
	return 0;
}
