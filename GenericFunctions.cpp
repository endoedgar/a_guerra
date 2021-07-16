#include "stdafx.h"
#include "defines.h"
#include "GameGlobals.h"
#include "3DMath.h"
#include "GenericFunctions.h"
#include "globalheader.h"

Smart::String GlobalIniBuffer;
Smart::Ptr<std::wofstream> t_output(new std::wofstream);

Vector *GetVector(Vector *vec, const INIParser::INIFile &ini, const Smart::String &section, const Smart::String &item, const Vector &def)
{
	if(vec) {
		Smart::String tmp;
		ini.GetStringF(tmp, section, item);
		const size_t subs = GetNSubs(tmp);


		if(subs >= 1) {
			vec->x = GetSub<real>(tmp, 0);
			if(subs >= 2) {
				vec->y = GetSub<real>(tmp, 1);
				if(subs >= 3) {
					vec->z = GetSub<real>(tmp, 2);
				} else
					vec->z = def.z;
			} else {
				vec->y = def.y;
				vec->z = def.z;
			}
		} else {
			*vec = def;
		}
	}
	return vec;
}

#define OTHER_PATH_SEPARATOR	'\\'
#define DEVICE_SEPARATOR		':'

struct al_ffblk			/* file info block for the al_find*() routines */
{
	int attrib;			/* actual attributes of the file found */
	time_t time;		/* modification time of file */

#ifdef UNICODE
	__int64 size;			/* size of file */
	wchar_t name[512];		/* name of file */
#else
	long size;			/* size of file */
	char name[512];		/* name of file */
#endif
	
	void *ff_data;		/* private hook */
};

/* structure for use by the directory scanning routines */
struct FF_DATA
{
#ifdef UNICODE
	struct _wfinddata64_t data;
#else
	struct _finddata_t data;
#endif
	intptr_t handle;
	int attrib;
};

static void fill_ffblk(struct al_ffblk *info)
{
	struct FF_DATA *ff_data = static_cast<struct FF_DATA *>(info->ff_data);

	info->attrib = ff_data->data.attrib;
	info->time = ff_data->data.time_write;
	info->size = ff_data->data.size;

#ifdef UNICODE
	wcscpy(info->name, ff_data->data.name);
#else
	strcpy_s(info->name, sizeof(info->name), ff_data->data.name);
#endif
}

