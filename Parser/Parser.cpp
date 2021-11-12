#include "LexicalAnalysis.h"
#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <set>
#include <vector>
#define mp make_pair

using namespace std;

const string terminalSymbol[] = { "+","-","*","/","(",")","¦Å","num" };
map<string, vector<string>>production;
map<string, set<string>>first;
map<string, set<string>>follow;
map<pair<string, string>, string>analyTable;
vector<pair<string, string>>symbolTable; //·ûºÅ±í
map<string, bool>terSymbol;
map<string, bool>visit;
map<string, int>preSize;

void init()
{
	production["E"].push_back("TE'");

	production["E'"].push_back("+TE'");
	production["E'"].push_back("-TE'");
	production["E'"].push_back("¦Å");

	production["T"].push_back("FT'");

	production["T'"].push_back("*FT'");
	production["T'"].push_back("/FT'");
	production["T'"].push_back("¦Å");

	production["F"].push_back("(E)");
	production["F"].push_back("num");

	for (const auto& i : terminalSymbol) terSymbol[i] = true;
}

void dfsFirst(const string& x)
{
	for (const auto& right : production[x])
	{
		string tmp = string(1, right[0]);

		if (terSymbol[tmp] == true) first[x].insert(tmp);
		else if (terSymbol[right] == true) first[x].insert(right);
		else
		{
			int l = right.size();
			int j = 0;
			bool epsilon = true;

			while (j < l)
			{
				tmp = string(1, right[j]);
				if (terSymbol[tmp] != true)
				{
					if (j + 1 < l && right[j + 1] == '\'')
					{
						tmp += "'";
						++j;
					}
					if (first[tmp].size() == 0) dfsFirst(tmp);
					first[x].insert(first[tmp].begin(), first[tmp].end());
					if (first[tmp].count("¦Å") == false)
					{
						epsilon = false;
						break;
					}
				}
				else
				{
					first[x].insert(tmp);
					epsilon = false;
					break;
				}
				++j;
			}
			if (epsilon == true) first[x].insert("¦Å");
		}
	}
}

void generaFirst()
{
	for (const auto& i : production)
	{
		if (first[i.first].size() > 0) continue;
		dfsFirst(i.first);
	}
}

void dfsFollow(const string& x)
{
	for (const auto& right : production[x])
	{
		if (terSymbol[right] == true) continue;
		string tmp;
		int l = right.size();
		int j = 0;

		while (j < l)
		{
			tmp = string(1, right[j]);
			if (terSymbol[tmp] != true)
			{
				if (j + 1 < l && right[j + 1] == '\'')
				{
					tmp += "'";
					++j;
				}

				int k = j + 1;
				string nonTerminal;
				bool epsilon = true;

				while (k < l)
				{
					nonTerminal = string(1, right[k]);
					if (terSymbol[nonTerminal] != true)
					{
						if (k + 1 < l && right[k + 1] == '\'')
						{
							nonTerminal += "'";
							++k;
						}
						follow[tmp].insert(first[nonTerminal].begin(), first[nonTerminal].end());
						if (first[nonTerminal].count("¦Å") == false)
						{
							epsilon = false;
							break;
						}
					}
					else
					{
						follow[tmp].insert(nonTerminal);
						epsilon = false;
						break;
					}
					++k;
				}
				if (epsilon == true) follow[tmp].insert(follow[x].begin(), follow[x].end());
			}
			++j;
		}
	}
}

void generaFollow()
{
	follow["E"].insert("$");
	preSize["E"] = 1;
	bool flag = true;

	while (flag)
	{
		flag = false;
		visit.clear();
		for (const auto& i : production)
		{
			if (visit[i.first]) continue;
			visit[i.first] = true;
			dfsFollow(i.first);
		}
		for (const auto& i : production)
		{
			if (follow[i.first].size() > preSize[i.first]) flag = true;
			preSize[i.first] = follow[i.first].size();
		}
	}

	for (const auto& i : production)
	{
		if (follow[i.first].count("¦Å") == true) follow[i.first].erase("¦Å");
	}
}

void generAnalyTable()
{
	for (const auto& i : production)
	{
		for (const auto& right : i.second)
		{
			string tmp = string(1, right[0]);
			set<string>rightFirst;
			bool epsilon = true;

			if (terSymbol[tmp] == true)
			{
				rightFirst.insert(tmp);
				if (tmp != "¦Å") epsilon = false;
			}
			else if (terSymbol[right] == true)
			{
				rightFirst.insert(right);
				if (right != "¦Å") epsilon = false;
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
						if (j + 1 < l && right[j + 1] == '\'')
						{
							tmp += "'";
							++j;
						}
						rightFirst.insert(first[tmp].begin(), first[tmp].end());
						if (first[tmp].count("¦Å") == false)
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
			for (const auto& k : rightFirst)
			{
				if (k == "¦Å") continue;
				p = mp(i.first, k);
				analyTable[p] = i.first + "->" + right;
			}
			if (epsilon == true)
			{
				for (const auto& k : follow[i.first])
				{
					p = mp(i.first, k);
					analyTable[p] = i.first + "->" + right;
				}
			}
		}
	}
}

void analyse(ifstream& infile)
{
	lexicalAnaly(infile, symbolTable);
	for (const auto& i : symbolTable) cout << i.first << " " << i.second << endl;
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
		cout << "File Not Found£¡";
		return 0;
	}

	analyse(infile);

	return 0;
}