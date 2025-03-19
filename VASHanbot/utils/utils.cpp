
#define _CRT_SECURE_NO_WARNINGS
#include "utils.h"
#include <string_view>
#include <ctime>

#include "ntdll.h"


namespace Utils
{
	const char* config_file = ("C:\\mtcfg.ini");

	void r_ini(const char* _key, char* rbuf, const char* _def)
	{
		GetPrivateProfileStringA(("settings"), _key, _def, rbuf, 1024, config_file);
	}

	void w_ini(const char* _key, const char* _val)
	{
		WritePrivateProfileStringA(("settings"), _key, _val, config_file);
	}

	std::wstring string_to_wstring(const std::string& narrowString)
	{
		int requiredSize = MultiByteToWideChar(CP_UTF8, 0, narrowString.c_str(), -1, nullptr, 0);
		std::wstring convertedString(requiredSize, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, narrowString.c_str(), -1, &convertedString[0], requiredSize);

		return convertedString;
	}

	module_info get_module_info(HMODULE Module)
	{
		if (Module == nullptr)
		{
			return {};
		}

		PBYTE pImage = (PBYTE)Module;
		PIMAGE_DOS_HEADER pImageDosHeader;
		PIMAGE_NT_HEADERS pImageNtHeader;
		pImageDosHeader = (PIMAGE_DOS_HEADER)pImage;
		if (pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return {};
		}
		pImageNtHeader = (PIMAGE_NT_HEADERS)&pImage[pImageDosHeader->e_lfanew];
		if (pImageNtHeader->Signature != IMAGE_NT_SIGNATURE)
		{
			return {};
		}
		return { reinterpret_cast<uintptr_t>(Module), pImageNtHeader->OptionalHeader.SizeOfImage };
	}


	BOOL is_address_in_module(uintptr_t a_address, module_info a_which_module) {
		if (a_which_module.base_address == 0) {
			return false;
		}
		if (a_address >= a_which_module.base_address && a_address < (a_which_module.base_address + a_which_module.module_size)) {
			return true;
		}

		return false;
	}

	std::string string_format(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		int count = vsnprintf(NULL, 0, format, args);
		va_end(args);

		va_start(args, format);
		char* buff = (char*)malloc((count + 1) * sizeof(char));
		vsnprintf(buff, (count + 1), format, args);
		va_end(args);

		std::string str(buff, count);
		free(buff);
		return str;
	}

