/*
 * ast.cpp
 *
 *  Created on: 17 okt 2014
 *      Author: Mattias Larsson Sköld
 */

#include "ast.h"
#include "tokenizer.h"
#include <list>
#include <fstream>
#include <sstream>
#include <dirent.h>

#include "AstExpression.h"
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
	if (content){
		delete content;
		content = 0;
	}
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
		tokenizer.SkipSpace(stream);
		ret = tokenizer.getNextToken(stream);
		tmp = findType(ret);
		if (tmp){
			DEBUG cout << "type " << tmp->name << endl;
			tokenizer.SkipSpace(stream);
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

			tokenizer.SkipSpace(stream, true);
		}

		auto contentBlock = new AstContentFunction(this);
		contentBlock->blockName = "function";
		contentBlock->arguments = arguments;
		contentBlock->load(stream);
		setContent(contentBlock);
	}
	else if (ret == "call"){
		tokenizer.SkipSpace(stream);
		ret = tokenizer.getNextToken(stream);
		DEBUG cout << "function call " << ret << endl;
		auto functionAst = findFunction(ret);
		dataTypePointer = functionAst;
		tokenizer.SkipSpace(stream, true);
		type = FunctionCall;
	}
	else if (ret == "using"){
		tokenizer.SkipSpace(stream);
		type = UsingStatement;
		ret = tokenizer.getNextToken(stream);
		DEBUG cout << "using-statement " << ret << endl;
		dataTypePointer = findType(ret);
		if (dataTypePointer == 0){
			throwError("type " + ret + " not found");
		}

	}
	else if ((tmp = findType(ret))){
		tokenizer.SkipSpace(stream);
		ret = tokenizer.getNextToken(stream);
		DEBUG cout << "variable definition " << ret << endl;
		name = ret;
		type = VariableDeclaration;
		dataTypePointer = tmp;
		tokenizer.SkipSpace(stream, true);
	}
	else if ((tmp = findVariable(ret))){
		type = Assignment;
		ret = Tokenizer::GetNextTokenAfterSpace(stream);
		if (ret == "="){
			setContent(new AstExpression(this));
			content->evaluate(stream);
		}
		DEBUG cout << "assignment " << ret << endl;

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

void Ast::save(std::ostream& stream, SaveTarget saveTarget, int level) {
	switch (type) {
		case VariableDeclaration:
			if (!dataTypePointer){
				cout << "error: datatype pointer empty " << __FILE__ << ":" << __LINE__ << endl;
				return;
			}
			intent(level);
			stream << dataTypePointer->name << " " << name << ";" << std::endl;
			break;
		case FunctionDefinition:
		{
			if (!dataTypePointer){
				cout << "error datatype pointer empty " << __FILE__ << ":" << __LINE__ << endl;
				return;
			}
			std::string arguments;
			auto functionContent = dynamic_cast<AstContentFunction*> (content);
			if (functionContent){
				arguments = functionContent->arguments;
			}
			std::string displayName = name;
			bool suppressReturnType = false;
			if (name == "destroy"){
				if (parent){
					displayName = "~" + parent->name;
					suppressReturnType = true;
				}
			}
			else if (name == "init"){
				if (parent){
					displayName = parent->name;
					suppressReturnType = true;
				}
			}

			if (saveTarget == Source){
				if (!suppressReturnType){
					stream << dataTypePointer->name << " ";
				}
				stream << getNameSpace() << "::" <<  displayName << "(" << arguments << "){" << std::endl;

				if (content){
					content->save(stream, saveTarget, level);
				}

				stream << "}" << endl;
			}
			else {
				intent(level);
				if (!suppressReturnType){
					stream << dataTypePointer->name << " ";
				}
				stream << displayName << "(" << arguments << ");" << std::endl;
			}

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
	//Try to find type on file system
	return 0;
}

Ast* Ast::findFunction(const std::string& name) {
	if (parent){
		return parent->findFunction(name);
	}
	if (name == this->name){
		return this;
	}
	return 0;
}

Ast* Ast::findVariable(const std::string& name) {
	if (parent){
		return parent->findVariable(name);
	}
	if (name == this->name){
		return this;
	}
	return 0;
}

void AstContentBlock::save(std::ostream& stream, SaveTarget saveTarget, int level) {
	for (auto it: commands){
		it->save(stream, saveTarget, level + 1);
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

std::string Ast::getNameSpace() {
	if (parent){
		return parent->getNameSpace();
	}
	else {
		return "";
	}
}
std::string Ast::getClassFileName(const std::string className){
	string fileName = className;
	for (auto &it: fileName){
		if (it == '.'){
			it = '/';
		}
	}
	return fileName + ".mcl";
}

Ast* Ast::LoadClassFile(const std::string& fileName) {
	auto ret = FindType(fileName);
	if (ret){
		return ret;
	}
	string className = fileName;
	for (int i = 0; i < 4; ++i){
		className.erase(className.size()-1);
	}
	for (auto &it: className){
		if (it == '/'){
			it = '.';
		}
	}
	char previous = '\0';
	for (auto it = className.begin(); it != className.end();){
		if (*it == '.' && previous == '.'){
			it = className.erase(it);
			++it;
		}
		else {
			++it;
		}
		previous = *it;
	}
	if (className[0] == '.'){
		className.erase(0, 1);
	}
	className[0] = toupper(className[0]);
	fstream file(fileName);
	if (file.is_open()){
		auto ast = new AstContentBlock(0);
		if (ast->load(file)){
			ast->name = className;
			types.push_back(ast);
			DEBUG cout << "class " << className << " loaded" << endl;
			return ast;
		}
		else {
			delete ast;
		}
	}

	return 0;
}



Ast* Ast::FindType(const std::string& name) {
	for (auto it: types){
		if (it->name == name){
			return it;
		}
	}
	return 0;
}

void Ast::LoadClassFolder(const std::string folder) {
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir (folder.c_str())) != NULL) {
		//Gå igenom alla filer i mappen
		while ((ent = readdir (dir)) != NULL) {
			//			cout << ent->d_name << endl;
			string dName = ent->d_name;
			if (dName == "." || dName == ".."){
				continue;
			}
			if (dName.find(".mcl") != dName.size() - 4){
				continue;
			}

			string classFile = string(folder + "/" + ent->d_name);
			LoadClassFile(classFile);
		}
		closedir (dir);
	} else {
		cerr << "Kunde inte öppna mapp " << folder << endl;
		return;
	}
}

void Ast::setContent(class AstContent* content) {
	if (this->content) {
		delete this->content;
	}
	this->content = content;
}

AstContentBlock* Ast::createAstFromStream(std::istream& stream) {
	auto block = new AstContentBlock(0);
	block->load(stream);
	return block;
}

std::string AstContentBlock::getNameSpace() {
	if (parent){
		return parent->getNameSpace() + "::" + parent->name;
	}
	else{
		return name;
	}
}

void AstContentBlock::printClassHeader(std::ostream& stream) {
//	stream << "#pragma once" << endl;
	stream << "class " << name << "{" << endl;
	stream << "public:" << endl;
}

Ast* Ast::evaluate(std::istream &stream) {
	auto expression = new AstExpression(this);
	expression->evaluate(stream);
	return expression;
}

Ast* Ast::evaluate(std::string str) {
	istringstream ss(str);
	evaluate(ss);
}

Ast* AstContentBlock::findVariable(const std::string& name) {
	for (auto it: commands){
		if (it->type == VariableDeclaration){
			if (it->name == name){
				return it;
			}
		}
	}
	return Ast::findVariable(name);
}
