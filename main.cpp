//CraXicS�� Integrated Windows Installation System
#include <cadoncio.h>

//��� ����
#define KMSPICO_FILENAME "KMSpico_setup.exe"
#define KMSPICO_VERSION "9.1.3"
#define UI_BG_DEFAULT "17"//���߿� ���������� ����
#define UI_BG_ERR "47"
#define UI_BG_WARN "17"
#define WANDRV_VERSION "WanDrv 6.6.41.565.2016.0815"//���߿� �ϵ��ڵ� �ּ�ȭ
const char* g_cstrTitle = "CraXicS(TM) Windows ���� ���� OEM ��ũ��Ʈ 3.0 Beta 5";
const char* g_cstrScriptsPath = "%systemroot%\\Setup\\Scripts\\";
//�������� ����
char g_strEdition[MAX_PATH];
bool g_bIsNFW4;
char g_strKernelVersion[4];
bool g_Is64BitSystem;
bool g_bRecommandReboot;
bool g_bReserveHideUpdates;

//�Լ� ����
void InitUI();
void ChkSystemEnv();
int IsRegistryKey(char* aRootKey, char* aSubKey);
void RequestOwner();
HKEY OpenRegistry(char* aRootKey, char* aSubKey, int nDesired, bool bIgnoreWOWRedirect=false);
int WriteRegistry(char* aRootKey, char* aSubKey, char* aKey, char* aValue, bool bIgnoreWOWRedirect);
int ReadRegistry(char* aRootKey, char* aSubKey, char* aKey, char** aBuf, int nBufSize, bool bIgnoreWOWRedirect);
int InstallKMSpico();
int ChkSupportedOSVersion();
void InstallDrv();
void FinishTask();
void ReserveTaskHideUpdates();
void ReserveTaskDeleteScripts();

int main()
{
	InitUI();
	ChkSystemEnv();
	RequestOwner();
	ReserveTaskHideUpdates();
	InstallKMSpico();
	InstallDrv();
	FinishTask();
	return 0;
}

void InitUI()
{
	char strTitleBuf[MAX_PATH] = {0,};//���߿� �����μ��Լ��� ������ ��.
	strcat(strTitleBuf, "title ");
	strcat(strTitleBuf, g_cstrTitle);
	system(strTitleBuf);
	system("mode con cols=80 lines=30");
	strset(strTitleBuf, 0);
	strcat(strTitleBuf, "color ");
	strcat(strTitleBuf, UI_BG_DEFAULT);
	system(strTitleBuf);
}

void ChkSystemEnv()
{
	char strConOut[MAX_PATH] = {0,};
	char* pstrVersion = 0;
	char* pBuf = g_strEdition;
	
	//����� ���ϱ�
	ReadRegistry(0, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "EditionID", &pBuf, sizeof(g_strEdition)/2, false);
	
	//��� �����ӿ�ũ 4 ��ġ ���� Ȯ��
	if (IsRegistryKey(0, "SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v4") == ERROR_SUCCESS) {
		g_bIsNFW4 = true;
	}

	//Ŀ�ι��� Ȯ��
	FILE* pFile = _popen("ver", "r");
	if(pFile != NULL) {
		while(fgets(strConOut, sizeof(strConOut), pFile) != NULL);//���� �ɶ����� pFile�� ��Ҹ� �о output�� output�� �����ŭ �����Ѵ�.
		pstrVersion = strstr(strConOut, "Version");
	}
	_pclose(pFile);
	strncpy(pstrVersion, pstrVersion+8, 3);
	pstrVersion[3] = '\0';
	strcpy(g_strKernelVersion, pstrVersion);

	//��Ʈ Ȯ��
	#if defined(_WIN64)
		g_Is64BitSystem = true;// 64-bit programs run only on Win64
	#elif defined(_WIN32)
		// 32-bit programs run on both 32-bit and 64-bit Windows. so must sniff.
		BOOL f64 = FALSE;
		g_Is64BitSystem = IsWow64Process(GetCurrentProcess(), &f64) && f64;
	#else
		g_Is64BitSystem = false; // Win64 does not support Win16
	#endif
}

