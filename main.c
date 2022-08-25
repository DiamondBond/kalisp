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

/* Create enumeration of possible lval types */
enum { LVAL_NUM, LVAL_ERR };

/* Create enumeration of possible error types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Declare new lval struct */
typedef struct {
  int type;
  long num;
  int err;
} lval;

/* Create a new number type lval */
lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

/* Create a new error type lval */
lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

/* Print an 'lval' */
void lval_print(lval v) {
  switch (v.type) {
  /* In the case the type is a number print it */
  /* Then break out of the switch. */
  case LVAL_NUM:
	printf("%li", v.num);
	break;
	/* Error case */
  case LVAL_ERR:
	if (v.err == LERR_DIV_ZERO) {
	  printf("Error: Division by zero.");
	}
	if (v.err == LERR_BAD_OP) {
	  printf("Error: Invalid operator.");
	}
	if (v.err == LERR_BAD_NUM) {
	  printf("Error: Invalid number.");
	}
	break;
  }
}

/* Print an 'lval' followed by a newline */
void lval_println(lval v) {
  lval_print(v);
  putchar('\n');
}

/* Use operator string to see which operation to perform */
lval eval_op(lval x, char *op, lval y) {
  /* If either value is an error return it. */
  if (x.type == LVAL_ERR) {
	return x;
  }
  if (y.type == LVAL_ERR) {
	return y;
  }

  /* Otherwise apply mathematical rules to number values. */
  if (strcmp(op, "+") == 0) {
	return lval_num(x.num + y.num);
  }
  if (strcmp(op, "-") == 0) {
	return lval_num(x.num - y.num);
  }
  if (strcmp(op, "*") == 0) {
	return lval_num(x.num * y.num);
  }
  if (strcmp(op, "/") == 0) {
	/* If second operand is zero return error. */
	return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
  }
  return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t *t) {

  if (strstr(t->tag, "number")) {
	/* Check if there is some error in conversion. */
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  /* The operator is always the second child. */
  char *op = t->children[1]->contents;

  /* We store the third child in `x` */
  lval x = eval(t->children[2]);

  /* Iterate the remaining children and combining. */
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
	x = eval_op(x, op, eval(t->children[i]));
	i++;
  }

  return x;
}

int main(int argc, char **argv) {

  /* Create parsers */
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Operator = mpc_new("operator");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Lisp = mpc_new("lisp");

  /* Define parsers language */
  mpca_lang(MPCA_LANG_DEFAULT,
			"                                                     \
	number   : /-?[0-9]+/ ;                             \
	operator : '+' | '-' | '*' | '/' ;                  \
	expr     : <number> | '(' <operator> <expr>+ ')' ;  \
	lisp    : /^/ <operator> <expr>+ /$/ ;             \
  ",
			Number, Operator, Expr, Lisp);

  /* Print info */
  puts("Diamond's Lisp Version 0.0.5");
  puts("Ctrl-C to Exit\n");

  /* Main loop */
  while (1) {

	char *input = readline(" > ");
	add_history(input);

	/* Attempt to parse user input */
	mpc_result_t r;
	if (mpc_parse("<stdin>", input, Lisp, &r)) {
	  /* DEBUG: Print AST */
	  /* mpc_ast_print(r.output); */
	  /* mpc_ast_delete(r.output); */

	  /* Print out the evaluation results */
	  lval result = eval(r.output);
	  lval_println(result);
	  mpc_ast_delete(r.output);
	} else {
	  mpc_err_print(r.error);
	  mpc_err_delete(r.error);
	}

	free(input);
  }

  /* Undefine and delete our parsers */
  mpc_cleanup(4, Number, Operator, Expr, Lisp);

  return 0;
}
