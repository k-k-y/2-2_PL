// ERROR1 : 연산자(사칙연산)가 연속해서 나오는 경우
// ERROR2 : 정의되지 않은 변수(IDENT)를 사용하는 경우
// ERROR3 : 괄호가 열렸는데 닫히지 않은 경우
// ERROR4 : Factor 토큰 오류
// ERROR5 : 피연산자(CONST, IDENT)가 연속해서 나오는 경우
// ERROR6 : 대입연산자가 나와야 할 자리에 나오지 않음.
// ERROR7 : Statement 맨 앞에 변수가 나와야 할 자리에 나오지 않음
// ERROR8 : 의미 없는 문장(아무것도 없이 ;만 있는 문장, % # 등 의미없는 문자열로만 이루어진 문장 등)
// ERROR9 : 0으로 나눎
// ERROR10 : 선언하지 않은 토큰이 등장함
// ERROR11 : 괄호가 열리지 않았는데 닫히는 경우

#include <iostream>
#include <cctype>
#include <fstream>
#include <string>
#include <unordered_map>
#include <stack>
#include <queue>

#define NC "\e[0m"
#define RED "\e[0;31m"
#define PUP "\e[0;35m"
#define BLUE "\e[0;94m"

using namespace std;

enum TokenType { IDENT, CONST, ASSIGNMENT_OP, ADD_OP, SUB_OP, MUL_OP, DIV_OP, LEFT_PAREN, RIGHT_PAREN, SEMICOLON, UNKNOWN, END_OF_FILE };

int next_token;
string token_string;

unordered_map<string, pair<bool, int> > SymbolTable; // name, {isInitial, value}
stack<pair<bool, int> > s; // 초기화 여부, 값
int idCnt = 0, constCnt = 0, opCnt = 0, parenCnt = 0;

bool isNEG = false; // 음수인지 양수인지 여부
bool isOP = false; // 연산자가 나올 자리인지 여부
bool isIdent = false; // 변수가 나왔는지 아닌지 여부

queue<pair<int, string> > errorQue; // errorCode, token_string

string input = "";
int pos = -1;
char cur_char;

void inputString(string);

// LexicalAnalyzer
void advance();
void getNonBlank();
string getConst();
string getIdent();
void lexical();

// SyntaxAnalyzer
void Statements(); // <statements> -> <statement> | <statement><semicolon><statements>
void Statement(); // <statement> -> <ident><assignment_op><expression>
void Expression(); // <expression> -> <term><term_tail>
void Term_tail(); // <term_tail> -> <add/sub><term><term_tail> | lambda
void Term(); // <term> -> <factor><factor_tail>
void Factor_tail(); // <factor_tail> -> <mult/div><factor><factor_tail> | lambda
void Factor(); // <factor> -> <left_paren><expression><right_paren> | <ident> | <const>


int main(int argc, char* argv[])
{
	if (argc > 1)
		inputString(argv[1]);

	Statements();

	return 0;
}

