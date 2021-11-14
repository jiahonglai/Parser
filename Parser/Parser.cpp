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

const string terminalSymbol[] = { "+","-","*","/","(",")","ε","num" };
map<string, vector<string>>production; //产生式
map<string, set<string>>first; //FIRST集
map<string, set<string>>follow; //FOLLOW集
map<pair<string, string>, string>analyTable; //分析表
vector<pair<string, string>>symbolTable; //符号表
map<string, bool>terSymbol; //终结符号表
map<string, bool>visit; //标记某非终结符号是否访问过
map<string, int>preSize; //存储上一次FOLLOW集元素个数

/// @brief 构造变换后的文法产生式及终结符号表
void init()
{
	production["E"].push_back("TE'");

	production["E'"].push_back("+TE'");
	production["E'"].push_back("-TE'");
	production["E'"].push_back("ε");

	production["T"].push_back("FT'");

	production["T'"].push_back("*FT'");
	production["T'"].push_back("/FT'");
	production["T'"].push_back("ε");

	production["F"].push_back("(E)");
	production["F"].push_back("num");

	for (const auto& i : terminalSymbol) terSymbol[i] = true;
}

/// @brief 通过dfs构造FIRST集
/// @param left 产生式左部
void dfsFirst(const string& left)
{
	for (const auto& right : production[left])
	{
		string tmp = string(1, right[0]);

		if (terSymbol[tmp] == true) first[left].insert(tmp); //右部第一个字符是终结符
		else if (terSymbol[right] == true) first[left].insert(right); //整个字符是终结符
		else
		{
			int l = right.size();
			int j = 0;
			bool epsilon = true; //是否会产生ε

			while (j < l)
			{
				tmp = string(1, right[j]);
				if (terSymbol[tmp] != true) //tmp不是终结符
				{
					if (j + 1 < l && right[j + 1] == '\'') //向后多读一个字符，判断是否是形如E'的非终结符
					{
						tmp += "'";
						++j;
					}
					if (first[tmp].size() == 0) dfsFirst(tmp); //tmp的FIRST集还没构造，先构造tmp的FIRST集
					first[left].insert(first[tmp].begin(), first[tmp].end()); //将tmp的FIRST集加入该非终结符的FIRST集
					if (first[tmp].count("ε") == false) //tmp的FIRST集不含ε，则不可能产生ε
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
			if (epsilon == true) first[left].insert("ε");
		}
	}
}

/// @brief 生成FIRST集
void generaFirst()
{
	for (const auto& i : production)
	{
		if (first[i.first].size() > 0) continue; //说明该非终结符号的FISRT集已经构造
		dfsFirst(i.first);
	}
}

/// @brief 构造FOLLOW集
/// @param left 产生式左部
void constrFollow(const string& left)
{
	for (const auto& right : production[left])
	{
		if (terSymbol[right] == true) continue; //终结符跳过
		string tmp;
		int l = right.size();
		int j = 0;

		while (j < l)
		{
			tmp = string(1, right[j]);
			if (terSymbol[tmp] != true) //tmp不是终结符
			{
				if (j + 1 < l && right[j + 1] == '\'') //向后多读一个字符，判断是否是形如E'的非终结符
				{
					tmp += "'";
					++j;
				}

				int k = j + 1; //从该非终结符后开始寻找其FOLLOW集
				string symbol;
				bool epsilon = true; //是否会产生ε

				while (k < l) 
				{
					symbol = string(1, right[k]);
					if (terSymbol[symbol] != true) //不是终结符
					{
						if (k + 1 < l && right[k + 1] == '\'') //向后多读一个字符，判断是否是形如E'的非终结符
						{
							symbol += "'";
							++k;
						}
						follow[tmp].insert(first[symbol].begin(), first[symbol].end()); //将非终结符symbol的FIRST集加入
						if (first[symbol].count("ε") == false)
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
				if (epsilon == true) follow[tmp].insert(follow[left].begin(), follow[left].end()); //后面为ε，则加入左部的FOLLOW集
			}
			++j;
		}
	}
}

/// @brief 生成FOLLOW集
void generaFollow()
{
	follow["E"].insert("$"); //启示符号的FOLLOW集加入$符号
	preSize["E"] = 1;
	bool flag = true;

	while (flag) //一轮循环中如果没有非终结符的FOLLOW加入新符号，则说明FOLLOW集构造完成
	{
		flag = false;
		visit.clear();
		for (const auto& i : production)
		{
			if (visit[i.first]) continue; //已访问过，则跳过
			visit[i.first] = true;
			constrFollow(i.first);
		}
		for (const auto& i : production)
		{
			if (follow[i.first].size() > preSize[i.first]) flag = true; //如果FOLLOW集加入新符号
			preSize[i.first] = follow[i.first].size();
		}
	}

	for (const auto& i : production)
	{
		if (follow[i.first].count("ε") == true) follow[i.first].erase("ε"); //FOLLOW集中不可能存在ε，故删除
	}
}

/// @brief 构造LL(1)分析表
void generAnalyTable()
{
	for (const auto& i : production)
	{
		for (const auto& right : i.second)
		{
			string tmp = string(1, right[0]);
			set<string>rightFirst; //right的FIRST集
			bool epsilon = true; //是否会产生ε

			if (terSymbol[tmp] == true)
			{
				rightFirst.insert(tmp);
				if (tmp != "ε") epsilon = false;
			}
			else if (terSymbol[right] == true)
			{
				rightFirst.insert(right);
				if (right != "ε") epsilon = false;
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
						if (j + 1 < l && right[j + 1] == '\'') //向后多读一个字符，判断是否是形如E'的非终结符
						{
							tmp += "'";
							++j;
						}
						rightFirst.insert(first[tmp].begin(), first[tmp].end());
						if (first[tmp].count("ε") == false) //tmp的FIRST集不含ε，则不可能产生ε
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

			for (const auto& k : rightFirst) //根据FIRST集的符号写入该产生式
			{
				if (k == "ε") continue;
				p = mp(i.first, k);
				analyTable[p] = right;
			}

			if (epsilon == true) //如果可以产生ε，则将该产生式写入FOLLOW集符号所在的位置
			{
				for (const auto& k : follow[i.first])
				{
					p = mp(i.first, k);
					analyTable[p] = right;
				}
			}
			else
			{
				for (const auto& k : follow[i.first]) //增加同步信息，错误处理
				{
					p = mp(i.first, k);
					analyTable[p] = "synch";
				}
			}
		}
	}
}

/// @brief 使用分析表进行分析
/// @param infile 输入文件
void analyse(ifstream& infile)
{
	lexicalAnaly(infile, symbolTable); //词法分析获取记号集合
	symbolTable.push_back(mp("$", "-")); //加入$符号

	vector<string>analyStack; //使用vector模拟分析栈
	analyStack.push_back("$"); //压入$符号
	analyStack.push_back("E"); //压入起始符号E

	int tableSize = symbolTable.size();
	int i = 0;
	pair<string, string>p;
	cout.setf(ios::left); //左对齐输出

	cout << setw(15) << "栈" << setw(30) << "输入" << "输出" << endl;
	
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
			if (stackTop == "$") cout << "分析完成";
			else cout << "匹配成功";
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
					if (tmp != "ε") analyStack.push_back(tmp);
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
				cout << "匹配失败,";
				if (analyTable[p] == "synch")
				{
					cout << "弹出" << stackTop;
					analyStack.pop_back();
				}
				else
				{
					cout << "跳过" << symbolTable[i].first;
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
		cout << "File Not Found！";
		return 0;
	}

	analyse(infile);

	infile.close();
	return 0;
}