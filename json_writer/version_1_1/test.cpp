/*####################################################
# File Name: test.cpp
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-11-28 22:17:50
# Last Modified: 2016-11-28 22:17:50
####################################################*/
#include <iostream>
#include <cmath>
#include "json_writer.hpp"

using namespace std;

#define BEGIN_TEST(name){\
	JsonWriter *w = new JsonWriter;\
	w->setInitialIndentDepth(2);\
	if(compress){\
		w->configureCompressedOutput();\
	}\
	cout << #name << ":" << endl << endl;

#define END_TEST\
	delete w;\
	cout << endl << endl;\
}                                             
									

int main(void){
	bool compress;
	for(int i = 0; i < 2; compress = i == 0, ++i){
		BEGIN_TEST(null)	
			w->nullValue();
		END_TEST

		BEGIN_TEST(bool-false)
			w->Value(false);
		END_TEST
		
		BEGIN_TEST(bool-true)
			w->Value(true);
		END_TEST

		BEGIN_TEST(int)
			w->Value(1500);
		END_TEST
		
		BEGIN_TEST(double)
			w->Value(12.469);
		END_TEST

		BEGIN_TEST(empty-string)
			w->Value("");
		END_TEST

		BEGIN_TEST(simple-string)
			w->Value("Hello World");
		END_TEST

		BEGIN_TEST(escaped-string)
			w->Value("\"newline\ntab\t\"");
		END_TEST

		BEGIN_TEST(empty-object)
			w->startObject();
			w->endObject();
		END_TEST
		
		BEGIN_TEST(empty-array)
			w->startArray();
			w->endArray();
		END_TEST
		
		BEGIN_TEST(short-object)
			w->startShortObject();
			w->keyValue("name", "Barry");
			w->endObject();
		END_TEST

		BEGIN_TEST(short-array)
			w->startShortArray();
			for(int i = 0; i < 10; ++i){
				w->Value(i);
			}
			w->endArray();
		END_TEST

		BEGIN_TEST(array-with-object)
			w->startArray();
				w->startShortObject();
					w->keyValue("name", "Tom");
					w->keyValue("age", "23");
				w->endObject();

				w->startObject();
					w->keyValue("gender", "male");
					w->keyValue("life-is-short", "be-happy");
					w->keyValue("ip", "127.0.0.1");
					w->Key("be-social");

					w->startArray();
						w->Value("movie");
						w->Value("music");
						w->Value("book");
					w->endArray();
				w->endObject();

				w->Value("shasing");
			w->endArray();
		END_TEST

		BEGIN_TEST()
			w->startObject();
				w->Key("animal");
				w->startObject();
					w->Key("dog");
					w->startObject();
						w->Key("info");
						w->startObject();
							w->keyValue("name", "yoyo");
						w->endObject();
					w->endObject();
				w->endObject();
			w->endObject();
		END_TEST

		BEGIN_TEST()
			w->startArray();
				w->startArray();
					w->startArray();
					w->endArray();
				w->endArray();
			w->endArray();
		END_TEST
	}

	//cout << endl;//若不加换行，则输出到终端时 最后一行会跟shell提示符在同一行
	return 0;
}
