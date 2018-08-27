/* Gregory McHugh
 * PID: GR795710
 * NID: G3437772
 * COP3402-17Spring 0001
 * HW3 Parsercodegen
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_IDENTIFIER_SIZE 11 	/* maximum number of characters for identifiers */
#define MAX_NUMBER_SIZE 5	/* maximum number of digits of number */
#define MAX_TABLE_SIZE 100
#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3

typedef struct {
    int op; // opcode.val
    int l;  // L
    int m;  // M
} instruction;

instruction code[MAX_CODE_LENGTH]; // stores assembly code virtual machine

typedef struct {
	int kind; 		// const = 1, var = 2, procedure = 3 in symbol table
					// internal value (identsym, numsym, etc.)
	char name[12];	// name up to 11 chars
	int val;		// number
	int level;		// lexicographical level
	int addr; 		// address in registers
} symbol;

symbol token_table[MAX_TABLE_SIZE];
symbol symbol_table[MAX_TABLE_SIZE];

int lexemes, symbols;

bool lexerPrint, parserPrint, vmPrint;

/* functions in lexer.c */
void lexer();
int letterfirst(char firstchar, int s);
int numberfirst(char firstchar, int s);
int symbolfirst(char firstchar, int s);
int checkKind(char *word);
void lexerPrinter();
char *sym(int kind);

/* funcitons in parser.c */
void parser();
void program();
void get();
void block(int l, int tx);
void constdeclaration(int l, int *tx, int *dx);
void vardeclaration(int l, int *tx, int *dx);
void statement(int lev, int *ptx);
void condition(int lev, int *ptx);
void expression(int lev, int *ptx);
void term(int lev, int *ptx);
void factor(int lev, int *ptx);
int position(char *id, int *ptx, int l);
void encode(int OP, int L, int M);
void enter(int *ptx, int *pdx, int l, int kind);
void parserPrinter();

/* functions in vm.c */
void vm();
void fetch();
void execute();
void OPR(int M);
void SIO(int M);
char *opName();
void printStack();
int base(int l, int b);
void printVmOutput();

FILE *input, *output, *outputFull;
// char *suffix;
int main(int argc, char *argv[]) {
	/* set to true if given compiler directive -l, -a, or -v, respectively
	bool printLexemes, printAssembly, printStack; */
	lexerPrint = false;
	parserPrint = false;
	vmPrint = false;

	while (argc > 1) {
		if (strcmp(argv[1], "-l") == 0) lexerPrint = true;
		if (strcmp(argv[1], "-a") == 0) parserPrint = true;
		if (strcmp(argv[1], "-v") == 0) vmPrint = true;

		argc--;
		argv++;
	}

  /*
    char inputStr[30];
    strcpy(inputStr, "error-input");
    strcat(inputStr, suffix);
  input = fopen(inputStr, "r");
  */

  /*
    int errorsuffix;
    scanf("%d", &errorsuffix);
    sprintf(suffix, "%d", errorsuffix);
    char outputStr[30];
    strcpy(outputStr, "error-output");
    strcat(outputStr, suffix);
    outputFull = fopen(outputStr, "w");
  */

        input = fopen("input.txt", "r");
  outputFull = fopen("output.txt", "w");

	lexer();

	parser();

	vm();

  fclose(outputFull);

	return 0;
}

int kind; // saves if const, var, or procedure was most recent
char word[12];

void lexer() {
	output = fopen("lexerOutput.txt", "w");

	char ch; // current character from input
	lexemes = 0; // position in token_table

	// get first character in input file
	ch = fgetc(input);

	// read first character of each word
	// goes through DFA (similar to one from Lecture 05) based on first character
	// creates a node for each word, number, or symbol (except for comments)
	// prints source code to output.txt (except for comments)
	while(!feof(input)) {
		// if first character is a letter
		if (isalpha(ch)) lexemes = letterfirst(ch, lexemes);
		// if first character is a number
		else if (isdigit(ch)) lexemes = numberfirst(ch, lexemes);
		// if first character is a symbol
		else if (ispunct(ch)) lexemes = symbolfirst(ch, lexemes);
		// otherwise it's whitespace
		else; // nothing happens because whitespace isn't a token

		ch = fgetc(input);
	}

	lexerPrinter(output);

  fclose(input);
  fclose(output);
}

