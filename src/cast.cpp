/*
 * cast.cpp
 *
 *  Created on: 19 okt 2014
 *      Author: Mattias Larsson Sköld
 */

#include "cast.h"

#include "tokenizer.h"
#include <sstream>
#define DEBUG if (0)
using namespace std;

CAst::~CAst() {
}

//Skip content of prackets
static void skipBrackets(std::istream &stream, std::string begin, std::string end){
	auto token = Tokenizer::GetNextCTokenAfterSpace(stream);
	while (token != end){
		token = Tokenizer::GetNextCTokenAfterSpace(stream);
		if (token == begin){
			skipBrackets(stream, begin, end);
		}
	}
}

bool CAst::load(std::istream& stream) {
	using namespace std;
	if (!stream){
		return false;
	}

	auto token = Tokenizer::GetNextCTokenAfterSpace(stream);
	Ast* tmp = 0;

	while (token.type == Token::PreprocessorCommand or token == "C" or token == "C++" or token == "{" or token == "}"){
		token = Tokenizer::GetNextCTokenAfterSpace(stream);
		if (!stream){
			return false;
		}
	}

	if (token == "static"){
		staticExpression = true;
		token = Tokenizer::GetNextCTokenAfterSpace(stream);
	}

	if (token == "inline"){
		inlineExpression = true;
		token = Tokenizer::GetNextCTokenAfterSpace(stream);
	}

	if (token == "const"){
		constExpression = true;
		token = Tokenizer::GetNextCTokenAfterSpace(stream);
	}

	DEBUG std::cout << "token: " << token << std::endl;

	if (token == "struct" or token == "union"){
		type = Struct;
		token = Tokenizer::GetNextCTokenAfterSpace(stream);
		DEBUG std::cout << "union or struct " << token << std::endl;
		if (token == "{"){
			DEBUG cout << "anonymous struct" << endl;
			name = "<anonymous>";
		}
		else {
			//Not anonymous
			if ((tmp = findType(token))){
				//Forward declaration? Todo: Check if it is that or struct declaration
				dataTypePointer = tmp;
				if (token == ";"){
					DEBUG cout << "redefinition of forward declaration.. " << endl;
					Tokenizer::SkipCSpace(stream);
					return true; //Todo: Return true? it will be a duplicate
				}
			}
			name = token;
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
		}

		if (token == ";"){
			DEBUG cout << "forward declaration" << endl;
			return true;
		}
		else {
			//Divide into two asts, one for the struct and the other
			//for defining the variable
			if (parent){
				if (token == "{"){
					skipBrackets(stream, "{", "}");
				}
				token = Tokenizer::GetNextCTokenAfterSpace(stream);
				if (token == ";"){
					DEBUG cout << "standard struct declaration";
					return true;
				}
				auto cast = new CAst(parent);
				cast->name = name;
				cast->type = type;
				auto blockParent = dynamic_cast<AstContentBlock*>(parent);
				blockParent->commands.push_back(cast);

				name = token;
				type = VariableDeclaration;

				token = Tokenizer::GetNextCTokenAfterSpace(stream);
				DEBUG cout << "struct + variable declaration" << endl;
				if (token != ";"){
					throwError("expected ';' after struct declaration");
				}
				return true;
			}
			else{
				//cannot do it without parent
			}
		}


	}
	else if (token == "typedef"){
		type = Typedef;

		token = Tokenizer::GetNextCTokenAfterSpace(stream);

		if (token == "struct" or token == "union" or token == "enum"){
			Ast::AstType typedefType;
			DEBUG cout << token << " declaration through typedef" << endl;
			if (token == "enum"){
				typedefType = Ast::Enum;
			} else if (token == "struct"){
				typedefType = Ast::Struct;
			}
			else {
				typedefType = Ast::Union;
			}
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
			if (token == "{"){
				if (typedefType == Ast::Enum){
					loadEnumContent(stream);
				}
				else {
					skipBrackets(stream, "{", "}"); //Todo: Load struct to the anonymous type
				}

				token = Tokenizer::GetNextCTokenAfterSpace(stream);
				if (token.type == Token::Word){
					name = token;
					token = Tokenizer::GetNextCTokenAfterSpace(stream);

					if (token == ";"){
						if (parent){
							auto ast = new CAst(parent);
							ast->name = "<anonymous>";
							ast->type = typedefType;
							dataTypePointer = ast;
							((CAstContentBlock*)parent)->commands.push_back(ast);
							DEBUG cout << "anonymous struct/union/enum " << name << endl;
						}

						return true;
					}
					else {
						throwError("expected ; at end of statement");
						return false;
					}
				}
				else{
					throwError("expected typedef name");
					return false;
				}
			}
			else {
				if ((tmp = findType(token))){
					dataTypePointer = tmp;
					//The type exists no need to forward declare
					token = Tokenizer::GetNextCTokenAfterSpace(stream);
				}
				else {
					if (parent){
						auto ast = new CAst(parent);
						ast->name = token;
						ast->type = Struct;
						((CAstContentBlock*)parent)->commands.push_back(ast);
						DEBUG cout << "struct  " << token << endl;
						dataTypePointer = ast;
					}
					token = Tokenizer::GetNextCTokenAfterSpace(stream);
				}
				if (token == "{"){
					if (type == Ast::Enum){
						loadEnumContent(stream);
					}
					else {
						skipBrackets(stream, "{", "}"); //Todo: Load struct to the anonymous type
					}
					DEBUG cout << "skipped content" << endl;
					token = Tokenizer::GetNextCTokenAfterSpace(stream);
				}
				//continue to define type
			}
		}
		else {

			std::string dataTypeName = token;
			tmp = findType(token);
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
			while (FindType(token)){ //locking for double word basic types
				dataTypeName += " " + token;
				token = Tokenizer::GetNextCTokenAfterSpace(stream);
				tmp = FindType(dataTypeName);
			}
			dataTypePointer = tmp;

		}

		while (token == "*" and stream){
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
			pointerDepth ++;
		}

		if (token == "("){
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
			if (token == "*"){
				token = Tokenizer::GetNextCTokenAfterSpace(stream);
				if (token.type == Token::Word){
					name = token;
					token = Tokenizer::GetNextCTokenAfterSpace(stream);
					if (token != ")"){
						throwError("expected ')' after function pointer name");
						return false;
					}
					token = Tokenizer::GetNextCTokenAfterSpace(stream);
					type = TypedefFunctionPointer;

					if (token == "("){
						skipBrackets(stream, "(", ")");
						token = Tokenizer::GetNextCTokenAfterSpace(stream);
						if (token == ";"){
							DEBUG std::cout << "typedef function pointer " << name << std::endl;
							return true;
						}
						else {
							throwError("expected ; after statement");
						}
					}
					else{
						throwError("expected arguments for typedef function pointer");
						return false;
					}
				}
			}
			else{
				throwError("expected * for function pointer");
				return false;
			}
		}
		else if (token.type == Token::Word){
			name = token; //take the last word before ";"
			type = Typedef;

			token = Tokenizer::GetNextCTokenAfterSpace(stream);

			if (token == ";"){
				DEBUG cout << "typedef " << name << endl;
			}
			else if (token == "("){
				skipBrackets(stream, "(", ")");
				token = Tokenizer::GetNextCTokenAfterSpace(stream);
				if (token == ";"){
					DEBUG cout << "typedef function (no pointer) " << name << endl;
					return true;
				}
				else{
					throwError("expected ; after function typedef definition");
				}
			}
			else if (token == "["){
				skipBrackets(stream, "[", "]");
				token = Tokenizer::GetNextTokenAfterSpace(stream);
				pointerDepth ++;
				if (token == ";"){
					DEBUG cout << "typedef array " << name << endl;
					return true;
				}
				else{
					throwError("expected ; after function typedef definition");
				}
			}
			else {
				throwError("expected ; after statement");
			}
		}
		else{
			throwError("expected string to have as name for typedef");
		}

	}
	else if (token == "enum"){
		token = Tokenizer::GetNextCTokenAfterSpace(stream);
		type = Ast::Enum;
		if (token.type == Token::Word){
			name = token;
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
		}
		else {
			name = "<anonymous enum>";
		}

//		skipBrackets(stream, "{", "}");
		loadEnumContent(stream);

		token = Tokenizer::GetNextCTokenAfterSpace(stream);
		if (token.type == Token::Word){
			//Name of instantiation of enum
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
			if (token != ";"){
				throwError("expected \";\"");
			}
		}
		else if (token != ";"){
			throwError("unexpected token: " + token);
		}
	}
	else if ((tmp = findType(token))){
		token = Tokenizer::GetNextCTokenAfterSpace(stream);
		std::string dataTypeName = tmp->name;
		type = VariableDeclaration;

		while (FindType(token)){ //locking for double word basic types
			dataTypeName += " " + token;
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
			tmp = FindType(dataTypeName);
		}
		dataTypePointer = tmp;

		doAnotherTime:

		while (token == "*" and stream){
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
			pointerDepth ++;
		}

		if (token == "("){
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
			if (token == "*"){
				token = Tokenizer::GetNextCTokenAfterSpace(stream);
				if (token.type == Token::Word){
					name = token;
					token = Tokenizer::GetNextCTokenAfterSpace(stream);
					if (token != ")"){
						throwError("expected ')' after function pointer name");
						return false;
					}
					token = Tokenizer::GetNextCTokenAfterSpace(stream);
					type = FunctionPointer;
				}
			}
			else{
				throwError("expected * for function pointer");
				return false;
			}
		}
		else if (token.type == Token::Word){
			if (token == "const"){
				//todo: implement this
				token = Tokenizer::GetNextCTokenAfterSpace(stream);
			}
			name = token;
			token = Tokenizer::GetNextCTokenAfterSpace(stream);

			while (token == "["){
				skipBrackets(stream, "[", "]");
				pointerDepth ++;
				token = Tokenizer::GetNextCTokenAfterSpace(stream);
			}
		}
		else {
			return false;
		}

		if (token == "("){
			if (type != FunctionPointer){
				type = FunctionDefinition;
				DEBUG std::cout << "function declaration " << name << std::endl;
			}
			else{
				DEBUG std::cout << "function pointer " << name << std::endl;
			}

			//Todo.. handle arguments
			skipBrackets(stream, "(", ")");
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
			if (token == ";"){
				return true;
			}
			else if (token == "{"){
				skipBrackets(stream, "{", "}");
			}
			else{
				throwError("expected ; or {");
				return false;
			}
		}
		else if (token == ";"){
			DEBUG std::cout << "variable declaration " << name << std::endl;
			return true; //done
		}
		else {
			while (token == "," and stream){
				auto blockParent = dynamic_cast<AstContentBlock*> (parent);
				if (blockParent){
					auto ast = new CAst(parent);
					ast->type = type;
					ast->name = name;
					ast->pointerDepth = pointerDepth;
					ast->constExpression = constExpression;
					ast->dataTypePointer = dataTypePointer;
					ast->staticExpression = staticExpression;
					blockParent->commands.push_back(ast);

					pointerDepth = 0;
					token = Tokenizer::GetNextCTokenAfterSpace(stream);
					goto doAnotherTime;
				}
//				token = Tokenizer::GetNextCTokenAfterSpace(stream);
//				//Todo.. add these variables as well
//				token = Tokenizer::GetNextCTokenAfterSpace(stream);
			}
			if (token == ";"){
				return true;
			}
			else{
				throwError("expected ;");
			}
		}
		//declaration of something
	}
	else {
		return false;
	}
	return true;
}

