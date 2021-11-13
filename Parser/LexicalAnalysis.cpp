#include "LexicalAnalysis.h"

/// @brief ��������
int lineNum = 1; //����
int charNum = 0; //�ַ���
int keywordNum = 0; //�ؼ�������
int punNum = 0; //�����ź��������������
int opNum = 0; //��ϵ���������
int idNum = 0; //��ʶ������
int numNum = 0; //���ֳ�������
int comNum = 0; //ע������

const string keywords[] = { "if" , "else", "break", "continue", "for", "while", "do", "switch", "case", "default",
"return", "sizeof", "char", "short", "int", "long", "float", "double",  "unsigned", "signed", "struct",
"union", "void", "enum", "const", "volatile", "typedef", "auto", "register", "static", "extern", "goto" };
map<string, bool>keyword; //�ؼ��ֱ�

/// @brief ����ؼ��ֱ�
static void init()
{
	for (const auto& i : keywords) keyword[i] = true;
}

/// @brief �ع�һ���ַ�
void retract(ifstream& infile)
{
	infile.seekg(-1, std::ios::cur);
	char ch = infile.peek();

	--charNum;
	if (ch == '\n') --lineNum;
}

/// @brief ��ȡһ���ַ�
char getChar(ifstream& infile)
{
	char ch = infile.get();

	if (ch != EOF && ch != '\n') ++charNum;
	if (ch == '\n') ++lineNum;
	return ch;
}

/// @brief ����ű�����ӼǺ�
void addSymbol(string& buf, int& state, vector<pair<string, string>>& symbolTable)
{
	if (state == 1)
	{
		++numNum;
		symbolTable.push_back(make_pair("num", buf));
	}
	else if (state == 2)
	{
		if (keyword[buf] == true)
		{
			++keywordNum;
			symbolTable.push_back(make_pair("keyword", buf));
		}
		else
		{
			++idNum;
			symbolTable.push_back(make_pair("id", buf));
		}
	}
	state = 0;
	buf = "";
}