// ident ::= letter {letter | digit}.
int letterfirst(char firstchar, int lexemes) {
	int nextchar; // next letter
	int i = 1; // letter position

	// allocates memory for word
	calloc(MAX_IDENTIFIER_SIZE + 1, sizeof(char));

	// put first character in string for word
	word[0] = firstchar;

	// get next character
	nextchar = fgetc(input);

	// keep building word string so long as next charcter is letter or number
	while(isalpha(nextchar) || isdigit(nextchar)) {
		// if word is too long then print error message and exit program
		if (i == MAX_IDENTIFIER_SIZE) {
			printf("ERROR: identifier can't be longer than 11 characters\n");
			fprintf(output, "ERROR: identifier can't be longer than 11 characters\n");
      fprintf(outputFull, "ERROR: identifier can't be longer than 11 characters\n");
			exit(1); // exits program
		}

		word[i] = nextchar;
		i++;
		nextchar = fgetc(input);
	}

	word[i] = '\0';

	token_table[lexemes].kind = checkKind(word);

	// save word in token table
	strcpy(token_table[lexemes].name,  word);
	lexemes++;

	// sets position input file stream so the character can be scanned again in main
	if (nextchar != EOF) fseek(input, -1, SEEK_CUR);

	// return new position in table
	return lexemes;
}
//  number ::= digit {digit}.
int numberfirst(char firstchar, int lexemes) {
	int i = 1;
	int nextchar;

  // allocates memory for word
	calloc(MAX_NUMBER_SIZE + 1, sizeof(char));


	// put first character in string for word
	strcpy(word, "");
	word[0] = firstchar;

  // get next character
	nextchar = fgetc(input);

	// put in string if next character is a number
	while(isdigit(nextchar)) {
    // if number is too long then print error message and exit program
		if (i == MAX_NUMBER_SIZE) {
			printf("ERROR: number can't be longer than 5 digits\n");
			fprintf(output, "ERROR: number can't be longer than 5 digits\n");
      fprintf(outputFull, "ERROR: number can't be longer than 5 digits\n");
			exit(25); // exits program
		}

		word[i] = nextchar;
		i++;
		nextchar = fgetc(input);
	}

  /* printf("%d", value); */ // for debugging

	// if next character is letter then print error message and exit program
	if (isalpha(nextchar)) {
		printf("ERROR: identifier must start with a letter\n");
		fprintf(output, "ERROR: identifier must start with a letter\n");
    fprintf(outputFull, "ERROR: identifier must start with a letter\n");
		exit(1); // exits program
	}

	word[i] = '\0';

	token_table[lexemes].kind = 3;

	// save number in token table
	strcpy(token_table[lexemes].name,  word);
  token_table[lexemes].val = atoi(word);

	lexemes++;

	// sets position input file stream so the character can be scanned again in main
	if (nextchar != EOF) fseek(input, -1, SEEK_CUR);

	return lexemes;
}

int symbolfirst(char firstchar, int lexemes) {
	char nextchar; // stores next symbol

	// allocates memory for word
	calloc(3, sizeof(char));

	// put first character in string for symbol
	strcpy(word, "");
	word[0] = firstchar;

	// check first character and check next character
	switch (firstchar) {
		// "/" or "/*"
		case '/':
			nextchar = fgetc(input); // gets next symbol

			// if /*, then continue scanning without saving characters until */ is reached
			if (nextchar == '*') {
				// continues retrieving characters without saving them until */ is reached
				// only checks for / if previous character is *
				nextchar = fgetc(input);
				do {
					// continue searching until * found
					// prints error message if / isn't found
					while (nextchar != '*') {
						// if no closing comment is found return error message and exit program
						if (nextchar == EOF) {
							printf("ERROR: comments must end with */\n");
							fprintf(output, "ERROR: comments must end with */\n");
              fprintf(outputFull, "ERROR: comments must end with */\n");
							exit(1); // exits program
						}
						nextchar = fgetc(input);
					}

					nextchar = fgetc(input);
				} while(nextchar != '/');

				// returns original lexemes becuase comments weren't saved into linked list
				return lexemes;
			}
			// if next character isn't '*' then respond as just a slash
			else {
				// sets position input file stream so the character can be scanned again in main
				// and null terminate symbol string
				if (nextchar != EOF) fseek(input, -1, SEEK_CUR);
				word[1] = '\0'; // null terminates symbol string
				token_table[lexemes].kind = 7;
			}

			break;
		// "<", "<=", or "<>"
		case '<' :
			nextchar = fgetc(input); // gets next symbol

			if (nextchar == '=') {
				token_table[lexemes].kind = 12;
				word[1] = nextchar;
				word[2] = '\0'; // null terminates symbol string
			}
			else if(nextchar == '>') {
				token_table[lexemes].kind = 10;
				word[1] = nextchar;
				word[2] = '\0'; // null terminates symbol string
			}
			// otherwise set position input file stream so the character can be scanned again in main
			else {
				token_table[lexemes].kind = 11;
				// sets position input file stream so the character can be scanned again in main
				// and null terminates symbol string
				if (nextchar != EOF) fseek(input, -1, SEEK_CUR);
				word[1] = '\0'; // null terminates symbol string
			}

			break;
		// ">" or ">="
		case '>' :
			nextchar = fgetc(input); // gets next symbol

			// if next character is '=' then add to symbol string
			if (nextchar == '=') {
				token_table[lexemes].kind = 14;
				word[1] = nextchar;
				word[2] = '\0'; // null terminates symbol string
			}
			// otherwise set position input file stream so the character can be scanned again in main
			else {
				token_table[lexemes].kind = 13;
				// sets position input file stream so the character can be scanned again in main
				// and null terminates symbol string
				if (nextchar != EOF) fseek(input, -1, SEEK_CUR);
				word[1] = '\0'; // null terminates symbol string
			}

			break;
		case ':' :	  // can only be :=
			nextchar = fgetc(input); // gets next symbol

			// if ":=" then save symbol
			if (nextchar == '=') {
				token_table[lexemes].kind = 20;
				word[1] = nextchar;
				word[2] = '\0'; // null terminates symbol string
			}
			// if not ":=" then print error message and exit program
			else {
				printf("ERROR: invalid symbol\n");
				fprintf(output, "ERROR: invalid symbol\n");
        fprintf(outputFull, "ERROR: invalid symbol\n");
				exit(1); // exits program
			}

			break;
		case '+' :
			token_table[lexemes].kind = 4;
			word[1] = '\0'; // null terminates symbol string
			break;
		case '-' :
			token_table[lexemes].kind = 5;
			word[1] = '\0'; // null terminates symbol string
			break;
		case '*' :
			token_table[lexemes].kind = 6;
			word[1] = '\0'; // null terminates symbol string
			break;
		case '(' :
			token_table[lexemes].kind = 15;
			word[1] = '\0'; // null terminates symbol string
			break;
		case ')' :
			token_table[lexemes].kind = 16;
			word[1] = '\0'; // null terminates symbol string
			break;
		case '=' :
			token_table[lexemes].kind = 9;
			word[1] = '\0'; // null terminates symbol string
			break;
		case ',' :
			token_table[lexemes].kind = 17;
			word[1] = '\0'; // null terminates symbol string
			break;
		case '.' :
			token_table[lexemes].kind = 19;
			word[1] = '\0'; // null terminates symbol string
			break;
		case ';' :
			token_table[lexemes].kind = 18;
			word[1] = '\0'; // null terminates symbol string
			break;
		// it's none of the approved symbols
		// print error message and exit program.
		default :
			printf("ERROR: invalid symbol\n");
			fprintf(output, "ERROR: invalid symbol\n");
      fprintf(outputFull, "ERROR: invalid symbol\n");
			exit(1); // exits program
	}

	// save symbol in token table
	strcpy(token_table[lexemes].name,  word);
	lexemes++;

	return lexemes;
}

