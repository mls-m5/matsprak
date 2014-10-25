/*
 * AstExpression.cpp
 *
 *  Created on: 18 okt 2014
 *      Author: Mattias Larsson Sköld
 */

#include "AstExpression.h"

#include "tokenizer.h"

AstExpression::~AstExpression() {
}

Ast* AstExpression::evaluate(std::istream& stream) {
	auto token = Tokenizer::GetNextToken(stream);
	auto var = parent->findVariable(token);

	while ((token = Tokenizer::GetNextTokenAfterSpace(stream)).type != Token::SpaceWithNewline
			and stream){
		auto expression = new AstRawContent(this, token);
		expressions.push_back(expression);
	}

	return var;

	//Todo: implement operators and functionlookups etc..
}

void AstExpression::save(std::ostream& stream, SaveTarget saveTarget,
		int level) {
	for (auto it: expressions){
		stream << " ";
		it->save(stream, saveTarget, level);
	}
}

void AstRawContent::save(std::ostream& stream, SaveTarget saveTarget,
		int level) {
	stream << name;
}

void AstBinaryOperator::save(std::ostream& stream, SaveTarget saveTarget,
		int level) {
	if (first and second){
		stream << first->name; //Todo: Fix this in some way
		stream << " " << name << " ";
		second->save(stream, saveTarget, level);
	}
}
