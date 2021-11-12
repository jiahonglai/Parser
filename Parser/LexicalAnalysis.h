#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>

using namespace std;

void lexicalAnaly(ifstream& infile, vector<pair<string, string>>& symbolTable);