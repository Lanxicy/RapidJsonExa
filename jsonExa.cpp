// jsonExa.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

// Hello World example
// This example shows basic usage of DOM-style API.

#include "rapidjson/document.h"     // rapidjson\"s DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"

//# std
#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include <iostream>

//# boost
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/any.hpp>
#include <boost/range.hpp>
#include <boost/filesystem.hpp>

//# atl
#include <atlcomcli.h>

//# namespaces
using namespace rapidjson;
using namespace std;


typedef std::map<std::string, boost::shared_ptr<rapidjson::Value> >		LocationVauleSet;
typedef	std::map<std::string, boost::any>	ChannelsValueTree;

const char empty_element[] =
"{\
\"ElementCode\":\"\",\
\"DeviceDatas\":[]\
}";

const char c_channelProperty[] =
"{\
\"channelType\":[\"Smoke\",\"IR\",\"Power\",\"Blower\",\"Pumb\",\"Temperature\",\"Humidity\",\"LiquidLevel\",\"Gas\",\"SerialServer\"],\
\"yxSignal\":[true,true,true,true,true,false,false,false,false,true],\
\"alarmDisIndex\":[\
[0,1,2,3,4,5,6,7],\
[0,1,2,3,4,5,6,7],\
[0,1,2,3,4,5,6,7],\
[0,1,2,3,4,5,6,7],\
[0,1,2,3,4,5,6,7],\
[0,1,2,3,4,5,6,7],\
[0,1,2,3,4,5,6,7],\
[0,1,2,3,4,5,6,7],\
[0,1,2,3,4,5,6,7],\
[0,1,0,1,4,5,6,7]]}";

const int32_t	array_length = 10;

const std::string c_channelTypeArray[array_length] = {
	"Smoke",
	"IR",
	"Power",
	"Blower",
	"Pump",
	"Temperature",
	"Humidity",
	"LiquidLevel",
	"Gas",
	"SerialServer"
};

const bool enable_list[array_length] = {
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	false,
	false
};

const std::string match[array_length] = {
	"烟感",
	"红外",
	"市电",
	"风机",
	"水泵",
	"温度",
	"湿度",
	"深度",
	"有毒气体"
};





const bool is_bool_list[array_length] = {
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	false,
	false,
};

ChannelsValueTree		m_channelsValue;
LocationVauleSet		json_set;

LocationVauleSet::mapped_type	get_json_ptr(const std::string& name, rapidjson::Document& doc) {
	LocationVauleSet::mapped_type	device_ptr;
	if (json_set.find(name) != json_set.end()) {
		device_ptr = json_set[name.c_str()];
	}
	else {
		device_ptr = LocationVauleSet::mapped_type(new rapidjson::Value(kObjectType));
		device_ptr->AddMember("DeviceCode", rapidjson::Value(name.c_str(), doc.GetAllocator()).Move(), doc.GetAllocator());
		device_ptr->AddMember("Datas", rapidjson::Value(kArrayType), doc.GetAllocator());
		json_set[name] = device_ptr;
	}
	return device_ptr;
}

bool insert_json_value(const std::string& device_name, const std::string& channel_name, const boost::any& data, rapidjson::Document& doc) {

	if (data.type() != typeid(double)) {
		cout << "#不是double类型,而是 " << data.type().name() << "-" << channel_name << endl;
		return false;
	}

	bool matched = false;
	int32_t	match_idx = 0;
	for ( ;match_idx < array_length; ++match_idx)	{
		if (boost::contains(channel_name, match[match_idx])){
			matched = true;
			break;
		}
	}

	if (!matched) return false;

	Value data_value;
	double dbval = boost::any_cast<double>(data);
	if (is_bool_list[match_idx]) {
		data_value.SetBool(dbval ? true : false);
	}
	else {
		data_value.SetDouble(dbval);
	}

	LocationVauleSet::mapped_type	device_ptr;
	if (enable_list[match_idx])	{
		device_ptr = get_json_ptr(channel_name, doc);
	} 
	else {
		device_ptr = get_json_ptr(device_name, doc);
	}	

	rapidjson::Value& channel = (*device_ptr)["Datas"];
	Value::ValueIterator itr = channel.Begin();
	for (; itr != channel.End(); ++itr) {
		if ((*itr)["DataCode"] == c_channelTypeArray[match_idx].c_str()) {
			cout << (*itr)["DataCode"].GetString() << endl;
			(*itr)["DataValue"] = data_value.Move();
			break;
		}
	}
	if (itr == channel.End()) {
		rapidjson::Value channel_data(kObjectType);
		channel_data.AddMember("DataCode", rapidjson::Value(c_channelTypeArray[match_idx].c_str(), doc.GetAllocator()), doc.GetAllocator());
		channel_data.AddMember("DataValue", data_value.Move(), doc.GetAllocator());
		channel.PushBack(channel_data.Move(), doc.GetAllocator());
	}
	return true;
}

