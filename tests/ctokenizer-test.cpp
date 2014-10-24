/*
 * ctokenizer-test.cpp
 *
 *  Created on: 23 okt 2014
 *      Author: Mattias Larsson Sköld
 */


#include "tokenizer.h"
#include "unittest.h"
#include <sstream>
using namespace std;

TEST_SUIT_BEGIN


TEST_CASE("characters"){
	{
		istringstream ss("' '");

		auto ret = Tokenizer::GetNextCTokenAfterSpace(ss);
		ASSERT_EQ(ret.type, Token::Char);
		ASSERT_EQ(ret, " ");
	}


	{ //with double citation token
		istringstream ss("'\"'");

		auto ret = Tokenizer::GetNextCTokenAfterSpace(ss);
		ASSERT_EQ(ret.type, Token::Char);
		ASSERT_EQ(ret, "\"");
	}
}

TEST_CASE("strings"){
	{
		istringstream ss("\"'\"");

		auto ret = Tokenizer::GetNextCToken(ss);
		ASSERT_EQ(ret.type, Token::String);
		ASSERT_EQ(ret, "'");
	}
}

TEST_SUIT_END


