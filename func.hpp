#pragma once


class MMFILE
{
private:

	void* m = 0;
	HANDLE hF = INVALID_HANDLE_VALUE;
	HANDLE hM = INVALID_HANDLE_VALUE;
	LARGE_INTEGER lu;

public:

	MMFILE(const TCHAR* f)
	{
		m = 0;
		hF = INVALID_HANDLE_VALUE;
		hM = INVALID_HANDLE_VALUE;
		hF = CreateFile(f, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (hF != INVALID_HANDLE_VALUE)
		{
			GetFileSizeEx(hF, &lu);
			hM = CreateFileMapping(hF, 0, PAGE_READONLY | SEC_COMMIT, lu.HighPart, lu.LowPart, 0);
			if (hM != INVALID_HANDLE_VALUE)
				m = MapViewOfFile(hM, FILE_MAP_READ, 0, 0, 0);
		}
	}

	~MMFILE()
	{
		if (m)
			UnmapViewOfFile(m);
		m = 0;
		if (hM != INVALID_HANDLE_VALUE)
			CloseHandle(hM);
		hM = INVALID_HANDLE_VALUE;
		if (hF != INVALID_HANDLE_VALUE)
			CloseHandle(hF);
		hF = INVALID_HANDLE_VALUE;
	}

	operator const char*()
	{
		return (const char*)m;
	}

	long long size()
	{
		return lu.QuadPart;
	}
};


class TRAY
{
private:

	HWND hh;
	NOTIFYICONDATA d;
	UINT cbm;



public:

	BOOL TRAY::Ic(HICON hhh)
	{
		d.uFlags = NIF_ICON;
		d.hIcon = hhh;

		BOOL rv = Shell_NotifyIcon(NIM_MODIFY, &d);
		return rv;
	}

	void Attach(HWND hX3, HICON h, UINT cb, int UID = 1)
	{
		// Create the icon
		memset(&d, 0, sizeof(d));
		hh = hX3;
		cbm = cb;

		d.cbSize = sizeof(d);
		d.hWnd = hh;
		d.uID = UID;
		d.uFlags = NIF_ICON | NIF_TIP;
		if (cbm)
			d.uFlags |= NIF_MESSAGE;
		d.hIcon = h;
		//#define NIIF_LARGE_ICON (0x00000020)
		d.dwInfoFlags |= NIIF_LARGE_ICON | NIIF_USER;
		d.uCallbackMessage = cbm;
		BOOL rv = Shell_NotifyIcon(NIM_ADD, &d);

		d.cbSize = sizeof(d);
		d.uVersion = NOTIFYICON_VERSION;
		rv = Shell_NotifyIcon(NIM_SETVERSION, &d);

		void ToastInit(const wchar_t* ttitle);
		ToastInit(ttitle);
	}
	BOOL Message(const TCHAR* TI, const TCHAR* msg)
	{
		void ShowToast(const wchar_t* ttitle, const wchar_t* msg);
		ShowToast(ttitle, msg);
		return true;
	}
	~TRAY()
	{
		Shell_NotifyIcon(NIM_DELETE, &d);
	}
};

template <typename T>
string StructSer(T& t)
{
	return XML3::Char2Base64((const char*)&t, sizeof(t),false);
}
template <typename T>
T StructUnser(const char* d)
{
	XML3::BXML out;
	XML3::Base64ToChar(d, strlen(d), out);
	T t;
	memcpy((void*)&t, out.p(), sizeof(t));
	return t;
}

// RKEY, quick registry access 
class RKEY
{
private:
	HKEY k = 0;
public:


	class VALUE
	{
	public:
		std::wstring name;
		vector<char> value; // For enums
		HKEY k = 0;
		mutable DWORD ty = 0;

		VALUE(const wchar_t* s, HKEY kk)
		{
			if (s)
				name = s;
			k = kk;
		}

