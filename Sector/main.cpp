/*
CodeName Sector™ Project
Copyright (c) 2014 by CraXicS™. Some Rights Reserved.
License: 이 프로그램은 무료이며 일반적인 자유 소프트웨어 라이센스(Free Sotfware License)를 따릅니다. Windows 로고 및 아이콘은 Microsoft Corporation의 등록 상표입니다.

이 프로그램에는다음의 외부 라이브러리가 포함되어 있습니다.
TinyXML 2.6.2

*/
#include <Windows.h>
#include "resource.h"
#include <WindowsX.h>
#include <Shlwapi.h>//PathFileExists()
#include <string.h>
//#include <stdlib.h>//System Commands
#include <cstdio>
//#include <stdio.h>
#include "tinyxml.h"

#pragma comment(lib, "tinyxml_x86.lib")
//#pragma comment(lib, "tinyxml_x64.lib")
#pragma comment(lib, "Shlwapi.lib")

//Define
#define	CTM_SYSABOUT	1005
#define	CTM_SYSCMD		1006
#define	CTM_SYSTASKMGR	1007
	//String 선언
#define	STR_COPYRIGHT			"Copyleft, 2014 by CraXicS™. Some Rights Reversed."
#define	STR_AUTOUNATTEND		TEXT(":\\AutoUnattend.xml")
#define STR_CMDLINE_UNATTEND	TEXT(":\\sources\\setup.exe /unattend:")
#define STR_CMDLINE				TEXT(":\\sources\\setup.exe")
#define	STR_CMDLINE_UNATTEND2	TEXT(":\\sources\\unattend\\")
#define	STR_CMDLINE_UNATTEND3	TEXT(":\\sources\\unattend")
#define STR_RECENV				TEXT(":\\sources\\recovery\\RecEnv.exe")
#define	STR_UNATTENDINFO		"\\Unattend_Info.xml"
#define STR_PATH_CMD			TEXT(":\\Windows\\System32\\cmd.exe")
#define STR_PATH_TASKMGR		TEXT(":\\Windows\\System32\\taskmgr.exe")

//함수 정방향 선언
TCHAR* GetCurDir(TCHAR strAppend[]);
INT_PTR CALLBACK AboutProc(HWND hDlgAbout, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyBDProc(int nKeyCode, WPARAM wParam, LPARAM lParam);

//전역변수
const TCHAR *g_pBufThisDir = GetCurDir(NULL);
TCHAR		g_Ary2_FileData[32][64] = { 0 };
TCHAR		g_Ary2_Desc[32][128] = { 0 };
int			g_nUnattendFiles = 0;
int			g_nComboxUnattendCurIdx = 0;
char		g_chDrvLtrOfUnattendDir = 0;
TCHAR		*strComboxFirst[2] = { 0 };
TCHAR		g_BufCreProcCmd[256] = { 0 };
bool		g_bPEMode = false;
STARTUPINFO	StartUpInfo = { 0 };
PROCESS_INFORMATION	ProcInfo = { 0 };
//플래그 선언
double		g_dUnattendInfoVer = 1.0;
bool		g_bIsUnattendInfoXML = false;
bool		g_bComboxIdx0 = true;
BOOL		g_bCreProc_StateOfAutoSetup = FALSE;
//bool		g_bFinishWaitOfAutoUnattend = 0;//WM_WINDOWPOSCHANGING에 의한 SW_HIDE 감지 및 설치 취소 후 SW_SHOW 목적.
	//핸들 선언
HHOOK		hHOOK;
HINSTANCE	g_hInst;
HWND		g_hWnd;
HWND		g_hComboxFirst = 0;
HWND		g_hComboxUnattend = 0;
//HWND		g_hDlgAbout = 0;

void IdentifyBufXML(const char* pBufXMLFile1, TCHAR Ary2_XMLFile2[][64], const char* pBufDesc) {//pBufXMLFile, g_Ary2_FileData
//	g_nUnattendFiles가 1이면 파일 하나가 있는 것이고 그 때 BufXMLFile 배열의 인덱스 최댓값은 0이다.
	int		Idx_XMLIndex = 0;
	bool	bSynced = false;
	int		Idx_XMLIndexOrigin = -1;
	char	Ary1_XMLFile2ANSI[64] = { 0 };
Goto_ReSearch:
	WideCharToMultiByte(CP_ACP, 0, Ary2_XMLFile2[Idx_XMLIndex], -1, Ary1_XMLFile2ANSI, 63, 0, 0);
	if (_stricmp(pBufXMLFile1, Ary1_XMLFile2ANSI) == 0) {//같다. //대소문자 구분 안 함
		TCHAR pBufDescWIDE[128] = { 0 };
		MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pBufDesc, -1, pBufDescWIDE, 127);
		for (int i = 0; i <= 127; i++) {
			g_Ary2_Desc[Idx_XMLIndex][i] = pBufDescWIDE[i];
		}
		bSynced = true;
	}
	if (bSynced) {//동기화 성공 시
		//이 함수를 나간다.
	} else {//동기화 실패 시: 아직 못 찾음 or 찾는 텍스트가 없음->파일 명을 Desc로 주기.

		if (Idx_XMLIndex < g_nUnattendFiles - 1) {
			++Idx_XMLIndex; goto Goto_ReSearch;
		} else {//두 변수가 같거나 그 이상. 즉, 찾는 텍스트가 없다.
			//함수를 나간다.
		}
	}

}//IdentifyXML

