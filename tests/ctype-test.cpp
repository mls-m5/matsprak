/*
 * ctype-test.cpp
 *
 *  Created on: 19 okt 2014
 *      Author: Mattias Larsson Sköld
 */

#include <cast.h>
#include "unittest.h"
#include <sstream>

#include "tokenizer.h"
using namespace std;

#define CREATE_CAST(x) istringstream ss(x); CAst ast(0, ss);
#define CREATE_CAST_BLOCK(x) istringstream ss(x); CAstContentBlock ast(0); ast.load(ss);

TEST_SUIT_BEGIN

TEST_CASE("function definition"){
	istringstream ss("void main();");

	CAst cast(0);
	cast.load(ss);

	ASSERT_EQ(cast.type, Ast::FunctionDefinition);

	return 0;
}

TEST_CASE("variable definition"){
	istringstream ss("float x;");

	CAst cast(0);
	cast.load(ss);

	ASSERT_EQ(cast.type, Ast::VariableDeclaration);

	return 0;
}

TEST_CASE("comment test"){
	istringstream ss("/*apa*/");

	auto token = Tokenizer::GetNextCToken(ss);

	ASSERT_EQ(token.type, Token::Space);

	return 0;
}

TEST_CASE("pointers"){
	istringstream ss("int x; int *y; int **z;");

	CAstContentBlock cast(0);
	cast.load(ss);

	auto ret = cast.findVariable("x");

	ASSERT(ret, "x not found");
	ASSERT_EQ(ret->pointerDepth, 0);

	ret = cast.findVariable("y");
	ASSERT(ret, "y not found");
	ASSERT_EQ(ret->pointerDepth, 1);

	ret = cast.findVariable("z");
	ASSERT(ret, "z not found");
	ASSERT_EQ(ret->pointerDepth, 2);

	return 0;
}

TEST_CASE("double word types"){
	istringstream ss("long int x;");

	CAst cast(0);
	cast.load(ss);
	auto ret = cast.findVariable("x");
	ASSERT(ret,  "variable not found");
	ASSERT(ret->dataTypePointer, "type not set for x");

	return 0;
}

TEST_CASE("find defined variable"){
	istringstream ss("int x; int y, z;");

	CAstContentBlock cast(0);
	cast.load(ss);
	auto ret = cast.findVariable("x");

	ASSERT(ret, "variabeln x hittades inte");
	ASSERT_EQ(ret->name, "x");
	ASSERT_EQ(ret->type, Ast::VariableDeclaration);

	ret = cast.findVariable("y");

	ASSERT(ret, "variablen y hittades ej");

	ret = cast.findVariable("z");
	ASSERT(ret, "failed with double defined variable z");

return 0;
}

TEST_CASE("typedef"){
	istringstream ss("typedef long int * apa;");
	CAst cast(0);
	cast.load(ss);

	ASSERT_EQ(cast.type, Ast::Typedef);
	ASSERT_EQ(cast.name, "apa");
	ASSERT(cast.dataTypePointer, "no datatype defined");
	ASSERT_EQ(cast.dataTypePointer, Ast::FindType("long int"));

	return 0;
}

TEST_CASE("typedef function"){
	CREATE_CAST("typedef int apa (int x);");

	ASSERT_EQ(ast.type, Ast::Typedef);
	ASSERT_EQ(ast.name, "apa");
	ASSERT(ast.dataTypePointer, "no datatype defined");
	ASSERT_EQ(ast.dataTypePointer, Ast::FindType("int"));

	return 0;
}

TEST_CASE("load header"){
	{
		auto ast = CAst::CreateHeaderFromCommand("#include <stdio.h>\n");

		ASSERT(ast, "no header created");

		//Kind of in the beginning
		ASSERT(ast->findType("int"), "int not defined, something is strange");
		ASSERT(ast->findType("_IO_FILE"), "type not defined");

		auto ret = ast->findFunction("printf");

		ASSERT(ret, "printf was not found");
	}
}

TEST_CASE("load SDL2 header"){
	{
		auto ast = CAst::CreateHeaderFromCommand("#include <SDL2/SDL.h>\n");

		ASSERT(ast, "no header created");

		//Kind of in the beginning
		ASSERT(ast->findType("int"), "int not defined, something is strange");
		ASSERT(ast->findType("_IO_FILE"), "type not defined");

		auto ret = ast->findFunction("SDL_Quit");
		//SDL_Quit is the last function in the file btw

		ASSERT(ret, "SDL_Quit was not found");
	}

}

TEST_CASE("typedef function pointers"){
	{
		CREATE_CAST("int (*apa) (int x);");

		ASSERT_EQ(ast.type, Ast::FunctionPointer)
		ASSERT(ast.dataTypePointer, "no datatype");
		ASSERT_EQ(ast.dataTypePointer->name, "int");
		ASSERT_EQ(ast.name, "apa");
	}

	{
		CREATE_CAST("typedef int (*funcname) (int x);");

		ASSERT_EQ(ast.type, Ast::TypedefFunctionPointer)
		ASSERT(ast.dataTypePointer, "no datatype");
		ASSERT_EQ(ast.dataTypePointer->name, "int");
		ASSERT_EQ(ast.name, "funcname");
	}
}

