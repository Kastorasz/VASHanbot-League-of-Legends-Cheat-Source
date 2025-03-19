#define _CRT_RAND_S
#include "permit.h"
#include <atlenc.h>//base64
#include <intrin.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <cmath>
#include "softxlic/include/softxlic.h"
#include "VMProtectSDK.h"
#include "../utils/utils.h"
#include "../BotSDK.h"


#pragma optimize( "", off )

#ifndef DEVELOPER
namespace permit
{
	uintptr_t* g_dumyArray = nullptr;
	//XAntiDebug antiDebug = { nullptr,0 };
	std::string end_time_str = {};
	HMODULE my_Module = 0;

	std::string end_seconds;
	std::string user_type;

	namespace EAD
	{
		std::string Rc4Encrypt(UCHAR* Data, int DataLen)
		{
			std::string result;

			unsigned char box[256];
			unsigned char pwd[256];
			int i, j, k, a;
			unsigned char tmp = 0;
			unsigned char key[] = "0x%8x(%u#)0x%d%s"; //���������RC4����KEY,����д �൱�ڰ�key���� ջ �ڴ���
			//unsigned char key[] = "0x%8x(%u#)0x%d%"; //���������RC4����KEY,����д �൱�ڰ�key���� ջ �ڴ���
			//$8#1*?hXp~ArlZEbS^tHlQFeFbT~Vjwq
			int pwd_length = (int)strlen((const char*)key);

			for (i = 0; i < 256; i++)
			{
				pwd[i] = (unsigned char)key[i % pwd_length];

				box[i] = i;
			}
			j = 0;
			for (i = 0; i < 256; i++)
			{
				j = (j + box[i] + pwd[i]) % 256;
				tmp = box[i];
				box[i] = box[j];
				box[j] = tmp;
			}

			a = 0;
			j = 0;

			for (i = 0; i < DataLen; i++)
			{
				a = (a + 1) % 256;
				j = (j + box[a]) % 256;
				tmp = box[a];
				box[a] = box[j];
				box[j] = tmp;
				k = box[((box[a] + box[j]) % 256)];
				Data[i] ^= k;
			}

			int enBufLen = Base64EncodeGetRequiredLength(DataLen);

			result.assign(enBufLen, '\0');

			Base64Encode(Data, DataLen, (LPSTR)result.c_str(), &enBufLen);

			return result;
		}

		std::vector<std::string> StringSplit(const  std::string& s, const std::string& delim)
		{
			std::vector<std::string> elems;
			size_t pos = 0;
			size_t len = s.length();
			size_t delim_len = delim.length();
			if (delim_len == 0) return elems;
			while (pos < len)
			{
				size_t find_pos = s.find(delim, pos);
				if (find_pos == std::string::npos)
				{
					elems.push_back(s.substr(pos, len - pos));
					break;
				}
				elems.push_back(s.substr(pos, find_pos - pos));
				pos = find_pos + delim_len;
			}
			return elems;
		}

	}


