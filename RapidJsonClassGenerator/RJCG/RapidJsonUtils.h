#ifndef RAPIDJSON_UTILS_H_
#define RAPIDJSON_UTILS_H_

#include <string>
#include "rapidjson/document.h"

using namespace rapidjson;
using namespace std;

namespace RapidJsonUtil
{
	class RapidJsonHelper
	{
	public:
		RapidJsonHelper() {};
		~RapidJsonHelper() {}

		void getDocumentFromPath(string& path, Document& doc);
	};
}

#endif // !RAPIDJSON_UTILS_H_