int checkKind(char *word) {
	if (!strcmp(word, "odd")) {
		return 8;
	}
	else if (!strcmp(word, "begin")) {
		return 21;
	}
	else if (!strcmp(word, "end")) {
		return 22;
	}
	else if (!strcmp(word, "if")) {
		return 23;
	}
	else if (!strcmp(word, "then")) {
		return 24;
	}
	else if (!strcmp(word, "while")) {
		return 25;
	}
	else if (!strcmp(word, "do")) {
		return 26;
	}
/*	else if (!strcmp(word, "call")) {
		return 27;
	}  */
	else if (!strcmp(word, "const")) {
		kind = 1;
		return 28;
	}
	else if (!strcmp(word, "var")) {
		kind = 2;
		return 29;
	}
/*	else if (!strcmp(word, "procedure")) {
		kind = 3;
		return 30;
	}  */
	else if (!strcmp(word, "write")) {
		return 31;
	}
	else if (!strcmp(word, "read")) {
		return 32;
	}
/*	else if (!strcmp(word, "else")) {
		return 33;
	}  */
	// otherwise, the token is an identifier
	else {
		return 2;
	}
}

void lexerPrinter() {
	int i;

  fprintf(output, "Symbolic Lexeme List\n");
  fprintf(outputFull, "Symbolic Lexeme List\n");

	for(i = 0; i < lexemes; ++i) {
		fprintf(output, "%s ", sym(token_table[i].kind));
    fprintf(outputFull, "%s ", sym(token_table[i].kind));
		if (token_table[i].kind == 2 || token_table[i].kind == 3){
      fprintf(output, "%s ", token_table[i].name);
      fprintf(outputFull, "%s ", token_table[i].name);
    }
	}

  fprintf(output, "\n\n");
  fprintf(outputFull, "\n\n");

  fprintf(output, "Lexeme List\n");
  fprintf(outputFull, "Lexeme List\n");

  for(i = 0; i < lexemes; ++i) {
    fprintf(output, "%d ", token_table[i].kind);
    fprintf(outputFull, "%d ", token_table[i].kind);
    if (token_table[i].kind == 2 || token_table[i].kind == 3) {
        fprintf(output, "%s ", token_table[i].name);
        fprintf(outputFull, "%s ", token_table[i].name);
    }
  }

	fprintf(output, "\n");
  fprintf(outputFull, "\n");


	if (lexerPrint) {
    printf("Symbolic Lexeme List\n");

		for(i = 0; i < lexemes; ++i) {
			printf("%s ", sym(token_table[i].kind));
			if (token_table[i].kind == 2 || token_table[i].kind == 3) printf("%s ", token_table[i].name);
		}

		printf("\n\n");

    printf("Lexeme List\n");

		for(i = 0; i < lexemes; ++i) {
			printf("%d ", token_table[i].kind);
			if (token_table[i].kind == 2 || token_table[i].kind == 3) printf("%s ", token_table[i].name);
		}

		printf("\n");
	}
}

char *sym(int kind) {
	switch (kind) {
		case 1:
			return "nulsym";
		case 2:
			return "identsym";
		case 3:
			return "numbersym";
		case 4:
			return "plussym";
		case 5:
			return "minussym";
		case 6:
			return "multsym";
		case 7:
			return "slashsym";
		case 8:
			return "oddsym";
		case 9:
			return "eqlsym";
		case 10:
			return "neqsym";
		case 11:
			return "lessym";
		case 12:
			return "leqsym";
		case 13:
			return "gtrsym";
		case 14:
			return "geqsym";
		case 15:
			return "lparentsym";
		case 16:
			return "rparentsym";
		case 17:
			return "commasym";
		case 18:
			return "semicolonsym";
		case 19:
			return "periodsym";
		case 20:
			return "becomesym";
		case 21:
			return "beginsym";
		case 22:
			return "endsym";
		case 23:
			return "ifsym";
		case 24:
			return "thensym";
		case 25:
			return "whilesym";
		case 26:
			return "dosym";
		case 27:
			return "callsym";
		case 28:
			return "constsym";
		case 29:
			return "varsym";
		case 30:
			return "procsym";
		case 31:
			return "writesym";
		case 32:
			return "readsym";
		case 33:
			return "elsesym";
		default:
			printf("\nERROR: invalid symbol\n");
			fprintf(output, "\nERROR: invalid symbol\n");
      fprintf(outputFull, "\nERROR: invalid symbol\n");
			exit(1); // Exit with a non-normal return value.
	}
}

int t = 0; // index in token table
int lparents; // left parenthesis without right parentheses seen

int cx, token, num, kind, diff, prevDiff = 0;
char id[12];

