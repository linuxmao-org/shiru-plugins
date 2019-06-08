#include "ev.h"


EV::EV(void)
{
}

EV::~EV()
{
}

//there is 26 variables named A-Z that can be used in an evtion
//their values can be set before calling Solve

void EV::ClearVars(void)
{
	memset(vars,0,sizeof(vars));
}



void EV::SetVar(char var,long long value)
{
	if(var<'A'||var>'Z') return;

	vars[var-'A']=value;
}



long long EV::GetVar(char var)
{
	return vars[var-OP_VAR_A];
}



bool EV::IsDigit(char c)
{
	return (c>='0'&&c<='9')?true:false;
}


bool EV::IsOperator(char c)
{
	if(c==OP_ADD) return true;
	if(c==OP_SUB) return true;
	if(c==OP_MUL) return true;
	if(c==OP_DIV) return true;
	if(c==OP_MOD) return true;
	if(c==OP_AND) return true;
	if(c==OP_OR ) return true;
	if(c==OP_EOR) return true;
	if(c==OP_SHL) return true;
	if(c==OP_SHR) return true;

	return false;
}



int EV::OpPrecedence(char c)
{
	if(c==OP_SHL||c==OP_SHR) return 1;
	if(c==OP_ADD||c==OP_SUB) return 2;
	if(c==OP_MUL||c==OP_DIV||c==OP_MOD) return 3;

	return 0;
}



bool EV::IsVariable(char c)
{
	if(c>=OP_VAR_A&&c<=OP_VAR_Z) return true;

	return false;
}



bool EV::QueueOp(char op)
{
	if(queue_ptr>=MAX_QUEUE) return false;

	queue[queue_ptr++].op=op;

	return true;
}



bool EV::QueueVar(char op)
{
	return QueueOp(op);
}



bool EV::QueueValue(long value)
{
	if(queue_ptr>=MAX_QUEUE) return false;

	queue[queue_ptr].op=OP_VALUE;
	queue[queue_ptr].value=value;

	++queue_ptr;

	return true;
}



bool EV::StackIsEmpty(void)
{
	return (op_stack_ptr)>=0?false:true;
}



bool EV::StackPush(char op)
{
	if(op_stack_ptr>=MAX_STACK-1) return false;

	op_stack[++op_stack_ptr]=op;

	return true;
}



char EV::StackPop(void)
{
	if(op_stack_ptr<0) return 0;

	return op_stack[op_stack_ptr--];
}



char EV::StackTop(void)
{
	if(op_stack_ptr<0) return 0;

	return op_stack[op_stack_ptr];
}



