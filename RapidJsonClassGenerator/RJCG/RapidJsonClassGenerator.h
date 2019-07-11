#ifndef RAPIDJSON_GENERATOR_H_
#define RAPIDJSON_GENERATOR_H_

#include <string>
#include <fstream>
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
		void outputCppHeader();
		void outputCppEnding();
		void outputSchema(Document& doc);
		void outputClassDefinition(const Value& object);
		void outputClassDefinitionCode(const Value& object, string& className);
		void outputClassPropertiesDefinitionCode(const Value& object, string& className);
		void outputExtraDefinitionCode(const Value& object);

		string work_path;
		ofstream m_file;
	};
}

#endif // RAPIDJSON_GENERATOR_H_
