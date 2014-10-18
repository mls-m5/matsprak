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
	std::vector<Ast*> expressions;
};

class AstBinaryOperator: public AstContent {
public:
	AstBinaryOperator(Ast *parent): AstContent(parent) {}

	Ast *first = 0;
	Ast *second = 0;
};

class AstRawContent: public AstContent {
public:
	AstRawContent(Ast *parent, std::string content): AstContent(parent) {
		name = content;
	}
};
