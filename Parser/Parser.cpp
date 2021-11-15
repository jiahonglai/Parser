#include "LexicalAnalysis.h"
#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <set>
#include <vector>
#include <stack>
#include <iomanip>
#define mp make_pair

using namespace std;

const string terminalSymbol[] = { "+","-","*","/","(",")","��","num" };
map<string, vector<string>>production; //����ʽ
map<string, set<string>>first; //FIRST��
map<string, set<string>>follow; //FOLLOW��
map<pair<string, string>, string>analyTable; //������
vector<pair<string, string>>symbolTable; //���ű�
map<string, bool>terSymbol; //�ս���ű�
map<string, bool>visit; //���ĳ���ս�����Ƿ���ʹ�
map<string, int>preSize; //�洢��һ��FOLLOW��Ԫ�ظ���

/// @brief ����任����ķ�����ʽ���ս���ű�
void init()
{
	production["E"].push_back("TE'");

	production["E'"].push_back("+TE'");
	production["E'"].push_back("-TE'");
	production["E'"].push_back("��");

	production["T"].push_back("FT'");

	production["T'"].push_back("*FT'");
	production["T'"].push_back("/FT'");
	production["T'"].push_back("��");

	production["F"].push_back("(E)");
	production["F"].push_back("num");

	for (const auto& i : terminalSymbol) terSymbol[i] = true;
}

/// @brief ͨ��dfs����FIRST��
/// @param left ����ʽ��
void dfsFirst(const string& left)
{
	for (const auto& right : production[left])
	{
		string tmp = string(1, right[0]);

		if (terSymbol[tmp] == true) first[left].insert(tmp); //�Ҳ���һ���ַ����ս��
		else if (terSymbol[right] == true) first[left].insert(right); //�����ַ����ս��
		else
		{
			int l = right.size();
			int j = 0;
			bool epsilon = true; //�Ƿ�������

			while (j < l)
			{
				tmp = string(1, right[j]);
				if (terSymbol[tmp] != true) //tmp�����ս��
				{
					if (j + 1 < l && right[j + 1] == '\'') //�����һ���ַ����ж��Ƿ�������E'�ķ��ս��
					{
						tmp += "'";
						++j;
					}
					if (first[tmp].size() == 0) dfsFirst(tmp); //tmp��FIRST����û���죬�ȹ���tmp��FIRST��
					first[left].insert(first[tmp].begin(), first[tmp].end()); //��tmp��FIRST������÷��ս����FIRST��
					if (first[tmp].count("��") == false) //tmp��FIRST�������ţ��򲻿��ܲ�����
					{
						epsilon = false;
						break;
					}
				}
				else
				{
					first[left].insert(tmp);
					epsilon = false;
					break;
				}
				++j;
			}
			if (epsilon == true) first[left].insert("��");
		}
	}
}

/// @brief ����FIRST��
void generaFirst()
{
	for (const auto& i : production)
	{
		if (first[i.first].size() > 0) continue; //˵���÷��ս���ŵ�FISRT���Ѿ�����
		dfsFirst(i.first);
	}
}

/// @brief ����FOLLOW��
/// @param left ����ʽ��
void constrFollow(const string& left)
{
	for (const auto& right : production[left])
	{
		if (terSymbol[right] == true) continue; //�ս������
		string tmp;
		int l = right.size();
		int j = 0;

		while (j < l)
		{
			tmp = string(1, right[j]);
			if (terSymbol[tmp] != true) //tmp�����ս��
			{
				if (j + 1 < l && right[j + 1] == '\'') //�����һ���ַ����ж��Ƿ�������E'�ķ��ս��
				{
					tmp += "'";
					++j;
				}

				int k = j + 1; //�Ӹ÷��ս����ʼѰ����FOLLOW��
				string symbol;
				bool epsilon = true; //�Ƿ�������

				while (k < l) 
				{
					symbol = string(1, right[k]);
					if (terSymbol[symbol] != true) //�����ս��
					{
						if (k + 1 < l && right[k + 1] == '\'') //�����һ���ַ����ж��Ƿ�������E'�ķ��ս��
						{
							symbol += "'";
							++k;
						}
						follow[tmp].insert(first[symbol].begin(), first[symbol].end()); //�����ս��symbol��FIRST������
						if (first[symbol].count("��") == false)
						{
							epsilon = false;
							break;
						}
					}
					else
					{
						follow[tmp].insert(symbol);
						epsilon = false;
						break;
					}
					++k;
				}
				if (epsilon == true) follow[tmp].insert(follow[left].begin(), follow[left].end()); //����Ϊ�ţ�������󲿵�FOLLOW��
			}
			++j;
		}
	}
}