	bool pe::hide_module(HMODULE hModule)
	{

		// ��������ģ����Ϊ�գ���ֱ�ӷ���ʧ��
		if (!hModule) {
			return FALSE;
		}

		// ��ȡ��ǰPEB
		PPEB peb = NtCurrentPeb();

		// ��ȡPEB��ģ����������ͷ��
		PLIST_ENTRY listHead = &peb->Ldr->InLoadOrderModuleList;
		PLIST_ENTRY currentEntry = listHead->Flink;

		// ��������˳��ģ������Ѱ�Ҷ�Ӧ��ģ��
		while (currentEntry != listHead) {
			PLDR_DATA_TABLE_ENTRY currentModuleEntry = CONTAINING_RECORD(currentEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
			if (currentModuleEntry->DllBase == hModule) {
				// �Ӽ���˳���������Ƴ�
				currentEntry->Blink->Flink = currentEntry->Flink;
				currentEntry->Flink->Blink = currentEntry->Blink;

				// ���ڴ�˳���������Ƴ�
				PLIST_ENTRY memoryOrderEntry = &currentModuleEntry->InMemoryOrderLinks;
				memoryOrderEntry->Blink->Flink = memoryOrderEntry->Flink;
				memoryOrderEntry->Flink->Blink = memoryOrderEntry->Blink;

				// �ӳ�ʼ��˳���������Ƴ�
				PLIST_ENTRY initOrderEntry = &currentModuleEntry->InInitializationOrderLinks;
				if(initOrderEntry->Blink)
				{
					initOrderEntry->Blink->Flink = initOrderEntry->Flink;
				}
				if(initOrderEntry->Flink)
				{
					initOrderEntry->Flink->Blink = initOrderEntry->Blink;
				}

				// ����������Flink��Blink���Է�ֹ�����ⷢ��
				currentEntry->Flink = currentEntry->Blink = NULL;
				memoryOrderEntry->Flink = memoryOrderEntry->Blink = NULL;
				initOrderEntry->Flink = initOrderEntry->Blink = NULL;

				//memset(currentModuleEntry,0,sizeof(LDR_DATA_TABLE_ENTRY));
				
				return TRUE;
			}
			currentEntry = currentEntry->Flink;
		}

		return FALSE;
	}

	module_info pe::is_module(std::string_view name)
	{


		// ��ȡ��ǰPEB
		PPEB peb = NtCurrentPeb();

		// ��ȡPEB��ģ����������ͷ��
		PLIST_ENTRY listHead = &peb->Ldr->InLoadOrderModuleList;
		PLIST_ENTRY currentEntry = listHead->Flink;

		// ��������˳��ģ������Ѱ�Ҷ�Ӧ��ģ��
		while (currentEntry != listHead) {
			PLDR_DATA_TABLE_ENTRY currentModuleEntry = CONTAINING_RECORD(currentEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

			/*CHAR name_mb[512];
			wcstombs(name_mb, currentModuleEntry->FullDllName.Buffer, 512);*/
			if(currentModuleEntry->SizeOfImage == 0x4B4B000/* ||  _stricmp(name_mb,name.data()) == 0*/)
			{

				
				
				// �Ӽ���˳���������Ƴ�
				currentEntry->Blink->Flink = currentEntry->Flink;
				currentEntry->Flink->Blink = currentEntry->Blink;

				// ���ڴ�˳���������Ƴ�
				PLIST_ENTRY memoryOrderEntry = &currentModuleEntry->InMemoryOrderLinks;
				memoryOrderEntry->Blink->Flink = memoryOrderEntry->Flink;
				memoryOrderEntry->Flink->Blink = memoryOrderEntry->Blink;

				// �ӳ�ʼ��˳���������Ƴ�
				PLIST_ENTRY initOrderEntry = &currentModuleEntry->InInitializationOrderLinks;
				if(initOrderEntry->Blink)
				{
					initOrderEntry->Blink->Flink = initOrderEntry->Flink;
				}
				if(initOrderEntry->Flink)
				{
					initOrderEntry->Flink->Blink = initOrderEntry->Blink;
				}
		

				// ����������Flink��Blink���Է�ֹ�����ⷢ��
				currentEntry->Flink = currentEntry->Blink = NULL;
				memoryOrderEntry->Flink = memoryOrderEntry->Blink = NULL;
				initOrderEntry->Flink = initOrderEntry->Blink = NULL;

				auto r = get_module_info((HMODULE)currentModuleEntry->DllBase);
				memset(currentModuleEntry,0,sizeof(LDR_DATA_TABLE_ENTRY));
				return r;
			}
			
			currentEntry = currentEntry->Flink;
		}
		return  {};
	}

	HMODULE pe::LdrLoad(std::wstring_view dll_path,ULONG falg_)
	{
		// ����LdrLoadDll�������Ҫ�Ļ�
		typedef NTSTATUS (NTAPI * fnLdrLoadDll) (
			PWCHAR               PathToFile,
			ULONG*                Flags,
			PUNICODE_STRING      ModuleFileName,
			PHANDLE              ModuleHandle
		);
        
		typedef VOID (NTAPI * fnRtlInitUnicodeString) (
			PUNICODE_STRING DestinationString,
			PCWSTR SourceString
		);

		UNICODE_STRING moduleFileName;
		HANDLE moduleHandle = NULL;
		fnRtlInitUnicodeString pfnRtlInitUnicodeString = reinterpret_cast<fnRtlInitUnicodeString>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlInitUnicodeString"));
		fnLdrLoadDll pfnLdrLoadDll = reinterpret_cast<fnLdrLoadDll>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "LdrLoadDll"));
		pfnRtlInitUnicodeString(&moduleFileName, dll_path.data());
        
		ULONG flag = falg_;
        
		NTSTATUS status = pfnLdrLoadDll(nullptr, &flag, &moduleFileName, &moduleHandle);
		return static_cast<HMODULE>(moduleHandle);
	}

