/*
 * tokenizer-test.cpp
 *
 *  Created on: 17 okt 2014
 *      Author: Mattias Larsson Sköld
 */

#include "tokenizer.h"
#include "unittest.h"
#include <sstream>

using namespace std;

TEST_SUIT_BEGIN

TEST_CASE("new line test"){

	istringstream ss1("  ");
	istringstream ss2(" \n ");

	Tokenizer tokenizer;
	auto tok = tokenizer.getNextToken(ss1);

	ASSERT_EQ(tok.type, Token::Space);

	auto tok2 = tokenizer.getNextToken(ss2);
	ASSERT_EQ(tok2.type, Token::SpaceWithNewline);

	return 0;
}

TEST_CASE("load file"){

	return 0;
}


TEST_SUIT_END


