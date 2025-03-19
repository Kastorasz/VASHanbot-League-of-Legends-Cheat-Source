
#include "softxlic.h"
#include "miniGmp.h"

#ifndef DEVELOPER	
CSoftLicTool slt;
CSoftLicTool* CSoftLicTool::pThis = NULL;

typedef int(__stdcall *PMessageBoxTimeoutA)(IN HWND hWnd, IN LPCSTR lpText, IN LPCSTR lpCaption, IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);


const BYTE CSoftLicTool::DATA_B642BIN[128] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xE0,0xF0,0xFF,0xFF,0xF1,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xE0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3E,0xFF,0xF2,0xFF,0x3F,
	0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0xFF,0xFF,0xFF,0x00,0xFF,0xFF,
	0xFF,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,
	0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
	0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0xFF,0xFF,0xFF,0xFF,0xFF
};

void CSoftLicTool::MsgBox(string astr, string title,int waittime)
{
	static PMessageBoxTimeoutA MsgBoxTimeoutA = NULL;
	if (!MsgBoxTimeoutA)
	{
		HMODULE hUser32 = GetModuleHandleA(("user32.dll"));
		if (hUser32)
		{
			MsgBoxTimeoutA = (PMessageBoxTimeoutA)GetProcAddress(hUser32, ("MessageBoxTimeoutA"));
		}
	}
	if (MsgBoxTimeoutA)
		MsgBoxTimeoutA(NULL, astr.c_str(), title.c_str(), MB_ICONASTERISK, 0, waittime);
	else
		MessageBoxA(NULL, astr.c_str(), title.c_str(), MB_ICONASTERISK);
}

CSoftLicTool::CSoftLicTool() :
	CacheAPI(TRUE)
	, g_rsaPubKey("010001")
	, g_rsaMods("")
	, configFilePath{0}
	, g_softhead{0}
{
	pThis = this;
	make_crc32table();
	
}

CSoftLicTool::~CSoftLicTool()
{
}
//�ص���������
int WINAPI CSoftLicTool::kscmdCallBack(const char * pread, char * pwrite, int CallBackType)
{
	string readstr = pread;
	string writestr = "";
	string mytag;
	switch (CallBackType)
	{
	case 1:   // ����ks_cmd(set, )ʱ�Ļص�
			  /*
			  ������������κ���
			  �������ڵ���ks_cmd("set","<softcode>1000001</softcode>") ʱ
			  ����д��
			  cc=�Զ������("<softcode>1000001</softcode>")
			  ks_cmd("set",cc)
			  Ȼ���������д��
			  writestr=�Զ������(readstr��
			  memcpy(pwrite,writestr.c_str(),writestr.size());
			  =====================================================================================
			  ����������ݵĻ����벻Ҫ��"д��ַ"��д������������֪��������ʲô
			  =========================================================
			  �������������ܶ��£��Լ�����
			  */
		//pThis->MsgBox("����Ϣ������softxlic.cpp�ļ� kscmdCallBack�ص������е���\n\r��ղŵ�����һ��ks_cmd(set,)�������ĵڶ��������ǣ�" + readstr, "����ѧϰģʽ��������");
		break;
	case 2:    //����ks_cmd(check,)ʱ��һ�׶εĻص�
			   /*
			   ������������κ���
			   �������ڵ���ks_cmd("check","<advapi>v_myapi,1</advapi>") ʱ
			   ����д��
			   cc=�Զ������("<advapi>v_myapi,1</advapi>")
			   ks_cmd("check",cc)
			   Ȼ���������д��
			   writestr=�Զ������(readstr��
			   memcpy(pwrite,writestr.c_str(),writestr.size());
			   =====================================================================================
			   ����������ݵĻ����벻Ҫ��"д��ַ"��д������������֪��������ʲô
			   =========================================================
			   �������������ܶ��£��Լ����ӣ����綨��һ�³������еı�Ҫ����
			   ��������һ������abc��������ֵ�� abc="F1ED1280"
			   abc="F000001"  ����ks_cmdǰ��ֵ�ĳɴ����
			   ks_cmd("check","******")
			   Ȼ���ڻص����aֵ��Ϊ����ֵ abc="F1ED1280"
			   */
		//pThis->MsgBox("����Ϣ������softxlic.cpp�ļ� kscmdCallBack�ص������е���\n\r��ղŵ�����һ��ks_cmd(check,)�������ĵڶ��������ǣ�" + readstr, "����ѧϰģʽ��������");
		break;
	case 9:   //IPC����ͨѶ�ص�
			  /*
			  * IPC�ͻ��˿ɷ����Զ���������ipc_cmd("ipc_check","mycmd:123123123")
			  * ��һ������������ipc_check
			  * �ڶ������������Լ������ݣ�Ϊ�˷������֣��Լ���������ü���һ��ͷ��Ϣ������ipc_cmd("ipc_check","mycmd:�豾�����")
			  * �������writestr��д�˶�����IPC����˲�����ȥ����pread������ֱ�ӽ���д������ݷ��ظ�IPC�ͻ���
			  */
		mytag = ("mycmd:");
		if (readstr.substr(0, mytag.size()) == mytag)
		{
			//��ʼ�������Լ�����Ϣ
			writestr = ("�㷢�͹������ı�") + readstr.substr(mytag.size()) + (",���յ��ˣ������Ҹ���ķ�����Ϣ");
		}

		if (writestr != "")
			memcpy(pwrite, writestr.c_str(), writestr.size());
		break;
	default:
		break;
	}
	return 0;
}

