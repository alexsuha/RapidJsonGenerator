
#include <shlobj.h>
#include <iostream>
#include <algorithm>
#include "PathUtils.h"

namespace PathUtils
{
	static int CALLBACK browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
	{

		if (uMsg == BFFM_INITIALIZED)
		{
			std::string tmp = (const char *)lpData;
			//cout << "path: " << tmp.c_str() << endl;
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		}

		return 0;
	}

	void searchFilesFromDirectory(string& path, vector<string>& names)
	{
		WIN32_FIND_DATAA fd;
		HANDLE hFind = ::FindFirstFileA(path.c_str(), &fd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do {
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					names.push_back(fd.cFileName);
				}
			} while (::FindNextFileA(hFind, &fd));
			::FindClose(hFind);
		}

		for (auto name : names)
		{
			transform(name.begin(), name.end(), name.begin(),
				[](unsigned char c) { return tolower(c); });
		}
	}

	string browseNativeFolder()
	{
		char path[MAX_PATH];

		const char* path_param = "C:\\";

		BROWSEINFOA bi = { 0 };
		bi.lpszTitle = ("Browse for folder...");
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		bi.lpfn = browseCallbackProc;
		bi.lParam = (LPARAM)path_param;

		LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);

		if (pidl != 0)
		{
			//get the name of the folder and put it in path
			SHGetPathFromIDListA(pidl, path);

			//free memory used
			IMalloc* imalloc = 0;
			if (SUCCEEDED(SHGetMalloc(&imalloc)))
			{
				imalloc->Free(pidl);
				imalloc->Release();
			}

			return path;
		}

		return "";
	}
}