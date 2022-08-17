#include <stdio.h>
#include <stdlib.h>

/* Micro Parser Combinators */
#include "mpc.h"

/* Redmond specific shit */
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Readline mimicry */
char *readline(char *prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char *cpy = malloc(strlen(buffer) + 1);
  strcpy(cpy, buffer);
  cpy(strlen(cpy) - 1) = '\0';
  return cpy;
}

void add_history(char *unused) {}

/* Otherwise include editline headers */
#else
#include <editline/readline.h>
#endif

int main(int argc, char **argv) {

  /* Create parsers */
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Operator = mpc_new("operator");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Lispy = mpc_new("lispy");

  /* Define parsers language */
  mpca_lang(MPCA_LANG_DEFAULT,
			"                                                     \
	number   : /-?[0-9]+/ ;                             \
	operator : '+' | '-' | '*' | '/' ;                  \
	expr     : <number> | '(' <operator> <expr>+ ')' ;  \
	lispy    : /^/ <operator> <expr>+ /$/ ;             \
  ",
			Number, Operator, Expr, Lispy);

  /* Print info */
  puts("Kalisp Version 0.0.3");
  puts("Ctrl-C to Exit\n");

  /* Main loop */
  while (1) {

	char *input = readline(" > ");
	add_history(input);

	/* Attempt to parse user input */
	mpc_result_t r;
	if (mpc_parse("<stdin>", input, Lispy, &r)) {
	  /* On success print the AST */
	  mpc_ast_print(r.output);
	  mpc_ast_delete(r.output);
	} else {
	  /* Otherwise print error */
	  mpc_err_print(r.error);
	  mpc_err_delete(r.error);
	}

	free(input);
  }

  /* Undefine and delete our parsers */
  mpc_cleanup(4, Number, Operator, Expr, Lispy);

  return 0;
}