void parser() {
    output = fopen("parserOutput.txt", "w");

    program();

    parserPrinter();

    fclose(output);
}

void parserPrinter() {
  fprintf(output, "No errors, program is syntactically correct.\n\n");
  fprintf(outputFull, "No errors, program is syntactically correct.\n\n");


  fprintf(output, "Assembly Code\n");
  fprintf(outputFull, "Assembly Code\n");
  fprintf(output, "Line\tOP\tL\tM\n");
  fprintf(outputFull, "Line\tOP\tL\tM\n");

  int i = 0;
  while (code[i].op) {
    fprintf(output, "%d\t%s\t%d\t%d\n", i, opName(code[i].op), code[i].l, code[i].m);
    fprintf(outputFull, "%d\t%s\t%d\t%d\n", i, opName(code[i].op), code[i].l, code[i].m);
    i++;
  }



  if (parserPrint) {
    if (lexerPrint) printf("\n");

    printf("No errors, program is syntactically correct.\n\n");


    printf("Assembly Code\n");

    i = 0;
    while (code[i].op) {
      printf("%d\t%s\t%d\t%d\n", i, opName(code[i].op), code[i].l, code[i].m);
      i++;
    }

    printf("\n");
  }
}
/*
  program ::= block "." .
*/
void program() {
    get();
    block(0, 0);

    if (token != 19) { //  .
      printf("\nERROR: period expected\n");
      fprintf(output, "\nERROR: period expected\n");
      fprintf(outputFull, "\nERROR: period expected\n");
      exit(1); // Exit with a non-normal return value.
    }

    encode(9, 0, 3);
}

/*
  block ::= const-declaration  var-declaration  statement.
*/
void block(int l, int tx) {
    int dx = 4, tx0 = tx, cx0;
    symbol_table[tx0].addr = cx;
    encode(7, 0, 0); // JMP 0, 0

    do {
        if (token == 28) {  // const
            get();
            do {
                constdeclaration(l, &tx, &dx);
                while (token == 17) {  //  ,
                    get();
                    constdeclaration(l, &tx, &dx);
                }
                if(token == 18) { //  ;
                    get();
                }
                else {
                  printf("\nERROR: semicolon or comma expected\n");
                  fprintf(output, "\nERROR: semicolon or comma expected\n");
                  fprintf(outputFull, "\nERROR: semicolon or comma expected\n");
                  exit(1); // Exit with a non-normal return value.
                }
            } while (token == 2); // identifier
        }
        if (token == 29) {  // var
            get();
            do {
                vardeclaration(l, &tx, &dx);
                while (token == 17) { //  ,
                    get();
                    vardeclaration(l, &tx, &dx);
                }
                if(token == 18) { //  ;
                    get();
                }
                else {
                  printf("\nERROR: semicolon or comma expected\n");
                  fprintf(output, "\nERROR: semicolon or comma expected\n");
                  fprintf(outputFull, "\nERROR: semicolon or comma expected\n");
                  exit(1); // Exit with a non-normal return value.
                }
            } while (token == 2);  //  identifier
        }

        while (token == 30) {  //  procedure
            get();

            if(token == 2) { // identifier
                enter(&tx, &dx, l, 3); // procedure
                get();
            }
            else {
              printf("\nERROR: procedure must be followed by identifier\n");
              fprintf(output, "\nERROR: procedure must be followed by identifier\n");
              fprintf(outputFull, "\nERROR: procedure must be followed by identifier\n");
              exit(1); // Exit with a non-normal return value.
            }
            if(token == 18) { // ;
                get();
            }
            else {
              printf("\nERROR: semicolon or comma expected\n");
              fprintf(output, "\nERROR: semicolon or comma expected\n");
              fprintf(outputFull, "\nERROR: semicolon or comma expected\n");
              exit(1); // Exit with a non-normal return value.
            }

            block(l + 1, tx);
            if(token == 18) { // ;
                get();
            }
            else {
              printf("\nERROR: semicolon or comma expected\n");
              fprintf(output, "\nERROR: semicolon or comma expected\n");
              fprintf(outputFull, "\nERROR: semicolon or comma expected\n");
              exit(1); // Exit with a non-normal return value.
            }
        }
    } while (token == 28 || token == 29 || token == 30);
    // const, var, procedure

    code[symbol_table[tx0].addr].m = cx;
    symbol_table[tx0].addr = cx;
    cx0 = cx;
    encode(6, 0, dx); // INC 0 M
    statement(l, &tx);
    encode(2, 0, 0); // OPR 0 0
}

/*
  constdeclaration ::= [ “const” ident "=" number {"," ident "=" number} “;"].
*/
void constdeclaration(int l, int *ptx, int *pdx) {
    if (token == 2) { // identifier
        get();
        if (token != 9) { //  =
          if (token == 20)  {  //  :=
            printf("\nERROR: use = instead of := when declaring a const\n");
            fprintf(output, "\nERROR: use = instead of := when declaring a const\n");
            fprintf(outputFull, "\nERROR: use = instead of := when declaring a const\n");
            exit(1); // Exit with a non-normal return value.
          }
          else {
            printf("\nERROR: = must follow identifier for const declarations\n");
            fprintf(output, "\nERROR: = must follow identifier for const declarations\n");
            fprintf(outputFull, "\nERROR: = must follow identifier for const declarations\n");
            exit(1); // Exit with a non-normal return value.
          }
        }

        get();
        if (token == 3) { // number
            enter(ptx, pdx, l, 1);  //const
            get();
        }
        else {
          printf("\nERROR: = must be followed by a number for const declarations\n");
          fprintf(output, "\nERROR: = must be followed by a number for const declarations\n");
          fprintf(outputFull, "\nERROR: = must be followed by a number for const declarations\n");
          exit(1); // Exit with a non-normal return value.
        }
    }
    else {
      printf("\nERROR: const must be followed by an identifier\n");
      fprintf(output, "\nERROR: const must be followed by an identifier\n");
      fprintf(outputFull, "\nERROR: const must be followed by an identifier\n");
      exit(1); // Exit with a non-normal return value.
    }
}

