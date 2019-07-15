
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
		outputCppHeader(path);
		outputSchema(doc);
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
	void RapidJsonCodeGenerator::outputCppHeader(string& path)
	{
		string fileName = path.substr(path.find_last_of('\\') + 1, path.find_last_of('.') - path.find_last_of('\\') - 1);
		INITIATE_FILE(work_path + "\\" +fileName+".h")
			for (auto& c : fileName)
			{
				if (c == '-')
				{
					c = '_';
				}
				else 
					c = toupper(c);
			}
			WRITE_LINE("#ifndef " + fileName + "_H_")
			WRITE_LINE("#define " + fileName + "_H_")
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
		outputExtraDefinitionCode(object);
		WRITE_CLASS_HEADER(className)
			WRITE_LINE("public:")
			// read properties
			outputClassPropertiesDefinitionCode(object, className);
		// write from json
		WRITE_CLASS_END
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

	void RapidJsonCodeGenerator::enumValueFromString(string& val, string& s_val)
	{
		if (val[0] <= '9' && val[0] >= '0')
		{
			val = '_' + s_val;
		}
		string temp;
		bool hasSignal = false;
		for (auto c : val)
		{
			if (c == '-' || c == '.' || c == '+' || c == ' ' || c == '/' || c == '(' || c == ')' || c == '\\')
			{
				hasSignal = true;
				continue;
			}
				//c = '_';
			else
				c = toupper(c);
			if (hasSignal) {
				temp.push_back('_');
				hasSignal = false;
			}
			temp.push_back(c);
		}
		val = temp;
	}

	void RapidJsonCodeGenerator::correctTextFromString(string& val, string& s_val)
	{
		for (auto c : s_val)
		{
			if (c == '\\')
			{
				val.push_back('\\');
			}
			val.push_back(c);
		}
	}

	void RapidJsonCodeGenerator::classNameFromRef(const Value& obj, string& target)
	{
		if (!obj.HasMember("$ref"))
		{
			return;
		}

		target = obj["$ref"].GetString();
		target = target.substr(target.find_last_of('/') + 1, target.size() - target.find_last_of('/') - 1);
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

					if (property_val.HasMember("type"))
					{
						Type type = property_val["type"].GetType();
						switch (type)
						{
						case kStringType:
							p.type = property_val["type"].GetString();
							if (p.type == "array")
							{
								const Value& type_val = itr->value.GetObjectW().FindMember("items")->value;
								if (type_val.HasMember("$ref"))
								{
									classNameFromRef(type_val, p.ref);
								}
								else if (type_val.HasMember("type"))
								{
									const Value& inner_type_val = type_val["type"];
									Type _type = inner_type_val.GetType();
									if (_type == kArrayType)
									{
										Value::ConstValueIterator vItr = inner_type_val.GetArray().Begin();
										p.ref = vItr->GetString();
									}
									else if (_type == kStringType)
									{
										p.ref = inner_type_val.GetString();
									}
								}
							}
							else if (p.type == "object")
							{
								classNameFromRef(property_val, p.ref);
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
							p.initialValue += string(vItr->GetString()) + ",";
						}
						string enumName = p.name;
						enumName[0] = toupper(enumName[0]);
						p.ref = enumName;
					}
					else if (property_val.HasMember("$ref"))
					{
						p.type = "object";
						classNameFromRef(property_val, p.ref);
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
							string className = vp.ref;
							WRITE_LINE("vector<" + className + "> " + vp.name + ";");
						}
						else if (vp.type == "enum")
						{
							WRITE_LINE("enum class " + vp.ref)
								WRITE_CURLY_BRACES
								//WRITE_LINE(vp.initialValue + "} " + vp.name + ";")
								vector<string> enums = split(vp.initialValue, ",");
							for (auto e : enums)
							{
								// TODO: 1
								string enum_val = e;
								enumValueFromString(enum_val, e);
								WRITE_LINE(enum_val + ",")
							}
							WRITE_LINE("} " + vp.name + ";")
								WRITE_ENTER
								WRITE_LINE(vp.ref + " fromStringTo" + vp.ref + "(string& s)")
								WRITE_CURLY_BRACES
								string enum_val = enums[0];
								enumValueFromString(enum_val, enums[0]);
								string enum_text;
								correctTextFromString(enum_text, enums[0]);
								WRITE_LINE("if (s == \"" + enum_text + "\") {")
								WRITE_LINE(vp.name + " = " + vp.ref + "::" + enum_val + "; }")
								size_t max = enums.size();
								for (size_t i = 1; i < max; ++i)
								{
									enum_val = enums[i];
									enumValueFromString(enum_val, enums[i]);
									correctTextFromString(enum_text, enums[i]);
									WRITE_LINE("else if (s == \"" + enum_text + "\") {")
									WRITE_LINE(vp.name + " = " + vp.ref + "::" + enum_val + "; }")
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
							else if (vp.type == "object")
							{
								cplusType = vp.ref;
							}
							else if (vp.type == "number")
							{
								cplusType = "double";
							}
							WRITE_LINE(cplusType + " " + vp.name + ";")
						}


						if (vp.ref == "integer")
						{
							vp.ref = "int";
						}
						else if (vp.ref == "boolean")
						{
							vp.ref = "bool";
						}
					}

				// write deserializing
				WRITE_LINE("void fromTheJson(const Value& object)")
				WRITE_CURLY_BRACES
				for (auto vp : properties)
				{
					if (vp.type == "string")
					{
						WRITE_LINE(vp.name + " = " + "object[\"" + vp.name + "\"].GetString();")
					}
					else if (vp.type == "array")
					{
						// get the array object, loop iterator
						WRITE_LINE("for (Value::ConstValueIterator vItr = object[\"" + vp.name + "\"].GetArray().Begin();")
							WRITE_LINE("vItr != object[\"" + vp.name + "\"].GetArray().End(); ++vItr)")
							WRITE_CURLY_BRACES
							WRITE_LINE(" " + vp.ref + " ele;")
							if (vp.ref == "string")
							{
								WRITE_LINE(" ele = vItr->GetString();")
							}
							else if (vp.ref == "bool")
							{
								WRITE_LINE(" ele = vItr->GetBool();")
							}
							else if (vp.ref == "int")
							{
								WRITE_LINE(" ele = vItr->GetInt();")
							}
							else
								WRITE_LINE(" ele.fromTheJson(*vItr);")
							WRITE_LINE(" " + vp.name + ".push_back(ele);")
							WRITE_CURLY_BRACES_END
					}
					else if (vp.type == "enum")
					{
						string enumName = vp.name;
						enumName[0] = toupper(enumName[0]);
						WRITE_LINE(vp.name + " = " + "fromStringTo" + enumName +"(string(object[\"" + vp.name + "\"].GetString()));")
					}
					else if (vp.type == "object")
					{
						WRITE_LINE(vp.name + ".fromTheJson(object[\"" + vp.name + "\"]);");
					}
				}
				WRITE_CURLY_BRACES_END
			}
		}
	}

	void RapidJsonCodeGenerator::topologicalDefSort(const Value& dvals, string& className, set<string>& classSet, queue<string>& classQueue)
	{
		classSet.insert(className);

		const Value& def_val = dvals[className.c_str()];
		const Value& pvals = def_val["properties"];

		for (Value::ConstMemberIterator pitr = pvals.MemberBegin();
			pitr != pvals.MemberEnd(); ++pitr)
		{
			const Value& p_val = pitr->value;
			string proName = pitr->name.GetString();
			string defName;
			if (p_val.HasMember("$ref"))
			{
				classNameFromRef(p_val, defName);
			}
			else if (p_val.HasMember("items"))
			{
				classNameFromRef(p_val["items"], defName);
			}
			if (!defName.empty())
			{
				if (classSet.find(defName) == classSet.end())
				{
					topologicalDefSort(dvals, defName, classSet, classQueue);
				}
			}
		}

		classQueue.push(className);
	}

	void RapidJsonCodeGenerator::outputExtraDefinitionCode(const Value& object)
	{
		if (object.HasMember("definitions"))
		{
			if (object["definitions"].GetType() == kObjectType)
			{
				const Value& dvals = object["definitions"];

				//stack<string> classStack;
				queue<string> classQueue;
				set<string> classSet;

				for (Value::ConstMemberIterator itr = dvals.MemberBegin();
					itr != dvals.MemberEnd(); ++itr)
				{
					const Value& def_val = itr->value;
					string defName = itr->name.GetString();
					if (classSet.find(defName) == classSet.end())
					{
						topologicalDefSort(dvals, defName, classSet, classQueue);
					}
				}

				while (!classQueue.empty())
				{
					string name = classQueue.front();
					classQueue.pop();
					
					const Value& def_val = dvals[name.c_str()];
					if (def_val.HasMember("title"))
					{
						outputClassDefinition(def_val);
					}
					else
					{
						outputClassDefinitionCode(def_val, name);
					}
				}
			}
		}
	}
}