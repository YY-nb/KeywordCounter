#include "pch.h"
#include "CppUnitTest.h"
#include "../Lab2/KeywordCounter.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(UnitTest)
	{
	public:
		int arr_size = sizeof(keywords) / sizeof(keywords[0]);
		TEST_METHOD(TestIfElse1)
		{
			string text = "if{}else if{if{}else if{}}else{}";
			Counter counter(keywords, arr_size, 4);
			counter.startCount(text, 4);
			Assert::AreEqual(counter.getOutput()->if_else_num,0);
			Assert::AreEqual(counter.getOutput()->if_elseif_else_num, 1);
			
		}
		TEST_METHOD(TestIfElse2) {
			string text = "if(1){if (3) {}else if (2) {}}else {}";
			Counter counter(keywords, arr_size, 4);
			counter.startCount(text, 4);
			Assert::AreEqual(counter.getOutput()->if_else_num, 1);
			Assert::AreEqual(counter.getOutput()->if_elseif_else_num, 0);
		}
		TEST_METHOD(TestSwitchCase) {
			string text = "switch{case 1: break; case 2: break;}switch{case 1: break;}";
			Counter counter(keywords, arr_size, 4);
			counter.startCount(text, 4);
			Assert::AreEqual(counter.getOutput()->switch_num, 2);
			vector<int> vec = counter.getOutput()->case_list;
			Assert::AreEqual(vec[0], 2);
			Assert::AreEqual(vec[1], 1);
		}
		TEST_METHOD(TestIgnore) {
			string s1 = "//int";
			string s2 = "/*//int double*/";
			string s3 = "string s=\"int\";";
			string s4 = "#int";
			string s5 =  "s = \"//int\" case\"" ;    
			Counter counter(keywords, arr_size, 4);
			counter.startCount(s1, 4);
			Assert::AreEqual(counter.getOutput()->keyword_num, 0);
			
			counter.startCount(s2, 4);
			Assert::AreEqual(counter.getOutput()->keyword_num, 0);
			counter.startCount(s3, 4);
			Assert::AreEqual(counter.getOutput()->keyword_num, 0);
			counter.startCount(s4, 4);
			Assert::AreEqual(counter.getOutput()->keyword_num, 0);
			
			counter.startCount(s5, 4);
			Assert::AreEqual(counter.getOutput()->keyword_num, 0);
			


		}
	};
}
