/*
 * ast-generator-test.cpp
 *
 *  Created on: 17 okt 2014
 *      Author: Mattias Larsson Sköld
 */



#include "unittest.h"
#include "ast.h"
#include <sstream>
#include <memory>

using namespace std;

TEST_SUIT_BEGIN

TEST_CASE("basic test"){
	Ast ast;

	istringstream ss("function apa(int x) \n"
			"call apa 1\n"
			"call bepa \n"
			"int i \n"
			"end function \n");

	ast.load(ss);
	ast.save(cout, Ast::Source);

	ASSERT_EQ(ast.type, Ast::FunctionDefinition);
	ASSERT_EQ(ast.name, "apa");
	ASSERT(ast.content, "apa har definitivt ingen argument lista");

	ASSERT_NE(ast.content, 0);
	auto content = (AstContentBlock*)ast.content;
	ASSERT_GT(content->commands.size(), 0);
	ASSERT_EQ(content->commands[0]->type, Ast::FunctionCall);
	ASSERT_EQ(content->commands[0]->dataTypePointer, &ast);

	return 0;
}

TEST_CASE("find defined functions"){
	istringstream ss ("function apa(int x) \n"
			"call apa \n"
			"end function \n"
			"function bepa() \n"
			"call apa\n"
			"end function\n");

	auto block = Ast::createAstFromStream(ss);

	ASSERT(block->findFunction("apa"), "function apa undefined");
	ASSERT(block->findFunction("bepa"), "function bepa undefined");
	ASSERT(!block->findFunction("cepa"), "function cepa should not be defined");

	block->save(cout, Ast::Source);

	if (block){
		delete block;
	}

	return 0;
}

TEST_CASE("function arguments"){
	istringstream ss("function apa(int x)\n"
			"end function\n");
}

TEST_CASE("command type test"){

	istringstream ss1("function apa()\n");

	Ast ast;
	ast.load(ss1);

	ASSERT_EQ(ast.type, Ast::FunctionDefinition);

	istringstream ss2("int apa\n");

	Ast ast2;
	ast2.load(ss2);
	ASSERT_EQ(ast2.type, Ast::VariableDeclaration);

	return 0;
}

//TEST_CASE("load class folder"){
//	Ast::LoadClassFolder(".");
//	auto ret = Ast::FindType("TestKlass");
//
//	ASSERT(ret, "type not found?");
//
//	return 0;
//}

TEST_CASE("evaluate expression"){
	AstContentBlock ast(0);
	istringstream ss("int x\n"
			"int y\n"
			"static int z\n"
			"z = x + y\n"
			"z = x + y * z ^ 2\n"
			"int w = x + y + z\n");
	ast.load(ss);
	ast.save(cout, Ast::Header, 0);

	ASSERT_EQ(ast.commands.size(), 6);
	ASSERT_EQ(ast.commands[3]->type, Ast::Assignment);
	ASSERT_EQ(ast.commands[4]->type, Ast::Assignment);
	ASSERT(ast.commands[2]->staticExpression, "z should be static");

	return 0;
}

TEST_CASE("call printf"){
	AstContentBlock ast(0);
	istringstream ss("include stdio\n"
			"function apa()\n"
			"call printf (\"hej\")\n"
			"end function\n");
	ast.load(ss);

	return 0;
}

TEST_CASE("call SDL_Init"){
	AstContentBlock ast(0);
	istringstream ss("include SDL2.SDL\n"
			"function apa()\n"
			"call SDL_Init SDL_INIT_VIDEO\n"
			"end function\n");
	ast.load(ss);

	return 0;
}

TEST_CASE("forward declared functions"){
	istringstream ss("function apa() \n"
			"call bepa\n"
			"end function \n"
			"function bepa()\n"
			"end function()\n");

	AstContentBlock ast(0);
	ast.load(ss);

	auto ret = ast.findFunction("apa");
	ASSERT(ret, "first function not declared");
	auto content = dynamic_cast<AstContentBlock*>(ret->content);
	ASSERT(content, "content not declared or wrong type");
	ASSERT_GT(content->commands.size(), 0);
	ASSERT(content->commands.front()->dataTypePointer, "bepa not found");
	ASSERT_EQ(content->commands.front()->dataTypePointer->name, "bepa");

	return 0;
}

TEST_SUIT_END
