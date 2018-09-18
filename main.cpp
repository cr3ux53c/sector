//CraXicS™ Integrated Windows Installation System
#include <cadoncio.h>

//상수 선언
#define KMSPICO_FILENAME "KMSpico_setup.exe"
#define KMSPICO_VERSION "9.1.3"
#define UI_BG_DEFAULT "17"//나중에 열거형으로 수정
#define UI_BG_ERR "47"
#define UI_BG_WARN "17"
#define WANDRV_VERSION "WanDrv 6.6.41.565.2016.0815"//나중에 하드코딩 최소화
const char* g_cstrTitle = "CraXicS(TM) Windows 시작 경험 OEM 스크립트 3.0 Beta 5";
const char* g_cstrScriptsPath = "%systemroot%\\Setup\\Scripts\\";
//전역변수 선언
char g_strEdition[MAX_PATH];
bool g_bIsNFW4;
char g_strKernelVersion[4];
bool g_Is64BitSystem;
bool g_bRecommandReboot;
bool g_bReserveHideUpdates;

//함수 선언
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
	char strTitleBuf[MAX_PATH] = {0,};//나중에 가변인수함수로 변경할 것.
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
	
	//에디션 구하기
	ReadRegistry(0, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "EditionID", &pBuf, sizeof(g_strEdition)/2, false);
	
	//닷넷 프레임워크 4 설치 여부 확인
	if (IsRegistryKey(0, "SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v4") == ERROR_SUCCESS) {
		g_bIsNFW4 = true;
	}

	//커널버전 확인
	FILE* pFile = _popen("ver", "r");
	if(pFile != NULL) {
		while(fgets(strConOut, sizeof(strConOut), pFile) != NULL);//널이 될때까지 pFile의 요소를 읽어서 output에 output의 사이즈만큼 저장한다.
		pstrVersion = strstr(strConOut, "Version");
	}
	_pclose(pFile);
	strncpy(pstrVersion, pstrVersion+8, 3);
	pstrVersion[3] = '\0';
	strcpy(g_strKernelVersion, pstrVersion);

	//비트 확인
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
	strcat(strTitleBuf, " - 소유자 정보 등록");
	system(strTitleBuf);
	system("cls");
	strset(strTitleBuf, 0);
	strcat(strTitleBuf, "color ");
	strcat(strTitleBuf, UI_BG_DEFAULT);
	system(strTitleBuf);

	puts("Windows 설치가 거의 마무리되었습니다.");
	puts("다음의 몇 가지 설정을 완료하십시오.");
	puts("");
	puts("Windows 정보에 등록될 소유자 이름을 입력하십시오.");
	puts("'Enter'만 입력할 시 공백으로 등록됩니다.");
	printf("사용자 이름: >");
	fgets(strOwner, sizeof(strOwner), stdin);
	puts("");
	puts("Windows 정보에 등록될 조직 이름을 입력하십시오.");
	puts("'Enter'만 입력할 시 공백으로 등록됩니다.");
	printf("조직 이름: >");
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
	strcat(strTitleBuf, " - Windows 정품 인증");
	system(strTitleBuf);
	system("cls");
	strset(strTitleBuf, 0);
	strcat(strTitleBuf, "color ");
	strcat(strTitleBuf, UI_BG_DEFAULT);
	system(strTitleBuf);

	puts("Windows 정품 인증을 위해 KMSpico을(를) 설치합니다.");
	puts("인증 키를 소유하고 있거나 다른 방법으로 Windows 정품 인증을 하려면 이 과정을 건너띄십시오.");
	puts("");
	for(;1;)
	{
		printf("KMSpico %s을(를) 설치하겠습니까?\n", KMSPICO_VERSION);//이부분도 함수로 만들기
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
		printf("잘못 입력했습니다. Y(예) 또는 N(아니오)를 입력하십시오.");
		system("color 47");//여기서부터는 그냥 너무 길어지니 문자열로 쓴다.
		puts("");
	}

	if(ChkSupportedOSVersion() == 1)
	{
		system("color 57");
		puts("");
		puts("현재 Windows 에디션은 볼륨 라이센스가 아니기 때문에 KMSpico은(는) Microsoft Office 2010/2013 제품군에 대한 정품 인증만 지원합니다.");
		puts("");
		for(;1;)
		{
			printf("그래도 KMSpico %s을(를) 설치하겠습니까?\n", KMSPICO_VERSION);
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
			printf("잘못 입력했습니다. Y(예) 또는 N(아니오)를 입력하십시오.");
			system("color 47");//여기서부터는 그냥 너무 길어지니 문자열로 쓴다.
			puts("");
		}
	}

	if(g_bIsNFW4)
	{
		char strKMSFullPath[MAX_PATH] = {0,};
		
		puts("");
		printf("KMSpico %s을(를) 설치합니다.", KMSPICO_VERSION);
		puts("");
		puts("KMSpico 설치 중...");
		strcat(strKMSFullPath, "start /wait ");
		strcat(strKMSFullPath, g_cstrScriptsPath);
		strcat(strKMSFullPath, KMSPICO_FILENAME);
		strcat(strKMSFullPath, " /verysilent /noicons");
		system(strKMSFullPath);
		//system("regedit /s RunOnce.reg");
	}else{
		system("color 57");
		puts("");
		puts("현재 Windows에 Microsoft .NET Framework 4이(가) 설치되어 있지 않습니다. KMSpico을(를) 설치하기 위해선 Microsoft .NET Framework 4을(를) 먼저 설치해야 합니다.");
		puts("계속하려면 아무키나 누르십시오.");
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
	strcat(strTitleBuf, " - 드라이버 설치");
	system(strTitleBuf);
	system("cls");
	strset(strTitleBuf, 0);
	strcat(strTitleBuf, "color ");
	strcat(strTitleBuf, UI_BG_DEFAULT);
	system(strTitleBuf);

	printf("Easy DriverPacks을(를) 실행하여 칩셋 및 네트워크 드라이버를 설치합니다.");
	puts("");
	for(;1;)
	{
		puts("");
		printf("Easy DriverPacks(%s)을(를) 실행하겠습니까?\n", WANDRV_VERSION);
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
		printf("잘못 입력했습니다. Y(예) 또는 N(아니오)를 입력하십시오.");
		system("color 47");//여기서부터는 그냥 너무 길어지니 문자열로 쓴다.
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
			//err처리 - 지원되지 않는 OS
		}
		if(g_Is64BitSystem) strcat_s(strCmdRunWanDrv, "amd64\\"); else strcat_s(strCmdRunWanDrv, "x86\\");
		strcat_s(strCmdRunWanDrv, "WanDrv6.exe");
		system(strCmdRunWanDrv);
		puts("");
		printf("Easy DriverPacks(%s)을(를) 실행합니다.", WANDRV_VERSION);
		puts("");
		g_bRecommandReboot = true;
	}
}