HKEY OpenRegistry(char* aRootKey, char* aSubKey, int nDesired, bool bIgnoreWOWRedirect/*=false*/)
{
	LONG ret;
	HKEY hKey;
	int hexDesired;
	int nLen;
	TCHAR tstrSubKey[MAX_PATH] = {0,};

	switch (nDesired)
	{
		case 0:
			hexDesired = KEY_QUERY_VALUE; 
			break;
		case 1:
			hexDesired = KEY_SET_VALUE;
			break;
		default:
			return 0;
			break;
	}

	nLen = MultiByteToWideChar(CP_ACP, 0, aSubKey, strlen(aSubKey), NULL, NULL);
	MultiByteToWideChar(CP_ACP, 0, aSubKey, strlen(aSubKey), tstrSubKey, nLen);

	if(bIgnoreWOWRedirect)
		hexDesired = KEY_SET_VALUE|KEY_WOW64_64KEY;
	ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, tstrSubKey, 0, hexDesired, &hKey);
	if(ret == ERROR_SUCCESS) return hKey; else return 0;
}

int ReadRegistry(char* aRootKey, char* aSubKey, char* aKey, char** aBuf, int nBufSize, bool bIgnoreWOWRedirect)
{
	LONG ret;
	HKEY hKey;
	DWORD data_type, data_size;
	int nLen;
	TCHAR tstrKey[32] = {0,};
	TCHAR tstrBuf[32] = {0,};

	if (bIgnoreWOWRedirect)
		hKey = OpenRegistry(0, aSubKey, 0, true);
	else
		hKey = OpenRegistry(0, aSubKey, 0, false);
	
	if(!hKey) return 1;


	nLen = MultiByteToWideChar(CP_ACP, 0, aKey, strlen(aKey), NULL, NULL);
	MultiByteToWideChar(CP_ACP, 0, aKey, strlen(aKey), tstrKey, nLen);

	data_size = (DWORD)nBufSize;
	ret = RegQueryValueExW(hKey, tstrKey, 0, &data_type, (LPBYTE)tstrBuf, &data_size);
	
	nLen = WideCharToMultiByte( CP_ACP, 0, tstrBuf, -1, NULL, 0, NULL, NULL );	
	ret = WideCharToMultiByte( CP_ACP, 0, tstrBuf, -1, *aBuf, nLen, NULL, NULL );
	//0 is faild.
	if(ret != ERROR_SUCCESS) return 2;
	if(hKey != NULL) RegCloseKey(hKey);
	return 0;
}

int WriteRegistry(char* aRootKey, char* aSubKey, char* aKey, char* aValue, bool bIgnoreWOWRedirect)
{
	LONG ret;
	HKEY hKey;

	hKey = OpenRegistry(0, aSubKey, 0, bIgnoreWOWRedirect);
	if(!hKey) return 1;

	ret = RegSetValueExA(hKey, aKey, 0, REG_SZ, (const BYTE*)aValue, 32);
	//ret = RegSetValueExW(hKey, aKey, 0, &data_type, (LPBYTE)*aBuf, &data_size);

	if(ret != ERROR_SUCCESS) return 2;
	if(hKey != NULL) RegCloseKey(hKey);
	return 0;
}

int IsRegistryKey(char* aRootKey, char* aSubKey)
{
	HKEY hKey = 0;
	return RegOpenKeyA(HKEY_LOCAL_MACHINE, aSubKey, &hKey);
}

