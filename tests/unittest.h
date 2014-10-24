/*
 * unittest.h
 *
 *  Created on: 6 okt 2014
 *      Author: Mattias Larsson Sköld
 *      https://github.com/mls-m5/mls-unit-test
 */


#pragma once

#include <map>
#include <string>
#include <iostream>

typedef int (*testFunction)();

extern std::map<std::string, testFunction> testMap;
extern int test_result;

inline int runTests(){
	using std::cout;
	using std::endl;
	int failed = 0;
	int numFailed = 0;
	int numSucceded = 0;
	cout << "==== Starts test suit ====" << endl << endl;

	for (auto it: testMap){
		cout << "--- running test " << it.first << " ---" << endl;
		test_result = 0;
		it.second();
		if (test_result){
			failed = 1;
			cout << " --> failed" << endl << endl;
			numFailed ++;
		}
		else{
			cout << " --> lyckades " << endl << endl;
			numSucceded ++;
		}

	}

	cout << endl;
	if (failed){
		cout << "MISSLYCKADES";
	}
	else{
		cout << "LYCKADES... hej då";
	}
	cout << endl;
	cout << "Failed: " << numFailed << " Succeded: " << numSucceded << endl;

	return failed;
}

#define TEST_SUIT_BEGIN  std::map<std::string, int(*)()> testMap; int test_result; void initTests(){

//Remember to return 0 on success!!!
#define TEST_CASE(name) ; testMap[name] = []() -> int

#define TEST_SUIT_END ; } int main() { initTests(); return runTests(); }

#define PRINT_INFO std::cout << __FILE__ << ":" << __LINE__ << ": " ;
#define ASSERT(x, error) if (!(x)) { PRINT_INFO; test_result ++; std::cout << #x << ": " << error << std::endl; return 1; }
#define ASSERT_EQ(x, y) if ((x != y)) { PRINT_INFO; test_result ++; std::cout << #x << " = " << x << \
	" is not equal to "	<< #y << " = " << y << std::endl; test_result ++; return 1; }
#define ASSERT_NE(x, y) if ((x == y)) { PRINT_INFO; std::cout << #x << " = " << x << \
	" is equal to "	<< #y << " = " << y << std::endl; test_result ++; return 1; }

#define ASSERT_GT(x, y) if (!(x > y)) { PRINT_INFO; std::cout << #x << " = " << x << \
	" is not greater than "	<< #y << " = " << y << std::endl; test_result ++; return 1; }

#define ASSERT_LT(x, y) if (!(x < y)) { PRINT_INFO; std::cout << #x << " = " << x << \
	" is not less than "	<< #y << " = " << y << std::endl; test_result ++; return 1; }

#define ERROR(error) PRINT_INFO; std::cout << error << std::endl; test_result ++; return 1;