		bool operator =(const wchar_t* val)
		{
			ty = REG_SZ;
			if (RegSetValueEx(k, name.c_str(), 0, REG_SZ, (BYTE*)val, (DWORD)(wcslen(val) * sizeof(wchar_t))) == ERROR_SUCCESS)
				return true;
			return false;
		}
		bool operator =(unsigned long val)
		{
			ty = REG_DWORD;
			return RegSetValueEx(k, name.c_str(), 0, REG_DWORD, (BYTE*)&val, sizeof(val)) == ERROR_SUCCESS;
		}
		bool operator =(unsigned long long val)
		{
			ty = REG_QWORD;
			return RegSetValueEx(k, name.c_str(), 0, REG_QWORD, (BYTE*)&val, sizeof(val)) == ERROR_SUCCESS;
		}


		bool Exists()
		{
			DWORD ch = 0;
			if (RegQueryValueEx(k, name.c_str(), 0, &ty, 0, &ch) == ERROR_FILE_NOT_FOUND)
				return false;
			return true;
		}

		template <typename T>
		operator T() const
		{
			T ch = 0;
			RegQueryValueEx(k, name.c_str(), 0, &ty, 0, &ch);
			std::vector<char> d(ch + 10);
			ch += 10;
			RegQueryValueEx(k, name.c_str(), 0, &ty, (LPBYTE)d.data(), &ch);
			T ret = 0;
			memcpy(&ret, d.data(), sizeof(T));
			return ret;
		}

		operator std::wstring() const
		{
			DWORD ch = 0;
			RegQueryValueEx(k, name.c_str(), 0, &ty, 0, &ch);
			std::vector<char> d(ch + 10);
			ch += 10;
			RegQueryValueEx(k, name.c_str(), 0, &ty, (LPBYTE)d.data(), &ch);
			return std::wstring((const wchar_t*)d.data());
		}

		bool Delete()
		{
			return (RegDeleteValue(k, name.c_str()) == ERROR_SUCCESS);
		}




	};



	RKEY(HKEY kk)
	{
		k = kk;
	}


	RKEY(const RKEY& k)
	{
		operator =(k);
	}
	void operator =(const RKEY& r)
	{
		Close();
		DuplicateHandle(GetCurrentProcess(), r.k, GetCurrentProcess(), (LPHANDLE)&k, 0, false, DUPLICATE_SAME_ACCESS);
	}

	RKEY(RKEY&& k)
	{
		operator =(std::forward<RKEY>(k));
	}
	void operator =(RKEY&& r)
	{
		Close();
		k = r.k;
		r.k = 0;
	}

	void operator =(HKEY kk)
	{
		Close();
		k = kk;
	}

	RKEY(HKEY root, const wchar_t* subkey, DWORD acc = KEY_ALL_ACCESS, bool Op = false)
	{
		Load(root, subkey, acc, Op);
	}
	bool Load(HKEY root, const wchar_t* subkey, DWORD acc = KEY_ALL_ACCESS, bool Op = false)
	{
		Close();
		if (Op)
			return (RegOpenKeyEx(root, subkey, 0, acc, &k) == ERROR_SUCCESS);
		return (RegCreateKeyEx(root, subkey, 0, 0, 0, acc, 0, &k, 0) == ERROR_SUCCESS);
	}

	void Close()
	{
		if (k)
			RegCloseKey(k);
		k = 0;
	}

	~RKEY()
	{
		Close();
	}

	bool Valid() const
	{
		if (k)
			return true;
		return false;
	}

	bool DeleteSingle(const wchar_t* sub)
	{
		return (RegDeleteKey(k, sub) == ERROR_SUCCESS);
	}

	bool Delete(const wchar_t* sub = 0)
	{
#if _WIN32_WINNT >= 0x600
		return (RegDeleteTree(k, sub) == ERROR_SUCCESS);
#else
		return false;
#endif
	}

	bool Flush()
	{
		return (RegFlushKey(k) == ERROR_SUCCESS);
	}

	vector<wstring> EnumSubkeys() const
	{
		vector<wstring> data;
		for (int i = 0;; i++)
		{
			vector<wchar_t> n(300);
			DWORD sz = (DWORD)n.size();
			if (RegEnumKeyEx(k, i, n.data(), &sz, 0, 0, 0, 0) != ERROR_SUCCESS)
				break;
			data.push_back(n.data());
		}
		return data;
	}