void FinishTask()
{
	char strTitleBuf[MAX_PATH] = {0,};
	strcat(strTitleBuf, "title ");
	strcat(strTitleBuf, g_cstrTitle);
	strcat(strTitleBuf, " - 완료");
	system(strTitleBuf);
	system("cls");
	system("color 17");

	ReserveTaskDeleteScripts();
	system("cls");
	puts("OOBE 스크립트 삭제 작업이 예약되었습니다.");
	puts("");
	puts("모든 설정이 완료되었습니다.");
	if (g_bRecommandReboot) puts("안정적인 컴퓨팅 환경을 위해 나중에 컴퓨터를 다시 시작하십시오.");
	puts("");
	puts("이제 Windows로 로그온 합니다.");
	for(int i=200;i>=0;i--)
	{
		printf("마치려면 아무키나 누르십시오. %d초 기다리는 중...  ", i/10);printf("\r");
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
	strcat(strTitleBuf, " - Windows 업데이트 숨기기");
	system(strTitleBuf);
	system("cls");
	system("color 17");
	
	//나중에 목록 보여주기
	puts("불필요한 Windows 업데이트(예: Windows 정품 인증 알림, Windows 10 업그레이드 등)들을 숨기는 작업을 실행합니다.");
	puts("Windows Updates 확인이 필요하므로 네트워크 연결 및 약 10분간의 백그라운드 작업이 필요합니다. Windows 작업 스케줄러에 등록되며 작업완료 후 자동으로 삭제됩니다.");
	for(;1;)
	{
		puts("");
		puts("불필요한 업데이트 숨기기 작업을 예약 및 실행합니까?");
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
		printf("잘못 입력했습니다. Y(예) 또는 N(아니오)를 입력하십시오.");
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