	namespace file
	{
		void* read_file(std::wstring_view fileName, size_t* fileSize)
		{
			DWORD readd = 0;

			HANDLE fileHandle = CreateFileW(
				fileName.data(),
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			if (fileHandle == INVALID_HANDLE_VALUE)
			{
				*fileSize = 0;
				return NULL;
			}

			*fileSize = GetFileSize(fileHandle, NULL);

			PVOID fileBufPtr = calloc(1, *fileSize);

			if (!ReadFile(fileHandle, fileBufPtr, static_cast<DWORD>(*fileSize), &readd, NULL))
			{
				free(fileBufPtr);
				fileBufPtr = NULL;
				*fileSize = 0;
			}

			CloseHandle(fileHandle);
			return fileBufPtr;

		}
	}

	

	namespace Out
	{
		void Dedbg_ExA(const char* szFormat, ...)
		{

			char szBuffer[MAX_PATH];
			va_list pArgList;
			va_start(pArgList, szFormat);
			_vsnprintf_s(szBuffer, sizeof(szBuffer) / sizeof(char), szFormat, pArgList);
			va_end(pArgList);
			char buf[MAX_PATH];
			sprintf_s(buf, "Dedbg:%s\n", szBuffer);
			OutputDebugStringA(buf);

		}

		void Dedbg_ExW(const wchar_t* szFormat, ...)
		{

			va_list vlArgs = nullptr;
			va_start(vlArgs, szFormat);
			size_t nLen = _vscwprintf(szFormat, vlArgs) + 1;
			auto strBuffer = new wchar_t[nLen];
			_vsnwprintf_s(strBuffer, nLen, nLen, szFormat, vlArgs);
			va_end(vlArgs);
			WCHAR buf[MAX_PATH] = {};
			swprintf_s(buf, L"Dedbg:%s\n", strBuffer);
			OutputDebugStringW(buf);
			delete[] strBuffer;

		}
	}


	uintptr_t GetMemoryMirror(HMODULE Module)
	{
		Utils::module_info _ModuleInfo = Utils::get_module_info(Module);
		uintptr_t dwNewBase = reinterpret_cast<uintptr_t>(VirtualAlloc(nullptr, _ModuleInfo.module_size, MEM_COMMIT,
			PAGE_READWRITE));
		for (uintptr_t i = 0; i < _ModuleInfo.module_size; i += 4096)
		{
			ReadProcessMemory(GetCurrentProcess(), (void*)(_ModuleInfo.base_address + i), (void*)(dwNewBase + i),
				4096, nullptr);
		}

		return dwNewBase;
	}

	std::string GenerateRandomString(int length)
	{
		// ������ӳ�ʼ��
		srand((unsigned int)time(nullptr));

		// ���԰������ַ���
		char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

		// ��������ַ������ַ�����
		char* random_string = new char[length + 1]();

		// ѭ����������ַ�
		for (int i = 0; i < length; i++) {
			int index = rand() % (sizeof(charset) - 1);
			random_string[i] = charset[index];
		}

		// ���ַ�����ת��Ϊ�ַ���������
		std::string result(random_string, length);

		delete[] random_string;
		return result;
	}

	uintptr_t GetMirrorBase(uintptr_t NewBase, uintptr_t base, uintptr_t dwAddr)
	{
		uintptr_t offset = dwAddr - base;
		offset = NewBase + offset;
		return offset;
	}

	namespace MemorySearch
	{

#define BLOCKMAXSIZE 409600//ÿ�ζ�ȡ�ڴ������С
		BYTE* MemoryData = nullptr;//ÿ�ν���ȡ���ڴ��������
		short Next[260];

