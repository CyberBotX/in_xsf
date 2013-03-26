/*  Copyright 2009-2010 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string>

#ifdef _MSC_VER
#define mkdir _mkdir
#endif

#if defined(_WINDOWS)
//#define WIN32_LEAN_AND_MEAN
//#include <WinSock2.h>
#include "windowsh_wrapper.h"
#include <direct.h>
#if !defined(WXPORT)
#include "resource.h"
#else
#include <glib.h>
#endif /* !WXPORT */
#else
#include <glib.h>
#endif /* _WINDOWS */

#include <cstdlib>
#include <ctime>
#include <cstdio>
#include "utils/xstring.h"

#ifdef _WINDOWS
#define FILE_EXT_DELIMITER_CHAR		'.'
#define DIRECTORY_DELIMITER_CHAR	'\\'
#else
#define FILE_EXT_DELIMITER_CHAR		'.'
#define DIRECTORY_DELIMITER_CHAR	'/'
#endif

#ifdef _WINDOWS
void FCEUD_MakePathDirs(const char *fname);
#endif

class Path
{
public:
	static bool IsPathRooted (const std::string &path);
	static std::string GetFileDirectoryPath(std::string filePath);
	static std::string GetFileNameFromPath(std::string filePath);
	static std::string GetFileNameWithoutExt(std::string fileName);
	static std::string GetFileNameFromPathWithoutExt(std::string filePath);
	static std::string GetFileExt(std::string fileName);
};

class PathInfo
{
public:

	std::string path;
	std::string RomName;
	std::string RomDirectory;

	#define MAX_FORMAT		20
	#define SECTION			"PathSettings"

	#define ROMKEY			"Roms"
	#define BATTERYKEY		"Battery"
	#define STATEKEY		"States"
	#define SCREENSHOTKEY	"Screenshots"
	#define AVIKEY			"AviFiles"
	#define CHEATKEY		"Cheats"
	#define SOUNDKEY		"SoundSamples"
	#define FIRMWAREKEY		"Firmware"
	#define FORMATKEY		"format"
	#define DEFAULTFORMATKEY "defaultFormat"
	#define NEEDSSAVINGKEY	"needsSaving"
	#define LASTVISITKEY	"lastVisit"
	#define LUAKEY			"Lua"
	char screenshotFormat[MAX_FORMAT];
	bool savelastromvisit;

	enum KnownPath
	{
		FIRSTKNOWNPATH = 0,
		ROMS = 0,
		BATTERY,
		STATES,
		SCREENSHOTS,
		AVI_FILES,
		CHEATS,
		SOUNDS,
		FIRMWARE,
		MODULE,
		MAXKNOWNPATH = MODULE
	};

	char pathToRoms[MAX_PATH];
	char pathToBattery[MAX_PATH];
	char pathToStates[MAX_PATH];
	char pathToScreenshots[MAX_PATH];
	char pathToAviFiles[MAX_PATH];
	char pathToCheats[MAX_PATH];
	char pathToSounds[MAX_PATH];
	char pathToFirmware[MAX_PATH];
	char pathToModule[MAX_PATH];
	char pathToLua[MAX_PATH];

	void init(const char * filename) {

		path = std::string(filename);

		//extract the internal part of the logical rom name
		std::vector<std::string> parts = tokenize_str(filename,"|");
		SetRomName(parts[parts.size()-1].c_str());
		LoadModulePath();
#ifndef WIN32
		//ReadPathSettings();
#endif

	}

	void LoadModulePath()
	{
#if defined(_WINDOWS)

		char *p;
		ZeroMemory(pathToModule, sizeof(pathToModule));

		GetModuleFileNameA(NULL, pathToModule, sizeof(pathToModule));
		p = pathToModule + strlen(pathToModule);
		while (p >= pathToModule && *p != DIRECTORY_DELIMITER_CHAR) p--;
		if (++p >= pathToModule) *p = 0;
#else
		char *cwd = g_build_filename(g_get_user_config_dir(), "desmume", NULL);
		g_mkdir_with_parents(cwd, 0755);
		strncpy(pathToModule, cwd, MAX_PATH);
		g_free(cwd);
#endif
	}