void TiXMLParseXML(HWND hWnd, TCHAR BufPathUnattend[]) {
	char BufFilePathUnattendANSI[128] = { 0 };//검색 버퍼 ANSI
	char temp[128] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, BufPathUnattend, -1, BufFilePathUnattendANSI, 127, 0, 0);
	for (int i = 0; i <= 127; i++) {
		temp[i] = BufFilePathUnattendANSI[i];
	}
	strcat(temp, STR_UNATTENDINFO);
	for (int j = 0; j <= 127; j++) {
		BufFilePathUnattendANSI[j] = temp[j];
	}
	TiXmlDocument TiXMLDoc(BufFilePathUnattendANSI);
	if (TiXMLDoc.LoadFile() == false) {
		MessageBox(hWnd, TEXT("응답 파일 정보 XML을 액세스할 수 없습니다."), TEXT("응답 파일 정보 XML 액세스 오류"), MB_OK | MB_ICONERROR);
	} else {
		TiXmlElement *pTiXMLEle = 0;
		pTiXMLEle = TiXMLDoc.FirstChildElement("UnattendInfo");
		if (!pTiXMLEle) {
			MessageBox(hWnd, TEXT("응답 파일 정보 XML의 구성이 올바르지 않습니다.\nUnattendInfo 노드가 없습니다."), TEXT("응답 파일 정보 XML 액세스 오류"), MB_OK | MB_ICONERROR);
		} else {
			const char* pBufSectorCompati = pTiXMLEle->Attribute("SectorCompatibility");
			double dSectorCompati = (double)atof(pBufSectorCompati);
			if (dSectorCompati != g_dUnattendInfoVer) {
				MessageBox(hWnd, TEXT("응답 파일 정보 XML 버전이 호환되지 않습니다."), TEXT("응답 파일 정보 XML 액세스 오류"), MB_OK | MB_ICONERROR);
			} else {
				pTiXMLEle = 0;
				pTiXMLEle = TiXMLDoc.FirstChildElement("UnattendInfo")->FirstChildElement("Index");
				if (!pTiXMLEle) {
					MessageBox(hWnd, TEXT("응답 파일 정보 XML의 구성이 올바르지 않습니다.\nIndex 노드가 없습니다."), TEXT("응답 파일 정보 XML 액세스 오류"), MB_OK | MB_ICONERROR);
				} else {
					for (int idx = 0; idx <= g_nUnattendFiles; idx++) {
						//Attribution에 대한 오류 체크는 생략 함.
						for (pTiXMLEle; pTiXMLEle; pTiXMLEle = pTiXMLEle->NextSiblingElement()) {//Next 노드 검색 //맨 마지막 단계의 Next()함수는 for문이 끝난 뒤 실행되므로 마지막 코드가 한번 더 실행된다.
							const char* pBufXMLFile = pTiXMLEle->Attribute("XMLFile");
							const char* pBufDesc = pTiXMLEle->Attribute("Description");
							IdentifyBufXML(pBufXMLFile, g_Ary2_FileData, pBufDesc);
						}//For_NextSibling
						pTiXMLEle = TiXMLDoc.FirstChildElement("UnattendInfo")->FirstChildElement("Index");
					}
				}
			}
		}
	}//if_TiXMLDoc.LoadFile
}//TiXMLParseXML