void CSoftLicTool::myExitProcess()
{
	ks_cmd("exit", "");
	__fastfail(0);
}

string  CSoftLicTool::RSADecode(string v_data64, string v_Pubkey16, string v_Mod16)
{

	cCrypt* rsa = new cCrypt(v_Mod16.c_str(), v_Pubkey16.c_str());
	char outbuf[1024] = { 0 };
	rsa->rsa_decrypt(v_data64.c_str(), (int)v_data64.size(), outbuf);
	delete rsa;
	return outbuf;
}

/************************************************************************/
//�Զ���Ľ��ܺ���,��Ҫ����վ��̨��Ӧ
/************************************************************************/
string CSoftLicTool::__myDecrypt(string&inData)
{
	int iRCKey = (int)inData.find(",");
	if (iRCKey>0) {
		string retRC4enKey = inData.substr(0, iRCKey);  //ȡ��","��ǰ��base64�����RSA���ܵ�RC4KEY	
		string RetEnData = inData.substr(iRCKey + 1);  //ȡ��","���RC4���ܵ�����
													   //����̬�ⲻ��Ҫ��CBigInt.h��CBigInt.cpp,�ɰ�RSADecode�������Ͷ���ɾ�������� 
													   //	Ȼ���滻�±�һ�е�RSADecodeΪRSA_Decode
		string RC4Key = RSADecode(retRC4enKey, g_rsaPubKey, g_rsaMods);	//��rsa����	���ܽ����ŵ�RCKey��
																		//====================================================================	
		unsigned char * buf = (unsigned char*)malloc(RetEnData.size());
		int deLen = BASE64_Decode(RetEnData, buf);
		*(buf + deLen) = '\0';
		RC4(RC4Key, buf, deLen);

		inData = (char *)buf;
		free(buf);
	}
	return inData;
}

/******��ʽ������**********************************************************/
string CSoftLicTool::FD_(string &ioData)
{
	size_t pos = (int)ioData.find(g_softhead);
	if (pos != string::npos) {//���ּ��ܱ�ʶͷ
		ioData = ioData.substr(pos + strlen(g_softhead));
		string data_s = __myDecrypt(ioData);  //�Զ������
		unsigned char* buf = (unsigned char*)malloc(data_s.size());
		int dlen = BASE64_Decode(data_s, buf); //������base64���룬data_s ���������<xml>��ͷ��һ��xml��ʽ��
		*(buf + dlen) = '\0';
		ioData = (char *)buf;
		free(buf);
	}
	else {
		if (ioData.find("<xml>") != 0) { //�����ϲ������е�����
			ioData = ("<xml><state>140</state><message>DLL�ڲ����󣬷��ص������쳣") + ioData + "</message></xml>";
		}
	}
	replace_all(ioData, "<br />", "");
	return ioData;
}

/******��ȡXML�е�ֵ**********************************************************/
string CSoftLicTool::GD_(const string key, const string data, string defstr)
{
	string result = "";
	string stag = "<" + key + ">";
	string etag = "</" + key + ">";

	size_t spos = data.find(stag);
	if (spos != string::npos)
	{
		size_t epos = data.find(etag);  //0123xx678
		if (epos != string::npos &&  epos> spos)
		{
			result = data.substr(spos + stag.size(), epos - spos - stag.size());
		}
	}
	if (result == "")result = defstr;
	return result;
}

