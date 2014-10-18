/*
 * ast.h
 *
 *  Created on: 17 okt 2014
 *      Author: Mattias Larsson Sköld
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>


class Ast {
public:
	Ast(Ast* parent = 0): parent(parent) {}
	virtual ~Ast();

	//Return true if loaded
	virtual bool load(std::istream &stream);
	virtual void save(std::ostream &stream, int intentation = 0);

	enum AstType{
		FunctionDefinition,
		VariableDeclaration,
		FunctionCall,
		Block,
		Assignment,
		None
	} type = None;

	Ast *dataTypePointer = 0;
	class AstContent *content = 0;

	virtual Ast *findType(const std::string &name);
	virtual Ast *findFunction(const std::string &name);
	virtual std::string getBlockName();

	std::string name;
	Ast *parent;

	static class AstContentBlock *createAstFromStream(std::istream &stream);
};

class AstContent: public Ast {
public:
	AstContent(Ast *parent): Ast(parent) {}
};

class AstContentBlock : public AstContent {
public:
	AstContentBlock(Ast *parent): AstContent(parent){
	}
	~AstContentBlock(){
		for (auto it: commands){
			delete it;
		}
	}

	bool load(std::istream &stream);
	void save(std::ostream &stream, int intendation = 0);

	Ast *findFunction(const std::string &name);

	std::vector <class Ast *> commands;

	std::string blockName;
};

class AstContentFunction : public AstContentBlock {
public:

	AstContentFunction (Ast *parent): AstContentBlock(parent) {
	}

	std::string arguments;
};