int main(int, char*[]) {
	//	////////////////////////////////////////////////////////////////////////////
	//	// 1. Parse a JSON text string to a document.
	//
	//	const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";
	//	printf("Original JSON:\n %s\n", json);
	//
	//	Document document;  // Default template parameter uses UTF8 and MemoryPoolAllocator.
	//
	//#if 0
	//						// "normal" parsing, decode strings to new buffers. Can use other input stream via ParseStream().
	//	if (document.Parse(json).HasParseError())
	//		return 1;
	//#else
	//						// In-situ parsing, decode strings directly in the source string. Source must be string.
	//	char buffer[sizeof(json)];
	//	memcpy(buffer, json, sizeof(json));
	//	if (document.ParseInsitu(buffer).HasParseError())
	//		return 1;
	//#endif
	//
	//	printf("\nParsing to document succeeded.\n");
	//
	//	////////////////////////////////////////////////////////////////////////////
	//	// 2. Access values in document. 
	//
	//	printf("\nAccess values in document:\n");
	//	assert(document.IsObject());    // Document is a JSON value represents the root of DOM. Root can be either an object or array.
	//
	//	assert(document.HasMember("hello"));
	//	assert(document["hello"].IsString());
	//	printf("hello = %s\n", document["hello"].GetString());
	//
	//	// Since version 0.2, you can use single lookup to check the existing of member and its value:
	//	Value::MemberIterator hello = document.FindMember("hello");
	//	assert(hello != document.MemberEnd());
	//	assert(hello->value.IsString());
	//	assert(strcmp("world", hello->value.GetString()) == 0);
	//	(void)hello;
	//
	//	assert(document["t"].IsBool());     // JSON true/false are bool. Can also uses more specific function IsTrue().
	//	printf("t = %s\n", document["t"].GetBool() ? "true" : "false");
	//
	//	assert(document["f"].IsBool());
	//	printf("f = %s\n", document["f"].GetBool() ? "true" : "false");
	//
	//	printf("n = %s\n", document["n"].IsNull() ? "null" : "?");
	//
	//	assert(document["i"].IsNumber());   // Number is a JSON type, but C++ needs more specific type.
	//	assert(document["i"].IsInt());      // In this case, IsUint()/IsInt64()/IsUInt64() also return true.
	//	printf("i = %d\n", document["i"].GetInt()); // Alternative (int)document["i"]
	//
	//	assert(document["pi"].IsNumber());
	//	assert(document["pi"].IsDouble());
	//	printf("pi = %g\n", document["pi"].GetDouble());
	//
	//	{
	//		const Value& a = document["a"]; // Using a reference for consecutive access is handy and faster.
	//		assert(a.IsArray());
	//		for (SizeType i = 0; i < a.Size(); i++) // rapidjson uses SizeType instead of size_t.
	//			printf("a[%d] = %d\n", i, a[i].GetInt());
	//
	//		int y = a[0].GetInt();
	//		(void)y;
	//
	//		// Iterating array with iterators
	//		printf("a = ");
	//		for (Value::ConstValueIterator itr = a.Begin(); itr != a.End(); ++itr)
	//			printf("%d ", itr->GetInt());
	//		printf("\n");
	//	}
	//
	//	// Iterating object members
	//	static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };
	//	for (Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr)
	//		printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
	//
	//	////////////////////////////////////////////////////////////////////////////
	//	// 3. Modify values in document.
	//
	//	// Change i to a bigger number
	//	{
	//		uint64_t f20 = 1;   // compute factorial of 20
	//		for (uint64_t j = 1; j <= 20; j++)
	//			f20 *= j;
	//		document["i"] = f20;    // Alternate form: document["i"].SetUint64(f20)
	//		assert(!document["i"].IsInt()); // No longer can be cast as int or uint.
	//	}
	//
	//	// Adding values to array.
	//	{
	//		Value& a = document["a"];   // This time we uses non-const reference.
	//		Document::AllocatorType& allocator = document.GetAllocator();
	//		for (int i = 5; i <= 10; i++)
	//			a.PushBack(i, allocator);   // May look a bit strange, allocator is needed for potentially realloc. We normally uses the document\"s.
	//
	//										// Fluent API
	//		a.PushBack("Lua", allocator).PushBack("Mio", allocator);
	//	}
	//
	//	// Making string values.
	//
	//	// This version of SetString() just store the pointer to the string.
	//	// So it is for literal and string that exists within value\"s life-cycle.
	//	{
	//		document["hello"] = "rapidjson";    // This will invoke strlen()
	//											// Faster version:
	//											// document["hello"].SetString("rapidjson", 9);
	//	}
	//	// This version of SetString() needs an allocator, which means it will allocate a new buffer and copy the the string into the buffer.
	//	Value author;
	//	{
	//		char buffer2[10];
	//		int len = sprintf(buffer2, "%s %s", "Milo", "Yip");  // synthetic example of dynamically created string.
	//
	//		author.SetString(buffer2, static_cast<SizeType>(len), document.GetAllocator());
	//		// Shorter but slower version:
	//		// document["hello"].SetString(buffer, document.GetAllocator());
	//
	//		// Constructor version: 
	//		// Value author(buffer, len, document.GetAllocator());
	//		// Value author(buffer, document.GetAllocator());
	//		memset(buffer2, 0, sizeof(buffer2)); // For demonstration purpose.
	//	}
	//	// Variable \"buffer\" is unusable now but \"author\" has already made a copy.
	//	document.AddMember("author", author, document.GetAllocator());
	//
	//	assert(author.IsNull());        // Move semantic for assignment. After this variable is assigned as a member, the variable becomes null.
	//
	//									////////////////////////////////////////////////////////////////////////////
	//									// 4. Stringify JSON
	//
	//	printf("\nModified JSON with reformatting:\n");
	//	StringBuffer sb;
	//	PrettyWriter<StringBuffer> writer(sb);
	//	document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
	//	puts(sb.GetString());

	////#测试数据1
	//location_device	data1;
	//data1.channels["温湿度1"]["民防1.温湿度1.温度.value"] = 25.0;
	//data1.channels["温湿度1"]["民防1.温湿度1.湿度.value"] = 62.0;
	//data1.channels["温湿度2"]["民防1.温湿度2.温度.value"] = 28.0;
	//data1.channels["温湿度2"]["民防1.温湿度2.湿度.value"] = 67.0;
	//data1.channels["液位"]["民防1.液位.深度.value"] = 2.0;
	//data1.channels["输入输出模块"]["民防1.输入输出模块.烟感1.value"] = true;
	//data1.channels["输入输出模块"]["民防1.输入输出模块.烟感2.value"] = false;
	//data1.channels["输入输出模块"]["民防1.输入输出模块.红外.value"] = true;
	//data1.channels["输入输出模块"]["民防1.输入输出模块.市电.value"] = false;
	//data1.channels["输入输出模块"]["民防1.输入输出模块.水泵.value"] = false;
	//data1.channels["输入输出模块"]["民防1.输入输出模块.风机.value"] = true;
	//data1.channels["综合气体"]["民防1.综合气体.有毒气体.value"] = 0.0250;
	//m_DeviceSet["民防1"] = data1;
	//
	////# 测试数据2
	//location_device	data2;
	//data2.channels["温湿度1"]["民防2.温湿度1.温度.value"] = 25.6;
	//data2.channels["温湿度1"]["民防2.温湿度1.湿度.value"] = 62.6;
	//data2.channels["温湿度2"]["民防2.温湿度2.温度.value"] = 28.6;
	//data2.channels["温湿度2"]["民防2.温湿度2.湿度.value"] = 67.6;
	//data2.channels["液位"]["民防2.液位.深度.value"] = 2.6;
	//data2.channels["输入输出模块"]["民防2.输入输出模块.烟感1.value"] = false;
	//data2.channels["输入输出模块"]["民防2.输入输出模块.烟感2.value"] = true;
	//data2.channels["输入输出模块"]["民防2.输入输出模块.红外.value"] = false;
	//data2.channels["输入输出模块"]["民防2.输入输出模块.市电.value"] = true;
	//data2.channels["输入输出模块"]["民防2.输入输出模块.水泵.value"] = false;
	//data2.channels["输入输出模块"]["民防2.输入输出模块.风机.value"] = false;
	//data2.channels["综合气体"]["民防2.综合气体.有毒气体.value"] = 0.025;
	//m_DeviceSet["民防2"] = data2;
	//
	//for (LocationDevicesTree::iterator location_iter = m_DeviceSet.begin();
	//	location_iter != m_DeviceSet.end();
	//	++location_iter) {
	//	string location_name = location_iter->first;
	//	doc["ElementCode"].SetString(location_name.c_str(), location_name.length(), doc.GetAllocator());
	//	doc["DeviceDatas"].Clear();
	//	//for (LocationDevicesTree::mapped_type::iter device_iter = location_iter->second.channels.begin();
	//	//	device_iter != location_iter->second.channels.end();
	//	//	++device_iter) {
	//	//	std::string device = device_iter->first;
	//	//	ChannelValueTree::mapped_type& channels = device_iter->second;
	//	//	for (ChannelValueTree::mapped_type::iterator channel_iter = channels.begin();
	//	//		channel_iter != channels.end();
	//	//		++channel_iter) {
	//	//		//# parse data
	//	//		if (insert_json_value(channel_iter->first, channel_iter->second, doc)) {
	//	//			cout << "#数据解析成功" << endl;
	//	//		}
	//	//		else {
	//	//			cout << "#数据解析失败" << endl;
	//	//		}
	//	//	}
	//	//}
	//	for (LocationVauleSet::iterator device_json_iter = json_set.begin();
	//		device_json_iter != json_set.end();
	//		++device_json_iter) {
	//		doc["DeviceDatas"].PushBack(device_json_iter->second->Move(), doc.GetAllocator());
	//	}
	//	std::string file_name(location_name + ".json");
	//	FILE *fp = fopen(file_name.c_str(), "w+");
	//	if (fp != 0) {
	//		char buffer[65536];
	//		FileWriteStream write_stream(fp, buffer, sizeof(buffer));
	//		PrettyWriter<FileWriteStream> writer(write_stream);
	//		doc.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
	//		fclose(fp);
	//	}
	//	else {
	//		cout << file_name.c_str() << " file open failed!" << endl;
	//	}
	//	json_set.clear();
	//};

	////# 测试数据3
	//m_channelsValue["COM1.民防2.温湿度1.温度.value"] = 25.0;
	//m_channelsValue["COM1.民防2.温湿度1.湿度.value"] = 62.0;
//m_channelsValue["COM2.民防2.温湿度2.温度.value"] = 28.0;
//m_channelsValue["COM2.民防2.温湿度2.湿度.value"] = 67.0;
//m_channelsValue["COM3.民防2.液位.深度.value"] = 2.0;
//m_channelsValue["COM4.民防2.输入输出模块.烟感1.value"] = 1;
//m_channelsValue["COM4.民防2.输入输出模块.烟感2.value"] = 0;
//m_channelsValue["COM4.民防2.输入输出模块.红外.value"] = 1;
//m_channelsValue["COM4.民防2.输入输出模块.市电.value"] = 0;
//m_channelsValue["COM4.民防2.输入输出模块.水泵.value"] = 1;
//m_channelsValue["COM4.民防2.输入输出模块.风机.value"] = 0;
//m_channelsValue["COM0.民防2.综合气体.有毒气体.value"] = 0.020;

////#测试数据4
//m_channelsValue["COM5.民防3.温湿度1.温度.value"] = 25.6;
//m_channelsValue["COM5.民防3.温湿度1.湿度.value"] = 62.6;

//m_channelsValue["COM6.民防3.温湿度2.温度.value"] = 28.6;
//m_channelsValue["COM6.民防3.温湿度2.湿度.value"] = 67.6;
//m_channelsValue["COM7.民防3.液位.深度.value"] = 2.6;
//m_channelsValue["COM8.民防3.输入输出模块.烟感1.value"] = 0;
//m_channelsValue["COM8.民防3.输入输出模块.烟感2.value"] = 1;
//m_channelsValue["COM8.民防3.输入输出模块.红外.value"] = 0;
//m_channelsValue["COM8.民防3.输入输出模块.市电.value"] = 1;
//m_channelsValue["COM8.民防3.输入输出模块.水泵.value"] = 0;
//m_channelsValue["COM8.民防3.输入输出模块.风机.value"] = 0;
//m_channelsValue["COM9.民防3.综合气体.有毒气体.value"] = 0.026;


//rapidjson::Document doc;
//doc.Parse(empty_element);
//if (doc.HasParseError()) {
//	cout << "#rapidjson parse empty element occur error!" << endl;
//	return 0;
//}
//ChannelsValueTree::iterator chaVlu_iter = m_channelsValue.begin();
//std::string loc_name("loc_name"), dev_name, chan_name;
//while (chaVlu_iter != m_channelsValue.end()) {
//	//# COM6.民防3.综合气体.有毒气体.value
//	std::vector<std::string> nmpts;
//	boost::algorithm::split(nmpts, chaVlu_iter->first, boost::algorithm::is_any_of(_T(".")));
//	if (5 > nmpts.size()) { cout << "#data format error" << endl; }
//	loc_name = nmpts.at(1), dev_name = nmpts.at(2), chan_name = nmpts.at(3);

//	if (insert_json_value(dev_name, chan_name, chaVlu_iter->second, doc)) {
//		cout << "#数据解析成功" << endl;
//	}
//	else {
//		cout << "#数据解析失败" << endl;
//	}

//	++chaVlu_iter;
//	
//	if (chaVlu_iter == m_channelsValue.end()||!boost::istarts_with(chaVlu_iter->first.substr(chaVlu_iter->first.find_first_of(".") + 1), loc_name)) {
//		doc["ElementCode"].SetString(loc_name.c_str(), loc_name.length(), doc.GetAllocator());
//		doc["DeviceDatas"].Clear();
//		for (LocationVauleSet::iterator device_json_iter = json_set.begin();
//			device_json_iter != json_set.end();
//			++device_json_iter) {
//			doc["DeviceDatas"].PushBack(device_json_iter->second->Move(), doc.GetAllocator());
//		}
//		json_set.clear();//#must clear

//						 //#获取doc中json字符串
//		StringBuffer sb;
//		PrettyWriter<StringBuffer> docWriter(sb);
//		doc.Accept(docWriter);
//		cout << "# 发送的内容->%s" << sb.GetString();

//		std::string file_name(loc_name + ".json");
//		FILE *fp = fopen(file_name.c_str(), "w+");
//		if (fp != 0) {
//			char buffer[65536];
//			FileWriteStream write_stream(fp, buffer, sizeof(buffer));
//			PrettyWriter<FileWriteStream> writer(write_stream);
//			doc.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
//			fclose(fp);
//		}
//		else {
//			cout << file_name.c_str() << " file open failed!" << endl;
//		}
//	}
//}

boost::filesystem::path curPath = boost::filesystem::current_path();
cout<<curPath.string()<<endl;
rapidjson::Document doc;
std::string file_name("channelProperty.json");
FILE *fp = fopen(file_name.c_str(), "rt");
if (fp != 0) {
	char buffer[65536];
	FileReadStream readStream(fp, buffer, sizeof(buffer));
	fclose(fp);
	doc.ParseStream(readStream);
	if (doc.HasParseError()) {
		cout << "#rapidjson parse json from channelProperty.json error =>" << doc.GetParseError() << endl;
	}
	else {
		StringBuffer sb;
		PrettyWriter<StringBuffer> docWriter(sb);
		doc.Accept(docWriter);
		cout << "# 发送的内容->%s" << sb.GetString();
		bool fileCheckOk = true;
		if (doc.FindMember("channelType") == doc.MemberEnd()) {
			fileCheckOk = false;
			cout << "#error=> channelPropertyJson did not find channelType element" << endl;
		}
		 if(doc.FindMember("yxSignal") == doc.MemberEnd()){
			 fileCheckOk = false;
			cout << "#error=> channelPropertyJson did not find yxSignal element" << endl;
		}
		 if (doc.FindMember("alarmDisIndex") == doc.MemberEnd()) {
			 fileCheckOk = false;
			cout << "#error=> channelPropertyJson did not find alarmDisIndex element" << endl;
		}
		 if (!doc["channelType"].IsArray()) {
			 fileCheckOk = false;
			cout << "#error=> channelPropertyJson's channelType element is not array" << endl;
		}
		 if (!doc["yxSignal"].IsArray()) {
			 fileCheckOk = false;
			cout << "#error=> channelPropertyJson's yxSignal element is not array" << endl;
		}
		 if (!doc["alarmDisIndex"].IsArray()) {
			 fileCheckOk = false;
			cout << "#error=> channelPropertyJson's alarmDisIndex element is not array" << endl;
		}
		 if (!doc["channelType"].Size() != doc["yxSignal"].Size() &&
			doc["yxSignal"].Size() != doc["alarmDisIndex"].Size()) {
			 fileCheckOk = false;
			cout << "#error=> channelPropertyJson's channelType\\yxSignal\\alarmDisIndex element number is not equal" << endl;
		}

		for (int32_t disIndex = 0; disIndex < doc["alarmDisIndex"].Size(); ++disIndex) {
			if (!doc["alarmDisIndex"][disIndex].IsArray()) {
				fileCheckOk = false;
				cout << "#error=> alarmDisIndex " << disIndex << "th element is not array" << endl;
			}
			else if (8 != doc["alarmDisIndex"][disIndex].Size()) {
				fileCheckOk = false;
				cout << "#error=> alarmDisIndex " << disIndex << "th element array length is not 8" << endl;
			}
		}

	}
	}
	else {
		cout << file_name.c_str() << " file open failed!" << endl;
	}


	//doc.Parse(c_channelProperty);
	//if (doc.HasParseError()) {
	//	cout << "#rapidjson parse empty element occur error!" << doc.GetParseError() << endl;
	//	return 0;
	//}

	//StringBuffer sb;
	//PrettyWriter<StringBuffer> docWriter(sb);
	//doc.Accept(docWriter);
	//cout << "# 发送的内容->%s" << sb.GetString();

	//std::string file_name("channelProperty.json");
	//FILE *fp = fopen(file_name.c_str(), "w+");
	//if (fp != 0) {
	//	char buffer[65536];
	//	FileWriteStream write_stream(fp, buffer, sizeof(buffer));
	//	PrettyWriter<FileWriteStream> writer(write_stream);
	//	doc.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
	//	fclose(fp);
	//}
	//else {
	//	cout << file_name.c_str() << " file open failed!" << endl;
	//}

	getchar();
	return 0;
}

