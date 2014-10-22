/*
 * cast.h
 *
 *  Created on: 19 okt 2014
 *      Author: Mattias Larsson Sköld
 */

#pragma once
#include "ast.h"

class CAst: public Ast {
public:
	CAst(Ast *parent): Ast(parent){};
	CAst(Ast *parent, std::istream &stream):
	Ast(parent){
		load(stream);
	}
	virtual ~CAst();

	bool load(std::istream &stream);
	void save(std::ostream &stream, SaveTarget, int level = 0);

	void loadHeaderFile(std::string fileName);
	static class CAstContentBlock *CreateHeaderFromCommand(std::string headerName);
};

class CAstContentBlock: public AstContentBlock {
public:

	CAstContentBlock(Ast *parent): AstContentBlock(parent) {}
	CAstContentBlock(Ast *parent, std::istream &stream): AstContentBlock(parent) {
		load(stream);
	}
	~CAstContentBlock() {}

	bool load(std::istream &stream);
	void save(std::ostream &stream, SaveTarget, int level = 0);

	Ast *findFunction(const std::string &name);
	Ast *findVariable(const std::string &name);
};

