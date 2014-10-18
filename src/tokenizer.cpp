/*
 * tokenizer.cpp
 *
 *  Created on: 13 okt 2014
 *      Author: Mattias Larsson Sköld
 */

#include "tokenizer.h"

#include <sstream>
#include <vector>
#include <fstream>
using namespace std;

string specialCharacters = "_{}[]#()<>%:;.?*+-/^&|∼!=,\'";

static class KeyWords: public vector<string>{
public:
	KeyWords(){
		ifstream file("keywords.txt");
		string word;
		while (file >> word){
			push_back(word);
		}
	}

	bool isKeyWord(const string &c){
		for (auto &it: *this){
			if (it == c){
				return true;
			}
		}
		return false;
	}
} keyWords;

Tokenizer::Tokenizer() {
}

Tokenizer::~Tokenizer() {
}

Token Tokenizer::getNextToken(std::istream& stream) {
	stringstream ss;
	bool hasNewLine = false;

	Token::TokenType mode = Token::None;

	while (stream){
		char c = stream.get();
		switch (mode){
		case Token::None:
		{
			if (isspace(c)){
				if (c == '\n'){
					hasNewLine = true;
				}
				ss.put(c);
				mode = Token::Space;
				continue;
			}
			else if (isalpha(c)){
				ss.put(c);
				mode = Token::Word;
				continue;
			}
			else if (isdigit(c) || c == '.'){
				ss.put(c);
				mode = Token::Digit;
				continue;
			}
			else if(c == '"'){
				mode = Token::String;
				continue;
			}
			else if(c == '#'){
				mode = Token::PreprocessorCommand;
				ss.put(c);

				char line[500];
				stream.getline(line, 500);
				ss << line;
				return Token(ss.str(), Token::PreprocessorCommand);
			}
			else if(specialCharacters.find(c) != string::npos && stream){
				mode = Token::SpacedOutCharacter;
				ss.put(c);
				return Token(ss.str(), Token::SpacedOutCharacter);
			}
		}
		break;
		case Token::Space:
			while (isspace(c) && stream){
				if (c == '\n'){
					hasNewLine = true;
				}
				ss.put(c);
				c = stream.get();
			}
			stream.unget();
			return Token(ss.str(), hasNewLine? Token::SpaceWithNewline : Token::Space);

		case Token::Word:
		{
			while (isalnum(c) && stream){
				ss.put(c);
				c = stream.get();
			}
			stream.unget();
			bool isKeyword = keyWords.isKeyWord(ss.str());
			return Token(ss.str(), isKeyword? Token::KeyWord : Token::Word);
		}

		case Token::Digit:
			while ((isdigit(c) || c == '.') && stream){
				ss.put(c);
				c = stream.get();
			}
			stream.unget();
			return Token(ss.str(), Token::Digit);

		case Token::String:
			while (c != '"' && stream){
				ss.put(c);
				c = stream.get();
				if (c == '\\'){
					ss.put(c);
					c = stream.get();
					ss.put(c);
					c = stream.get();
				}
			}

			//No unget here, we dont want to keep trailing '"'
			return Token(ss.str(), Token::String);
		}
	}

	return Token(ss.str(), Token::None);
}

Token Tokenizer::skipSpace(std::istream& stream, bool newline) {
	auto token = getNextToken(stream);
	if (newline){
		if (token.type == Token::SpaceWithNewline){
			return token;
		}
	}
	else if (token.type == Token::Space){
		return token;
	}
	cout << "error.. not space" << endl;
	//Todo error handling
	return token;
}