void inputString(string fileName)
{
	ifstream file(fileName);
	string temp;
	if (file.is_open())
		while (getline(file, temp))
			input += temp;
		
}
void advance()
{
	pos++;
	cur_char = (pos < input.size()) ? input[pos] : -1;
}
void getNonBlank()
{
	while (cur_char != -1 && cur_char <= 32)
		advance();
}
string getConst()
{
	string result = "";
	if (isNEG == true) result += "-";

	while (cur_char && isdigit(cur_char))
	{
		result += cur_char;
		advance();
	}

	return result;
}
string getIdent()
{
	string result = "";
	while (cur_char && (isalnum(cur_char) || cur_char == '_')) 
	{
		result += cur_char;
		advance();
	}
	return result;
}
void lexical()
{
	getNonBlank();
	
	if (cur_char == -1) 
	{
		next_token = END_OF_FILE;
		token_string = "";
	}
	else if (isdigit(cur_char))
	{
		isNEG = false;
		next_token = CONST;
		token_string = getConst();
	}
	else if (isalpha(cur_char) || cur_char == '_')
	{
		next_token = IDENT;
		token_string = getIdent();
	}
	else if (cur_char == '+')
	{
		advance();
		next_token = ADD_OP;
		token_string = "+";
	}
	else if (cur_char == '-') // 음수 or 뺄셈 연산자
	{
		advance();
		if (cur_char && isdigit(cur_char))
		{
			isNEG = true;
			next_token = CONST;
			token_string = getConst();
		}
		else
		{
			next_token = SUB_OP;
			token_string = "-";
		}
	}
	else if (cur_char == '*')
	{
		advance();
		next_token = MUL_OP;
		token_string = "*";
	}
	else if (cur_char == '/')
	{
		advance();
		next_token = DIV_OP;
		token_string = "/";
	}
	else if (cur_char == ':')
	{
		advance();
		if (cur_char == '=')
		{
			advance();
			next_token = ASSIGNMENT_OP;
			token_string = ":=";
		}
	}
	else if (cur_char == ';')
	{
		advance();
		next_token = SEMICOLON;
		token_string = ";";
	}
	else if (cur_char == '(')
	{
		advance();
		next_token = LEFT_PAREN;
		token_string = "(";
		parenCnt++;
	}
	else if (cur_char == ')')
	{
		advance();
		next_token = RIGHT_PAREN;
		token_string = ")";
		if (parenCnt <= 0)
		{
			errorQue.push(make_pair(11, token_string));
			lexical();
			return;
		}
		else
			parenCnt--;
	}
	else
	{
		next_token = UNKNOWN;
		string s;
		s += cur_char;
		token_string = s;
		advance();

		errorQue.push(make_pair(10, token_string));
		lexical();
		return;
	}


	if ((next_token == ADD_OP || next_token == SUB_OP || next_token == MUL_OP || next_token == DIV_OP)) // ERROR1 : 연속된 연산자 검사
	{
        while (1)
		{
            getNonBlank();
			
            if (cur_char == '+')
			{
				errorQue.push(make_pair(1, "+"));
				advance();
			}
			else if (cur_char == '-')
			{
				errorQue.push(make_pair(1, "-"));
				advance();
			}
			else if (cur_char == '*')
			{
				errorQue.push(make_pair(1, "*"));
				advance();
			}
			else if (cur_char == '/')
			{
				errorQue.push(make_pair(1, "/"));
				advance();
			}
			else
                break;
        }
	}

	if ((next_token == IDENT || next_token == CONST) && !(idCnt == 0 && constCnt == 0)) // ERROR5 : 연속된 피연산자 검사 (대입문이 나오기 전에는 검사하지 않음)
	{
		while (1)
		{
			string temp = "";
			getNonBlank();

			if (isalpha(cur_char) || cur_char == '_')
			{
				while(cur_char != ' ' && (isalnum(cur_char) || cur_char == '_'))
				{
					temp += cur_char;
					advance();
				}
				errorQue.push(make_pair(5, temp));
			}
			else if (isdigit(cur_char))
			{
				while(cur_char != ' ' && isdigit(cur_char))
				{
					temp += cur_char;
					advance();
				}
				errorQue.push(make_pair(5, temp));
			}
			else
				break;
		}
	}

	// print lexeme
	if ((next_token == ADD_OP || next_token == SUB_OP || next_token == MUL_OP || next_token == DIV_OP) && isOP && isIdent)
		cout << token_string << " ";
	else if (next_token == SEMICOLON)
		cout << "\b" << token_string;
	else if (next_token == IDENT || ((next_token == CONST || next_token == LEFT_PAREN || next_token == RIGHT_PAREN) && isIdent))
		cout << token_string << " ";
	
	
	isOP = false;
	isIdent = true;
}