void CAst::save(std::ostream& stream, SaveTarget saveTarget, int level) {
}

void CAst::loadHeaderFile(std::string fileName) {
}

Ast* CAstContentBlock::findFunction(const std::string& name) {
	if (name == this->name){
		return this;
	}
	for (auto it: commands){
		if (it->type == FunctionDefinition){
			if (it->name == name){
				return it;
			}
		}
	}
	if (parent){
		return parent->findFunction(name);
	}
	return 0;
}

Ast* CAstContentBlock::findVariable(const std::string& name) {
	for (auto it: commands){
		if (it->type == VariableDeclaration){
			if (it->name == name){
				return it;
			}
		}
	}
	return Ast::findVariable(name);
}

bool CAstContentBlock::load(std::istream& stream) {
	type = Block;
	CAst *command = new CAst(this);
	while (command->load(stream)){
		if (command->type == Include){
			//Before everything
			commands.insert(commands.begin(), command);
		}
		else{
			commands.push_back(command);
		}
		command = new CAst(this);
	}
	delete command;
	return true;
}

void CAstContentBlock::save(std::ostream& stream, SaveTarget saveTarget,
		int level) {
}

std::string runGccCommand(std::string cmd){
	DEBUG std::cout << "running " << cmd << std::endl;

	FILE* pipe;

	std::ostringstream ss;

	const int MAX_BUFFER = 2048;
	char buffer[MAX_BUFFER];
	FILE *file = popen(cmd.c_str(), "r");
	if (file){
		while (!feof(file))
		{
			if (fgets(buffer, MAX_BUFFER, file) != NULL)
			{
				ss << buffer;
			}
		}
		pclose(file);
	}

	return ss.str();
}
class CAstContentBlock* CAst::CreateHeaderFromCommand(std::string headerName) {
	std::string definesToMakeThingsEasier =
			"#define extern\n" //Remove unused keywords and macros
			"#define __attribute__(x)\n"
			"#define __format__(x)\n"
			"#define __inline inline\n"
			"#define __inline__ inline\n"
			"#define __extension__\n"
			"#define __asm(x)\n"
			"#define throw(x)\n";

	std::string commandLine = "echo '" + definesToMakeThingsEasier
	+ headerName + "' | g++ -x c++ -E -";

	auto ret = runGccCommand(commandLine);
	std::istringstream ss(ret);

	auto ast = new CAstContentBlock(0);
	if (ast->load(ss)){
		return ast;
	}
	else{
		delete ast;
		return 0;
	}
}

void CAst::loadEnumContent(std::istream& stream) {
	auto blockParent = dynamic_cast<AstContentBlock*>(parent);
	if (!blockParent){
		skipBrackets(stream, "{", "}");
		return;
	}

	auto token = Tokenizer::GetNextCTokenAfterSpace(stream);
	while (token != "}"){
		auto ast = new Ast(parent);
		ast->name = token;
		ast->type = Ast::VariableDeclaration;
		ast->constExpression = true;
		ast->dataTypePointer = this;
		blockParent->commands.push_back(ast);
		while (token != "}"){
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
			if (token == ","){
				token = Tokenizer::GetNextCTokenAfterSpace(stream);
				break;
			}
		}
	}
}