void RequestOwner()
{
	char strOwner[32] = {0,};
	char strOrgan[32] = {0,};
	
	char strTitleBuf[MAX_PATH] = {0,};
	strcat(strTitleBuf, "title ");
	strcat(strTitleBuf, g_cstrTitle);
	strcat(strTitleBuf, " - ������ ���� ���");
	system(strTitleBuf);
	system("cls");
	strset(strTitleBuf, 0);
	strcat(strTitleBuf, "color ");
	strcat(strTitleBuf, UI_BG_DEFAULT);
	system(strTitleBuf);

	puts("Windows ��ġ�� ���� �������Ǿ����ϴ�.");
	puts("������ �� ���� ������ �Ϸ��Ͻʽÿ�.");
	puts("");
	puts("Windows ������ ��ϵ� ������ �̸��� �Է��Ͻʽÿ�.");
	puts("'Enter'�� �Է��� �� �������� ��ϵ˴ϴ�.");
	printf("����� �̸�: >");
	fgets(strOwner, sizeof(strOwner), stdin);
	puts("");
	puts("Windows ������ ��ϵ� ���� �̸��� �Է��Ͻʽÿ�.");
	puts("'Enter'�� �Է��� �� �������� ��ϵ˴ϴ�.");
	printf("���� �̸�: >");
	fgets(strOrgan, sizeof(strOrgan), stdin);
	WriteRegistry(0, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "RegisteredOwner", strOwner, true);
	WriteRegistry(0, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "RegisteredOrganization", strOrgan, true);
}

int InstallKMSpico()
{
	char chrBufYN;
	char strTitleBuf[MAX_PATH] = {0,};
	strcat(strTitleBuf, "title ");
	strcat(strTitleBuf, g_cstrTitle);
	strcat(strTitleBuf, " - Windows ��ǰ ����");
	system(strTitleBuf);
	system("cls");
	strset(strTitleBuf, 0);
	strcat(strTitleBuf, "color ");
	strcat(strTitleBuf, UI_BG_DEFAULT);
	system(strTitleBuf);

	puts("Windows ��ǰ ������ ���� KMSpico��(��) ��ġ�մϴ�.");
	puts("���� Ű�� �����ϰ� �ְų� �ٸ� ������� Windows ��ǰ ������ �Ϸ��� �� ������ �ǳʶ�ʽÿ�.");
	puts("");
	for(;1;)
	{
		printf("KMSpico %s��(��) ��ġ�ϰڽ��ϱ�?\n", KMSPICO_VERSION);//�̺κе� �Լ��� �����
		printf("(Y/N): >");
		chrBufYN = (char)_fgetchar();
		fflush(stdin);
		//int c; while((c=getchar())!='\n'&&c!=EOF);
		chrBufYN = toupper(chrBufYN);
		if(chrBufYN == 'Y' || chrBufYN == 'N')
		{
			system("color 17");
			if(chrBufYN == 'N') return 1;
			break;
		}
		puts("");
		printf("�߸� �Է��߽��ϴ�. Y(��) �Ǵ� N(�ƴϿ�)�� �Է��Ͻʽÿ�.");
		system("color 47");//���⼭���ʹ� �׳� �ʹ� ������� ���ڿ��� ����.
		puts("");
	}

	if(ChkSupportedOSVersion() == 1)
	{
		system("color 57");
		puts("");
		puts("���� Windows ������� ���� ���̼����� �ƴϱ� ������ KMSpico��(��) Microsoft Office 2010/2013 ��ǰ���� ���� ��ǰ ������ �����մϴ�.");
		puts("");
		for(;1;)
		{
			printf("�׷��� KMSpico %s��(��) ��ġ�ϰڽ��ϱ�?\n", KMSPICO_VERSION);
			printf("(Y/N): >");
			chrBufYN = (char)_fgetchar();
			fflush(stdin);
			//int c; while((c=getchar())!='\n'&&c!=EOF);
			chrBufYN = toupper(chrBufYN);
			if(chrBufYN == 'Y' || chrBufYN == 'N')
			{
				system("color 17");
				if(chrBufYN == 'N') return 1;
				break;
			}
			puts("");
			printf("�߸� �Է��߽��ϴ�. Y(��) �Ǵ� N(�ƴϿ�)�� �Է��Ͻʽÿ�.");
			system("color 47");//���⼭���ʹ� �׳� �ʹ� ������� ���ڿ��� ����.
			puts("");
		}
	}

	if(g_bIsNFW4)
	{
		char strKMSFullPath[MAX_PATH] = {0,};
		
		puts("");
		printf("KMSpico %s��(��) ��ġ�մϴ�.", KMSPICO_VERSION);
		puts("");
		puts("KMSpico ��ġ ��...");
		strcat(strKMSFullPath, "start /wait ");
		strcat(strKMSFullPath, g_cstrScriptsPath);
		strcat(strKMSFullPath, KMSPICO_FILENAME);
		strcat(strKMSFullPath, " /verysilent /noicons");
		system(strKMSFullPath);
		//system("regedit /s RunOnce.reg");
	}else{
		system("color 57");
		puts("");
		puts("���� Windows�� Microsoft .NET Framework 4��(��) ��ġ�Ǿ� ���� �ʽ��ϴ�. KMSpico��(��) ��ġ�ϱ� ���ؼ� Microsoft .NET Framework 4��(��) ���� ��ġ�ؾ� �մϴ�.");
		puts("����Ϸ��� �ƹ�Ű�� �����ʽÿ�.");
		_fgetchar();
	}
	return 0;
}