/// @brief �ʷ���������
/// state 1 ��ʾĿǰʶ���״̬������
/// state 2 ��ʾĿǰʶ���״̬�Ǳ�ʶ��
/// state 3 ��ʾĿǰʶ���״̬����ע��
/// state 4 ��ʾĿǰʶ���״̬�Ƕ���ע��
/// state 0 ��ʾĿǰʶ����ǳ�����4��״̬���״̬
void lexicalAnaly(ifstream& infile, vector<pair<string, string>>& symbolTable)
{
	char ch, ch2, ch3;
	string buf;
	int state = 0;

	while ((ch = getChar(infile)) != EOF)
	{
		switch (ch)
		{
		case ' ': case '\t':
			if (state == 1 || state == 2)
			{
				addSymbol(buf, state, symbolTable);
			}
			else
			{
				buf += ch;
				break;
			}
			state = 0;
			break;

		case '\n':
			if (state == 3)
			{
				++comNum;
				symbolTable.push_back(make_pair("comment", buf));
				buf = "";
				state = 0;
			}
			else if (state == 4) buf += ch;
			break;

		case '(': case ')': case ';': case '\\': case '_':
		case '^': case '?': case ':': case '{':	case '}':
		case '[': case ']': case '#': case ',':
			if (state == 1 || state == 2)
			{
				addSymbol(buf, state, symbolTable);
			}
			else if (state == 3 || state == 4)
			{
				buf += ch;
				break;
			}
			++punNum;
			state = 0;
			symbolTable.push_back(make_pair(string(1, ch), "-"));
			break;

		case '+':
			if (state == 0)
			{
				ch2 = getChar(infile);
				if (ch2 == '+' || ch2 == '=') //++��+=
				{
					++punNum;
					symbolTable.push_back(make_pair(string(1, ch) + string(1, ch2), "-"));
				}
				else if (isalpha(ch2) || ch2 == '-') //������+
				{
					++punNum;
					symbolTable.push_back(make_pair("+", "-"));
					retract(infile);
				}
				else //����ǰ��+
				{
					state = 1;
					buf = "+";
					retract(infile);
				}
			}
			else if (state == 1) //֮ǰ������
			{
				addSymbol(buf, state, symbolTable);
				++punNum;
				symbolTable.push_back(make_pair("+", "-"));
			}
			else if (state == 2) //֮ǰ��һ����ʶ��
			{
				addSymbol(buf, state, symbolTable);
				ch2 = getChar(infile);
				++punNum;
				if (ch2 == '+' || ch2 == '=') //++��+=
				{
					symbolTable.push_back(make_pair(string(1, ch) + string(1, ch2), "-"));
				}
				else
				{
					symbolTable.push_back(make_pair("+", "-"));
					retract(infile);
				}
			}
			else buf += ch; //���ֽ����е�+����1e+6
			break;

		case '-': //��+���ж�����
			if (state == 0)
			{
				ch2 = getChar(infile);
				if (ch2 == '-' || ch2 == '=')
				{
					++punNum;
					symbolTable.push_back(make_pair(string(1, ch) + string(1, ch2), "-"));
				}
				else if (isalpha(ch2) || ch2 == '+')
				{
					++punNum;
					symbolTable.push_back(make_pair("-", "-"));
					retract(infile);
				}
				else
				{
					state = 1;
					buf = "-";
					retract(infile);
				}
			}
			else if (state == 1)
			{
				addSymbol(buf, state, symbolTable);
				++punNum;
				symbolTable.push_back(make_pair("-", "-"));
			}
			else if (state == 2)
			{
				addSymbol(buf, state, symbolTable);
				++punNum;
				symbolTable.push_back(make_pair("-", "-"));
			}
			else buf += ch;
			break;

		case '*':
			if (state == 0)
			{
				ch2 = getChar(infile);
				++punNum;
				if (ch2 == '=')
				{
					symbolTable.push_back(make_pair("*=", "-"));
				}
				else
				{
					symbolTable.push_back(make_pair("*", "-"));
					retract(infile);
				}
			}
			else if (state == 1 || state == 2)
			{
				addSymbol(buf, state, symbolTable);
				retract(infile);
			}
			else if (state == 3) buf += ch;
			else //ע���е�*
			{
				ch2 = getChar(infile);
				buf += ch;
				buf += ch2;
				if (ch2 == '/')
				{
					++comNum;
					symbolTable.push_back(make_pair("comment", buf));
					state = 0;
					buf = "";
				}
			}
			break;

		case '/':
			if (state == 0)
			{
				ch2 = getChar(infile);
				++punNum;
				if (ch2 == '=')
				{
					symbolTable.push_back(make_pair("/=", "-"));
				}
				else if (isalnum(ch2) || ch2 == '+' || ch2 == '-' || ch2 == '(')
				{
					symbolTable.push_back(make_pair("/", "-"));
					retract(infile);
				}
				else if (ch2 == '/') //��ע��
				{
					state = 3;
					buf = ch;
					buf += ch2;
				}
				else if (ch2 == '*') //����ע��
				{
					state = 4;
					buf = ch;
					buf += ch2;
				}
			}
			else if (state == 1 || state == 2)
			{
				addSymbol(buf, state, symbolTable);
				retract(infile);
			}
			else buf += ch;
			break;

		case '%':
			if (state == 0)
			{
				ch2 = getChar(infile);
				++punNum;
				if (ch2 == '=')
				{
					symbolTable.push_back(make_pair("%=", "-"));
				}
				else
				{
					symbolTable.push_back(make_pair("%", "-"));
					retract(infile);
				}
			}
			else if (state == 1 || state == 2)
			{
				addSymbol(buf, state, symbolTable);
				retract(infile);
			}
			break;

		case '=':
			if (state == 0)
			{
				ch2 = getChar(infile);
				++opNum;
				if (ch2 == '=')
				{
					symbolTable.push_back(make_pair("relop", "EQ"));
				}
				else
				{
					symbolTable.push_back(make_pair("assign-op", "="));
					retract(infile);
				}
			}
			else if (state == 1 || state == 2)
			{
				addSymbol(buf, state, symbolTable);
				retract(infile);
			}
			else buf += ch;
			break;

		case '!':
			if (state == 0)
			{
				ch2 = getChar(infile);
				if (ch2 == '=')
				{
					++opNum;
					symbolTable.push_back(make_pair("relop", "NE"));
				}
				else
				{
					++punNum;
					symbolTable.push_back(make_pair("!", "-"));
					retract(infile);
				}
			}
			else if (state == 1 || state == 2)
			{
				addSymbol(buf, state, symbolTable);
				retract(infile);
			}
			else buf += ch;
			break;

		case '<':
			if (state == 0)
			{
				ch2 = getChar(infile);
				if (ch2 == '=')
				{
					++opNum;
					symbolTable.push_back(make_pair("relop", "LE"));
				}
				else
				{
					++punNum;
					symbolTable.push_back(make_pair("relop", "LT"));
					retract(infile);
				}
			}
			else if (state == 1 || state == 2)
			{
				addSymbol(buf, state, symbolTable);
				retract(infile);
			}
			else buf += ch;
			break;

		case '>':
			if (state == 0)
			{
				ch2 = getChar(infile);
				if (ch2 == '=')
				{
					++opNum;
					symbolTable.push_back(make_pair("relop", "GE"));
				}
				else
				{
					++punNum;
					symbolTable.push_back(make_pair("relop", "GT"));
					retract(infile);
				}
			}
			else if (state == 1 || state == 2)
			{
				addSymbol(buf, state, symbolTable);
				retract(infile);
			}
			else buf += ch;
			break;

		case '&':
			if (state == 0)
			{
				ch2 = getChar(infile);
				++punNum;
				if (ch2 == '&')
				{
					symbolTable.push_back(make_pair("&&", "-"));
				}
				else
				{
					symbolTable.push_back(make_pair("&", "-"));
					retract(infile);
				}
			}
			else if (state == 1 || state == 2)
			{
				addSymbol(buf, state, symbolTable);
				retract(infile);
			}
			else buf += ch;
			break;

		case '|':
			if (state == 0)
			{
				ch2 = getChar(infile);
				++punNum;
				if (ch2 == '|')
				{
					symbolTable.push_back(make_pair("||", "-"));
				}
				else
				{
					symbolTable.push_back(make_pair("|", "-"));
					retract(infile);
				}
			}
			else if (state == 1 || state == 2)
			{
				addSymbol(buf, state, symbolTable);
				retract(infile);
			}
			else buf += ch;
			break;

		case 'A': case 'B': case 'C': case 'D': case 'F': case 'G':
		case 'H': case 'I': case 'J': case 'K': case 'L': case 'M':
		case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S':
		case 'T': case 'U':	case 'V': case 'W': case 'X': case 'Y': case 'Z':
		case 'a': case 'b': case 'c': case 'd': case 'f': case 'g':
		case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
		case 'n': case 'o': case 'p': case 'q': case 'r': case 's':
		case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
			if (state == 0)
			{
				state = 2;
				buf = ch;
			}
			else if (state == 1) //�������
			{
				cout << "[ERROR]Line " << lineNum << ": Identifier cannot start with a number. ";
				while (isalnum(ch))
				{
					buf += ch;
					ch = getChar(infile);
				}
				cout << "\"" << buf << "\"" << endl;
				buf = "";
				state = 0;
				retract(infile);
			}
			else buf += ch;
			break;

		case 'E': case 'e':
			if (state == 0)
			{
				state = 2;
				buf = ch;
			}
			else if (state == 1) //�����е�E��e
			{
				buf += ch;
				ch2 = getChar(infile);
				if ('0' <= ch2 && ch2 <= '9')
				{
					buf += ch2;
					break;
				}
				ch3 = getChar(infile);
				if ((ch2 == '+' || ch2 == '-') && ('0' <= ch3 && ch3 <= '9'))
				{
					buf += ch2;
					buf += ch3;
				}
				else
				{
					cout << "[ERROR]Line " << lineNum << ": Illegal number. ";
					buf += ch2;
					ch = ch3;
					while (isalnum(ch))
					{
						buf += ch;
						ch = getChar(infile);
					}
					cout << "\"" << buf << "\"" << endl;
					buf = "";
					state = 0;
					retract(infile);
				}
			}
			else buf += ch;
			break;

		case '.':
			if (state == 2)
			{
				addSymbol(buf, state, symbolTable);
			}
			else
			{
				buf += ch;
				break;
			}
			++punNum;
			state = 0;
			symbolTable.push_back(make_pair(".", "-"));
			break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			if (state == 0) state = 1;
			buf += ch;
			break;

		case '`': case '@': case '$': //�Ƿ��ַ�
			if (state == 3 || state == 4) buf += ch; //ע���в�����
			else
			{
				if (state == 1 || state == 2)
				{
					addSymbol(buf, state, symbolTable);
				}
				state = 0;
				cout << "[ERROR]Line " << lineNum << ": Invalid character. \"" << ch << "\"" << endl;
			}
			break;

		default:
			break;
		}
	}
}