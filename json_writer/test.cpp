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

int main(void){
	auto writer = new JsonWriter;
	writer->startArray();

	writer->startShortObject();
	writer->keyValue("name", "shiyan");
	writer->keyValue("age", "25");
	writer->endObject();

	writer->startObject();
	writer->keyValue("skills", "C++");
	writer->keyValue("skills", "java");
	writer->keyValue("skills", "python");
	writer->keyValue("skills", "php");

	writer->keyValue("url", "www.google.com");

	writer->Key("path");
	writer->startArray();
	writer->Value("blog");
	writer->Value("linux");
	writer->Value("study");
	writer->endArray();

	writer->Key("short-array");
	writer->startShortArray();
	writer->Value(10);
	writer->Value((uint64_t)0xabcde12345);
	writer->Value(M_PI);
	writer->endContainer();

	writer->endObject();
	writer->Value(false);
	writer->endArray();

	cout << endl;//若不加换行，则输出到终端时 最后一行会跟shell提示符在同一行
	return 0;
}
