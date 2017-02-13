/*####################################################
# File Name: json_writer.cpp
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-11-28 21:35:15
# Last Modified: 2016-11-28 21:35:15
####################################################*/
#include "json_writer.hpp"

//如果是同一个深度就缩进同样的格数
void JsonWriter::Indent(){
	for(int i = 0, s = initialIndentDepth + depth.size(); i < s; ++i){
		Write() << indent;
	}
}

//元素在 开始、结束时都需要函数来操作，分别为startContainer()、endContainer(), Container指的是对象 或者 数组，对象用{}，数组用[]
void JsonWriter::startContainer(ContainerType type, ContainerLayout layout){
	if(forceDefaultContainerLayout){
		layout = defaultContainerLayout;
	}else if(layout == CONTAINER_LAYOUT_INHERIT){
		if(depth.size() > 0){
			layout = depth.top()->layout;
		}else{
			layout = defaultContainerLayout;
		}
	}

	startChild();
	depth.push(new Container(type, layout));
	Write() << (type == CONTAINER_TYPE_OBJECT ? '{' : '[');
}

//元素结束时要添加 ']' 或者 '}'
void JsonWriter::endContainer(){
	Container *container = depth.top();
	depth.pop();

	if(container->childCount > 0){
		if(container->layout == CONTAINER_LAYOUT_MULIT_LINE){
			Write() << endl;
			Indent();
		}else{
			Write() << containerPadding;
		}
	}

	Write() << (container->type == CONTAINER_TYPE_OBJECT ? '}' : ']');

	delete container;
}

//如果一个节点的depth 为0，则认为它是 根节点，根节点只需考虑 缩进
//若为子节点，先分析其类型：数组、对象
//如果是数组 或 对象中的值，在继续写的时候要加','，因为数据与数据之间用','隔开
//单行布局 与 多行布局 的主要区别: 换行、缩进
//startChild()主要用于 对象、元素、键值的缩进和标点符号问题
void JsonWriter::startChild(bool isKey){
	if(depth.size() == 0){
		if(initialIndentDepth > 0){
			Indent();
		}
		return;
	}

	Container *container = depth.top();
	if(container->childCount > 0 && 
		(container->type == CONTAINER_TYPE_ARRAY || 
			(container->type == CONTAINER_TYPE_OBJECT && !container->isKey))){
		Write() << ",";

		if(container->layout == CONTAINER_LAYOUT_SINGLE_LINE){
			Write() << containerPadding;
		}else{
			Write() << endl;
			Indent();
		}
	}else if(container->childCount == 0){
		Write() << containerPadding;
		if(container->layout == CONTAINER_LAYOUT_MULIT_LINE){
			Write() << endl;
			Indent();
		}
	}

	container->isKey = isKey;
	container->childCount++;
}

void JsonWriter::writeString(const char *str){
	Write() << "\"";
	for(int i = 0; str[i] != 0; ++i){
		writeEscapedChar(str[i]);
	}

	Write() << "\"";
}

void JsonWriter::writeEscapedChar(char c){
	switch(c){
		case '"':
			Write() << "\\\"";
			break;
		case '\\':
			Write() << "\\\\";
			break;
		case '\b':
			Write() << "\\b";
			break;
		case '\f':
			Write() << "\\f";
			break;
		case '\n':
			Write() << "\\n";
			break;
		case '\r':
			Write() << "\\r";
			break;
		case '\t':
			Write() << "\\t";
			break;
		default:
			Write() << c;
			break;
	}
}

void JsonWriter::Value(const char *value){
	startChild();
	writeString(value);
}

void JsonWriter::Value(string value){
	startChild();
	writeString(value.c_str());
}

void JsonWriter::Value(bool value){
	startChild();
	Write() << (value ? "true" : "false");
}

void JsonWriter::Key(const char *key){
	startChild(true);
	writeString(key);
	Write() << keyPaddingLeft << ":" << keyPaddingRight;
}

void JsonWriter::nullValue(){
	startChild();
	Write() << "null";
}
