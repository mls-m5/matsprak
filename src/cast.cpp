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

CAst::~CAst() {
}

//Skip content of prackets
void skipBrackets(std::istream &stream, std::string begin, std::string end){
	auto token = Tokenizer::GetNextCTokenAfterSpace(stream);
	while (token != end){
		token = Tokenizer::GetNextCTokenAfterSpace(stream);
		if (token == begin){
			skipBrackets(stream, begin, end);
		}
	}
}

bool CAst::load(std::istream& stream) {
	if (!stream){
		return false;
	}

	auto token = Tokenizer::GetNextCTokenAfterSpace(stream);
	Ast* tmp = 0;

	while (token.type == Token::PreprocessorCommand or token == "C" or token == "{" or token == "}"){
		token = Tokenizer::GetNextCTokenAfterSpace(stream);
		if (!stream){
			return false;
		}
	}

	if (token == "const"){
		constExpression = true;
		token = Tokenizer::GetNextCTokenAfterSpace(stream);
	}

	if (token == "typedef" or token == "struct"){
		type = Typedef;

		//cheeting
		auto previous = token = Tokenizer::GetNextCTokenAfterSpace(stream);
		while (token != ";" and stream){
			previous = token;
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
			if (token == "{"){
				skipBrackets(stream, "{", "}");
			}
		}
		name = previous; //take the last word before ";"
		DEBUG std::cout << "typedef or struct " << name << std::endl;
	}
//	else if (token == "struct"){
//		name = token;
//	}
	else if (token == "enum"){
		token = Tokenizer::GetNextCTokenAfterSpace(stream);
		type = Ast::Enum;
		name = token;
		skipBrackets(stream, "{", "}");
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
		std::string dataTypeName = token;
		type = VariableDeclaration;

		token = Tokenizer::GetNextCTokenAfterSpace(stream);
		while (FindType(token)){ //locking for double word basic types
			dataTypeName += " " + token;
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
			tmp = FindType(dataTypeName);
		}
		dataTypePointer = tmp;

		while (token == "*" and stream){
			token = Tokenizer::GetNextCTokenAfterSpace(stream);
			pointerDepth ++;
		}

		if (token.type == Token::Word){
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

			if (token == "("){
				type = FunctionDefinition;

				//Todo.. handle arguments
				skipBrackets(stream, "(", ")");
				DEBUG std::cout << "function " << name << std::endl;
				token = Tokenizer::GetNextCTokenAfterSpace(stream);
				if (token == ";"){
					return true;
				}
				else{
					throwError("expected ;");
					return false;
				}
			}
			else if (token == ";"){
				return true; //done
			}
			else {
				while (token == "," and stream){
					token = Tokenizer::GetNextCTokenAfterSpace(stream);
					//Todo.. add these variables as well
					token = Tokenizer::GetNextCTokenAfterSpace(stream);
				}
				if (token == ";"){
					return true;
				}
				else{
					throwError("expected ;");
				}
			}
		}
		//declaration of something
	}
	else {
		auto ret = findType(token);
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
	std::string definesToMakeThingsEasier = "#define extern\n"
			"#define __attribute__(x)\n"
			"#define __format__(x)\n"
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