/*
  var-declaration  ::= [ "var" ident {"," ident} “;"].
*/
void vardeclaration(int l, int *ptx, int *pdx) {

    if (token == 2) { // identifier
        enter(ptx, pdx, l, 2); //var
        get();
    }
    else {
      printf("\nERROR: var must be followed by identifier\n");
      fprintf(output, "\nERROR: var must be followed by identifier\n");
      fprintf(outputFull, "\nERROR: var must be followed by identifier\n");
      exit(1); // Exit with a non-normal return value.
    }
}

/*
  statement   ::= [ ident ":=" expression
          | "begin" statement { ";" statement } "end"
          | "if" condition "then" statement
          | "while" condition "do" statement
          | "read" ident
          | "write"  ident
          | e ] .
*/
void statement(int l, int *ptx) {

    int i, cx1, cx2;

  switch(token) {
    case 2:  //  identifier
        i = position(id, ptx, l);
        if (i == 0) {
          // ident not in symbol_table
          printf("\nERROR: undeclared identifier\n");
          fprintf(output, "\nERROR: undeclared identifier\n");
          fprintf(outputFull, "\nERROR: undeclared identifier\n");
          exit(1); // Exit with a non-normal return value.
        }
        else if (symbol_table[i].kind != 2) { // var
            printf("\nERROR: identifier is not a variable\n");
            fprintf(output, "\nERROR: identifier is not a variable\n");
            fprintf(outputFull, "\nERROR: identifier is not a variable\n");
            exit(1); // Exit with a non-normal return value.
        }

        get();
        if (token == 20) {  //  :=
            get();
        }
        else {
          printf("\nERROR: assignment operator expected\n");
          fprintf(output, "\nERROR: assignment operator expected\n");
          fprintf(outputFull, "\nERROR: assignment operator expected\n");
          exit(1); // Exit with a non-normal return value.
        }
        expression(l, ptx);
        if (i != 0) {
            encode(4, l - symbol_table[i].level, symbol_table[i].addr); // STO L, M
        }
        break;
    case 27:  // call
        get();
        if (token != 2) { // identifier
          printf("\nERROR: var must be followed by identifier\n");
          fprintf(output, "\nERROR: var must be followed by identifier\n");
          fprintf(outputFull, "\nERROR: var must be followed by identifier\n");
          exit(1); // Exit with a non-normal return value.
        }
        else {
            i = position(id, ptx, l);
            if(i == 0) {
              printf("\nERROR: undeclared identifier\n");
              fprintf(output, "\nERROR: undeclared identifier\n");
              fprintf(outputFull, "\nERROR: undeclared identifier\n");
              exit(1); // Exit with a non-normal return value.
            }
            else if (symbol_table[i].kind == 3) {  // procedure
              encode(5, l - symbol_table[i].level, symbol_table[i].addr);// CAL L, M
            }
            else {
              printf("\nERROR: calling a contant or variable is pointless\n");
      				fprintf(output, "\nERROR: calling a contant or variable is pointless\n");
      				fprintf(outputFull, "\nERROR: calling a contant or variable is pointless\n");
      				exit(1); // Exit with a non-normal return value.
            }
            get();
        }
        break;

    case 23:  //  if
        get();
        condition(l, ptx);
        if(token == 24) { //  then
            get();
        }
        else {
          printf("\nERROR: then expected\n");
          fprintf(output, "\nERROR: then expected\n");
          fprintf(outputFull, "\nERROR: then expected\n");
          exit(1); // Exit with a non-normal return value.
        }

        cx1 = cx;
        encode(8, 0, 0); // JPC 0, 0
        statement(l, ptx);


        if(token == 33) { // else
            get();

            code[cx1].m = cx + 1;
            cx1 = cx;
            encode(7, 0, 0); // JMP 0, 0
            statement(l, ptx);
        }
        code[cx1].m = cx;
        break;

    case 21:  //  begin
        get();
        statement(l, ptx);

         while (token == 18) { // ;
            get();
            statement(l, ptx);
         }

        if (token == 22) {  //  end
            get();
        }
        else {
          printf("\nERROR: semicolon or end expected\n");
          fprintf(output, "\nERROR: semicolon or end expected\n");
          fprintf(outputFull, "\nERROR: semicolon or end expected\n");
          exit(1); // Exit with a non-normal return value.
        }
        break;

    case 25:  //  while
        cx1 = cx;
        get();
        condition(l, ptx);
        cx2 = cx;
        encode(8, 0, 0); // JPC 0, 0
        if(token == 26) { //  do
            get();
        }
        else {
          printf("\nERROR: do expected\n");
          fprintf(output, "\nERROR: do expected\n");
          fprintf(outputFull, "\nERROR: do expected\n");
          exit(1); // Exit with a non-normal return value.
        }
        statement(l, ptx);
        encode(7, 0, cx1); // JMP 0, M
        code[cx2].m = cx;
        break;

    case 31: //  write
        get();
        if (token != 2) {
          printf("\nERROR: write must be followed by identifier\n");
          fprintf(output, "\nERROR: write must be followed by identifier\n");
          fprintf(outputFull, "\nERROR: write must be followed by identifier\n");
          exit(1); // Exit with a non-normal return value.
        }
        expression(l, ptx);
        encode(9, 0, 1); // SIO1
        break;

    case 32:   //  read
        get();

        if (token != 2) {
          printf("\nERROR: read must be followed by identifier\n");
          fprintf(output, "\nERROR: read must be followed by identifier\n");
          fprintf(outputFull, "\nERROR: read must be followed by identifier\n");
          exit(1); // Exit with a non-normal return value.
        }

        encode(9, 0, 2); // SIO2

        i = position(id, ptx, l);
        if(i == 0) {
          printf("\nERROR: undeclared identifier\n");
          fprintf(output, "\nERROR: undeclared identifier\n");
          fprintf(outputFull, "\nERROR: undeclared identifier\n");
          exit(1); // Exit with a non-normal return value.
        }
        else if (symbol_table[i].kind != 2) { //var
          printf("\nERROR: read must be followed by a variable\n");
          fprintf(output, "\nERROR: read must be followed by a variable\n");
          fprintf(outputFull, "\nERROR: read must be followed by a variable\n");
          exit(1); // Exit with a non-normal return value.
          i = 0;
        }
        if (i != 0) {
            encode(4, l - symbol_table[i].level, symbol_table[i].addr); // STO L, M
        }
        get();
        break;
  }
}