	vector<VALUE> EnumValues() const
	{
		vector<VALUE> data;
		for (int i = 0;; i++)
		{
			vector<wchar_t> n(300);
			DWORD sz = (DWORD)n.size();
			DWORD ay = 0;
			RegEnumValue(k, i, n.data(), &sz, 0, 0, 0, &ay);
			vector<char> v(ay);
			DWORD ty = 0;
			sz = (DWORD)n.size();
			if (RegEnumValue(k, i, n.data(), &sz, 0, &ty, (LPBYTE)v.data(), &ay) != ERROR_SUCCESS)
				break;

			VALUE x(n.data(), k);
			x.ty = ty;
			x.value = v;
			data.push_back(x);
		}
		return data;
	}

	VALUE operator [](const wchar_t* v) const
	{
		VALUE kv(v, k);
		return kv;
	}

	operator HKEY()
	{
		return k;
	}
};


// ystring class, wstring <-> string wrapper
class ystring : public std::wstring
{
private:
	mutable std::string asc_str_st;
public:

	// Constructors
	ystring(HWND hh) : std::wstring()
	{
		AssignFromHWND(hh);
	}
	ystring(HWND hh, int ID) : std::wstring()
	{
		AssignFromHWND(GetDlgItem(hh, ID));
	}
	ystring::ystring() : std::wstring()
	{
	}
	ystring(const char* v, int CP = CP_UTF8)
	{
		EqChar(v, CP);
	}
	ystring(const std::string& v, int CP = CP_UTF8)
	{
		EqChar(v.c_str(), CP);
	}
	ystring(const wchar_t* f)
	{
		if (!f)
			return;
		assign(f);
	}
	/*
				// Constructor and format for sprintf-like
				ystring(const wchar_t* f, ...) : std::wstring()
					{
					va_list args;
					va_start(args, f);

					int len = _vscwprintf(f, args) + 100;
					if (len < 8192)
						len = 8192;
					vector<wchar_t> b(len);
					vswprintf_s(b.data(), len, f, args);
					assign(b.data());
					va_end(args);
					}
	*/
	ystring& Format(const wchar_t* f, ...)
	{
		va_list args;
		va_start(args, f);

		int len = _vscwprintf(f, args) + 100;
		if (len < 8192)
			len = 8192;
		vector<wchar_t> b(len);
		vswprintf_s(b.data(), len, f, args);
		assign(b.data());
		va_end(args);
		return *this;
	}

	// operator =
	void operator=(const char* v)
	{
		EqChar(v);
	}
	void operator=(const wchar_t* v)
	{
		assign(v);
	}
	void operator=(const wstring& v)
	{
		assign(v.c_str());
	}
	void operator=(const ystring& v)
	{
		assign(v.c_str());
	}
	void operator=(const string& v)
	{
		EqChar(v.c_str());
	}
	CLSID ToCLSID()
	{
		CLSID a;
		CLSIDFromString(c_str(), &a);
		return a;
	}
	void operator=(CLSID cid)
	{
		wchar_t ad[100] = { 0 };
		StringFromGUID2(cid, ad, 100);
		assign(ad);
	}

	operator const wchar_t*()
	{
		return c_str();
	}

	// asc_str() and a_str() and operator const char*() 
	const std::string& asc_str(int CP = CP_UTF8) const
	{
		const wchar_t* s = c_str();
		int sz = WideCharToMultiByte(CP, 0, s, -1, 0, 0, 0, 0);
		vector<char> d(sz + 100);
		WideCharToMultiByte(CP, 0, s, -1, d.data(), sz + 100, 0, 0);
		asc_str_st = d.data();
		return asc_str_st;
	}
	operator const char*() const
	{
		return a_str();
	}
	const char* a_str(int CP = CP_UTF8) const
	{
		asc_str(CP);
		return asc_str_st.c_str();
	}

	long long ll() const
	{
		return atoll(a_str());
	}

	// Internal Convertor
	void EqChar(const char* v, int CP = CP_UTF8)
	{
		clear();
		if (!v)
			return;
		int sz = MultiByteToWideChar(CP, 0, v, -1, 0, 0);
		vector<wchar_t> d(sz + 100);
		MultiByteToWideChar(CP, 0, v, -1, d.data(), sz + 100);
		assign(d.data());
	}