	enum Action
	{
		GET,
		SET
	};

	void GetDefaultPath(char *pathToDefault, const char *, int maxCount)
	{
#ifdef _WINDOWS
		std::string temp = (std::string)"." + DIRECTORY_DELIMITER_CHAR + pathToDefault;
		strncpy(pathToDefault, temp.c_str(), maxCount);
#else
		strncpy(pathToDefault, pathToModule, maxCount);
#endif
	}

	/*void ReadKey(char *pathToRead, const char *key)
	{
#ifdef _WINDOWS
		GetPrivateProfileString(SECTION, key, key, pathToRead, MAX_PATH, IniName);
		if(strcmp(pathToRead, key) == 0) {
			//since the variables are all intialized in this file they all use MAX_PATH
			GetDefaultPath(pathToRead, key, MAX_PATH);
		}
#else
		//since the variables are all intialized in this file they all use MAX_PATH
		GetDefaultPath(pathToRead, key, MAX_PATH);
#endif
	}*/

	/*void ReadPathSettings()
	{
		if( ( strcmp(pathToModule, "") == 0) || !pathToModule)
			LoadModulePath();

		ReadKey(pathToRoms, ROMKEY);
		ReadKey(pathToBattery, BATTERYKEY);
		ReadKey(pathToStates, STATEKEY);
		ReadKey(pathToScreenshots, SCREENSHOTKEY);
		ReadKey(pathToAviFiles, AVIKEY);
		ReadKey(pathToCheats, CHEATKEY);
		ReadKey(pathToSounds, SOUNDKEY);
		ReadKey(pathToFirmware, FIRMWAREKEY);
		ReadKey(pathToLua, LUAKEY);
#ifdef _WINDOWS
		GetPrivateProfileString(SECTION, FORMATKEY, "%f_%s_%r", screenshotFormat, MAX_FORMAT, IniName);
		savelastromvisit	= GetPrivateProfileBool(SECTION, LASTVISITKEY, true, IniName);
		currentimageformat	= (ImageFormat)GetPrivateProfileInt(SECTION, DEFAULTFORMATKEY, PNG, IniName);
#endif*/
	/*
		needsSaving		= GetPrivateProfileInt(SECTION, NEEDSSAVINGKEY, true, IniName);
		if(needsSaving)
		{
			needsSaving = false;
			WritePathSettings();
		}*/
	/*}*/

	void SwitchPath(Action action, KnownPath knownPath, char * buffer)
	{
		char *pathToCopy = 0;
		switch(knownPath)
		{
		case ROMS:
			pathToCopy = pathToRoms;
			break;
		case BATTERY:
			pathToCopy = pathToBattery;
			break;
		case STATES:
			pathToCopy = pathToStates;
			break;
		case SCREENSHOTS:
			pathToCopy = pathToScreenshots;
			break;
		case AVI_FILES:
			pathToCopy = pathToAviFiles;
			break;
		case CHEATS:
			pathToCopy = pathToCheats;
			break;
		case SOUNDS:
			pathToCopy = pathToSounds;
			break;
		case FIRMWARE:
			pathToCopy = pathToFirmware;
			break;
		case MODULE:
			pathToCopy = pathToModule;
			break;
		}

		if(action == GET)
		{
			std::string thePath = pathToCopy;
			std::string relativePath = (std::string)"." + DIRECTORY_DELIMITER_CHAR;

			int len = (int)thePath.size()-1;

			if(len == -1)
				thePath = relativePath;
			else
				if(thePath[len] != DIRECTORY_DELIMITER_CHAR)
					thePath += DIRECTORY_DELIMITER_CHAR;

			if(!Path::IsPathRooted(thePath))
			{
				thePath = (std::string)pathToModule + thePath;
			}

			strncpy(buffer, thePath.c_str(), MAX_PATH);
			#ifdef _WINDOWS
			FCEUD_MakePathDirs(buffer);
			#endif
		}
		else if(action == SET)
		{
			int len = strlen(buffer)-1;
			if(buffer[len] == DIRECTORY_DELIMITER_CHAR)
				buffer[len] = '\0';

			strncpy(pathToCopy, buffer, MAX_PATH);
		}
	}

