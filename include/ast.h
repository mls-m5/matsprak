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
#include <cmath>
#include <iosfwd>


class Ast {
public:
	Ast(Ast* parent = 0): parent(parent) {}
	virtual ~Ast();

	enum SaveTarget{
		Header,
		Source
	};

	//Return true if loaded
	virtual bool load(std::istream &stream);
	virtual void save(std::ostream &stream, SaveTarget, int level = 0);

	enum AstType{
		None,
		FunctionDefinition,
		VariableDeclaration,
		Typedef,
		FunctionCall,
		Enum,
		UsingStatement,
		Block,
		Assignment,
		Include,
		ClassHeader,
		ClassEnding,
	} type = None;

	Ast *dataTypePointer = 0;
	class AstContent *content = 0;

	virtual Ast *findType(const std::string &name);

	static Ast *FindType(const std::string &name);

	static std::string getClassFileName(const std::string className);
	static Ast *LoadClassFile(const std::string &classFile);
	static void LoadClassFolder(const std::string folder);
	virtual Ast *findFunction(const std::string &name);
	virtual Ast *findVariable(const std::string &name);
	virtual std::string getBlockName();
	virtual std::string getNameSpace();
	virtual Ast* evaluate(std::istream &);
	virtual Ast* evaluate(std::string str);
	void printClassHeader(std::ostream &stream);

	void setContent(class AstContent* content);

	std::string name;
	Ast *parent = 0;
	int pointerDepth = 0;
	bool constExpression = false;

	static class AstContentBlock *createAstFromStream(std::istream &stream);

protected:

	void throwError(std::string description);
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
	void save(std::ostream &stream, SaveTarget, int level = 0);

	Ast *findFunction(const std::string &name);
	Ast *findVariable(const std::string &name);
	Ast *findType(const std::string &name);
	virtual std::string getNameSpace();

	std::vector <class Ast *> commands;

	std::string blockName;
};

class AstContentFunction : public AstContentBlock {
public:

	AstContentFunction (Ast *parent): AstContentBlock(parent) {
	}

	std::string arguments;
};
