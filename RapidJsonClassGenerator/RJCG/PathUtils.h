#ifndef _PATH_UTILS_H_
#define _PATH_UTILS_H_

#include <Windows.h>
#include <vector>
#include <iostream>

using namespace std;

#define Log(x) { cout << x << endl; }

namespace PathUtils
{
	static int CALLBACK browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

	void searchFilesFromDirectory(string& path, vector<string>& names);

	string browseNativeFolder();
}

#endif
