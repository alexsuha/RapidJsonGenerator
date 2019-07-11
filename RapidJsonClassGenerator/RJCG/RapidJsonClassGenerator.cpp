
#include "PathUtils.h"
#include <iostream>
#include <vector>

#include "RapidJsonUtils.h"
#include "RapidJsonClassGenerator.h"

using namespace PathUtils;

namespace RapidJsonUtil
{
#define INITIATE_FILE(x) \
	m_file.open((x), std::ios::out | std::ios::binary);
#define WRITE_LINE(x) \
	m_file << (x) << endl;
#define WRITE_ENTER \
	m_file << endl;
#define WRITE_CURLY_BRACES \
	m_file << "{" << endl;
#define WRITE_CURLY_BRACES_END \
	m_file << "}" << endl;
#define WRITE_CLASS_HEADER(x) \
	WRITE_LINE("class " + x) \
	WRITE_CURLY_BRACES
#define WRITE_CLASS_END \
	m_file << "};" << endl;


	RapidJsonCodeGenerator::RapidJsonCodeGenerator(string & dir)
		: work_path(dir)
	{
	}
	RapidJsonCodeGenerator::~RapidJsonCodeGenerator()
	{
	}
	void RapidJsonCodeGenerator::generateCode()
	{
		string path = work_path + "\\*schema.json";
		vector<string> _schemaJsonFiles;
		searchFilesFromDirectory(path, _schemaJsonFiles);

		for (auto _schema : _schemaJsonFiles)
		{
			string _schemaPath = work_path + "\\" + _schema;
			generateCodeByFile(_schemaPath);
		}
	}
	void RapidJsonCodeGenerator::generateCodeByFile(string & path)
	{
		RapidJsonHelper* helper = new RapidJsonHelper;

		Document doc;
		helper->getDocumentFromPath(path, doc);

		// verify is schema
		if (!isNotSchema(doc))
		{
			Log("This is not schema");
			return;
		}
		// write the header of the h file.
		outputCppHeader();
		// get the type
		outputSchema(doc);
		// get the title
		// version ignore if check
		// first parse all properties
		// then handle all definitions
		outputCppEnding();
	}
	bool RapidJsonCodeGenerator::isNotSchema(Document & doc)
	{
		if (doc.HasMember("$schema"))
		{
			return true;
		}
		return false;
	}
	void RapidJsonCodeGenerator::outputCppHeader()
	{
		INITIATE_FILE(work_path + "\\testcpp.h")
			WRITE_LINE("#ifndef TEST_CPP_H_")
			WRITE_LINE("#define TEST_CPP_H_")
			WRITE_ENTER
			WRITE_LINE("#include <vector>")
			WRITE_LINE("#include <string>")
			WRITE_LINE("#include \"rapidjson/document.h\"")
			WRITE_ENTER
			WRITE_LINE("using namespace std;")
			WRITE_LINE("using namespace rapidjson;")
			WRITE_ENTER
			WRITE_LINE("namespace TEST")
			WRITE_CURLY_BRACES
	}
	void RapidJsonCodeGenerator::outputCppEnding()
	{
		WRITE_LINE("}")
			WRITE_LINE("#endif")
	}

	void RapidJsonCodeGenerator::outputSchema(Document& doc)
	{
		const Value& object = doc.GetObjectW();
		const Value& v = object["type"];
		
		if (v.GetType() == kStringType)
		{
			string realv(v.GetString());
			if (realv == "object")
			{
				outputClassDefinition(object);
			}
		}
	}

	void RapidJsonCodeGenerator::outputClassDefinition(const Value& object)
	{
		string title = object["title"].GetString();
		outputClassDefinitionCode(object, title);
	}

	void RapidJsonCodeGenerator::outputClassDefinitionCode(const Value& object, string& className)
	{
		WRITE_CLASS_HEADER(className)
			WRITE_LINE("public:")
			// read properties
			outputClassPropertiesDefinitionCode(object, className);
		// write from json
		WRITE_CLASS_END
		outputExtraDefinitionCode(object);
	}

	std::vector<std::string> split(const std::string& s, const std::string& delims)
	{
		vector<string> v;

		// Start of an element.
		size_t elemStart = 0;

		// We start searching from the end of the previous element, which
		// initially is the start of the string.
		size_t elemEnd = 0;

		// Find the first non-delim, i.e. the start of an element, after the end of the previous element.
		while ((elemStart = s.find_first_not_of(delims, elemEnd)) != string::npos)
		{
			// Find the first delem, i.e. the end of the element (or if this fails it is the end of the string).
			elemEnd = s.find_first_of(delims, elemStart);
			// Add it.
			v.emplace_back(s, elemStart, elemEnd == string::npos ? string::npos : elemEnd - elemStart);
		}
		// When there are no more non-spaces, we are done.

		return v;
	}