	std::string getpath(KnownPath knownPath)
	{
		char temp[MAX_PATH];
		SwitchPath(GET, knownPath, temp);
		return temp;
	}

	void getpath(KnownPath knownPath, char *buffer)
	{
		SwitchPath(GET, knownPath, buffer);
	}

	void setpath(KnownPath knownPath, char *buffer)
	{
		SwitchPath(SET, knownPath, buffer);
	}

	void getfilename(char *buffer, int)
	{
		strcpy(buffer,noextension().c_str());
	}

	void getpathnoext(KnownPath knownPath, char *buffer)
	{
		getpath(knownPath, buffer);
		strcat(buffer, GetRomNameWithoutExtension().c_str());
	}

	std::string extension()
	{
		return Path::GetFileExt(path);
	}

	std::string noextension()
	{
		std::string romNameWithPath = Path::GetFileDirectoryPath(path) + DIRECTORY_DELIMITER_CHAR + Path::GetFileNameWithoutExt(RomName);

		return romNameWithPath;
	}

	void formatname(char *output)
	{
		std::string file;
		time_t now = time(NULL);
		tm *time_struct = localtime(&now);
		srand((unsigned int)now);

		for(int i = 0; i < MAX_FORMAT;i++)
		{
			char *c = &screenshotFormat[i];
			char tmp[MAX_PATH] = {0};

			if(*c == '%')
			{
				c = &screenshotFormat[++i];
				switch(*c)
				{
				case 'f':

					strcat(tmp, GetRomNameWithoutExtension().c_str());
					break;
				case 'D':
					strftime(tmp, MAX_PATH, "%d", time_struct);
					break;
				case 'M':
					strftime(tmp, MAX_PATH, "%m", time_struct);
					break;
				case 'Y':
					strftime(tmp, MAX_PATH, "%Y", time_struct);
					break;
				case 'h':
					strftime(tmp, MAX_PATH, "%H", time_struct);
					break;
				case 'm':
					strftime(tmp, MAX_PATH, "%M", time_struct);
					break;
				case 's':
					strftime(tmp, MAX_PATH, "%S", time_struct);
					break;
				case 'r':
					sprintf(tmp, "%d", rand() % RAND_MAX);
					break;
				}
			}
			else
			{
				int j;
				for(j=i;j<MAX_FORMAT-i;j++)
					if(screenshotFormat[j] != '%')
						tmp[j-i]=screenshotFormat[j];
					else
						break;
				tmp[j-i]='\0';
			}
			file += tmp;
		}
		strncpy(output, file.c_str(), MAX_PATH);
	}

	enum ImageFormat
	{
#if defined(_WINDOWS) && !defined(WXPORT)
		PNG = IDC_PNG,
		BMP = IDC_BMP
#else
		PNG,
		BMP
#endif
	};

	ImageFormat currentimageformat;

	ImageFormat imageformat() {
		return currentimageformat;
	}

	void SetRomName(const char *filename)
	{
		std::string romPath = filename;

		RomName = Path::GetFileNameFromPath(romPath);
		RomDirectory = Path::GetFileDirectoryPath(romPath);
	}

	const char *GetRomName()
	{
		return RomName.c_str();
	}

	std::string GetRomNameWithoutExtension()
	{
		return Path::GetFileNameWithoutExt(RomName);
	}

	bool isdsgba(std::string fileName)
	{
		size_t i = fileName.find_last_of(FILE_EXT_DELIMITER_CHAR);

		if (i != std::string::npos) {
			fileName = fileName.substr(i - 2);
		}

		if(fileName == "ds.gba") {
			return true;
		}

		return false;
	}
};

extern PathInfo path;
