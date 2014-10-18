/*
 * ast.cpp
 *
 *  Created on: 17 okt 2014
 *      Author: Mattias Larsson Sköld
 */

#include "ast.h"
#include "tokenizer.h"
#include <list>
#define DEBUG if(1)

using namespace std;

std::list<Ast*> types;

//For initializing stuff
static class InitializerClass{
public:
	InitializerClass(){
		if (!types.size()){
			auto intAst = new Ast;
			intAst->type = Ast::VariableDeclaration;
			intAst->name = "int";
			types.push_back(intAst);
			auto voidAst = new Ast;
			voidAst->type = Ast::VariableDeclaration;
			voidAst->name = "void";
			types.push_back(voidAst);
		}
	}
} _initializer;

void intent(int level){
	for (int i = 0; i < level; ++i){
		cout << "     ";
	}
}

Ast::~Ast() {
}

void throwError(std::string description){
	cout << description << endl;
}

bool Ast::load(std::istream& stream) {
	Tokenizer tokenizer;
	Ast *tmp;
	auto ret = tokenizer.getNextToken(stream);
	if (!stream){
		return false; //end of stream
	}
	else if (ret == "end"){
		ret = tokenizer.getNextToken(stream);
		if (ret.type == Token::SpaceWithNewline){
			return false; //end of statement
		}
		else if (ret.type == Token::Space){
			ret = tokenizer.getNextToken(stream);
			if (ret.type == Token::Word){
				if (ret == getBlockName()){
					return false; // correct
				}
			}
		}
		throwError("expected newline or end [name of block] eg.\"" + name + "\". something wrong with nesting?");
		return false;
	}
	else if (ret == "function"){
		tokenizer.skipSpace(stream);
		ret = tokenizer.getNextToken(stream);
		tmp = findType(ret);
		if (tmp){
			DEBUG cout << "type " << tmp->name << endl;
			tokenizer.skipSpace(stream);
			ret = tokenizer.getNextToken(stream);
			dataTypePointer = tmp;
		}
		else{
			DEBUG cout << "void " << endl;
			dataTypePointer = findType("void");
		}
		DEBUG cout << "function definition " << ret << endl;
		name = ret;
		type = FunctionDefinition;

		//check for arguments
		std::string arguments;
		ret = tokenizer.getNextToken(stream);
		if (ret.type == Token::SpaceWithNewline){
			//Do nothing
		}
		else if (ret.type == Token::SpacedOutCharacter && ret == "("){
			//Todo: read argument list

			ret = tokenizer.getNextToken(stream);

			while (ret != ")"){
				//Skip arguments
				arguments += ret;
				ret = tokenizer.getNextToken(stream);
			}

			tokenizer.skipSpace(stream, true);
		}

		auto contentBlock = new AstContentFunction(this);
		contentBlock->blockName = "function";
		contentBlock->arguments = arguments;
		contentBlock->load(stream);
		content = contentBlock;
	}
	else if (ret == "call"){
		tokenizer.skipSpace(stream);
		ret = tokenizer.getNextToken(stream);
		cout << "function call " << ret << endl;
		auto functionAst = findFunction(ret);
		dataTypePointer = functionAst;
		tokenizer.skipSpace(stream, true);
		type = FunctionCall;
	}
	else if ((tmp = findType(ret))){
		tokenizer.skipSpace(stream);
		ret = tokenizer.getNextToken(stream);
		cout << "variable definition " << ret << endl;
		name = ret;
		type = VariableDeclaration;
		dataTypePointer = tmp;
	}
	else {
		//Copy verbatim or evaluate expression
	}
	return true;
}

bool AstContentBlock::load(std::istream& stream) {
	type = Block;
	Ast *command = new Ast(this);
	while (command->load(stream)){
		commands.push_back(command);
		command = new Ast(this);
	}
	delete command;
	return true;
}

void Ast::save(std::ostream& stream, int level) {
	switch (type) {
		case VariableDeclaration:
			if (!dataTypePointer){
				cout << "error: datatype pointer empty " << __FILE__ << ":" << __LINE__ << endl;
				return;
			}
			intent(level);
			stream << dataTypePointer->name << " " << name << std::endl;
			break;
		case FunctionDefinition:
		{
			if (!dataTypePointer){
				cout << "error datatype pointer empty " << __FILE__ << ":" << __LINE__ << endl;
				return;
			}
			intent(level);
			std::string arguments;
			auto functionContent = dynamic_cast<AstContentFunction*> (content);
			if (functionContent){
				arguments = functionContent->arguments;
			}
			stream << dataTypePointer->name << " " << name << "(" << arguments << "){" << std::endl;

			if (content){
				content->save(stream, level);
			}

			intent(level);
			stream << "}" << endl;
			break;

		}
		case FunctionCall:

			if (dataTypePointer){
				intent(level);
				stream << dataTypePointer->name << "();" << endl;
			}
			else{
				intent(level);
				cout << "undefined function" << endl;
			}

			break;

		default:
			break;
	}
}

Ast* Ast::findType(const std::string & name) {
	for (auto it: types){
		if (name.compare(it->name) == 0){
			return it;
		}
	}
	return 0;
}

Ast* Ast::findFunction(const std::string& name) {
	if (name == this->name){
		return this;
	}
	if (parent){
		return parent->findFunction(name);
	}
	return 0;
}

void AstContentBlock::save(std::ostream& stream, int level) {
	for (auto it: commands){
		it->save(stream, level + 1);
	}
}

Ast* AstContentBlock::findFunction(const std::string& name) {
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

std::string Ast::getBlockName() {
	switch (type) {
		case FunctionDefinition:
			return "function";
			break;
		default:
			if (parent){
				return parent->getBlockName();
			}
			else {
				return "";
			}
			break;
	}
}

AstContentBlock* Ast::createAstFromStream(std::istream& stream) {
	auto block = new AstContentBlock(0);
	block->load(stream);
	return block;
}