/************************************************************************/
//chkPass���ܣ���ks_cmd��check����Ļ�����֤��һ����װ��������ߵ�Ч�鷽�������Լ���ӻ��޸�
//����connect��Ϊ0ʱ�Զ��ж��Ƿ����ӷ�������Ϊ1ʱǿ�����ӷ�����
/************************************************************************/
string CSoftLicTool::chkPass(int connect)
{
	int randomstr = rand_(100000000, 200000000);
	char buf[128] = { 0 };
	sprintf(buf, ("<connect>%d</connect><randomstr>%d</randomstr>"), connect, randomstr);
	string sData = ks_cmd("check", buf);

	FD_(sData);
	if (GD_("state", sData) != "100") {//��֤ʧ��
		string errinfo = GD_("message", sData);
		errinfo += "\r\n" + GD_("webdata", sData);
		//MessageBoxA(0, errinfo.c_str(), "��֤ʧ��", MB_OK);
		myExitProcess();
	}
	else {
		//��֤�ɹ���Ҫ�����ݶ�ȡ�Ͱ�ȫЧ����
		string Srandomstr = GD_(("randomstr"), sData);//����˷��ص�randomstr
													//����ֻ�����򵥵ĵ��ڱȶԣ�����Ч�����ַ�������Զ�����������д������û��ʲô��ȫ�Կ��ԣ�
		if (randomstr != atoi(Srandomstr.c_str())) 
		{
			myExitProcess();
		}
	}
	return sData;
}
/************************************************************************/
//advapi���ܣ���ks_cmd��check����ȡ�߼�API������һ����װ����
//����һ��advapi�Ľӿ����Ͳ���,���� 'v_getb,100,200'
//�������������û���߼�APIʱ�����������Ƿ��ͷŵ�ǰAPI���棬�ӷ���˴��»�ȡ
//������������ʱ�Ƿ񵯳�������Ϣ
/************************************************************************/
string CSoftLicTool::advapi(string advapicmd, bool freecache, bool msgbox)
{
	string sResult;

	unsigned int c32;
	if (CacheAPI)
	{
		c32 = crc32((char*)advapicmd.c_str(), (int)advapicmd.size());
		if (freecache)
			CacheTable.erase(c32);
		else
		{
			map<unsigned int, string>::iterator iter = CacheTable.find(c32);
			if (iter != CacheTable.end())
				return iter->second;
		}
	}

	int randomstr = rand_(100000000, 200000000);
	char buf[1024] = { 0 };
	sprintf(buf, ("<advapi>%s</advapi><randomstr>%d</randomstr>"), advapicmd.c_str(), randomstr);
	string sData = ks_cmd("check", buf);

	FD_(sData);
	if (GD_("state", sData) != "100") {//��֤ʧ��
		sResult = GD_("message", sData);
		sResult += "\r\n" + GD_("webdata", sData);
		if (msgbox) {
			//MsgBox(sResult, "��֤ʧ��");
		}
	}
	else {
		//��֤�ɹ���Ҫ�����ݶ�ȡ�Ͱ�ȫЧ����
		string Srandomstr = GD_("randomstr", sData);//����˷��ص�randomstr
													//����ֻ�����򵥵ĵ��ڱȶԣ�����Ч�����ַ�������Զ�����������д������û��ʲô��ȫ�Կ��ԣ�
		if (randomstr != atoi(Srandomstr.c_str())) {
			myExitProcess();
		}
		sResult = GD_("advapi", sData);
		if (CacheAPI)
			CacheTable[c32] = sResult;
	}
	return sResult;
}

/************************************************************************/
//�۵㺯��������advapi�ӿ�ʵ�ֿ۵㹦�ܡ��ɹ�����ʣ��ĵ�����ʧ�ܷ���-1
/************************************************************************/
int CSoftLicTool::kpoints(int v_points, string&v_errinfo)
{
	char buf[32] = { 0 };
	sprintf(buf, "v_points,%d", v_points);
	string tresult = advapi(buf, true);
	int c = atoi(tresult.c_str());
	if (c == 0) {   //����ֵ����������Ϊ0��˵���۵�ʧ��
		v_errinfo = tresult;
		c = -1;
	}
	else
		v_errinfo = "�۵�ɹ�";
	return c;
}

int CSoftLicTool::rand_(int v_min, int v_max)
{
	int rNum = 0;
	srand(GetTickCount());
	for (int i = 0; i < 31; i++)
		rNum |= (rand() & 1) << i;
	return v_min + rNum % (v_max - v_min + 1);
}

