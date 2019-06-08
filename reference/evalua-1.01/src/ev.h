#ifndef _EV_H_
#define _EV_H_

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <memory.h>



#define MAX_QUEUE		16384
#define MAX_STACK		16384

#define OP_END			0
#define OP_DUMMY		1
#define OP_VALUE		2
#define OP_OP			3
#define OP_ADD			'+'
#define OP_SUB			'-'
#define OP_MUL			'*'
#define OP_DIV			'/'
#define OP_MOD			'%'
#define OP_OR			'|'
#define OP_AND			'&'
#define OP_EOR			'^'
#define OP_SHL			'<'
#define OP_SHR			'>'
#define OP_PAR_L		'('
#define OP_PAR_R		')'
#define OP_VAR_A		'A'
#define OP_VAR_Z		'Z'



typedef struct {
	long value;
	char op;
} EV_ENTRY;



class EV
{
public:

	EV();
	~EV();

	void ClearVars(void);
	void SetVar(char var,long long value);

	const char* Parse(const char* str);
	long long Solve(void);
	
private:

	EV_ENTRY queue[MAX_QUEUE];

	int queue_ptr;

	char op_stack[MAX_STACK];

	int op_stack_ptr;

	long long solve_stack[MAX_QUEUE];

	int solve_stack_ptr;

	long long vars[26];

	long long GetVar(char var);
	bool IsDigit(char c);
	bool IsOperator(char c);
	int OpPrecedence(char c);
	bool IsVariable(char c);
	bool QueueOp(char op);
	bool QueueVar(char op);
	bool QueueValue(long value);
	bool StackIsEmpty(void);
	bool StackPush(char op);
	char StackPop(void);
	char StackTop(void);
};

#endif