/*
  condition ::= "odd" expression
                | expression  rel-op  expression.
*/
void condition(int l, int *ptx) {
  int rel;

  if (token == 8) { // ODD
    get();
    expression(l, ptx);
    encode(2, 0, 6); // ODD
  }
  else {  //  expression rel-op expression
        expression(l,ptx);
        if ( (token != 9 )  //  EQL
          && (token != 10)  //  NEQ
          && (token != 11)  //  LSS
          && (token != 12)  //  LEQ
          && (token != 13)  //  GTR
          && (token != 13)) //  GEQ
        {
            printf("\nERROR: relation expected");
    				fprintf(output, "\nERROR: relation expected");
    				fprintf(outputFull, "\nERROR: relation expected");
    				exit(1); // Exit with a non-normal return value.
        }
        else {
            rel = token;
            get();
            expression(l, ptx);
            //  rel-op ::= "="|“<>"|"<"|"<="|">"|">=“.
            switch(rel) {
                case 9:
                    encode(2, 0, 8);  // EQL
                    break;
                case 10:
                    encode(2, 0, 9);  // NEQ
                    break;
                case 11:
                    encode(2, 0, 10); // LSS
                    break;
                case 12:
                    encode(2, 0, 11); // LEQ
                    break;
                case 13:
                    encode(2, 0, 12); // GTR
                    break;
                case 14:
                    encode(2, 0, 13); // GEQ
                    break;
            }
        }
    }
}

/*
  expression ::= [ "+"|"-"] term { ("+"|"-") term}.
*/
void expression(int l, int *ptx) {
  if (token == 4) { //  +
        get();
        term(l, ptx);
  }
  else if (token == 5) {  //  -
    get();
    term(l, ptx);
    encode(2, 0, 1);  // NEG
    }
    else {
        term(l, ptx);
    }

    while (token == 4 || token == 5) {  //  + or -
      if (token == 4) {  //  +
        get();
        term(l, ptx);
        encode(2, 0, 2);  // ADD
      }
      else {  //  -
        get();
        term(l, ptx);
        encode(2, 0, 3);  // SUB
      }
    }
}

/*
  term ::= factor {("*"|"/") factor}.
*/
void term(int l, int *ptx) {
  factor(l, ptx);
  while (token == 6|| token == 7) {
    if(token == 6) {  //  *
      get();
      factor(l, ptx);
      encode(2, 0, 4);  // MUL
    }
    else {  //  /
      get();
      factor(l, ptx);
      encode(2, 0, 5);  // DIV
    }
  }
}

/*
  factor ::= ident | number | "(" expression ")“.
*/
void factor(int level, int *ptx) {
    while ((token == 2)||(token == 3)||(token == 15)) {
        if (token == 2) { // identifier
            int i = position(id, ptx, level);
            if (i == 0) {
              printf("\nERROR: undeclared identifier\n");
              fprintf(output, "\nERROR: undeclared identifier\n");
              fprintf(outputFull, "\nERROR: undeclared identifier\n");
              exit(1); // Exit with a non-normal return value.
            }
            else {
                if (symbol_table[i].kind == 3) {  //  procedure
                  printf("\nERROR: identifier must be a constant or a variable\n");
          				fprintf(output, "\nERROR: identifier must be a constant or a variable\n");
          				fprintf(outputFull, "\nERROR: identifier must be a constant or a variable\n");
          				exit(1); // Exit with a non-normal return value.
                }
                else if (symbol_table[i].kind == 1) {  //  const
                    encode(1, 0, symbol_table[i].val);  // LIT 0, M
                }
                else if (kind == 2) { //  var
                    encode(3, level - symbol_table[i].level, symbol_table[i].addr); //  LOD L, M
                }
            }

            get();
        }
        else if (token == 3) {  //  number
          encode(1, 0, num);  //  LIT 0, M
          get();
        }
        else if(token == 15) {  //  (
            get();
            expression(level, ptx);
            if (token == 16) get(); //  )
            else {
              printf("\nERROR: right parenthesis missing\n");
              fprintf(output, "\nERROR: right parenthesis missing\n");
              fprintf(outputFull, "\nERROR: right parenthesis missing\n");
              exit(1); // Exit with a non-normal return value.
            }
        }
    }
}