int ChkSupportedOSVersion()
{
	char arstrEditions[][16] = {"Business", "BusinessN", "Enterprise", "EnterpriseN", "Datacenter", "Standard", "Professional", "ProfessionalN"};
	for(int i=0;i<=8;i++){
		if(strcmp(g_strEdition, arstrEditions[i]) == 0) return 0;
	}
	if ((strcmp(g_strKernelVersion, "6.2") == 0) || (strcmp(g_strKernelVersion, "6.3") == 0)) return 0;
	
	return 1;
}

void InstallDrv()
{
	char strCmdRunWanDrv[MAX_PATH] = {0,};
	char chrBufYN;
	char strTitleBuf[MAX_PATH] = {0,};
	strcat(strTitleBuf, "title ");
	strcat(strTitleBuf, g_cstrTitle);
	strcat(strTitleBuf, " - ����̹� ��ġ");
	system(strTitleBuf);
	system("cls");
	strset(strTitleBuf, 0);
	strcat(strTitleBuf, "color ");
	strcat(strTitleBuf, UI_BG_DEFAULT);
	system(strTitleBuf);

	printf("Easy DriverPacks��(��) �����Ͽ� Ĩ�� �� ��Ʈ��ũ ����̹��� ��ġ�մϴ�.");
	puts("");
	for(;1;)
	{
		puts("");
		printf("Easy DriverPacks(%s)��(��) �����ϰڽ��ϱ�?\n", WANDRV_VERSION);
		printf("(Y/N): >");
		chrBufYN = (char)_fgetchar();
		fflush(stdin);
		//int c; while((c=getchar())!='\n'&&c!=EOF);
		chrBufYN = toupper(chrBufYN);
		if(chrBufYN == 'Y' || chrBufYN == 'N')
		{
			system("color 17");
			break;
		}
		puts("");
		printf("�߸� �Է��߽��ϴ�. Y(��) �Ǵ� N(�ƴϿ�)�� �Է��Ͻʽÿ�.");
		system("color 47");//���⼭���ʹ� �׳� �ʹ� ������� ���ڿ��� ����.
		puts("");
	}

	if(chrBufYN == 'Y')
	{
		strcat_s(strCmdRunWanDrv, "start /wait ");
		strcat_s(strCmdRunWanDrv, g_cstrScriptsPath);
		strcat_s(strCmdRunWanDrv, "WanDrv\\");
		if(strcmp(g_strKernelVersion, "6.1") == 0)
		{
			strcat_s(strCmdRunWanDrv, "Win6.1_");
		}else if(strcmp(g_strKernelVersion, "6.2") == 0){
			strcat_s(strCmdRunWanDrv, "Win10.0_");
		}else if(strcmp(g_strKernelVersion, "6.3") == 0){
			strcat_s(strCmdRunWanDrv, "Win10.0_");
		}else if(strcmp(g_strKernelVersion, "10.") == 0){
			strcat_s(strCmdRunWanDrv, "Win10.0_");
		}else{
			//erró�� - �������� �ʴ� OS
		}
		if(g_Is64BitSystem) strcat_s(strCmdRunWanDrv, "amd64\\"); else strcat_s(strCmdRunWanDrv, "x86\\");
		strcat_s(strCmdRunWanDrv, "WanDrv6.exe");
		system(strCmdRunWanDrv);
		puts("");
		printf("Easy DriverPacks(%s)��(��) �����մϴ�.", WANDRV_VERSION);
		puts("");
		g_bRecommandReboot = true;
	}
}