	// From HWND
	void AssignFromHWND(HWND hh)
	{
		int zl = GetWindowTextLength(hh);
		std::vector<wchar_t> n(zl + 100);
		GetWindowTextW(hh, n.data(), zl + 100);
		assign(n.data());
	}
};


namespace sqlite
{
	using namespace std;
	class query
	{
	private:

		sqlite3* db = 0;
		sqlite3_stmt *stmt = 0;
		string q;

	public:

		int Count()
		{
			if (!stmt)
				return 0;
			int n = 0;
			for (;;)
			{
				int st = sqlite3_step(stmt);
				if (st == SQLITE_DONE)
					break;
				if (st == SQLITE_ROW)
				{
					n = sqlite3_column_int(stmt, 0);
				}
			}
			return n;
		}

		bool R()
		{
			if (!stmt)
				return false;
			int st = sqlite3_step(stmt);
			if (st != SQLITE_ROW)
				return false;
			return true;
		}

		int BindText(int idx,const char* t,size_t st)
		{
			return sqlite3_bind_text(stmt, idx, t, (int)st, 0);
		}


		bool NextRow(vector<string>& r)
		{
			if (!stmt)
				return false;
			r.clear();
			int st = sqlite3_step(stmt);
			if (st != SQLITE_ROW)
				return false;

			int num_cols = sqlite3_column_count(stmt);
			for (int i = 0; i < num_cols; i++)
			{
				const char* qr = (const char*)sqlite3_column_text(stmt, i);
				if (qr)
					r.push_back(qr);
				else
					r.push_back("");
			}
			return true;
		}

		bool NextRow(map<string, string>& r)
		{
			if (!stmt)
				return false;
			r.clear();
			int st = sqlite3_step(stmt);
			if (st != SQLITE_ROW)
				return false;

			int num_cols = sqlite3_column_count(stmt);
			for (int i = 0; i < num_cols; i++)
			{
				const char* qr = (const char*)sqlite3_column_text(stmt, i);
				const char* qn = (const char*)sqlite3_column_name(stmt, i);
				if (!qn)
					qn = "";
				if (!qr)
					qr = "";
				r[qn] = qr;
			}
			return true;
		}

		query(sqlite3* d, const char*qq)
		{
			if (qq == 0 || d == 0)
				return;
			db = d;
			q = qq;
			sqlite3_prepare_v2(db, qq, -1, &stmt, NULL);
		}
		~query()
		{
			if (stmt)
				sqlite3_finalize(stmt);
			stmt = 0;
		}

		query(const query& qu)
		{
			q = qu.q;
			db = qu.db;
			sqlite3_prepare_v2(db, q.c_str(), -1, &stmt, NULL);
		}

		operator bool()
		{
			if (stmt)
				return true;
			return false;
		}

	};

	class sqlite
	{
	private:
		sqlite3* db = 0;

	public:


		sqlite3* h() { return db; }

		sqlite(const sqlite&) = delete;
		sqlite& operator=(const sqlite&) = delete;


		auto last()
		{
			return sqlite3_last_insert_rowid(db);
		}

		sqlite(const char* fn)
		{
			if (fn)
				sqlite3_open(fn, &db);
		}

		~sqlite()
		{
			if (db)
				sqlite3_close(db);
			db = 0;
		}