void BufferingFileDataToDesc() {
	for (int i = 0; i <= g_nUnattendFiles - 1; i++) {
		if (g_Ary2_Desc[i][0] == 0) {
			for (int j = 0; j <= 63; j++) {
				g_Ary2_Desc[i][j] = g_Ary2_FileData[i][j];
				g_Ary2_Desc[i][64] = 0;
			}
		}
	}
}

TCHAR* GetCurDir(TCHAR strAppend[]) {
	static TCHAR strPath[1024] = { 0 };
	DWORD RetV = GetModuleFileName(GetModuleHandle(0), strPath, MAX_PATH);
	if (RetV == 0) return NULL;
	int i, len = wcslen(strPath);
	int pos = -1;

	for (i = 0; i < len; i++) {
		if (strPath[i] == '\\')
		{
			pos = i;
		}
	}
	if (pos != -1) {
		strPath[pos + 1] = 0;
	}
	if (strAppend != NULL) {
		wcscat(strPath, strAppend);
	}
	return strPath;
}

void FindXMLFile(HWND hWnd, TCHAR* SearchRoute, TCHAR SaveArrary[][64]) {
	HANDLE hFindFile;
	WIN32_FIND_DATA FileData;
	BOOL bRet = TRUE; //성능 저하 경고를 피하기 위해 BOOL 타입으로 선언
	TCHAR BufSearchPath[128] = { 0 };
	for (int i = 0; i < 127; i++) BufSearchPath[i] = SearchRoute[i];
	wcscat(BufSearchPath, TEXT("\\*.xml"));
	hFindFile = FindFirstFile(BufSearchPath, &FileData);
	if (hFindFile == INVALID_HANDLE_VALUE)	MessageBox(hWnd, TEXT("파일 검색(*.xml)을 실패했습니다."), TEXT("파일 검색 실패"), MB_OK);
	while (hFindFile != INVALID_HANDLE_VALUE && bRet) {//파일 검색이 실패가 아님 && bRet가 TRUE 임
		if ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {//두 값을 AND 연산한 값이 0이면 검색한 객체가 파일이다.
			if (_wcsicmp(FileData.cFileName, TEXT("Unattend_Info.xml")) == 0) {//대소문자 구분 안 함
				g_bIsUnattendInfoXML = true;
			} else {
				for (int i = 0; i <= 63; i++)	SaveArrary[g_nUnattendFiles][i] = FileData.cFileName[i];
				SaveArrary[g_nUnattendFiles][64] = 0;
				g_nUnattendFiles++;
			}//if
		}//if
		bRet = FindNextFile(hFindFile, &FileData);
	}//while
	FindClose(hFindFile);
}//FindXMLFile