void FinishTask()
{
	char strTitleBuf[MAX_PATH] = {0,};
	strcat(strTitleBuf, "title ");
	strcat(strTitleBuf, g_cstrTitle);
	strcat(strTitleBuf, " - �Ϸ�");
	system(strTitleBuf);
	system("cls");
	system("color 17");

	ReserveTaskDeleteScripts();
	system("cls");
	puts("OOBE ��ũ��Ʈ ���� �۾��� ����Ǿ����ϴ�.");
	puts("");
	puts("��� ������ �Ϸ�Ǿ����ϴ�.");
	if (g_bRecommandReboot) puts("�������� ��ǻ�� ȯ���� ���� ���߿� ��ǻ�͸� �ٽ� �����Ͻʽÿ�.");
	puts("");
	puts("���� Windows�� �α׿� �մϴ�.");
	for(int i=200;i>=0;i--)
	{
		printf("��ġ���� �ƹ�Ű�� �����ʽÿ�. %d�� ��ٸ��� ��...  ", i/10);printf("\r");
		if(_kbhit()) break;
		Sleep(100);
	}
}

void ReserveTaskHideUpdates()
{
	char chrBufYN;
	char strTitleBuf[MAX_PATH] = {0,};
	strcat(strTitleBuf, "title ");
	strcat(strTitleBuf, g_cstrTitle);
	strcat(strTitleBuf, " - Windows ������Ʈ �����");
	system(strTitleBuf);
	system("cls");
	system("color 17");
	
	//���߿� ��� �����ֱ�
	puts("���ʿ��� Windows ������Ʈ(��: Windows ��ǰ ���� �˸�, Windows 10 ���׷��̵� ��)���� ����� �۾��� �����մϴ�.");
	puts("Windows Updates Ȯ���� �ʿ��ϹǷ� ��Ʈ��ũ ���� �� �� 10�а��� ��׶��� �۾��� �ʿ��մϴ�. Windows �۾� �����ٷ��� ��ϵǸ� �۾��Ϸ� �� �ڵ����� �����˴ϴ�.");
	for(;1;)
	{
		puts("");
		puts("���ʿ��� ������Ʈ ����� �۾��� ���� �� �����մϱ�?");
		printf("(Y/N): >");
		chrBufYN = (char)_fgetchar();
		fflush(stdin);
		//int c; while((c=getchar())!='\n'&&c!=EOF);
		chrBufYN = toupper(chrBufYN);
		if(chrBufYN == 'Y' || chrBufYN == 'N')
		{
			system("color 17");
			break;
		}
		puts("");
		printf("�߸� �Է��߽��ϴ�. Y(��) �Ǵ� N(�ƴϿ�)�� �Է��Ͻʽÿ�.");
		system("color 47");
		puts("");
	}
	if(chrBufYN == 'Y')
	{
		g_bReserveHideUpdates = true;
		if (g_bReserveHideUpdates){
			system("schtasks /create /ru SYSTEM /sc ONSTART /tn \"Hide Unwanted Windows Updates\" /tr \"wscript %systemroot%\\setup\\scripts\\hideUpdates.vbs\" /F /RL HIGHEST");
			system("schtasks /run /tn \"Hide Unwanted Windows Updates\"");
		}
	}
}

void ReserveTaskDeleteScripts()
{
	system("schtasks /create /ru SYSTEM /sc ONSTART /tn \"Delete OOBE Scripts\" /tr \"%systemroot%\\setup\\scripts\\DeleteScripts.cmd\" /F /RL HIGHEST");
}