int al_findnext(struct al_ffblk *info)
{
	struct FF_DATA *ff_data = static_cast<struct FF_DATA *>(info->ff_data);

	do {
#ifdef UNICODE
		if (_wfindnext64(ff_data->handle, &ff_data->data) != 0) {
#else
		if (_findnext(ff_data->handle, &ff_data->data) != 0) {
#endif
			return -1;
		}

	} while (ff_data->data.attrib & ~ff_data->attrib);

	fill_ffblk(info);
	return 0;
}

/* al_findclose:
 *  Cleans up after a directory search.
 */
void al_findclose(struct al_ffblk *info)
{
	struct FF_DATA *ff_data = static_cast<struct FF_DATA *>(info->ff_data);

	if (ff_data) {
		_findclose(ff_data->handle);
		delete ff_data;
		info->ff_data = NULL;
	}
}

/* _findfirst:
 *  Initiates a directory search.
 */
int al_findfirst(const Smart::String &pattern, struct al_ffblk *info, int attrib)
{
	struct FF_DATA *ff_data;

	/* allocate ff_data structure */
	ff_data = new struct FF_DATA;

	if (!ff_data) {
		return -1;
	}

	/* attach it to the info structure */
	info->ff_data = static_cast<void*>(ff_data);

	/* Windows defines specific flags for NTFS permissions:
	*	FA_TEMPORARY			0x0100
	*	FA_SPARSE_FILE			0x0200 
	*	FA_REPARSE_POINT		0x0400
	*	FA_COMPRESSED			0x0800
	*	FA_OFFLINE				0x1000
	*	FA_NOT_CONTENT_INDEXED	0x2000
	*	FA_ENCRYPTED			0x4000
	* so we must set them in the mask by default; moreover,
	* in order to avoid problems with flags added in the
	* future, we simply set all bits past the first byte.
	*/
	ff_data->attrib = attrib | 0xFFFFFF00;

	/* start the search */

#ifdef UNICODE
	ff_data->handle = _wfindfirst64(pattern.c_str(), &ff_data->data);
#else
	ff_data->handle = _findfirst(pattern.c_str(), &ff_data->data);
#endif

	if (ff_data->handle < 0) {
		delete ff_data;
		info->ff_data = NULL;
		return -1;
	}

	if (ff_data->data.attrib & ~ff_data->attrib) {
		if (al_findnext(info) != 0) {
			al_findclose(info);
			return -1;
		}
		else
			return 0;
	}

	fill_ffblk(info);
	return 0;
}

Smart::String &replace_filename(Smart::String &dest, const Smart::String &path, const Smart::String &filename, int size)
{
	Smart::String tmp;
	size_t pos;
	Smart::String::char_type c;

	pos = path.GetLength();

	while (pos>0) {
		c = path[pos-1];
		if ((c == '/') || (c == OTHER_PATH_SEPARATOR) || (c == DEVICE_SEPARATOR)) break;
		pos--;
	}

	tmp.Reserve(pos+1);

	for(register size_t a = 0; a < pos; ++a)
		tmp[a] = path[a];
	tmp[pos] = '\0';
	tmp.SetLength(pos);
	tmp += filename;

	dest = tmp;

	return dest;
}

/* for_each_file_ex:
 *	Finds all the files on disk which match the given wildcard specification
 *	and file attributes, and executes callback() once for each. callback()
 *	will be passed three arguments: the first is a string which contains the
 *	completed filename, the second is the actual attributes of the file, and
 *	the third is a void pointer which is simply a copy of param (you can use
 *	this for whatever you like). It must return 0 to let the enumeration
 *	proceed, or any non-zero value to stop it. If an error occurs, the error
 *	code will be stored in errno but the enumeration won't stop. Returns the
 *	number of successful calls made to callback(), that is the number of
 *	times callback() was called and returned 0. The file attribute masks may
 *	contain any of the FA_* flags from dir.h.
 */
int for_each_file_ex(const Smart::String &name, int in_attrib, int out_attrib, int (*callback)(const Smart::String &filename, int attrib, void *param), void *param)
{
	static Smart::String buf;
	struct al_ffblk info;
	int ret, c = 0;

	if(!buf.GetSize())
		buf.Reserve(256);

	if (al_findfirst(name, &info, ~out_attrib) != 0) {
		/* no entry is not an error for for_each_file_ex() */

		return 0;
	}

	do {
		if ((~info.attrib & in_attrib) == 0) {
			replace_filename(buf, name, info.name, buf.GetSize());
			ret = (*callback)(buf, info.attrib, param);

			if (ret != 0)
				break;

			++c;
		}
	} while (al_findnext(&info) == 0);

	al_findclose(&info);

	return c;
}

Smart::String &GetSubString(Smart::String &dest, const Smart::String &str, const unsigned int pos)
{
	const size_t len = str.GetLength();
	#ifdef UNICODE
		std::wostringstream ss;
	#else
		std::ostringstream ss;
	#endif
	unsigned int nesp = 0;
	int gravando = false;
	for(register unsigned int i = 0; i < len; ++i)
	{
		Smart::String::char_type c = str[i];
		if(nesp == pos && !gravando)
		{
			gravando = true;
		}
		if(gravando)
		{
			ss << c;
		}
		if(c == ' ' || c == '\n' || c == '\0')
		{
			++nesp;
			if(gravando)
				break;
		}
	}
	dest = ss.str().c_str();
	return dest;//(float)atof(ss.str().c_str());
}

unsigned int GetNSubs(const Smart::String &str)
{
	const size_t len = str.GetLength();
	if(len) {
		unsigned int nesp = 1;
		bool insidestring = false;
		for(register unsigned int i = 0; i < len; ++i)
		{
			Smart::String::char_type c = str[i];
	
			if(c == '"')
				insidestring = !insidestring;
			if(!insidestring && (c == ' ' || c == '\n' || c == '\0'))
				++nesp;
		}

		return nesp;
	}
	return 0;
}

Smart::String GetSubString(const Smart::String &str, const unsigned int pos)
{
	Smart::String result = "";
	const size_t len = str.GetLength();

	if(len) {
		#ifdef UNICODE
			std::wostringstream ss;
		#else
			std::ostringstream ss;
		#endif
		unsigned int nesp = 0;
		int gravando = false;
		bool insidestring = false;
		for(register unsigned int i = 0; i < len; ++i)
		{
			Smart::String::char_type c = str[i];

			if(c == '"') {
				insidestring = !insidestring;
			}


			if(nesp == pos && !gravando && c != '"')
			{
				gravando = true;
			}
			if(!insidestring && (c == ' ' || c == '\n' || c == '\0'))
			{
				++nesp;
				if(gravando)
					break;
			}
			if(gravando && c != '"')
			{
				ss << c;
			}
		}

		result = ss.str().c_str();
	}
	return result;//(float)atof(ss.str().c_str());
}

#include "debugt.h"

void RuntimeError(const wchar_t *msg, ...)
{
	static Smart::StringW buf;
	static bool error = false;

	std::wostringstream ss;

	if(!buf.GetSize())
		buf.Reserve(1024);
	ASSERT(wcslen(msg) < buf.GetSize())

	if(error)
		return;
	
	va_list ap;
	va_start(ap, msg);

	buf.SetLength(vswprintf(buf.GetWritableBuffer(), buf.GetSize(), msg, ap));

	va_end(ap);
	
	ss << "Erro de Execução: " << buf.c_str();

	TRACEW(ss.str().c_str());

	MessageBoxW(hWnd, buf.c_str(), Smart::StringW("Erro de Execução").c_str(), MB_OK | MB_ICONERROR);
	
	if(hWnd)
		SendMessage(hWnd, WM_CLOSE, NULL, NULL);
	else {
		unexpected();
		terminate();
	}
}

void RuntimeError(const char *msg, ...)
{
	static Smart::StringA buf;
	static bool error = false;
	std::ostringstream *ss;

	if(!buf.GetSize())
		buf.Reserve(1024);
	ASSERT(strlen(msg) < buf.GetSize())

	if(error)
		return;
	
	va_list ap;
	va_start(ap, msg);

	buf.SetLength(vsnprintf(buf.GetWritableBuffer(), buf.GetSize(), msg, ap));

	va_end(ap);

	ss = new std::ostringstream;
	
	*ss << "Erro de Execução: " << buf.c_str();

	TRACE(ss->str().c_str());
	TRACE("\n");

	MessageBoxA(hWnd, buf.c_str(), Smart::StringA("Erro de Execução").c_str(), MB_OK | MB_ICONERROR);

	delete ss;
	
	if(hWnd)
		SendMessage(hWnd, WM_CLOSE, NULL, NULL);
	else {
		//unexpected();
		exit(0);
	}
}

bool SetIniString(const Smart::String &FileINI, const Smart::String &Section, const Smart::String &Key, const Smart::String &Value)
{
	int ret = WritePrivateProfileString(Section.c_str(), Key.c_str(), Value.c_str(), FileINI.c_str());
	if(ret != 1) {
		return false;
	}
	return true;
}

Smart::String &GetIniString(const Smart::String &FileINI, const Smart::String &Section, const Smart::String &Key, const Smart::String &DefaultValue)
{
	GlobalIniBuffer.SetLength(GetPrivateProfileString(Section.c_str(), Key.c_str(), DefaultValue.c_str(), GlobalIniBuffer.GetWritableBuffer(), GlobalIniBuffer.GetSize(), FileINI.c_str()));
	return GlobalIniBuffer;
}

int GetIniInt(const Smart::String &FileINI, const Smart::String &Section, const Smart::String &Key, const int DefaultValue)
{
	#ifdef UNICODE
		std::wstringstream stream;
		std::wstring temp;
	#else
		std::stringstream stream;
		std::string temp;
	#endif
	stream << DefaultValue;
	stream >> temp;
	GetIniString(FileINI, Section, Key, temp.c_str());
	return atoi(GlobalIniBuffer.charstr());
}

real GetIniReal(const Smart::String &FileINI, const Smart::String &Section, const Smart::String &Key, const real DefaultValue)
{
	#ifdef UNICODE
		std::wstringstream stream;
		std::wstring temp;
	#else
		std::stringstream stream;
		std::string temp;
	#endif
	stream << DefaultValue;
	stream >> temp;
	GetIniString(FileINI, Section, Key, temp.c_str());
	return r(atof(GlobalIniBuffer.charstr()));
}

bool GetIniBool(const Smart::String &FileINI, const Smart::String &Section, const Smart::String &Key, const bool DefaultValue)
{
	#ifdef UNICODE
		std::wstringstream stream;
		std::wstring temp;
	#else
		std::stringstream stream;
		std::string temp;
	#endif
	stream << DefaultValue;
	stream >> temp;
	GetIniString(FileINI, Section, Key, temp.c_str());
	return atoi(GlobalIniBuffer.charstr()) ? true : false;
}

Smart::String IntToStr(const int i)
{
	#ifdef UNICODE
		std::wstringstream stream;
		std::wstring str;
	#else
		std::stringstream stream;
		std::string str;
	#endif
	stream << i;
	stream >> str;
	return str.c_str();
}