//메인 폼 프로시저
INT_PTR CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{//프로시저 함수 안에서만 사용하는 변수 선언
	int wmId, wmEvent;
	//GDI 초기화
	HDC hDC, hMemDC;	PAINTSTRUCT ps;
	HBITMAP hCurBMP, hOldBMP;	BITMAP BMP;
	RECT ClientRect;	GetClientRect(hWnd, &ClientRect);
	RECT WndRect;	GetWindowRect(hWnd, &WndRect);
	HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
	HFONT hFont;
	HICON hIcon;

	int SysWidth = GetSystemMetrics(SM_CXSCREEN);	int SysHeight = GetSystemMetrics(SM_CYSCREEN);

	StartUpInfo.cb = sizeof(StartUpInfo);
	//	 si.dwFlags = STARTF_USEPOSITION | STARTF_USESIZE;
		 //STARTF_USEPOSITION :: dwX, dwY 위치 사용
		 //STARTF_USESIZE     :: dwXSize, dwYSize 사용
		 //Flags 값이 주어지지 않는 si구조체 값은 새로 만들어지는 프로세스에 영향을 주지 않습니다.
		 //si.dwX = 100;
		 //si.dwY = 100;
		 //si.dwXSize = 300;
		 //si.dwYSize = 300;  //dw ~ 는 사실 잘 쓰이지 않습니다.
		 //si.lpTitle = _T(" Child process! ");

	switch (message)
	{
		//if (g_bFinishWaitOfAutoUnattend){
		//	ShowWindow(hWnd, SW_SHOW);
		//	g_bFinishWaitOfAutoUnattend = false;
		//}

	case WM_INITDIALOG: {
		//핸들 할당
		g_hWnd = hWnd;
		g_hComboxFirst = GetDlgItem(hWnd, IDC_COMBO1);
		g_hComboxUnattend = GetDlgItem(hWnd, IDC_COMBO2);
		//아이콘 설정
		hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		//시스템 메뉴
		AppendMenu(hSysMenu, MF_SEPARATOR, NULL, NULL); //구분선 추가
		AppendMenu(hSysMenu, MF_STRING, CTM_SYSCMD, TEXT("명령 프롬프트"));
		AppendMenu(hSysMenu, MF_STRING, CTM_SYSTASKMGR, TEXT("작업 관리자"));
		AppendMenu(hSysMenu, MF_STRING, CTM_SYSABOUT, TEXT("Sector 프로젝트 정보"));
		//파일 유무 확인
			//AutoUnattend.xml 체크
		TCHAR BufA[64] = { 0 }; BufA[0] = g_pBufThisDir[0];
		wcscat(BufA, STR_AUTOUNATTEND);
		if (PathFileExists(BufA) == TRUE) {//현재 프로세스의 드라이브에서 검색
			g_BufCreProcCmd[0] = g_pBufThisDir[0];
			wcscat(g_BufCreProcCmd, STR_CMDLINE_UNATTEND);
			wcscat(g_BufCreProcCmd, BufA);

			g_bCreProc_StateOfAutoSetup = CreateProcess(
				NULL, //생성될 프로세스의 이름, 확장명을 포함해야한다. (NULL을 넣고 IpCommandLine로 전달 가능) 첫번째로 전달되는 인자를 통해서 실행파일의 이름이 전달하는 경우에는
				//현재 디렉터리를 기준으로 실행파일을 찾게 된다. 하지만 첫번째에 NULL을 넣고 두번째 인자로 전달되는 경우에는 표준 검색경로 순서대로 실행파일을 찾는다.
				g_BufCreProcCmd, //argc / argv[] 인자로 전달할 문자 매개변수. 표준 검색 경로를 기준으로 찾는다.
				//1.프로세스의 실행파일이 존재하는 풀더
				//2.실행중인 프로세스의 현재 풀더
				//3.windows의 시스템 풀더(system Directory)
				//4.windows풀더 (windows directory)
				//5.환경변수 PATH
				NULL,//보안 속성을 지정하는 인자. NULL=기본값
				NULL, //스레드의 보안 속성을 지정하는 인자. NULL=기본값
				TRUE, //부모프로세스중 상속가능한 핸들 상속 할 것인가
				0, //이전 값: CREATE_NEW_CONSOLE// 프로세스의 특성을 결정짓는 옵션. 사용 안할경우 0 입력
				NULL, //환경변수 지정. NULL 전달시 부모는 자식에게 환경변수 복사
				NULL,//생성하는 프로세스의 현재 디렉터리. NULL시 자식위치 = 부모위치
				&StartUpInfo, //STARTUPINFO 구조체 변수 초기화후 변수의 포인터를 매개변수 로 전달. STARTUPINFO 구조체 변수들은 프로세스의 속성 정보를 전달한다.
				&ProcInfo  //생성하는 프로세스의 정보를 얻기 위한 매개변수. PROCESS_INFORMATION 구조체 변수의 주소값을 받음
			);

			if (g_bCreProc_StateOfAutoSetup) {

			} else {//실행 실패
				MessageBox(hWnd, TEXT("\\sources\\setup.exe를 시작하지 못했습니다."), TEXT("setup.exe 실행 오류"), MB_OK | MB_ICONERROR);
			}
		} else {//모든 드라이브에서 검색
			for (char ch = 65; ch <= 90; ch++) {//A~Z 드라이브 명 //나중에 시디 디렉터리 얻는 방법 강구
				TCHAR BufA[64] = { 0 }; BufA[0] = ch;
				wcscat(BufA, STR_AUTOUNATTEND);
				if (PathFileExists(BufA)) {
					g_BufCreProcCmd[0] = g_pBufThisDir[0];
					wcscat(g_BufCreProcCmd, STR_CMDLINE_UNATTEND);
					wcscat(g_BufCreProcCmd, BufA);
					g_bCreProc_StateOfAutoSetup = CreateProcess(NULL, g_BufCreProcCmd, NULL, NULL, TRUE, NULL, NULL, NULL, &StartUpInfo, &ProcInfo);
					if (g_bCreProc_StateOfAutoSetup) { // 실행 성공
					} else {//실행 실패
						MessageBox(hWnd, TEXT("\\sources\\setup.exe를 시작하지 못했습니다."), TEXT("setup.exe 실행 오류"), MB_OK | MB_ICONERROR);
					}
					break;
				}//if
			}//for
		}//if

			//ISO\sources\Unattend 디렉터리 존재 확인
		for (char ch = 65; ch <= 90; ch++) {//드라이브 문자
			TCHAR BufPathUnattend[64] = { 0 }; BufPathUnattend[0] = ch;//드라이브 문자 할당
			wcscat(BufPathUnattend, STR_CMDLINE_UNATTEND3);
			if (PathFileExists(BufPathUnattend)) {//unattend 폴더 존재
				FindXMLFile(hWnd, BufPathUnattend, g_Ary2_FileData);
				if (g_bIsUnattendInfoXML) {
					TiXMLParseXML(hWnd, BufPathUnattend);
					BufferingFileDataToDesc();
					g_chDrvLtrOfUnattendDir = BufPathUnattend[0];
				}
				break;
			}//if
		}//for

		//콤보박스 설정
			//ComBoxFirst
		strComboxFirst[0] = TEXT("Windows 설치");
		ComboBox_AddString(g_hComboxFirst, strComboxFirst[0]);
		ComboBox_SetCurSel(g_hComboxFirst, 0);

		//RecEnv 존재 확인
		TCHAR BufSearchFilePath[32] = { 0 };
		BufSearchFilePath[0] = g_pBufThisDir[0];
		wcscat(BufSearchFilePath, STR_RECENV);
		if (PathFileExists(BufSearchFilePath)) {
			strComboxFirst[1] = TEXT("Windows 복구 모드");
			ComboBox_AddString(g_hComboxFirst, strComboxFirst[1]);
		}
		//ComBoxUnattend
		if (g_bIsUnattendInfoXML) {
			ShowWindow(g_hComboxUnattend, SW_SHOW);
			TCHAR* strComboxUnattend[128] = { 0 };
			strComboxUnattend[0] = TEXT("응답 파일 사용 안 함");
			for (int j = 0; j <= g_nUnattendFiles - 1; j++) {
				strComboxUnattend[j + 1] = g_Ary2_Desc[j];
			}
			for (int i = 0; i <= g_nUnattendFiles; i++) {
				ComboBox_AddItemData(g_hComboxUnattend, strComboxUnattend[i]);
			}
			ComboBox_SetCurSel(g_hComboxUnattend, 0);
		}
		//AutoUnattend에 의한 setup.exe 종료 감지
		if (g_bCreProc_StateOfAutoSetup) {
			WaitForSingleObject(ProcInfo.hProcess, INFINITE);
			//	g_bFinishWaitOfAutoUnattend = true;
		}

		//WM_CLOSE 사용자 확인
		TCHAR BufCheckCurDir[32] = { 0 }; BufCheckCurDir[0] = g_pBufThisDir[0];
		wcscat(BufCheckCurDir, TEXT(":\\sources"));
		if (PathFileExists(BufCheckCurDir)) {
			ZeroMemory(BufCheckCurDir, sizeof(BufCheckCurDir));
			BufCheckCurDir[0] = g_pBufThisDir[0];
			wcscat(BufCheckCurDir, TEXT(":\\setup.exe"));
			if (PathFileExists(BufCheckCurDir))
				g_bPEMode = true;
		}

	}//case_WM_INITDIALOG
						break;
	case WM_COMMAND://ref: http://msdn.microsoft.com/en-us/library/windows/desktop/ms647591(v=vs.85).aspx
		switch (LOWORD(wParam)) {////WPARAM에 BN_CLICKED(HI WORD)와 해당 컨트롤의 ID(LOW WORD)가 저장되고 LPARAM에 해당 버튼 컨트롤의 핸들이 저장된 상태로 전달됩니다.
		case IDC_COMBO1://작업 선택 콤보
			switch (HIWORD(wParam)) {
			case CBN_SELENDOK:
				switch (SendMessage(g_hComboxFirst, CB_GETCURSEL, NULL, NULL)) {
				case 0://Windows 설치 선택
					if (g_bIsUnattendInfoXML) ShowWindow(g_hComboxUnattend, SW_SHOW);
					g_bComboxIdx0 = true;
					break;
				case 1://Windows 복구 모드 선택
					if (g_bIsUnattendInfoXML) ShowWindow(g_hComboxUnattend, SW_HIDE);
					g_bComboxIdx0 = false;
					break;
				}//switch
				InvalidateRect(hWnd, NULL, true);//클라이언트 전체 다시 그리기
				break;
			}
			break;
		case IDC_COMBO2://응답 파일 콤보 박스
			switch (HIWORD(wParam)) {
			case CBN_SELENDOK:
				g_nComboxUnattendCurIdx = SendMessage(g_hComboxUnattend, CB_GETCURSEL, NULL, NULL);
				break;
			}
			break;
		case IDC_BUTTON1://다음 버튼
			ZeroMemory(g_BufCreProcCmd, sizeof(g_BufCreProcCmd));
			switch (SendMessage(g_hComboxFirst, CB_GETCURSEL, NULL, NULL)) {
			case 0: {//Windows 설치
				if (!g_bIsUnattendInfoXML) {//UnattendInfo가 존재하지 않음
				Goto_ForceNotUnattendSetup:
					g_BufCreProcCmd[0] = g_pBufThisDir[0];
					wcscat(g_BufCreProcCmd, STR_CMDLINE);
					if (CreateProcess(0, g_BufCreProcCmd, 0, 0, TRUE, 0, 0, 0, &StartUpInfo, &ProcInfo)) {
						ShowWindow(hWnd, SW_HIDE);
						WaitForSingleObject(ProcInfo.hProcess, INFINITE);
						ShowWindow(hWnd, SW_SHOW);
					} else {
						MessageBox(hWnd, TEXT("setup.exe를 실행할 수 없습니다."), TEXT("setup.exe 실행 오류"), MB_OK | MB_ICONERROR);
					}//CreateProcess
				} else {//UnattendInfo가 존재 함.
					if (g_nComboxUnattendCurIdx == 0) goto Goto_ForceNotUnattendSetup;

					g_BufCreProcCmd[0] = g_pBufThisDir[0];
					wcscat(g_BufCreProcCmd, STR_CMDLINE_UNATTEND);
					int len = wcslen(g_BufCreProcCmd);
					g_BufCreProcCmd[len] = g_chDrvLtrOfUnattendDir; g_BufCreProcCmd[len + 1] = 0;
					wcscat(g_BufCreProcCmd, STR_CMDLINE_UNATTEND2);
					wcscat(g_BufCreProcCmd, g_Ary2_FileData[g_nComboxUnattendCurIdx - 1]);
					if (CreateProcess(0, g_BufCreProcCmd, 0, 0, TRUE, 0, 0, 0, &StartUpInfo, &ProcInfo)) {
						ShowWindow(hWnd, SW_HIDE);
						WaitForSingleObject(ProcInfo.hProcess, INFINITE);
						ShowWindow(hWnd, SW_SHOW);
					} else {
						MessageBox(hWnd, TEXT("setup.exe를 실행할 수 없습니다."), TEXT("setup.exe 실행 오류"), MB_OK | MB_ICONERROR);
					}//CreateProcess
				}//g_bIsUnattendInfoXML	
			}//Windows 설치
					break;
			case 1://Windows 복구 모드
				g_BufCreProcCmd[0] = g_pBufThisDir[0];
				wcscat(g_BufCreProcCmd, STR_RECENV);
				if (CreateProcess(0, g_BufCreProcCmd, 0, 0, TRUE, 0, 0, 0, &StartUpInfo, &ProcInfo)) {
					ShowWindow(hWnd, SW_HIDE);
					WaitForSingleObject(ProcInfo.hProcess, INFINITE);
					ShowWindow(hWnd, SW_SHOW);
				} else {
					MessageBox(hWnd, TEXT("RecEnv.exe를 실행할 수 없습니다."), TEXT("RecEnv.exe 실행 오류"), MB_OK | MB_ICONERROR);
				}
				break;
			case CB_ERR://오류
				MessageBox(hWnd, TEXT("콤보 박스 인덱스 오류"), TEXT("콤보 박스 인덱스 오류"), MB_OK | MB_ICONERROR);
				break;
			}//다음 버튼 스위치
			break;
		}//버튼 구분 스위치
		break;
	case WM_SYSCOMMAND:
		wmId = LOWORD(wParam);
		switch (wmId)
		{
		case CTM_SYSABOUT:
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, AboutProc);
			break;
		case CTM_SYSCMD: {//함수화 할 것
			TCHAR BufFindPath[128] = { 0 };
			BufFindPath[0] = g_pBufThisDir[0];
			wcscat(BufFindPath, STR_PATH_CMD);
			if (PathFileExists(BufFindPath)) {
				CreateProcess(BufFindPath, 0, 0, 0, 0, 0, 0, 0, &StartUpInfo, &ProcInfo);
			} else {
				MessageBox(hWnd, TEXT("cmd.exe를 실행할 수 없습니다."), TEXT("cmd.exe 실행 오류"), MB_OK | MB_ICONERROR);
			}
		}
						 break;
		case CTM_SYSTASKMGR: {
			TCHAR BufFindPath[128] = { 0 };
			BufFindPath[0] = g_pBufThisDir[0];
			wcscat(BufFindPath, STR_PATH_TASKMGR);
			if (PathFileExists(BufFindPath)) {
				CreateProcess(BufFindPath, 0, 0, 0, 0, 0, 0, 0, &StartUpInfo, &ProcInfo);
			} else {
				MessageBox(hWnd, TEXT("taskmgr.exe를 실행할 수 없습니다."), TEXT("taskmgr.exe 실행 오류"), MB_OK | MB_ICONERROR);
			}
		}
							 break;
		}
		break;
	case WM_WINDOWPOSCHANGING: {
		//if (g_bCreProc_StateOfAutoSetup){//성공 시
		//	WINDOWPOS *WinPos = (WINDOWPOS*)lParam;
		//	WinPos->flags &= ~SWP_SHOWWINDOW;
		//}//원래 다이얼로그 박스 실행 시 숨기기 위해 있던 코드였으나 WaitFor 함수로 인해 숨기는 효과가 있으니 필요없음. WM_COMMAND 맨 뒤에 ShowWindow 함수가 호출됨
	}//case_WM_WINDOWPOSCHANGING
							   break;
	case WM_PAINT:
		hDC = BeginPaint(g_hWnd, &ps);
		hCurBMP = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));//BLUE 배경
		hMemDC = CreateCompatibleDC(hDC);
		GetObject(hCurBMP, sizeof(BITMAP), &BMP);
		hOldBMP = (HBITMAP)SelectObject(hMemDC, hCurBMP); //새 BMP를 받고 기본 BMP는 OLD에 백업
		BitBlt(hDC, 0, 0, ClientRect.right, ClientRect.bottom, hMemDC, 0, 0, SRCCOPY);

		hCurBMP = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));//win7logo 24.0.82
		GetObject(hCurBMP, sizeof(BITMAP), &BMP);
		SelectObject(hMemDC, hCurBMP);
		BitBlt(hDC, ((ClientRect.right - ClientRect.left) - BMP.bmWidth) / 2, ((ClientRect.bottom - ClientRect.top) - BMP.bmHeight) / 2 - 120, BMP.bmWidth, BMP.bmHeight, hMemDC, 0, 0, SRCCOPY);
		//	TransparentBlt(hDC, ((ClientRect.right - ClientRect.left) - BMP.bmWidth)/2, ((ClientRect.bottom - ClientRect.top) - BMP.bmHeight)/2-120, BMP.bmWidth, BMP.bmHeight, hMemDC, 0, 0, BMP.bmWidth, BMP.bmHeight, RGB(0, 0, 0));

			//텍스트 출력
		hFont = CreateFont(14, 0, 0, 0, 400, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("맑은 고딕"));
		SelectObject(hDC, hFont);
		SetBkMode(hDC, TRANSPARENT);
		SetTextColor(hDC, RGB(255, 255, 255));
		TextOut(hDC, 15, ClientRect.bottom - 25, TEXT(STR_COPYRIGHT), strlen(STR_COPYRIGHT) - 2);
		hFont = CreateFont(20, 0, 0, 0, 500, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("맑은 고딕"));
		SelectObject(hDC, hFont);
		TextOut(hDC, 160, (ClientRect.bottom - ClientRect.top) / 2 - 12, TEXT("작업 선택: "), lstrlen(TEXT("작업 선택: ")));
		if (g_bIsUnattendInfoXML && g_bComboxIdx0) TextOut(hDC, 125, (ClientRect.bottom - ClientRect.top) / 2 + 28, TEXT("응답 파일 선택: "), lstrlen(TEXT("응답 파일 선택: ")));
		//DC 해제
		DeleteObject(hCurBMP); DeleteObject(hFont);
		DeleteDC(hMemDC); DeleteDC(hDC);
		EndPaint(g_hWnd, &ps);
		break;
	case WM_CLOSE:
		if (g_bPEMode) {
			if (MessageBox(hWnd, TEXT("Windows 준비를 종료하시겠습니까?\n컴퓨터가 다시 부팅됩니다."), TEXT("Sector 종료"), MB_ICONEXCLAMATION | MB_YESNO) == IDYES) {
				EndDialog(hWnd, NULL);
				return TRUE;
			}
		} else {
			EndDialog(hWnd, NULL);
			return TRUE;
		}
		break;
	}//메시지 스위치
	return 0;
}//WndProc