	void RapidJsonCodeGenerator::outputClassPropertiesDefinitionCode(const Value& object, string& className)
	{
		if (object.HasMember("properties"))
		{
			if (object["properties"].GetType() == kObjectType)
			{
				const Value& pvals = object["properties"];
				vector<VariableProperty> properties;
				for (Value::ConstMemberIterator itr = pvals.MemberBegin();
					itr != pvals.MemberEnd(); ++itr)
				{
					VariableProperty p;
					p.name = itr->name.GetString();
					const Value& property_val = itr->value;
					//Value::ConstMemberIterator type_itr = itr->value.GetObjectW().FindMember("type");
					//Type type = type_itr->value.GetType();
					if (property_val.HasMember("type"))
					{
						Type type = property_val["type"].GetType();
						switch (type)
						{
						case kStringType:
							p.type = property_val["type"].GetString();
							if (p.type == "array")
							{
								Value::ConstMemberIterator type_itr = itr->value.GetObjectW().FindMember("items")->value.FindMember("$ref");
								p.ref = type_itr->value.GetString();
							}
							break;
						case kArrayType:
						{
							Value::ConstValueIterator vItr = property_val["type"].GetArray().Begin();
							p.type = vItr->GetString();
							++vItr;
							p.initialValue = vItr->GetString();
							break;
						}
						case kNullType:
						case kFalseType:
						case kTrueType:
						case kObjectType:
						case kNumberType:
							break;
						default:
							break;
						}
					}
					else if (property_val.HasMember("enum"))
					{
						// TODO: add enum definiation.
						p.type = "enum";
//						p.isEnum = true;
						for (Value::ConstValueIterator vItr = property_val["enum"].GetArray().Begin();
							vItr != property_val["enum"].GetArray().End(); ++vItr)
						{
							p.initialValue += string(vItr->GetString()) + " ";
						}
					}
					properties.push_back(p);
				}

				// construct and destruct
				WRITE_LINE(className + "() {")

					for (auto vp : properties)
					{
						if (vp.initialValue != "" && vp.type != "enum")
						{
							WRITE_LINE(" " + vp.name + " = " + (vp.initialValue == "null" ? "nullptr" : vp.initialValue) + ";");
						}
					}
				WRITE_LINE("}")

					WRITE_LINE("~" + className + "() {}")

				// set variable and convert function
					for (auto vp : properties)
					{
						//string msg = vp.name + "@" + vp.type + "@" + vp.initialValue;
						if (vp.type == "array")
						{
							//msg += "@" + vp.ref;
							string className = vp.ref.substr(vp.ref.find_last_of('/') + 1, vp.ref.size() - vp.ref.find_last_of('/') - 1);
							WRITE_LINE("vector<" + className + "> " + vp.name + ";");
						}
						else if (vp.type == "enum")
						{
							string enumName = vp.name;
							enumName[0] = toupper(enumName[0]);
							WRITE_LINE("enum " + enumName)
								WRITE_CURLY_BRACES
								//WRITE_LINE(vp.initialValue + "} " + vp.name + ";")
								vector<string> enums = split(vp.initialValue, " ");
							for (auto e : enums)
							{
								WRITE_LINE(e + ",")
							}
							WRITE_LINE("} " + vp.name + ";")
								WRITE_ENTER
								WRITE_LINE(enumName + " fromStringTo" + enumName + "(string& s)")
								WRITE_CURLY_BRACES
								WRITE_LINE("if (s == " + enums[0] + ") {")
								WRITE_LINE(vp.name + " = " + enums[0] + "; }")
								int max = enums.size();
								for (int i = 1; i < max; ++i)
								{
									WRITE_LINE("else if (s == \"" + enums[i] + "\") {")
									WRITE_LINE(vp.name + " = " + enums[i] + "; }")
								}
								WRITE_CURLY_BRACES_END
								WRITE_ENTER
						}
						else
						{
							string cplusType;
							if (vp.type == "string")
							{
								cplusType = "string";
							}
							else if (vp.type == "integer")
							{
								cplusType = "int";
							}
							else if (vp.type == "boolean")
							{
								cplusType = "bool";
							}
							WRITE_LINE(cplusType + " " + vp.name + ";")
						}
					}

				// write deserializing
				WRITE_LINE("void fromTheJson(const Value& object)")
				WRITE_CURLY_BRACES
				for (auto vp : properties)
				{
				if (vp.type == "string")
				{
				WRITE_LINE(vp.name + " = " + "doc[\"" + vp.name + "\"];")
				}
				else if (vp.type == "array")
				{
				WRITE_LINE("Value::ConstValueIterator vItr = type_itr->value.GetArray().Begin();")
				}
				}
				WRITE_CURLY_BRACES_END
			}
		}
	}
	void RapidJsonCodeGenerator::outputExtraDefinitionCode(const Value& object)
	{
		if (object.HasMember("definitions"))
		{
			if (object["definitions"].GetType() == kObjectType)
			{
				const Value& dvals = object["definitions"];
				for (Value::ConstMemberIterator itr = dvals.MemberBegin();
					itr != dvals.MemberEnd(); ++itr)
				{
					//string msg = "class name:" + string(itr->name.GetString());
					const Value& def_val = itr->value;
					if (def_val.HasMember("title"))
					{
						outputClassDefinition(def_val);
					}
					else
					{
						string className = string(itr->name.GetString());
						outputClassDefinitionCode(def_val, className);
					}
					//Value::Object def_obj = def_val.GetObjectW()
					//outputClassDefinitionCode(Value::Object(itr->value.GetObjectW()), string(itr->name.GetString()));
				}
			}
		}
	}
}