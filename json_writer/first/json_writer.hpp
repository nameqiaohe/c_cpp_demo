/*####################################################
# File Name: json_writer.hpp
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-11-28 20:36:02
# Last Modified: 2016-11-28 20:36:02
####################################################*/

#ifndef JSONWRITER_H
#define JSONWRITER_H

#include <iostream>
#include <string>
#include <stack>

using namespace std;

class JsonWriter{
public:
	//json主要就两种数据类型: array, object
	enum ContainerType{
		CONTAINER_TYPE_ARRAY,
		CONTAINER_TYPE_OBJECT
	};

	//json的布局主要有三种: 继承父类的格式、单行元素、多行元素
	enum ContainerLayout{
		CONTAINER_LAYOUT_INHERIT,
		CONTAINER_LAYOUT_MULIT_LINE,
		CONTAINER_LAYOUT_SINGLE_LINE
	};

	explicit JsonWriter()
		:writer(NULL),
		initialIndentDepth(0),
		indent("	"),
		containerPadding(" "),
		keyPaddingLeft(""),
		keyPaddingRight(" "),
		defaultContainerLayout(CONTAINER_LAYOUT_MULIT_LINE),
		forceDefaultContainerLayout(false)
	{
	}

	//压缩: 减少空格、换行
	void configureCompressedOutput(){
		setIndent("");
		setContainerPadding("");
		setKeyPaddingLeft("");
		setKeyPaddingRight("");
		setDefaultContainerLayout(CONTAINER_LAYOUT_SINGLE_LINE);
		setForceDefaultContainerLayout(true);
	}


	std::ostream *getWriter(){
		return writer;
	}

	void setWriter(std::ostream *writer){
		this->writer = writer;
	}

	//缩进在该代码中很重要，针对不同情况编写而对应的 缩进函数
	int getInitialIndentDepth(){
		return initialIndentDepth;
	}

	void setInitialIndentDepth(int depth){
		initialIndentDepth = depth;
	}

	const char *getIndent(){
		return indent;
	}

	void setIndent(const char *indent){
		this->indent = indent;
	}

	//布局，即左右的空格控制
	const char *getContainerPadding(){
		return containerPadding;
	}

	void setContainerPadding(const char *padding){
		containerPadding = padding;
	}

	const char *getKeyPaddingLeft(){
		return keyPaddingLeft;
	}

	void setKeyPaddingLeft(const char *padding){
		keyPaddingLeft = padding;
	}

	const char *getKeyPaddingRight(){
		return keyPaddingRight;
	}

	void setKeyPaddingRight(const char *padding){
		keyPaddingRight = padding;
	}

	ContainerLayout getDefaultContainerLayout(){
		return defaultContainerLayout;
	}

	void setDefaultContainerLayout(ContainerLayout layout){
		defaultContainerLayout = layout;
	}

	bool getForceDefaultContainerLayout(){
		return forceDefaultContainerLayout;
	}

	void setForceDefaultContainerLayout(bool force){
		forceDefaultContainerLayout = force;
	}

	//VALUE_DEF(t) 指 值，用于数组列表中全是单值的情况，如："short-array":[1, 12345, 2.1532]
	//KEY_VALUE_DEF(t) 指 键值对，如："animal" : "dog"
	//如果Value()函数只能添加 int值，则需要写很多函数，调用起来很麻烦，此时用到了泛型，它将若干格式都用一个函数实现，
	//但是对 string、char *、bool，则需要单独定义函数
#define VALUE_DEF(t) void Value(t value){\
	startChild();\
	Write() << value;\
}

#define KEY_VALUE_DEF(t) void keyValue(const char *key, t value){\
	Key(key);\
	Value(value);\
}

	VALUE_DEF(int8_t)
	VALUE_DEF(uint8_t)
	VALUE_DEF(int16_t)
	VALUE_DEF(uint16_t)
	VALUE_DEF(int32_t)
	VALUE_DEF(uint32_t)
	VALUE_DEF(int64_t)
	VALUE_DEF(uint64_t)
	VALUE_DEF(float)
	VALUE_DEF(double)

	void keyNullValue(const char *key){
		Key(key);
		nullValue();
	}

	KEY_VALUE_DEF(const char *)
	KEY_VALUE_DEF(std::string)
	KEY_VALUE_DEF(bool)
	KEY_VALUE_DEF(int8_t)
	KEY_VALUE_DEF(uint8_t)
	KEY_VALUE_DEF(int16_t)
	KEY_VALUE_DEF(uint16_t)
	KEY_VALUE_DEF(int32_t)
	KEY_VALUE_DEF(uint32_t)
	KEY_VALUE_DEF(int64_t)
	KEY_VALUE_DEF(uint64_t)
	KEY_VALUE_DEF(float)
	KEY_VALUE_DEF(double)

	//一些核心函数
	//输出: 文件 或者 终端 (在没有指定文件时，输出到终端--标准输出上)
	std::ostream &Write(){
		if(writer == NULL){
			return std::cout;
		}

		return *writer;
	}
	void writeEscapedChar(char c);
	void writeString(const char *str);

	void startChild(bool isKey);
	void startChild(){
		startChild(false);
	}
	void startContainer(ContainerType type, ContainerLayout layout);
	void endContainer();

	void startArray(){
		startContainer(CONTAINER_TYPE_ARRAY, CONTAINER_LAYOUT_INHERIT);
	}

	void startArray(ContainerLayout layout){
		startContainer(CONTAINER_TYPE_ARRAY, layout);
	}

	void startShortArray(){
		startContainer(CONTAINER_TYPE_ARRAY, CONTAINER_LAYOUT_SINGLE_LINE);
	}

	void endArray(){
		endContainer();
	}

	void startObject(){
		startContainer(CONTAINER_TYPE_OBJECT, CONTAINER_LAYOUT_INHERIT);
	}

	void startObject(ContainerLayout layout){
		startContainer(CONTAINER_TYPE_OBJECT, layout);
	}

	void startShortObject(){
		startContainer(CONTAINER_TYPE_OBJECT, CONTAINER_LAYOUT_SINGLE_LINE);
	}

	void endObject(){
		endContainer();
	}

	void Key(const char *key);
	void nullValue();

	void Value(const char *value);
	void Value(std::string value);
	void Value(bool value);

private:
	std::ostream *writer;
	int initialIndentDepth;
	const char *indent;
	const char *containerPadding;
	const char *keyPaddingLeft;
	const char *keyPaddingRight;
	ContainerLayout defaultContainerLayout;
	bool forceDefaultContainerLayout;

	struct Container{
		ContainerType type;
		ContainerLayout layout;

		bool isKey;//判断元素是键 还是值 (键 和 值的布局，符号有所不同，需要分开看)
		int childCount;

		Container(ContainerType type, ContainerLayout layout)
			:type(type),
			layout(layout),
			isKey(false),
			childCount(0)
		{

		}
	};

	std::stack<Container *> depth;

	void Indent();
};
#endif
