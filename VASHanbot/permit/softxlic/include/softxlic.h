/*//============================================================================================
  //
  //          �ɿ���֤����ͷ�ļ� softXlic.h
  //          ����޸����ڣ�2019-03-03 18:49
  //
  //============================================================================================*/
#if !defined(MYFX_softxlic_H__33234425_E7D0_40CE_A3F0_325A6DF20696__INCLUDED_)
#define MYFX_softxlic_H__33234425_E7D0_40CE_A3F0_325A6DF20696__INCLUDED_
#pragma warning(disable : 4786)   //VC6���Ǹ�4786̫���ˣ�һˢһ��Ƭ
#pragma warning(disable : 4996)   //VC6���Ǹ�4786̫���ˣ�һˢһ��Ƭ
#include <windows.h>
#include <map>
#include <string>

#ifdef _WIN64
#define SOFTLICLIB "softlic64.lib"
#else
#define SOFTLICLIB "softlic32.lib"
#endif

using namespace std;

//============================================================================================
//���ʹ�ô���̬���붨���  
#define LIB_NO_DLL  

#ifdef LIB_NO_DLL
#	define SOFTLIC_EXPORT extern "C" 
#	define SOFTLIC_WINAPI  WINAPI
#else
#	define SOFTLIC_EXPORT  __declspec(dllimport)
#	define SOFTLIC_WINAPI  WINAPI
#endif

//�ж��Ƿ�ʹ����MFC
#if defined(__AFX_H__)
#	if defined(_UNICODE)
//string
#		define Cs_s(cstr) CStr2str(cstr)

//����һ��Cstring
#		define s_Cs(str) str2Cstr(str) 
string CStr2str(const CString &cstrSrcW);
CString str2Cstr(const char* cstrSrcA);
#	else
#		define Cs_s(cstr) cstr.GetBuffer(0)   
#		define s_Cs(str) str
#	endif
#endif

#define DISKCODE 1  //Ӳ������
#define CPUCODE 2	//CPU������
#define MACCODE 4	//������
#define BOARDSN 8	//�����


#ifndef INVALID_FILE_ATTRIBUTES
#	define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#endif 


// SoftLic32.Lib��SoftLic64.Lib��ͷ��Ϣ
//======================================================
#ifdef __cplusplus
	extern "C" {
#endif
		//===============================================================
		SOFTLIC_EXPORT char* SOFTLIC_WINAPI ks_cmd(const char * cmdType, const char * cmdData);

		/*===============================================================
		//���ܣ�http���ļ��������������㹻�Ļ��棬�����뻺���С������reSize��ֵ���������ļ��Ĵ�С
		//		int reSize=1024*100
		//		char * buf = new char[reSize];
		//		httpRead("http://www.baidu.com/d.exe",buf,&reSize);   
		//��ɺ�buf��ǰreSize���ֽھ��Ƕ�ȡ������  */
		SOFTLIC_EXPORT int SOFTLIC_WINAPI httpRead(const char * url, void ** binbuffer, int * reSize);


		/*===============================================================
		//���ܣ�ȡ������
		//		char diskbuf[256];
		//		char macbuf[256];
		//		char boardbuf[256];
		//		GetPcCode(diskbuf, NULL, macbuf, boardbuf);   //ĳ����ȡ��NULL����,���粻ȡcpu */
		SOFTLIC_EXPORT void  SOFTLIC_WINAPI GetPcCode(char* diskbuf, char* cpubuf, char* macbuf, char* boardbuf);


		/*===============================================================
		//���ܣ�RSA��Կ����, ����ֻ�д���̬���ʹ��
		//		���ʹ�ñ��ӿ�RSA_Decode����RSADecode,
		//		����ɾ��CBigInt.h��CBigInt.cpp,
		//		�ҵ���ɾ�� RSADecode��������ʵ�֣�
		//		Ȼ��ѳ����е���RSADecode�ĵط��ĳ�RSA_Decode   */
		SOFTLIC_EXPORT char* SOFTLIC_WINAPI RSA_Decode(const char*  v_data64, const char* v_Pubkey16, const char* v_Mod16);
#ifdef __cplusplus
	}
#endif

#pragma comment(lib,SOFTLICLIB)

class CSoftLicTool
{
private:
	static const BYTE DATA_B642BIN[128];
	static CSoftLicTool* pThis;
	unsigned int crc32table[256];	
	map<unsigned int,string> CacheTable;
private:
	void make_crc32table();
	string __myDecrypt(string&inData);
	string  RSADecode(string v_data64, string v_Pubkey16, string v_Mod16);

public:
	string g_rsaPubKey;
	string g_rsaMods;
	string g_strIPCGuid;
	char configFilePath[256];
	bool CacheAPI;
	char g_softhead[128];
	int softcode;
	int softver;
public:
	CSoftLicTool();
	~CSoftLicTool();

	static int WINAPI kscmdCallBack(const char * pread, char * pwrite, int CallBackType);
	void MsgBox(string astr, string title = "", int waittime = 10000);
	string  FD_(string &ioData);
	string GD_(const string key, const string data, string defstr = "");
	string  chkPass(int connect);
	string advapi(string advapicmd, bool freecache = false, bool msgbox = false);
	int kpoints(int v_points, string&v_errinfo);
	int  rand_(int v_min, int v_max);
	void myExitProcess();

	void BASE64ToHex(const string inputStr, string&outputStr);
	int BASE64_Decode(string inputStr, unsigned char*outputBuffer);
	void RC4(string key, unsigned char *Data, int data_length);

	void r_ini(const char* _key, char* rbuf, const char* _def="");
	void w_ini(const char* _key, const char* _val);
	string trim(string s);

	BOOL IsFileExist(const string & csFile);
	unsigned int crc32(char* buff, int nLength);
	void replace_all(string & s, string  t, string  w);

	void Edit(const char* _username, const char* _password2, const char* _password, const char* _bdinfo);
	void reguser(const char* _username, const char* _password2, const char* _password, const char* _bdinfo, const char* _tguser, const char* _keys);
	void cz(const char* _username, const char* _keys);
	void Unbind(const char* _UsernameOrKeyStr, const char* _password, const char* _clientid);
	void UnbindK(const char* KeyStr, const char* _clientid);
	void EditK(const char* _keystr, const char* _bdinfo);
	void View(const char* _UsernameOrKeyStr);
};
//ȫ�ֱ��� ������
extern CSoftLicTool slt;



#endif		// !defined(MYFX_softxlic_H__33234425_E7D0_40CE_A3F0_325A6DF20696__INCLUDED_)



