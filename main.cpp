#include "ast.h"

#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]){
	std::ifstream file("Testkod/TestKlass.mcl");

	auto ast = Ast::createAstFromStream(file);
	ast->save(cout);

//	ant.compile();
}