INT_PTR CALLBACK AboutProc(HWND hDlgAbout, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message) {
	case WM_INITDIALOG: {
		//	g_hDlgAbout = hDlgAbout;
		//	hHOOK = SetWindowsHookEx(WH_KEYBOARD, KeyBDProc, 0, GetCurrentThreadId());
		HWND hLabelAbout = GetDlgItem(hDlgAbout, IDC_STATIC);
		SetWindowText(hLabelAbout, TEXT("\n   라이센스(License):\n    이 프로그램은 무료이며 일반적인 자유 소프트웨어 라이센스(Free Sotfware License)를 따릅니다.\n    Windows 아이콘 및 로고는 Microsoft Corporation의 등록 상표입니다.\n\n  이 프로그램에는 다음의 외부 라이브러리가 포함되어 있습니다.\n    TinyXML 2.6.2\n\n  이 프로그램은 다음의 언어 및 플랫폼으로 개발되었습니다.\n    Visual C++ 10.0 / Win32 API\n    Microsoft Windows 7 SP1 x64\n    Microsoft Visual Studio 2010 SP1 x86\n\n  기술 지원: http://windowsforum.kr\n"));
	}
						break;
	case WM_CLOSE:
		//	UnhookWindowsHookEx(hHOOK);
		EndDialog(hDlgAbout, 0);
		return 1;
		break;
	}//switch_Message
	return 0;
}

/*LRESULT CALLBACK KeyBDProc(int nKeyCode, WPARAM wParam, LPARAM lParam){
	if (nKeyCode < 0) return CallNextHookEx(hHOOK, nKeyCode, wParam, lParam);

	switch (wParam){
		case VK_LEFT:
			MessageBox(g_hDlgAbout, TEXT("눌렀당"), 0, MB_OK);
			break;
	}//if_KeyDown
	return CallNextHookEx(hHOOK, nKeyCode, wParam, lParam);
}
*/

//WinMain 함수
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, WndProc);
	return 0;
}