void encode(int op, int l, int m) {
  if (cx > MAX_CODE_LENGTH) {
    printf("ERROR: code too long\n");
    fprintf(output, "ERROR: code too long\n");
    fprintf(outputFull, "ERROR: code too long\n");
    exit(1); //  exits with non-normal return value
  }

  code[cx].op = op;
  code[cx].l = l;
  code[cx].m = m;

  /* printf("%d\t%d\t%d\n", op, l, m);  // for debugging */

  cx++;
}

void enter(int *ptx, int *pdx, int level, int kind) {
  char *name;  // saves copy of id
  name = id;
  int i;
  (*ptx)++;
    for (i = 0; i <= strlen(id); ++i) {
        symbol_table[*ptx].name[i] = *name;
        name++;
    }

    symbol_table[*ptx].kind = kind;
    if (kind == 1) { //  const
        symbol_table[*ptx].val = num;
    }
    else if (kind == 2) { //  var
        symbol_table[*ptx].level = level;
        symbol_table[*ptx].addr = *pdx;
        (*pdx)++;
    }
    else if (kind == 3){  //  procedure
        symbol_table[*ptx].level = level;
    }
}

void get() {
  token = token_table[t].kind;

  /* printf("\tget %s\n", sym(token_table[t].kind)); // used for debugging */

  if (token == 2) { //  identifier
    strcpy(id, token_table[t].name);
  }
  else if (token == 3) {  //  number
    num = token_table[t].val;
  }

  t++;

  // if token is lparentsym then increase counter of unpaired lparentsyms
  if (token == 15) ++lparents;	//	(
  // if token is rparentsym then decrease counter of unpaired lparentsyms
  else if (token == 16) --lparents; // )

  // if lparensym counter is negative then an lparentsym is missing
  if (lparents < 0) {
    printf("\nERROR: left parenthesis missing\n");
    fprintf(output, "\nERROR: left parenthesis missing\n");
    fprintf(outputFull, "\nERROR: left parenthesis missing\n");
    exit(1); // exit with non-normal return value
  }


}

int position(char *id, int *ptx, int level) {
   /* printf("position(%s)\n", sym(token));  //  for debugging */
   /* printf("%s\n", id);  //  for debugging */

  int s = *ptx, S, diffCount = 0;

  while (s != 0) {
    if (strcmp(symbol_table[s].name, id) == 0) {
      if (symbol_table[s].level <= level) {
        if (diffCount != 0) prevDiff = diff;

        diff = level - symbol_table[s].level;

        if(diffCount == 0 || diff < prevDiff) S = s;

        diffCount++;
      }
    }

    s--;
  }

  /* printf("%d\n", S); //  for debugging */

  return S;
}

int stack[MAX_STACK_HEIGHT]; // holds all entries in stack
int bp, sp, pc; // base pointer, stack pointer, counter
instruction ir; // instruction reference
bool halt; // halt condition
instruction code[MAX_CODE_LENGTH]; // all instructions
int lines; // lines
int line;

void vm() {
  output = fopen("vmOutput.txt", "w");

	bp = 1;
	sp = 0;
	pc = 0;

	ir.op = 0;
	ir.l = 0;
	ir.m = 0;

	halt = false;

	int i;
	for (i = 0; i < MAX_STACK_HEIGHT; ++i) stack[i] = 0; // initializes all entries as 0

	// prints heading of stack trace
  fprintf(output, "\t\t\t\t\tpc\tbp\tsp\tstack\n");
	fprintf(output, "Initial\tvalues\t\t\t\t%d\t%d\t%d\n", pc, bp, sp);

  fprintf(outputFull, "\t\t\t\t\tpc\tbp\tsp\tstack\n");
	fprintf(outputFull, "Initial\tvalues\t\t\t\t%d\t%d\t%d\n", pc, bp, sp);

	while (!halt && bp) {
		line = pc;

		fetch(); // fetches instruction

		// prints instruction
		fprintf(output, "%d\t%s\t%d\t%d\t\t", line, opName(ir.op), ir.l, ir.m);
    fprintf(outputFull, "%d\t%s\t%d\t%d\t\t", line, opName(ir.op), ir.l, ir.m);

		pc++;

		execute(); // executes instruction

		// prints PC, BP, and SP
		fprintf(output, "%d\t%d\t%d\t", pc, bp, sp);
    fprintf(outputFull, "%d\t%d\t%d\t", pc, bp, sp);

		printStack(sp, bp);

		fprintf(output, "\n");
    fprintf(outputFull, "\n");
	}

  fprintf(output, "%d\tsio\t0\t3\t\t0\t0\t0\n", line + 1);
  fprintf(outputFull, "%d\tsio\t0\t3\t\t0\t0\t0\n", line + 1);

  fclose(output);



  if (vmPrint) {
    input = fopen("vmOutput.txt", "r");
    printVmOutput();
    fclose(input);
  }

}

/* fetches instruction */
void fetch() {
	// instruction reference stores op, l, and m
	ir.op = code[pc].op;
	ir.l = code[pc].l;
	ir.m = code[pc].m;
}

/* executes instruction */
void execute() {
	switch(ir.op) {
		case 1: // LIT
			sp++;
			stack[sp] = ir.m;
			break;
		case 2: // OPR
			OPR(ir.m);
			break;
		case 3: // LOD
			sp++;
			stack[sp] = stack[base(ir.l, bp) + ir.m];
			break;
		case 4: //STO
			stack[base(ir.l, bp) + ir.m] = stack[sp];
			sp--;
			break;
		case 5: // CAL
			stack[sp + 1] = 0;
			stack[sp + 2] = base(ir.l, bp);
			stack[sp + 3] = bp;
			stack[sp + 4] = pc;
			bp = sp + 1;
			pc = ir.m;
			break;
		case 6: // INC
			sp += ir.m;
			break;
		case 7: // JMP
			pc = ir.m;
			break;
		case 8: // JPC
			if (stack[sp] == 0 ) pc = ir.m;
			sp--;
			break;
		case 9: // SIO
			SIO(ir.m);
			break;
		default:
			printf("\nERROR: %d is not a valid op code\n", ir.op);
			fprintf(output, "\nERROR: %d is not a valid op code\n", ir.op);
			fprintf(outputFull, "\nERROR: %d is not a valid op code\n", ir.op);
			exit(1); // exits with non-normal return value
	}
}