TEST_CASE("enum"){
	{
		CREATE_CAST_BLOCK("enum apa {bepa};");

		ASSERT_EQ(ast.commands.size(), 2);
		ASSERT_EQ(ast.commands[0]->type, Ast::VariableDeclaration);
		ASSERT_EQ(ast.commands[1]->type, Ast::Enum);
	}

	{
		//Use enum just for declaration of constants
		CREATE_CAST_BLOCK("enum {apa, bepa};");
		ASSERT_EQ(ast.commands.size(), 3);
		ASSERT_EQ(ast.commands[0]->type, Ast::VariableDeclaration);
		ASSERT_EQ(ast.commands[1]->type, Ast::VariableDeclaration);
		ASSERT(ast.findVariable("apa"), "could not find typedef constant apa");
		ASSERT(ast.findVariable("bepa"), "could not find typedef constant bpa");
		ASSERT_EQ(ast.commands[2]->type, Ast::Enum);
		ASSERT_EQ(ast.commands[0]->dataTypePointer, ast.commands[2]);
	}


	{
		//Use enum just for declaration of constants
		CREATE_CAST_BLOCK("typedef enum {apa, bepa} cepa;");
		ASSERT_EQ(ast.commands.size(), 4);
		ASSERT_EQ(ast.commands[0]->type, Ast::VariableDeclaration);
		ASSERT_EQ(ast.commands[1]->type, Ast::VariableDeclaration);
		ASSERT_EQ(ast.commands[2]->type, Ast::Enum);
		ASSERT_EQ(ast.commands[3]->type, Ast::Typedef);
		ASSERT_EQ(ast.commands[3]->dataTypePointer, ast.commands[2]);
		ASSERT(ast.findVariable("apa"), "could not find typedef constant apa");
		ASSERT(ast.findVariable("bepa"), "could not find typedef constant bpa");
		ASSERT(ast.findType("cepa"), "could not find typedef cepa");
	}
}

TEST_CASE("skip preprocessor commands"){
	istringstream ss("#define apa(x) bepa cepa depa\n"
			"int bepa;");

	CAstContentBlock ast(0);
	ast.load(ss);
	auto ret = ast.findVariable("bepa");
	ASSERT(ret, "något fel.. variabeln not declared");
}

TEST_CASE("const expressions"){
	CREATE_CAST("const int apa;");

	auto ret = ast.findVariable("apa");
	ASSERT(ret, "apa is not defined");
	ASSERT(ret->constExpression, "variable is not const");
}

TEST_CASE("array expression"){
	CREATE_CAST("int apa[];");

	auto ret = ast.findVariable("apa");
	ASSERT(ret, "apa is not defined");
	ASSERT_EQ(ret->pointerDepth, 1);
}


TEST_CASE("struct test"){
	{
		//Forward declaration
		CREATE_CAST_BLOCK("struct apa;");
		ASSERT_EQ(ast.commands.size(), 1);
		ASSERT(ast.commands[0], "command not defined");
		ASSERT_EQ(ast.commands[0]->type, Ast::Struct);
	}

	{
		//Struct definition only
		CREATE_CAST_BLOCK("struct apa {};");
		ASSERT_EQ(ast.commands.size(), 1);
		ASSERT(ast.commands[0], "command not defined");
		ASSERT_EQ(ast.commands[0]->type, Ast::Struct);
		ASSERT_EQ(ast.commands[0]->name, "apa");
	}

	{
		//Struct definition and variable declaration
		CREATE_CAST_BLOCK("struct apa {} bepa;");
		ASSERT_EQ(ast.commands.size(), 2);
		ASSERT(ast.commands[0], "command not defined");
		ASSERT(ast.commands[1], "command 2 not defined");
		ASSERT_EQ(ast.commands[0]->type, Ast::Struct);
		ASSERT_EQ(ast.commands[0]->name, "apa");
		ASSERT_EQ(ast.commands[1]->type, Ast::VariableDeclaration);
		ASSERT_EQ(ast.commands[1]->name, "bepa");
	}

	{
		//Anonymous struct
		CREATE_CAST_BLOCK("struct {} apa;");
		ASSERT_EQ(ast.commands.size(), 2);
		ASSERT(ast.commands[0], "command not defined");
		ASSERT(ast.commands[1], "command 2 not defined");
		ASSERT_EQ(ast.commands[0]->type, Ast::Struct);
		ASSERT_EQ(ast.commands[1]->type, Ast::VariableDeclaration);
		ASSERT_EQ(ast.commands[1]->name, "apa");
	}

	{
		//Anonymous struct through typedef
		CREATE_CAST_BLOCK("typedef struct {} apa;");
		ASSERT_EQ(ast.commands.size(), 2);
		ASSERT(ast.commands[0], "command not defined");
		ASSERT(ast.commands[1], "command 2 not defined");
		ASSERT_EQ(ast.commands[0]->type, Ast::Struct);
		ASSERT_EQ(ast.commands[1]->type, Ast::Typedef);
		ASSERT_EQ(ast.commands[1]->name, "apa");
	}
}

TEST_SUIT_END