const char* EV::Parse(const char* str)
{
	static const char err_queue_overflow[]="Queue overflow";
	static const char err_stack_overflow[]="Stack overflow";
	static const char err_syntax_error[]="Syntax error";

	char c,cc,t,prev_token;
	long num;
	int pnt_count;	//parenthesis count to make sure it does match

	queue_ptr=0;
	op_stack_ptr=-1;

	prev_token=OP_OP;

	pnt_count=0;

	QueueOp(OP_END);	//to be replaced with dummy, to prevent uncontrolled loops when trying to solve incomplete queue

	while(1)
	{
		//read a character of token and next character

		c =*str++;
		cc=*str;

		if(!c) break;

		if(c<=' ') continue;//skip all spaces, tabs, and other non-ASCII symbols

		if(c>='a'&&c<='z') c-=0x20;	//all letters to caps

		if(IsDigit(c))
		{
			//value/var can't be preceded by a value/var or closing parenthesis

			if(prev_token==OP_VALUE||prev_token==OP_PAR_R) return err_syntax_error;

			//if the token is a number, then push it to the output queue

			if(c=='0'&&cc=='X')	//parse as hex
			{
				++str;

				num=0;

				while(1)
				{
					c=*str;

					if(c>='a'&&c<='f') c-=0x20;//all letters to caps

					if(c>='0'&&c<='9')
					{
						num=(num<<4)+(c-'0');
					}
					else
					if(c>='A'&&c<='F')
					{
						num=(num<<4)+(c-'A'+10);
					}
					else break;

					++str;
				}	
			}
			else			//parse as dec
			{
				num=c-'0';

				while(1)
				{
					c=*str;

					if(!IsDigit(c)) break;

					num=num*10+(c-'0');

					++str;
				}
			}

			//printf("number %i\n",num);

			if(!QueueValue(num)) return err_queue_overflow;

			prev_token=OP_VALUE;
		}
		else
		if(IsVariable(c))
		{
			//printf("variable %c\n",c);
			
			//value/var can't be preceded by a value/var or closing parenthesis

			if(prev_token==OP_VALUE||prev_token==OP_PAR_R) return err_syntax_error;

			//variable treated as values, the same rules apply

			if(!QueueVar(c)) return err_queue_overflow;

			prev_token=OP_VALUE;
		}
		else
		if(IsOperator(c))
		{
			//printf("operator %c\n",c);

			//check for double symbol operators, skip second symbol or throw error

			if(c==OP_SHL)
			{
				if(cc==OP_SHL) ++str; else return err_syntax_error;
			}

			if(c==OP_SHR)
			{
				if(cc==OP_SHR) ++str; else return err_syntax_error;
			}

			//operator can't be preceded by an operator or opening parenthesis

			if(prev_token==OP_OP||prev_token==OP_PAR_L) return err_syntax_error;

			//if the token is an operator, then
			//while ((there is an operator at the top of the operator stack with greater or evl precedence (all left-associative)
            //  and (the operator at the top of the operator stack is not a left parenthesis)
            //  pop operators from the operator stack onto the output queue

			while(!StackIsEmpty())
			{
				t=StackTop();

				if((t==OP_PAR_L)||(OpPrecedence(t)<OpPrecedence(c))) break;

				t=StackPop();

				if(!QueueOp(t)) return err_queue_overflow;
			}

			//after that, push the operator from token onto the operator stack

			if(!StackPush(c)) return err_stack_overflow;

			prev_token=OP_OP;
		}
		else
		if(c==OP_PAR_L)
		{
			//opening parentesis can't be preceded by a value or closing parenthesis

			if(prev_token==OP_VALUE||prev_token==OP_PAR_R) return err_syntax_error;

			//if the token is a left paren (i.e. "("), then push it onto the operator stack

			if(!StackPush(c)) return err_stack_overflow;

			++pnt_count;

			prev_token=OP_PAR_L;
		}
		else
		if(c==OP_PAR_R)
		{
			//closing parentesis can't be preceded by an operator or opening parenthesis

			if(prev_token==OP_OP||prev_token==OP_PAR_L) return err_syntax_error;

			//if the token is a right paren (i.e. ")"), then:
			//while the operator at the top of the operator stack is not a left paren:
			//	pop the operator from the operator stack onto the output queue.

			while(!StackIsEmpty())
			{
				t=StackTop();

				if(t==OP_PAR_L) break;

				t=StackPop();

				if(!QueueOp(t)) return err_queue_overflow;
			}

			//if there is a left paren at the top of the operator stack, then
            //	pop the operator from the operator stack and discard it

			if(!StackIsEmpty())
			{
				if(StackTop()==OP_PAR_L) StackPop();
			}

			--pnt_count;

			prev_token=OP_PAR_R;
		}
		else
		{
			return "Unexpected symbol during parsing";
		}
	}

	//if there are no more tokens to read:
    //while there are still operator tokens on the stack:
    //    pop the operator from the operator stack onto the output queue

	while(!StackIsEmpty())
	{
		t=StackPop();

		if(t==OP_PAR_L||t==OP_PAR_R) continue;

		if(!QueueOp(t)) return err_queue_overflow;
	}

	if(!QueueOp(OP_END)) return err_queue_overflow;

	if(pnt_count>0) return "Missing closing parenthesis";
	if(pnt_count<0) return "Extra closing parenthesis";

	queue[0].op=OP_DUMMY;

	return 0;
}



long long EV::Solve(void)
{
	int ptr;
	long long a,b,n;
	char op;

	solve_stack_ptr=0;

	ptr=0;

	if(queue[ptr].op==OP_DUMMY) ++ptr;

	while(1)
	{
		op=queue[ptr].op;

		if(op==OP_END) break;

		if(op==OP_VALUE)
		{
			//if token is an operand push token onto the stack

			solve_stack[solve_stack_ptr++]=queue[ptr].value;
		}
		else
		if(IsVariable(op))
		{
			//if token is an variable push token onto the stack

			solve_stack[solve_stack_ptr++]=GetVar(op);
		}
		else
		{
			//if token is an operator:
			//	operand_2 pop from the stack
			//	operand_1 pop from the stack
			//	evaluate token with operand_1 and operand_2
			//	push result back onto the stack

			b=solve_stack[--solve_stack_ptr];
			a=solve_stack[--solve_stack_ptr];

			switch(op)
			{
			case OP_ADD: n=a+b; break;
			case OP_SUB: n=a-b; break;
			case OP_MUL: n=a*b; break;
			case OP_DIV: if(!b) b=1; n=a/b; break;	//prevent division by zero
			case OP_MOD: if(!b) b=1; n=a%b; break;	//prevent division by zero
			case OP_OR:  n=a|b; break;
			case OP_AND: n=a&b; break;
			case OP_EOR: n=a^b; break;
			case OP_SHL: n=a<<b; break;
			case OP_SHR: n=a>>b; break;
			}

			solve_stack[solve_stack_ptr++]=n;
		}

		if(solve_stack_ptr<0||solve_stack_ptr>=MAX_STACK) return 0;	//stack out of bounds

		++ptr;
	}

	return solve_stack[0];
}