void CSoftLicTool::BASE64ToHex(const string inputStr, string&outputStr)
{
	int i, j;
	BYTE b[4];
	char chHex[] = "0123456789ABCDEF";
	int inputCount, outlen, padlen;

	padlen = 0;
	inputCount = (int)(inputStr.size());
	if (inputStr[inputCount - 1] == '=')
		padlen++;
	if (inputStr[inputCount - 2] == '=')
		padlen++;

	outlen = inputCount / 4 * 3 - padlen;

	j = 0;
	for (i = 0; i < inputCount; i += 4)
	{
		b[0] = DATA_B642BIN[(char)inputStr[i]];
		b[1] = DATA_B642BIN[(char)inputStr[i + 1]];
		b[2] = DATA_B642BIN[(char)inputStr[i + 2]];
		b[3] = DATA_B642BIN[(char)inputStr[i + 3]];

		outputStr += chHex[(b[0] >> 2)];
		outputStr += chHex[((b[0] & 3) << 2) | b[1] >> 4];
		j++;
		if (j >= outlen)
			break;

		outputStr += chHex[(b[1] & 15)];
		outputStr += chHex[(b[2] >> 2)];
		j++;
		if (j >= outlen)
			break;

		outputStr += chHex[((b[2] & 3) << 2) | b[3] >> 4];
		outputStr += chHex[(b[3] & 15)];
		j++;
		if (j >= outlen)
			break;
	}

}
int CSoftLicTool::BASE64_Decode(string inputStr, unsigned char*outputBuffer)
{
	INT i, j;
	BYTE b[4];
	int inputCount, outlen, padlen;

	padlen = 0;
	inputCount = (int)inputStr.size();
	if (inputStr[inputCount - 1] == '=')
		padlen++;
	if (inputStr[inputCount - 2] == '=')
		padlen++;
	// 00123456 00781234 00567812 00345678 00123456 0078xxxx 00xxxxxx,00xxxxxx
	outlen = (inputCount >> 2) * 3 - padlen;

	j = 0;
	for (i = 0; i < inputCount; i += 4)
	{
		b[0] = DATA_B642BIN[(BYTE)inputStr[i]];
		b[1] = DATA_B642BIN[(BYTE)inputStr[i + 1]];
		b[2] = DATA_B642BIN[(BYTE)inputStr[i + 2]];
		b[3] = DATA_B642BIN[(BYTE)inputStr[i + 3]];

		*outputBuffer++ = (b[0] << 2) | (b[1] >> 4);
		j++;
		if (j >= outlen)
			break;
		*outputBuffer++ = (b[1] << 4) | (b[2] >> 2);
		j++;
		if (j >= outlen)
			break;
		*outputBuffer++ = (b[2] << 6) | b[3];
	}
	return outlen;
}
void CSoftLicTool::RC4(string key, unsigned char *Data, int data_length)
{
	unsigned char box[256];
	unsigned char pwd[256];
	int i, j, k, a;
	unsigned char tmp = 0;

	int pwd_length = (int)key.size();
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

	for (i = 0; i < data_length; i++)
	{
		a = (a + 1) % 256;
		j = (j + box[a]) % 256;
		tmp = box[a];
		box[a] = box[j];
		box[j] = tmp;
		k = box[((box[a] + box[j]) % 256)];
		Data[i] ^= k;
	}

}

void CSoftLicTool::r_ini(const char* _key, char* rbuf, const char* _def)
{
	GetPrivateProfileStringA("config", _key, _def, rbuf, 1024, configFilePath);
}

void CSoftLicTool::w_ini(const char* _key, const char* _val)
{
	WritePrivateProfileStringA("config", _key, _val, configFilePath);
}


string CSoftLicTool::trim(string s)
{
	if (s.empty())return s;
	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}

void CSoftLicTool::replace_all(string & s, string  t, string   w)
{
	size_t pos = s.find(t), t_size = t.size(), r_size = w.size();

	while (pos != string::npos) { // found   
		s.replace(pos, t_size, w);
		pos = s.find(t, pos + r_size);
	}
}