	int OnInitKs()
	{
		//��������Ƿ񻺴�߼�API�ķ���ֵ��Ϊtureʱ �ӿںͲ�����ͬ�ĸ߼�API�����л���ʱֱ�Ӵӻ����ж�ȡ
		slt.CacheAPI = false;

		//�������IPC����ͨѶģʽ��guid��ֻ����a-zA-Z0-9֮����ַ��������ǽ���ͨѶģʽ������""
		//slt.g_strIPCGuid = "YYDS11";

		//��������Զ������RSA��Կ
		slt.g_rsaPubKey = "010001";

		//��������Զ������RSAģ��
		slt.g_rsaMods = "C5E1C658F8834BAE1EBF334DB598BF491A77A57A6E7A1EEB68423666BBDBF9B9CE8E76734A0A4E991A2ACED50247D26E6A0B13EC193A52AAA41523369832D2FF";

		//���������ȨLicKey����߰������ӷ�������������Ϣ(��̬��汾���Ѽ��ɵ�����)
		char buf[1024] = { 0 };
		string Lickey = "==|";
		sprintf_s(buf, "<lickey>%s</lickey>", Lickey.c_str());
		ks_cmd("set", buf);

		buf[0] = 0;

		//������������ļ��ļ�·�������¼��֤��־)

		const char* cfgfile = "c:\\mtcfg.ini";
		strcpy(slt.configFilePath, cfgfile);
		sprintf(buf, ("%s<ininame>%s</ininame>"), buf, cfgfile);

		//������������ţ��������վ������б���������������ɲ�ѯ��������
		slt.softcode = 1000007;

		sprintf(buf, ("%s<softcode>%d</softcode>"), buf, slt.softcode);

		//��������Ƿ����ñ��������ñ����Ļ�����1
		sprintf(buf, ("%s<is2web>%d</is2web>"), buf, 0);

		//��������Զ�������룬���Զ����ע�͵���һ�У��Զ��������ĳ����ַ������ȱ������5λ
		sprintf(buf, ("%s<pccode>%s</pccode>"), buf, "");

		//����������ÿ��ڲ�ȡ��Щ��Ŀ����ʾ������,�����һ���������� �������.�Զ�������� ��Ϊ���ҳ��ȴ���5����ô�����趨��Ч
		sprintf(buf, ("%s<pccodemode>%d</pccodemode>"), buf, CPUCODE | MACCODE);  //DISKCODE | CPUCODE | MACCODE
		//�����������汾�� ����Ҫ������»�ǿ�Ƹ���ʱ��

		slt.softver = mt_softver;

		sprintf(buf, ("%s<softver>%d</softver>"), buf, slt.softver);  //DISKCODE | CPUCODE | MACCODE

		//������������������ͷ��ʶ ���������վ����������������ѯ�������ݣ��޸ĵĻ�������������ͬ����
		const char* softhead = ("_Data");
		strcpy(slt.g_softhead, softhead);
		sprintf(buf, ("%s<softhead>%s</softhead>"), buf, softhead);

		//�������ȡ���������õ��������������1��2 Ĭ��Ϊ1  1����winhttpAPIȡ�������ݣ�2����WinHttpRequest COM����ȡ��������
		sprintf(buf, ("%s<httpmode>%d</httpmode>"), buf, 1);

		//��������Ƿ�����https, Ĭ��ֵ��http������Ϊ1ʱ����https
		sprintf(buf, ("%s<ssl>%d</ssl>"), buf, 0);

		//�����������һ����ص�����������ͨѶ ����� ks_cmd(set,) ks_cmd(check,) IPC����ͨѶʱ��ִ�еĻص�����
		// ���巽����μ� kscmdCallBack�ص������ӳ��򣬲�ʹ�ûص���������ΪNULL
#ifdef _WIN64
		sprintf(buf, ("%s<kscmdapi>%I64d</kscmdapi>"), buf, (__int64)&CSoftLicTool::kscmdCallBack);
#else
		sprintf(buf, "%s<kscmdapi>%u</kscmdapi>", buf, (unsigned long)&CSoftLicTool::kscmdCallBack);
		//sprintf(buf, ("%s<kscmdapi>%u</kscmdapi>"), buf, (unsigned long)0);
#endif

		string sData = ks_cmd(("set"), buf);;
		if (slt.GD_(("state"), sData) != "100")
			return ErrCode_A;
		sData = ks_cmd("get", "");
		if (slt.GD_(("state"), sData) != "100")
			return ErrCode_B;

		return ErrCode_No;
	}