// for OPR instruction
// performs operation corresponding to M
void OPR(int M) {
	switch(M) {
		case 0: // RET
			sp = bp - 1;
			bp = stack[sp + 3];
			pc = stack[sp + 4];
			break;
		case 1: // NEG
			stack[sp] = -stack[sp];
			break;
		case 2: // ADD
			sp--;
			stack[sp] = stack[sp] + stack[sp + 1];
			break;
		case 3: // SUB
			sp--;
			stack[sp] = stack[sp] - stack[sp + 1];
			break;
		case 4: // MUL
			sp--;
			stack[sp] = stack[sp] * stack[sp + 1];
			break;
		case 5: // DIV
			sp--;
			stack[sp] = stack[sp] / stack[sp + 1];
			break;
		case 6: // ODD
			stack[sp] = stack[sp] % 2;
			break;
		case 7: // MOD
			sp--;
			stack[sp] = stack[sp] % stack[sp + 1];
			break;
		case 8: // EQL
			sp--;
			stack[sp] = stack[sp] == stack[sp + 1];
			break;
		case 9: // NEQ
			sp--;
			stack[sp] = stack[sp] != stack[sp + 1];
			break;
		case 10: // LSS
			sp--;
			stack[sp] = stack[sp] < stack[sp + 1];
			break;
		case 11: // LEQ
			sp--;
			stack[sp] = stack[sp] <= stack[sp + 1];
			break;
		case 12: // GTR
			sp--;
			stack[sp] = stack[sp] > stack[sp + 1];
			break;
		case 13: // GEQ
			sp--;
			stack[sp] = stack[sp] >= stack[sp + 1];
			break;
		default:
			printf("\nERROR: %d is not valid. Must be a value >= 0 and <= 13\n", ir.op);
			fprintf(output, "\nERROR: %d is not valid. Must be a value >= 0 and <= 13\n", ir.op);
			fprintf(outputFull, "\nERROR: %d is not valid. Must be a value >= 0 and <= 13\n", ir.op);
			exit(1); // exits with non-normal return value
	}
}

void SIO(int M) {
	switch(M) {
		case 1: // SIO1
			printf("%d\n", stack[sp]);
			sp--;
			break;
		case 2: // SIO2
			sp++;
			scanf("%d", &stack[sp]);
			break;
		case 3: // SIO3
			halt = true;
			pc = 0;
			bp = 0;
			sp = 0;
			break;
		default:
			printf("\nERROR: %d is not valid. Must be 1, 2, or 3\n", ir.op);
			fprintf(output, "\nERROR: %d is not valid. Must be 1, 2, or 3\n", ir.op);
			fprintf(outputFull, "\nERROR: %d is not valid. Must be 1, 2, or 3\n", ir.op);
			exit(1); // exits with non-normal return value
	}
}

/* returns name corresponding to given op code */
char *opName(int OP) {
	switch(OP) {
		case 1:
			return "lit";
			break;
		case 2:
			return "opr";
			break;
		case 3:
			return "lod";
			break;
		case 4:
			return "sto";
			break;
		case 5:
			return "cal";
			break;
		case 6:
			return "inc";
			break;
		case 7:
			return "jmp";
			break;
		case 8:
			return "jpc";
			break;
		case 9:
			return "sio";
			break;
		case 10:
			return "sio";
			break;
		case 11:
			return "sio";
			break;
		default:
			printf("\nERROR: %d is not a valid op code", ir.op);
			fprintf(output, "\nERROR: %d is not a valid op code", ir.op);
			fprintf(outputFull, "\nERROR: %d is not a valid op code", ir.op);
			exit(1); // exits with non-normal return value
	}
}

// prints line in stack trace
// iteratively goes through stack and prints each entry
void printStack(int SP, int BP) {
	int i;

	if (BP == 0) return;
	else if (BP == 1) {
		// prints all entries in stack
		for (i = 1; i <= SP; ++i) {
			fprintf(output, "%d ", stack[i]);
      fprintf(outputFull, "%d ", stack[i]);
		}
	}
	else {
		printStack(BP - 1, stack[BP + 2]);

		if (SP < BP) {
			fprintf(output, "| ");
      fprintf(outputFull, "| ");

			for (i = 0; i < 4; ++i) {
				fprintf(output, "%d ", stack[BP + i]);
        fprintf(outputFull, "%d ", stack[BP + i]);
			}
		}
		else {
			fprintf(output, "| ");
      fprintf(outputFull, "| ");

			for (i = BP; i <= SP; ++i) {
				fprintf(output, "%d ", stack[i]);
        fprintf(outputFull, "%d ", stack[i]);
			}
		}
	}
}

void printVmOutput() {
  char s[9999];

  fgets(s, 9999, input);

  while (!feof(input)) {
    printf("%s\n", s);
    fgets(s, 9999, input);
  }
}

/* finds base l levels down */
int base(int l, int b) // l stand for L in the instruction format
{
  int b1; //find base L levels down
  b1 = b;
  while (l > 0)
  {
    b1 = stack[b1 + 1];
    l--;
  }
  return b1;
}