void Statements()
{
	if (next_token == END_OF_FILE) return;

	lexical();
	Statement();

	cout << "\n";
	cout << BLUE "ID: " << idCnt << "; " << "CONST: " << constCnt << "; " << "OP: " << opCnt << ";\n" NC;
	if (errorQue.empty())
		cout << RED "(OK)\n" NC;
	while (!errorQue.empty()) // ERROR 출력
	{
		if (errorQue.front().first == 1)
		{
			cout << RED "(Warning) “중복 연산자(" << errorQue.front().second << ") 제거 후 진행”\n" NC;
			errorQue.pop();
		}
		else if (errorQue.front().first == 2)
		{
			cout << RED "(Error) 정의되지 않은 변수(" << errorQue.front().second << ")가 참조됨\n" NC;
			errorQue.pop();
		}
		else if (errorQue.front().first == 3)
		{
			cout << RED "(Warning) 괄호가 열린 후 닫히지 않음. \")\" 추가\n" NC;
			errorQue.pop();
		}
		else if (errorQue.front().first == 4)
		{
			cout << RED "(Warning) Factor 토큰 오류. \"" << errorQue.front().second << "\" 제거 후 진행\n" NC;
			errorQue.pop();
		}
		else if (errorQue.front().first == 5)
		{
			cout << RED "(Warning) 피연산자가 연속해서 나옴. \"" << errorQue.front().second << "\" 제거 후 진행\n" NC;
			errorQue.pop();
		}
		else if (errorQue.front().first == 6)
		{
			cout << RED "(Warning) 대입연산자가 나오지 않음. 추가 후 진행\n" NC;
			errorQue.pop(); 
		}
		else if (errorQue.front().first == 7)
		{
			cout << RED "(Warning) 변수가 나오지 않음. \"" << errorQue.front().second << "\" 제거 후 진행\n" NC;
			errorQue.pop();
		}
		else if (errorQue.front().first == 8)
		{
			cout << RED "(Error) 의미 없는 문장\n" NC;
			errorQue.pop();
		}
		else if (errorQue.front().first == 9)
		{
			cout << RED "(Error) 0으로 나눎\n" NC;
			errorQue.pop();
		}
		else if (errorQue.front().first == 10)
		{
			cout << RED "(Warning) 선언하지 않은 토큰 \"" << errorQue.front().second << "\" 제거 후 진행\n" NC;
			errorQue.pop();
		}
		else if (errorQue.front().first == 11)
		{
			cout << RED "(Warning) 괄호가 열리지 않았는데 닫힘. \")\" 제거 후 진행\n" NC;
			errorQue.pop();
		}

	}
	
	idCnt = 0; constCnt = 0; opCnt = 0; parenCnt = 0; isIdent = false;

	if (next_token == SEMICOLON)
		Statements();
	
	cout << PUP "Result ==> ";
	if (SymbolTable.size() != 0)
		for (auto it : SymbolTable)
		{
			if (it.second.first == true)
				cout << it.first << ": " << it.second.second << "; ";
			else
				cout << it.first << ": UNKNOWN; ";
		}
	else cout << "변수가 없습니다.";

	cout << "\n" NC;

	exit(0);
}
void Statement()
{
	if (next_token == END_OF_FILE || next_token == SEMICOLON) 
	{
		errorQue.push(make_pair(8, ""));
		return;
	}
	if (next_token == IDENT)
	{
		idCnt++;
		isIdent = true;
		string name = token_string;
		cout << ":= ";
		lexical();
		if (next_token == ASSIGNMENT_OP)
		{
			lexical();
			Expression();
			if (!s.empty())
			{
				pair<bool, int> value = s.top(); s.pop();
				SymbolTable[name] = value;
			}
		}
		else
		{
			errorQue.push(make_pair(6, ":="));
			Expression();
			if (!s.empty())
			{
				pair<bool, int> value = s.top(); s.pop();
				SymbolTable[name] = value;
			}
		}
	}
	else
	{
		errorQue.push(make_pair(7, token_string));
		isIdent = false;
		lexical();

		Statement();
	}
}
void Expression()
{
	Term();
	Term_tail();
}
void Term_tail()
{
	pair<bool, int> operand1;
	pair<bool, int> operand2;
	if (next_token == ADD_OP)
	{	
		opCnt++;
		lexical();
		Term();
		if(!s.empty())
		{
			operand2 = s.top(); s.pop();
			operand1 = s.top(); s.pop();
			s.push(make_pair(operand1.first & operand2.first, operand1.second + operand2.second));
		}
		Term_tail();
	}
	else if (next_token == SUB_OP)
	{
		opCnt++;
		lexical();
		Term();
		if(!s.empty())
		{
			operand2 = s.top(); s.pop();
			operand1 = s.top(); s.pop();
			s.push(make_pair(operand1.first & operand2.first, operand1.second - operand2.second));
		}
		Term_tail();
	}
	else
		return;
}
void Term()
{
	Factor();
	Factor_tail();
}
void Factor_tail()
{
	pair<bool, int> operand1;
	pair<bool, int> operand2;
	if (next_token == MUL_OP)
	{
		opCnt++;
		lexical();
		Factor();
		if(!s.empty())
		{
			operand2 = s.top(); s.pop();
			operand1 = s.top(); s.pop();
			s.push(make_pair(operand1.first & operand2.first, operand1.second * operand2.second));
		}
		Factor_tail();
	}
	else if (next_token == DIV_OP)
	{
		opCnt++;
		lexical();
		Factor();
		if(!s.empty())
		{
			operand2 = s.top(); s.pop();
			operand1 = s.top(); s.pop();
			if (operand2.second != 0)
				s.push(make_pair(operand1.first & operand2.first, operand1.second / operand2.second));
			else // 0으로 나누면 unknown으로 설정. 나눗셈 시행 x
			{
				errorQue.push(make_pair(9, ""));
				s.push(make_pair(false, 1));
			}
				
		}
		Factor_tail();
	}
	else 
		return;
}
void Factor()
{
	if (next_token == LEFT_PAREN)
	{
		lexical();
		Expression();
		if (next_token != RIGHT_PAREN)
		{
			errorQue.push(make_pair(3, "parenError"));
			cout << ") ";
		}
		else
		{
			isOP = true;
			lexical();
		}
			
	}
	else if (next_token == IDENT) 
	{
		idCnt++;
		if (SymbolTable.find(token_string) != SymbolTable.end()) // IDENT exists in table
			s.push(SymbolTable[token_string]);
		else
		{
			errorQue.push(make_pair(2, token_string)); // ERROR2: Undefined variable referenced
			pair<bool, int> value = make_pair(false, 2147483647);
			SymbolTable.insert(make_pair(token_string, value));
			s.push(SymbolTable[token_string]);
		}
		isOP = true;
		lexical();
	}
	else if (next_token == CONST)
	{
		constCnt++;
		s.push(make_pair(true, stoi(token_string)));
		isOP = true;
		lexical();
	}
	else if (next_token == END_OF_FILE || next_token == SEMICOLON)
		return;
	else
	{
		errorQue.push(make_pair(4, token_string));
		lexical();
		Factor();
	}
}