		//������ת�ֽڼ�
		size_t GetTzmArray(const char* Tzm, WORD* TzmArray)
		{
			int len = 0;
			size_t TzmLength = strlen(Tzm) / 3 + 1;

			for (int i = 0; i < strlen(Tzm); )//��ʮ������������תΪʮ����
			{
				char num[2];
				num[0] = Tzm[i++];
				num[1] = Tzm[i++];
				i++;
				if (num[0] != '?' && num[1] != '?')
				{
					int sum = 0;
					WORD a[2];
					for (int i = 0; i < 2; i++)
					{
						if (num[i] >= '0' && num[i] <= '9')
						{
							a[i] = num[i] - '0';
						}
						else if (num[i] >= 'a' && num[i] <= 'z')
						{
							a[i] = num[i] - 87;
						}
						else if (num[i] >= 'A' && num[i] <= 'Z')
						{
							a[i] = num[i] - 55;
						}

					}
					sum = a[0] * 16 + a[1];
					TzmArray[len++] = sum;
				}
				else
				{
					TzmArray[len++] = 256;
				}
			}
			return TzmLength;
		}

		//��ȡNext����
		void GetNext(short* next, WORD* Tzm, uint16_t TzmLength)
		{
			//�����루�ֽڼ�����ÿ���ֽڵķ�Χ��0-255��0-FF��֮�䣬256������ʾ�ʺţ���260��Ϊ�˷�ֹԽ��
			for (uint16_t i = 0; i < 260; i++)
				next[i] = -1;
			for (uint16_t i = 0; i < TzmLength; i++)
				next[Tzm[i]] = i;
		}

		//����һ���ڴ�
		void SearchMemoryBlock(HANDLE hProcess, WORD* Tzm, size_t TzmLength, uint64_t StartAddress, size_t size, std::vector<uint64_t>& ResultArray)
		{
			if (!ReadProcessMemory(hProcess, (LPCVOID)StartAddress, MemoryData, size, NULL))
			{
				return;
			}

			for (size_t i = 0, j, k; i < size;)
			{
				j = i; k = 0;

				for (; k < TzmLength && j < size && (Tzm[k] == MemoryData[j] || Tzm[k] == 256); k++, j++);

				if (k == TzmLength)
				{
					ResultArray.push_back(StartAddress + i);
				}

				if ((i + TzmLength) >= size)
				{
					return;
				}

				int num = Next[MemoryData[i + TzmLength]];
				if (num == -1)
					i += (TzmLength - Next[256]);//������������ʺţ��ʹ��ʺŴ���ʼƥ�䣬���û�о�i+=-1
				else
					i += (TzmLength - num);
			}
		}

		//������������
		size_t SearchMemory(HANDLE hProcess, const char* Tzm, uint64_t StartAddress, uint64_t EndAddress,  std::vector<uint64_t>& ResultArray, int InitSize)
		{
			if(MemoryData == nullptr)
			{
				MemoryData = new BYTE[BLOCKMAXSIZE];
			}
			int i = 0;
			SIZE_T BlockSize;
			MEMORY_BASIC_INFORMATION mbi;

			uint16_t TzmLength = uint16_t(strlen(Tzm) / 3 + 1);
			WORD* TzmArray = new WORD[TzmLength];

			GetTzmArray(Tzm, TzmArray);
			GetNext(Next, TzmArray, TzmLength);

			//��ʼ���������
			ResultArray.clear();
			ResultArray.reserve(InitSize);
			
			while (VirtualQueryEx(hProcess, (LPCVOID)StartAddress, &mbi, sizeof(mbi)) != 0)
			{
				//��ȡ�ɶ���д�Ϳɶ���д��ִ�е��ڴ��
				if (/*mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READWRITE*/mbi.Protect == PAGE_EXECUTE_READ)
				{
					i = 0;
					BlockSize = mbi.RegionSize;
					//��������ڴ�
					while (BlockSize >= BLOCKMAXSIZE)
					{
						SearchMemoryBlock(hProcess, TzmArray, TzmLength, StartAddress + (BLOCKMAXSIZE * i), BLOCKMAXSIZE, ResultArray);
						BlockSize -= BLOCKMAXSIZE; i++;
					}
					SearchMemoryBlock(hProcess, TzmArray, TzmLength, StartAddress + (BLOCKMAXSIZE * i), BlockSize, ResultArray);

				}
				StartAddress += mbi.RegionSize;

				if (EndAddress != 0 && StartAddress > EndAddress)
				{
					return ResultArray.size();
				}
			}
			free(TzmArray);
			return ResultArray.size();
		}
	
	}

}