	void install(HMODULE h_module)
	{
		int Code = OnInitKs();
		char textdata[50] = { 0 };

		if (Code != ErrCode_No)
		{
			sprintf_s(textdata, "data error code %d", Code);
			//slt.MsgBox(textdata);
			MessageBoxA(0, VMProtectDecryptStringA(textdata), VMProtectDecryptStringA(""), 0);
			slt.myExitProcess();
		}


		my_Module = h_module;
#pragma region permit

		char keystr[68] = { };
		Utils::r_ini(ens_a("mt_key"), keystr, "");
		char buf[512] = { 0 };
		sprintf(buf, ens_a("<keystr>%s</keystr><clientid>%d</clientid>"), keystr, 1);
		string sData = ks_cmd("set", buf);

		if (slt.GD_("state", sData) != "100")
		{
			//slt.MsgBox(slt.GD_("message", sData));
			MessageBoxA(0, ens_a("error 1"), ens_a(""), 0);
			slt.myExitProcess();
		}

		int randomstr = slt.rand_(10000000, 20000000);
		sprintf(buf, "<randomstr>%d</randomstr>", randomstr);
		sData = ks_cmd("check", buf);

		//���ܲ���ʽ������
		slt.FD_(sData);
		if (slt.GD_("state", sData) != "100")
		{
			std::string err_out = slt.GD_("message", sData) + "\r\n" + slt.GD_("webdata", sData);
			//slt.MsgBox(slt.GD_("message", sData) + "\r\n" + slt.GD_("webdata", sData));
			if (err_out.find("�汾") != std::string::npos)
			{
				MessageBoxA(0, ens_a("����汾�Ѹ��£���������°汾"), ens_a(""), 0);
				slt.myExitProcess();
			}
			else
			{
				MessageBoxA(0, ens_a("������Կ��ǰ������"), ens_a(""), 0);
				slt.myExitProcess();
			}
		}

		//�жϽ��ܳ����������Ƿ�Ϸ�
		string randomstrS = slt.GD_(ens_a("randomstr"), sData);
		if (randomstr != atoi(randomstrS.c_str()))   //����ֻ�����򵥵ĵ��ڱȶԣ�����Ч�����ַ�������Զ�����������д������û��ʲô��ȫ�Կ��ԣ�
		{

			slt.myExitProcess();
		}

		end_time_str = slt.GD_(ens_a("endtime"), sData);
		end_seconds = slt.GD_(ens_a("ShengYuMiaoShu"), sData);
		user_type = slt.GD_(ens_a("tag"), sData);


		

		g_dumyArray = new uintptr_t[256];



		uintptr_t key[4] = { 0x30a38afd, 0x7a2a5009, 0x6e32a1d3, 0x31be00e1 };
		uintptr_t sum = 0x587653da;

		char tmpbufStr[64];

		bool timeAnti = false;
		bool debugAnti = false;
		bool crcAnti = false;

		DWORD cid[4];
		uintptr_t stackData[24];
		uintptr_t s1, s2;
		uintptr_t tmp;

		uintptr_t* heapData = new uintptr_t[24];//����,��ֹ�ƽ����ҵ�����ջ������

		s1 = GetTickCount64();
		s1 += 1;
		

#if 0 
		antiDebug = XAntiDebug(h_module, FLAG_DETECT_DEBUGGER);
		antiDebug.XAD_Initialize();
		debugAnti = (bool)antiDebug.XAD_ExecuteDetect();
#endif 

		tmp = reinterpret_cast<uintptr_t>(FindWindowW(ens_w(L"Progman"), ens_w(L"Program Manager")));
		stackData[12] = tmp;
		for (int i = 0; i < 0x1000; i++)
		{
			auth_xxx();
		}
		
		//ջ��
		stackData[0] = auth_get_rsp();

		//ThreadLocalStoragePointer
		stackData[2] = auth_get_teb_tlsp();

		//ClientId.UniqueProcess
		stackData[4] = auth_get_teb_pid();

		//ClientId.UniqueThread
		stackData[6] = auth_get_teb_tid();
		 
		//PEB
		stackData[8] = auth_get_peb_osver();
		tmp = stackData[8];
		rand_s((unsigned int*)&tmp);
		stackData[10] = tmp;
		stackData[14] = (uintptr_t)__rdtsc();
		g_dumyArray[200] = stackData[14];
		__cpuid((int*)cid, 1);
		stackData[16] = cid[0] | cid[1] ^ cid[2];
		stackData[18] = (uintptr_t)__rdtsc();
		s2 = GetTickCount64();
		if ((s1 == s2) ||
			((s2 + 1 - s1) > 2000)) //�����δ��� ���г���1.5��, ˵�������ڵ���
		{
			timeAnti = true;
		}
		
		//crcAnti = VMProtectIsValidImageCRC() == false;
		crcAnti = false;
		stackData[20] = (uintptr_t)g_dumyArray;
		for (int i = 0; i < _countof(stackData); i++)
		{
			if (stackData[i] < 0xffff && stackData[i] != 0)
			{
				stackData[i] += 0xffff;
			}
			else if (stackData[i] == 0)
			{
				stackData[i] = (uintptr_t)__rdtsc() ^ (i * (uintptr_t)stackData);
			}

			stackData[i] &= 0x7fffffff;
		}

		if (timeAnti)
			stackData[18] |= (1 << 4);

		else
			stackData[18] &= ~(1 << 4);

		
		if (debugAnti)
			stackData[18] |= (1 << 6);

		else
			stackData[18] &= ~(1 << 6);


		if (crcAnti)
			stackData[18] |= (1 << 10);

		else
			stackData[18] &= ~(1 << 10);


		for (int Index = 0; Index < _countof(stackData); Index++)
		{
			heapData[Index] = stackData[Index] ^ 0x7628B463;
		}

		int nSize = VMProtectGetCurrentHWID(nullptr, 0);
		char* vmp_hwid_buff = new char[nSize];
		VMProtectGetCurrentHWID(vmp_hwid_buff, nSize);

		std::string strr = ens_a("v_panda,");
		std::string vmphwid = vmp_hwid_buff;
		strr += EAD::Rc4Encrypt((UCHAR*)vmphwid.c_str(), (int)vmphwid.length());


		strr = slt.advapi(strr, 0, 0);//vmp��֤
		int deVmpLen = Base64DecodeGetRequiredLength((int)strr.length());
		char* deVmpString = (char*)calloc(1, deVmpLen + 1);

		Base64Decode(strr.c_str(), (int)strr.length(), (UCHAR*)deVmpString, &deVmpLen);
		EAD::Rc4Encrypt((UCHAR*)deVmpString, deVmpLen);

		if (VMProtectSetSerialNumber(deVmpString) == 0)//vmp��Ȩ�ɹ�
		{
			free(deVmpString);
			std::string advapi = ens_a("v_fuckJs,");
			std::string enCode;

			for (int Index = 0; Index < _countof(stackData); Index++)
			{
				sprintf_s(tmpbufStr, "%016llX", heapData[Index]);
				enCode += tmpbufStr;
			}

			advapi += EAD::Rc4Encrypt((UCHAR*)enCode.c_str(), (int)enCode.length());

			advapi = slt.advapi(advapi.c_str());
			int deLen = Base64DecodeGetRequiredLength((int)advapi.length());
			char* deString = (char*)calloc(1, deLen + 1);
			std::vector<std::string> str_array;
			uintptr_t* int_array = NULL;

			/*if (VMProtectIsValidImageCRC() == false)
			{
				TerminateProcess((HANDLE)-1, 0);
				__asm xor esp, esp;
				return;
			}*/

			Base64Decode(advapi.c_str(), (int)advapi.length(), (UCHAR*)deString, &deLen);
			EAD::Rc4Encrypt((UCHAR*)deString, deLen);
			str_array = EAD::StringSplit(deString, "|");
			free(deString);
			int_array = new uintptr_t[str_array.size()];

			for (size_t i = 0; i < str_array.size(); i++)
			{
				int_array[i] = (uintptr_t)strtoull(str_array[i].c_str(), NULL, 10);
			}

			for (int i = 0; i < _countof(stackData); i++)
			{
				if (i % 2 == 0) {
					stackData[i] += ((stackData[i] << 4) + key[0]) ^ (stackData[i] + sum) ^ ((stackData[i] >> 5) + key[1]);
				}
				else {
					stackData[i] += ((stackData[i] << 4) + key[2]) ^ (stackData[i] + sum) ^ ((stackData[i] >> 5) + key[3]);
				}
			}

			for (int i = 0; i < _countof(stackData); i++)
			{
				stackData[i] &= 0x7fffffff;
			}

			for (int i = 0; i < _countof(stackData); i++)
			{
				if (stackData[i] != int_array[i])
				{
					/*__asm xor esp, esp;
					__asm mov eax, 1;
					__asm mov eax, [eax];*/
					return;

				}

			}

			str_array.clear();

			//��һ���Զ�������
			g_dumyArray[101] = int_array[24];
			g_dumyArray[102] = int_array[25];

			//�ڶ����Զ�������
			g_dumyArray[103] = int_array[26];
			g_dumyArray[104] = int_array[27];

			g_dumyArray[200] &= 0x7fffffff;
			g_dumyArray[201] = int_array[28];

			//��װ
			/*if ((permit::g_dumyArray[200] ^ permit::g_dumyArray[201]) != ((uintptr_t)permit::g_dumyArray & 0x7fffffff))
			{
				return;
			}*/

			//a lamba that can combine two uint32_t into one uint64_t
			auto combineIntegers = [](uint32_t high, uint32_t low) {
				std::stringstream high_ss;
				high_ss << std::hex << high;

				std::stringstream low_ss;
				low_ss << std::hex << low;

				std::string combined_str = high_ss.str() + low_ss.str();

				uint64_t combined = std::stoull(combined_str, nullptr, 16);
				return combined;
			};

			auto a = combineIntegers((uint32_t)(g_dumyArray[101] ^ ((uintptr_t)permit::g_dumyArray & 0x7fffffff)), (uint32_t)(g_dumyArray[102] ^ ((uintptr_t)permit::g_dumyArray & 0x7fffffff)));
			auto b = combineIntegers((uint32_t)(g_dumyArray[103] ^ ((uintptr_t)permit::g_dumyArray & 0x7fffffff)), (uint32_t)(g_dumyArray[104] ^ ((uintptr_t)permit::g_dumyArray & 0x7fffffff)));

			//if (VMProtectIsValidImageCRC() == TRUE)
			{

				//if (permit::user_type == VMProtectDecryptStringA("PREMIUM") || permit::user_type == VMProtectDecryptStringA("DEV"))
				//{			
				//	/*char is_bypass[68] = { 0 };
				//	r_ini(VMProtectDecryptStringA("bypass"), is_bypass, "");

				//	if (std::string(is_bypass) == "1")
				//	{
				//		Ace_Patch::ace_bypass = true;
				//	}*/
				//}
				//CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)AutoMataInit, 0, 0, 0));

				BotSDK::BotInit(0);
			}
		}
		////else//��Ȩʧ��
		////{
		////	MessageBoxA(0, "��Ȩʧ��", 0, 0);

		////}

#pragma endregion permit

		return;
	}

}
#endif
#pragma optimize( "", on )