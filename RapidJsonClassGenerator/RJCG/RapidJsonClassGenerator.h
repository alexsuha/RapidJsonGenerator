#ifndef RAPIDJSON_GENERATOR_H_
#define RAPIDJSON_GENERATOR_H_

#include <string>
#include <fstream>
//#include <stack>
#include <queue>
#include <set>
#include "rapidjson/document.h"

using namespace std;
using namespace rapidjson;

namespace RapidJsonUtil
{
	struct VariableProperty
	{
		string name;
		string type;
		string initialValue;
		string ref;
		/*bool isEnum;

		VariableProperty()
			: isEnum(false)
		{}

		~VariableProperty() {}*/
	};

	class RapidJsonCodeGenerator
	{
	public:
		RapidJsonCodeGenerator(string& dir);
		~RapidJsonCodeGenerator();

		void generateCode();

	private:
		void generateCodeByFile(string& path);
		bool isNotSchema(Document& doc);
		void outputCppHeader(string& path);
		void outputCppEnding();
		void outputSchema(Document& doc);
		void outputClassDefinition(const Value& object);
		void outputClassDefinitionCode(const Value& object, string& className);
		void outputClassPropertiesDefinitionCode(const vector<VariableProperty>& properties, string& className);
		void getAllPropertiesFromObj(const Value& object, vector<VariableProperty>& properties);
		void outputExtraDefinitionCode(const Value& object);
		void enumValueFromString(string& val, string& s_val);
		void correctValueFromString(string& val);
		void correctTextFromString(string& val, string& s_val);
		void classNameFromRef(const Value& obj, string& target);
		//void topologicalDefSort(const Value& dvals, string& className, set<string>& classSet, stack<string>& classStack);
		void topologicalDefSort(const Value& dvals, string& className, set<string>& classSet, queue<string>& classQueue);

		string work_path;
		ofstream m_file;
	};
}

#endif // RAPIDJSON_GENERATOR_H_
