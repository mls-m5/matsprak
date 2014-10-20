/*
 * AstExpression.h
 *
 *  Created on: 18 okt 2014
 *      Author: Mattias Larsson Sköld
 */

#pragma once
#include <vector>

#include "ast.h"

class AstExpression: public AstContent {
public:
	AstExpression(Ast *parent): AstContent(parent) {}
	virtual ~AstExpression();

	Ast *evaluate (std::istream &stream);

	void save(std::ostream &stream, SaveTarget, int level = 0);
	std::vector<Ast*> expressions;
};

class AstBinaryOperator: public AstContent {
public:
	enum {
		Assignment,
		Add,
		Subtract,
		Multiply,
		Divide,
		Power,
	};
	AstBinaryOperator(Ast *parent): AstContent(parent) {}


	void save(std::ostream &stream, SaveTarget, int level = 0);

	~AstBinaryOperator(){
		if (ownFirst){
			delete first;
		}
		if (ownSecond){
			delete second;
		}
	}

	void setFirst(Ast* f, bool own){
		if (ownFirst){
			delete first;
		}
		first = f;
	}

	void setSecond(Ast *s, bool own){
		if (ownSecond){
			delete second;
		}
		second = s;
	}

	Ast *first = 0;
	Ast *second = 0;
	bool ownFirst = 0;
	bool ownSecond = 0;
};

class AstRawContent: public AstContent {
public:
	AstRawContent(Ast *parent, std::string content): AstContent(parent) {
		name = content;
	}

	void save(std::ostream &stream, SaveTarget, int level = 0);
};