		operator bool()
		{
			if (db)
				return true;
			return false;
		}
	};

};

shared_ptr<sqlite::sqlite> sql;

void SQLPrep()
{
	sqlite::query q0(sql->h(), "PRAGMA foreign_keys = TRUE"); q0.R();
	sqlite::query q1(sql->h(), "CREATE TABLE IF NOT EXISTS TORRENTS (ID INTEGER PRIMARY KEY, MAGNET TEXT,FILE TEXT,TORRENTFILE TEXT,HASH TEXT,RD TEXT,SCANNED INTEGER) "); q1.R();
	sqlite::query q2(sql->h(), "CREATE TABLE IF NOT EXISTS SETTINGS (ID INTEGER PRIMARY KEY, NAME TEXT,VALUE TEXT) "); q2.R();
	sqlite::query q3(sql->h(), "VACUUM"); q3.R();
}

HRESULT MagnetUnregister()
{
	RKEY r(HKEY_CLASSES_ROOT);
	r.DeleteSingle(L"magnet");
	return S_OK;
}

bool IsMagnet()
{
	// Check installation
	RKEY r3(HKEY_CLASSES_ROOT, L"magnet", KEY_READ, true);
	if (r3.operator HKEY())
		return true;
	return false;
}

bool IsTorrent()
{
	// Check installation
	RKEY r3(HKEY_CLASSES_ROOT, L".torrent", KEY_READ, true);
	if (r3.operator HKEY())
		return true;
	return false;
}

HRESULT MagnetRegister()
{
	// Check installation
	RKEY r3(HKEY_CLASSES_ROOT, L"magnet", KEY_READ, true);
	if (r3.operator HKEY())
		return S_FALSE;

	wchar_t fi[10000] = { 0 };
	GetModuleFileName(0, fi, 10000);

	RKEY r(HKEY_CLASSES_ROOT, L"magnet");
	if (!r.operator HKEY())
		return E_ACCESSDENIED;
	r[L""] = L"URL:magnet";
	ystring y;
	y.Format(L"%s,1", fi);
	r[L"URL Protocol"] = L"";
	r[L"DefaultIcon"] = y;

	RKEY shell(r.operator HKEY(), L"shell");
	RKEY open(shell.operator HKEY(), L"open");
	RKEY command(open.operator HKEY(), L"command");
	y.Format(L"\"%s\" \"%%1\"", fi);
	command[L""] = y;

	return S_OK;
}

HRESULT TorrentUnregister()
{
//	RKEY r(HKEY_CLASSES_ROOT);
	//r.DeleteSingle(L".torrent");
	RKEY r2(HKEY_CLASSES_ROOT);
	r2.Delete(L"FluentTorrentFile");
	return S_OK;
}

HRESULT TorrentRegister()
{
	// Check installation
	wchar_t fi[10000] = { 0 };
	GetModuleFileName(0, fi, 10000);

	RKEY r2(HKEY_CLASSES_ROOT, L".torrent");
	if (!r2.operator HKEY())
		return E_ACCESSDENIED;
	r2[L""] = L"FluentTorrentFile";

	if (true)
	{
		RKEY r(HKEY_CLASSES_ROOT, L"FluentTorrentFile");
		ystring y;
		y.Format(L"%s,1", fi);
		r[L"DefaultIcon"] = y;

		RKEY shell(r.operator HKEY(), L"shell");
		RKEY open(shell.operator HKEY(), L"open");
		RKEY command(open.operator HKEY(), L"command");
		y.Format(L"\"%s\" \"%%1\"", fi);
		command[L""] = y;
	}
	return S_OK;
}



BOOL RunAsAdmin(HWND hWnd, LPTSTR lpFile, LPTSTR lpParameters)
{
	SHELLEXECUTEINFO sei = { 0 };
	sei.cbSize = sizeof(SHELLEXECUTEINFOW);
	sei.hwnd = hWnd;
	sei.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
	sei.lpVerb = _TEXT("runas");
	sei.lpFile = lpFile;
	sei.lpParameters = lpParameters;
	sei.nShow = SW_SHOWNORMAL;

	BOOL X = ShellExecuteEx(&sei);
	WaitForSingleObject(sei.hProcess, INFINITE);
	CloseHandle(sei.hProcess);
	return X;
}

ystring SizeValue(unsigned long long s, bool ps = false)
{
	ystring f;
	if (s < 1000000)
	{
		// KB
		double d = ((double)s / 1024.0);
		f.Format(L"%.1f KB", d);
	}
	else
		if (s < 1048576000)
		{
			// MB
			s /= 1024;
			double d = ((double)s / 1024.0);
			f.Format(L"%.1f MB", d);
		}
		else
		{
			// GB
			s /= 1024;
			s /= 1024;
			double d = ((double)s / 1024.0);
			f.Format(L"%.1f GB", d);
		}
	if (ps)
		f += L"/s";
	return f;
}
