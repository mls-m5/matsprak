/*
 * tokenizer.h
 *
 *  Created on: 13 okt 2014
 *      Author: Mattias Larsson Sköld
 */

#pragma once
#include <iostream>
#include <string>

class Token: public std::string{
public:
	enum TokenType {
		Space,
		SpaceWithNewline,
		Word,
		KeyWord,
		Digit,
		String,
		Char,
		StartScope,
		SpacedOutCharacter,
		PreprocessorCommand,
		None
	};
	Token (std::string str, TokenType type):
		type(type){
		this->std::string::operator=(str);
	}

	TokenType type;

};

class Tokenizer {
public:


	Tokenizer();
	virtual ~Tokenizer();

	Token getNextToken(std::istream &stream);
	Token skipSpace(std::istream &stream, bool newline = false);
};

