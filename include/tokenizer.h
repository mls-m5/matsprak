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
	std::string formatOutput(){
		if (type == String){
			return "\"" + *this + "\"";
		}
		else{
			return *this;
		}
	}
	virtual ~Token(){

	}

	TokenType type;

};

class Tokenizer {
public:
	Tokenizer();
	virtual ~Tokenizer();

	Token getNextToken(std::istream &stream);
	static Token GetNextToken(std::istream &stream);
	static Token GetNextTokenAfterSpace(std::istream &stream, bool forceSpace = false);

	static Token GetNextCToken(std::istream &stream);
	static Token SkipCSpace(std::istream &stream, bool forceSpace = false);
	static Token GetNextCTokenAfterSpace(std::istream &stream, bool forceSpace = false);

	static Token SkipSpace(std::istream &stream, bool newline = false);
};

