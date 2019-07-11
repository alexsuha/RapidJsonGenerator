
#include "PathUtils.h"
#include "RapidJsonUtils.h"
#include "rapidjson/filereadstream.h"

namespace RapidJsonUtil
{
	void RapidJsonHelper::getDocumentFromPath(string & path, Document& doc)
	{
		// TODO: insert return statement here
		FILE *fp;
		fopen_s(&fp, path.c_str(), "rb");

		char readBuffer[4096];
		FileReadStream is(fp, readBuffer, sizeof(readBuffer));

		doc.ParseStream(is);

		if (doc.HasParseError())
		{
			Log("parse error:" + doc.GetParseError());
		}

		fclose(fp);
	}
}