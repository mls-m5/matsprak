#include "ast.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;


int compile(string code){
	auto cmd = "g++ -x c++ -c -o test.out -";
	cout << cmd << endl;

	FILE* pipe;
	pipe = popen(cmd, "w");
	if (!pipe) return 1;

	fprintf(pipe, code.c_str());
	pclose(pipe);
}


int main(int argc, char *argv[]){
	string file("Testkod/simple.mcl");

	stringstream ss;

	auto ast = Ast::LoadClassFile(file);
	cout << "header" << endl;
	ast->name = "Simple";
	ast->save(ss, Ast::Header);
	ss << "};" << endl;


	cout << endl << "source: " << endl;
	ast->save(ss, Ast::Source);

	for (auto it: ss.str()){
		cout.put(it);
	}

	cout << endl << "running gcc" << endl;
	compile(ss.str());

//	ant.compile();
}
