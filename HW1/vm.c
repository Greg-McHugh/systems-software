// Gregory McHugh
// COP3402-17Summer C001
// GR795710
// G3437772
// HW1

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3


FILE *inputFP, *outputFP;

int pc, bp, sp;

typedef struct {
	int op, l, m;
} instruction;

instruction ir; // current instruction
int stack[MAX_STACK_HEIGHT]; // holds all entries in stack
instruction code[MAX_CODE_LENGTH]; // all instructions
int lines; // lines
int line;

bool halt;

/* functions in vm.c */
void storeAndPrintCode();
void fetch();
void execute();
void OPR(int M);
void SIO(int M);
char *opName();
void printStack(int SP, int BP);
int base(int l, int base);

int main() {
	inputFP = fopen("pmasm.txt", "r"); // reads instructions
	outputFP = fopen("vmoutput.txt", "w"); // writes stack trace
	
	storeAndPrintCode();
	
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
	fprintf(outputFP, "\t\t\t\t\tpc\tbp\tsp\tstack\n");
	fprintf(outputFP, "Initial\tvalues\t\t\t\t%d\t%d\t%d\n", pc, bp, sp);
	
	printf("\t\t\t\t\tpc\tbp\tsp\tstack\n");
	printf("Initial\tvalues\t\t\t\t%d\t%d\t%d\n", pc, bp, sp);
	
	while (!halt) {
		line = pc;
		
		fetch(); // fetches instruction
		
		// prints instruction
		fprintf(outputFP, "%d\t%s\t%d\t%d\t\t", line, opName(ir.op), ir.l, ir.m);
		printf("%d\t%s\t%d\t%d\t\t", line, opName(ir.op), ir.l, ir.m);
		
		pc++;
		
		execute(); // executes instruction
		
		// prints PC, BP, and SP
		fprintf(outputFP, "%d\t%d\t%d\t", pc, bp, sp);		
		printf("%d\t%d\t%d\t", pc, bp, sp);
		
		printStack(sp, bp);
		
		fprintf(outputFP, "\n");
		printf("\n");
	}
	
	fclose(inputFP);
	fclose(outputFP);
}

void storeAndPrintCode() {
	fprintf(outputFP, "Line\tOP\tL\tM\n");
	printf("Line\tOP\tL\tM\n");
	
	int i = 0;
	
	while(fscanf(inputFP, "%d", &code[i].op) == 1) {
		fscanf(inputFP, "%d", &code[i].l);
		
		fscanf(inputFP, "%d", &code[i].m);
		
		fprintf(outputFP, "%d\t%s\t%d\t%d\n", i, opName(code[i].op), code[i].l, code[i].m);
		printf("%d\t%s\t%d\t%d\n", i, opName(code[i].op), code[i].l, code[i].m);
		
		++i;
	}
	
	lines = i;
	
	fprintf(outputFP, "\n");
	printf("\n");
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
			fprintf(outputFP, "\nERROR: %d is not a valid op code\n", ir.op);
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
			sp--;
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
			fprintf(outputFP, "\nERROR: %d is not valid. Must be a value >= 0 and <= 13\n", ir.op);
			exit(1); // exits with non-normal return value
	}
}

void SIO(int M) {
	switch(M) {
		case 1: // SIO1
			printf("out: %d\n", stack[sp]);
			fprintf(outputFP, "out: %d\n", stack[sp]);
			sp--;
			break;
		case 2: // SIO2
			sp++;
			printf("in: ");
			scanf("%d", &stack[sp]);
			fprintf(outputFP, "in: ");
			fscanf(outputFP, "%d", &stack[sp]);
			break;
		case 3: // SIO3
			halt = true;
			pc = 0;
			bp = 0;
			sp = 0;
			break;
		default:
			printf("\nERROR: %d is not valid. Must be 1, 2, or 3\n", ir.op);
			fprintf(outputFP, "\nERROR: %d is not valid. Must be 1, 2, or 3\n", ir.op);
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
			fprintf(outputFP, "\nERROR: %d is not a valid op code", ir.op);
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
			printf("%d ", stack[i]);
			fprintf(outputFP, "%d ", stack[i]);
		}
	}
	else {
		printStack(BP - 1, stack[BP + 2]);
		
		if (SP < BP) {
			fprintf(outputFP, "| ");
			printf("| ");
			
			for (i = 0; i < 4; ++i) {
				printf("%d ", stack[BP + i]);
				fprintf(outputFP, "%d ", stack[BP + i]);
			}
		}
		else {
			fprintf(outputFP, "| ");
			printf("| ");
			
			for (i = BP; i <= SP; ++i) {
				printf("%d ", stack[i]);
				fprintf(outputFP, "%d ", stack[i]);
			}
		}
	}
}

/* finds base l levels down */
int base(int l, int base) // l stand for L in the instruction format
{  
  int b1; //find base L levels down
  b1 = base; 
  while (l > 0)
  {
    b1 = stack[b1 + 1];
    l--;
  }
  return b1;
}