BOOL CSoftLicTool::IsFileExist(const string & csFile)
{
	DWORD dwAttrib = GetFileAttributesA(csFile.c_str());
	return 0xFFFFFFFF != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
void CSoftLicTool::make_crc32table()
{
	int i, j;
	for (i = 0; i < 256; i++)
	{
		for (j = 0, crc32table[i] = i; j < 8; j++)
		{
			crc32table[i] = (crc32table[i] >> 1) ^ ((crc32table[i] & 1) ? 0xEDB88320 : 0);
		}
	}
}

unsigned int CSoftLicTool::crc32(char* buff, int nLength)
{
	unsigned crc = 0xFFFFFFFF;
	for (int i = 0; i < nLength; i++)
		crc = (crc >> 8) ^ crc32table[(crc ^ buff[i]) & 0xff];
	return ~crc;
}

void CSoftLicTool::Edit(const char* _username, const char* _password2, const char* _password, const char* _bdinfo)
{

	char buf[512] = { 0 };
	sprintf(buf, ("<username>%s</username><password2>%s</password2><password>%s</password><bdinfo>%s</bdinfo>"), _username, _password2, _password, _bdinfo);
	string sData = ks_cmd("edit", buf);
	MsgBox(GD_("message", sData) + "\r\n" + GD_("webdata", sData), ("�޸���Ϣ"));
}

void CSoftLicTool::EditK(const char* _keystr, const char* _bdinfo)
{

	char buf[512] = { 0 };
	sprintf(buf, ("<keystr>%s</keystr><bdinfo>%s</bdinfo>"), _keystr, _bdinfo);
	string sData = ks_cmd("edit", buf);
	MsgBox(GD_("message", sData) + "\r\n" + GD_("webdata", sData), ("�޸���Ϣ"));
}

//HACK ע���ʺ� 
void CSoftLicTool::reguser(const char* _username, const char* _password2, const char* _password, const char* _bdinfo, const char* _tguser, const char* _keys)
{
	char buf[1024] = { 0 };
	sprintf(buf, ("<username>%s</username><password2>%s</password2><password>%s</password><bdinfo>%s</bdinfo><puser>%s</puser><czkey>%s</czkey>"),
		_username, _password2, _password, _bdinfo, _tguser, _keys);

	string sData = ks_cmd("reg", buf);
	MsgBox(GD_("message", sData) + "\r\n" + GD_("webdata", sData), ("ע��"));

}

//HACK �ʺų�ֵ
void CSoftLicTool::cz(const char* _username, const char* _keys)
{
	char buf[1024] = { 0 };
	sprintf(buf, ("<username>%s</username><czkey>%s</czkey>"),
		_username, _keys);

	string sData = ks_cmd("cz", buf);
	MsgBox(GD_("message", sData) + "\r\n" + GD_("webdata", sData), "��ֵ");
}


//HACK ����ʺŻ������
void CSoftLicTool::Unbind(const char* _UsernameOrKeyStr, const char* _password, const char* _clientid)
{
	char buf[1024] = { 0 };
	sprintf(buf, ("<username>%s</username><password>%s</password><clientid>%s</clientid>"),
		_UsernameOrKeyStr, _password, _clientid);

	string sData = ks_cmd("unbind", buf);
	MsgBox(GD_("message", sData) + "\r\n" + GD_("webdata", sData), ("��������"));
}

//HACK ���Ż������
void CSoftLicTool::UnbindK(const char* KeyStr, const char* _clientid)
{
	char buf[1024] = { 0 };
	sprintf(buf, ("<keystr>%s</keystr><clientid>%s</clientid>"),
		KeyStr, _clientid);

	string sData = ks_cmd("unbind", buf);
	MsgBox(GD_("message", sData) + "\r\n" + GD_("webdata", sData), ("��������"));
}

//HACK ����ʺŻ򿨺Ż������
void CSoftLicTool::View(const char* _UsernameOrKeyStr)
{
	char buf[1024] = { 0 };
	sprintf(buf, ("<keyorusername>%s</keyorusername>"),
		_UsernameOrKeyStr);

	string sData = ks_cmd("search", buf);
	MsgBox(GD_("message", sData) + "\r\n" + GD_("webdata", sData), ("�� ѯ"));
}

//HACK �±�רΪMFC UNICODE����׼��
#if defined(__AFX_H__) && defined(_UNICODE)
CString str2Cstr(const char* str)
{
	int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	wchar_t *wstr = new wchar_t[len];
	memset(wstr, 0, len * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, len);
	CString cstrDest = wstr;
	delete[] wstr;
	return cstrDest;
}


string CStr2str(const CString &cstrSrcW)
{
	int len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(LPCTSTR)cstrSrcW, -1, NULL, 0, NULL, NULL);
	char *str = new char[len];
	memset(str, 0, len);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(LPCTSTR)cstrSrcW, -1, str, len, NULL, NULL);
	string cstrDestA = str;
	delete[] str;

	return cstrDestA;
}
#endif
#endif