/// @brief ����FOLLOW��
void generaFollow()
{
	follow["E"].insert("$"); //��ʾ���ŵ�FOLLOW������$����
	preSize["E"] = 1;
	bool flag = true;

	while (flag) //һ��ѭ�������û�з��ս����FOLLOW�����·��ţ���˵��FOLLOW���������
	{
		flag = false;
		visit.clear();
		for (const auto& i : production)
		{
			if (visit[i.first]) continue; //�ѷ��ʹ���������
			visit[i.first] = true;
			constrFollow(i.first);
		}
		for (const auto& i : production)
		{
			if (follow[i.first].size() > preSize[i.first]) flag = true; //���FOLLOW�������·���
			preSize[i.first] = follow[i.first].size();
		}
	}

	for (const auto& i : production)
	{
		if (follow[i.first].count("��") == true) follow[i.first].erase("��"); //FOLLOW���в����ܴ��ڦţ���ɾ��
	}
}

/// @brief ����LL(1)������
void generAnalyTable()
{
	for (const auto& i : production)
	{
		for (const auto& right : i.second)
		{
			string tmp = string(1, right[0]);
			set<string>rightFirst; //right��FIRST��
			bool epsilon = true; //�Ƿ�������

			if (terSymbol[tmp] == true)
			{
				rightFirst.insert(tmp);
				if (tmp != "��") epsilon = false;
			}
			else if (terSymbol[right] == true)
			{
				rightFirst.insert(right);
				if (right != "��") epsilon = false;
			}
			else
			{
				int l = right.size();
				int j = 0;
				while (j < l)
				{
					tmp = string(1, right[j]);
					if (terSymbol[tmp] != true)
					{
						if (j + 1 < l && right[j + 1] == '\'') //�����һ���ַ����ж��Ƿ�������E'�ķ��ս��
						{
							tmp += "'";
							++j;
						}
						rightFirst.insert(first[tmp].begin(), first[tmp].end());
						if (first[tmp].count("��") == false) //tmp��FIRST�������ţ��򲻿��ܲ�����
						{
							epsilon = false;
							break;
						}
					}
					else
					{
						rightFirst.insert(tmp);
						epsilon = false;
						break;
					}
					++j;
				}
			}

			pair<string, string>p;

			for (const auto& k : rightFirst) //����FIRST���ķ���д��ò���ʽ
			{
				if (k == "��") continue;
				p = mp(i.first, k);
				analyTable[p] = right;
			}

			if (epsilon == true) //������Բ����ţ��򽫸ò���ʽд��FOLLOW���������ڵ�λ��
			{
				for (const auto& k : follow[i.first])
				{
					p = mp(i.first, k);
					analyTable[p] = right;
				}
			}
			else
			{
				for (const auto& k : follow[i.first]) //����ͬ����Ϣ��������
				{
					p = mp(i.first, k);
					analyTable[p] = "synch";
				}
			}
		}
	}
}

/// @brief ʹ�÷�������з���
/// @param infile �����ļ�
void analyse(ifstream& infile)
{
	lexicalAnaly(infile, symbolTable); //�ʷ�������ȡ�Ǻż���
	symbolTable.push_back(mp("$", "-")); //����$����

	vector<string>analyStack; //ʹ��vectorģ�����ջ
	analyStack.push_back("$"); //ѹ��$����
	analyStack.push_back("E"); //ѹ����ʼ����E

	int tableSize = symbolTable.size();
	int i = 0;
	pair<string, string>p;
	cout.setf(ios::left); //��������

	cout << setw(15) << "ջ" << setw(30) << "����" << "���" << endl;
	
	while (i < tableSize)
	{
		string output = "";

		for (const auto& j : analyStack) output += j;
		cout << setw(15) << output;
		
		output = "";
		for (int k = i; k < tableSize; ++k) output += symbolTable[k].first;
		cout << setw(30) << output;

		string stackTop = analyStack[analyStack.size() - 1];

		if (stackTop == symbolTable[i].first)
		{
			if (stackTop == "$") cout << "�������";
			else cout << "ƥ��ɹ�";
			++i;
			analyStack.pop_back();
		}
		else
		{
			p = mp(stackTop, symbolTable[i].first);
			if (analyTable.count(p) == true && analyTable[p] != "synch")
			{
				string tmp = analyTable[p];
				int l = tmp.size() - 1;

				cout << stackTop << "->" << tmp;
				analyStack.pop_back();
				if (terSymbol[tmp] == true)
				{
					if (tmp != "��") analyStack.push_back(tmp);
				}
				else
				{

					while (l >= 0)
					{
						if (tmp[l] == '\'')
						{
							analyStack.push_back(tmp.substr(l - 1, 2));
							--l;
						}
						else
						{
							analyStack.push_back(string(1, tmp[l]));
						}
						--l;
					}
				}
			}
			else
			{
				cout << "ƥ��ʧ��,";
				if (analyTable[p] == "synch")
				{
					cout << "����" << stackTop;
					analyStack.pop_back();
				}
				else
				{
					cout << "����" << symbolTable[i].first;
					++i;
				}
			}
		}
		cout << endl;
	}
}

int main()
{
	init();

	generaFirst();

	generaFollow();

	generAnalyTable();

	ifstream infile("test5.txt");

	if (!infile.is_open())
	{
		cout << "File Not Found��";
		return 0;
	}

	analyse(infile);

	infile.close();
	return 0;
}