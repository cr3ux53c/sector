/*
CodeName Sector�� Project
Copyright (c) 2014 by CraXicS��. Some Rights Reserved.
License: �� ���α׷��� �����̸� �Ϲ����� ���� ����Ʈ���� ���̼���(Free Sotfware License)�� �����ϴ�. Windows �ΰ� �� �������� Microsoft Corporation�� ��� ��ǥ�Դϴ�.

�� ���α׷����´����� �ܺ� ���̺귯���� ���ԵǾ� �ֽ��ϴ�.
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
	//String ����
#define	STR_COPYRIGHT			"Copyleft, 2014 by CraXicS��. Some Rights Reversed."
#define	STR_AUTOUNATTEND		TEXT(":\\AutoUnattend.xml")
#define STR_CMDLINE_UNATTEND	TEXT(":\\sources\\setup.exe /unattend:")
#define STR_CMDLINE				TEXT(":\\sources\\setup.exe")
#define	STR_CMDLINE_UNATTEND2	TEXT(":\\sources\\unattend\\")
#define	STR_CMDLINE_UNATTEND3	TEXT(":\\sources\\unattend")
#define STR_RECENV				TEXT(":\\sources\\recovery\\RecEnv.exe")
#define	STR_UNATTENDINFO		"\\Unattend_Info.xml"
#define STR_PATH_CMD			TEXT(":\\Windows\\System32\\cmd.exe")
#define STR_PATH_TASKMGR		TEXT(":\\Windows\\System32\\taskmgr.exe")

//�Լ� ������ ����
TCHAR* GetCurDir(TCHAR strAppend[]);
INT_PTR CALLBACK AboutProc(HWND hDlgAbout, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyBDProc(int nKeyCode, WPARAM wParam, LPARAM lParam);

//��������
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
//�÷��� ����
double		g_dUnattendInfoVer = 1.0;
bool		g_bIsUnattendInfoXML = false;
bool		g_bComboxIdx0 = true;
BOOL		g_bCreProc_StateOfAutoSetup = FALSE;
//bool		g_bFinishWaitOfAutoUnattend = 0;//WM_WINDOWPOSCHANGING�� ���� SW_HIDE ���� �� ��ġ ��� �� SW_SHOW ����.
	//�ڵ� ����
HHOOK		hHOOK;
HINSTANCE	g_hInst;
HWND		g_hWnd;
HWND		g_hComboxFirst = 0;
HWND		g_hComboxUnattend = 0;
//HWND		g_hDlgAbout = 0;

void IdentifyBufXML(const char* pBufXMLFile1, TCHAR Ary2_XMLFile2[][64], const char* pBufDesc) {//pBufXMLFile, g_Ary2_FileData
//	g_nUnattendFiles�� 1�̸� ���� �ϳ��� �ִ� ���̰� �� �� BufXMLFile �迭�� �ε��� �ִ��� 0�̴�.
	int		Idx_XMLIndex = 0;
	bool	bSynced = false;
	int		Idx_XMLIndexOrigin = -1;
	char	Ary1_XMLFile2ANSI[64] = { 0 };
Goto_ReSearch:
	WideCharToMultiByte(CP_ACP, 0, Ary2_XMLFile2[Idx_XMLIndex], -1, Ary1_XMLFile2ANSI, 63, 0, 0);
	if (_stricmp(pBufXMLFile1, Ary1_XMLFile2ANSI) == 0) {//����. //��ҹ��� ���� �� ��
		TCHAR pBufDescWIDE[128] = { 0 };
		MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pBufDesc, -1, pBufDescWIDE, 127);
		for (int i = 0; i <= 127; i++) {
			g_Ary2_Desc[Idx_XMLIndex][i] = pBufDescWIDE[i];
		}
		bSynced = true;
	}
	if (bSynced) {//����ȭ ���� ��
		//�� �Լ��� ������.
	} else {//����ȭ ���� ��: ���� �� ã�� or ã�� �ؽ�Ʈ�� ����->���� ���� Desc�� �ֱ�.

		if (Idx_XMLIndex < g_nUnattendFiles - 1) {
			++Idx_XMLIndex; goto Goto_ReSearch;
		} else {//�� ������ ���ų� �� �̻�. ��, ã�� �ؽ�Ʈ�� ����.
			//�Լ��� ������.
		}
	}

}//IdentifyXML

void TiXMLParseXML(HWND hWnd, TCHAR BufPathUnattend[]) {
	char BufFilePathUnattendANSI[128] = { 0 };//�˻� ���� ANSI
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
		MessageBox(hWnd, TEXT("���� ���� ���� XML�� �׼����� �� �����ϴ�."), TEXT("���� ���� ���� XML �׼��� ����"), MB_OK | MB_ICONERROR);
	} else {
		TiXmlElement *pTiXMLEle = 0;
		pTiXMLEle = TiXMLDoc.FirstChildElement("UnattendInfo");
		if (!pTiXMLEle) {
			MessageBox(hWnd, TEXT("���� ���� ���� XML�� ������ �ùٸ��� �ʽ��ϴ�.\nUnattendInfo ��尡 �����ϴ�."), TEXT("���� ���� ���� XML �׼��� ����"), MB_OK | MB_ICONERROR);
		} else {
			const char* pBufSectorCompati = pTiXMLEle->Attribute("SectorCompatibility");
			double dSectorCompati = (double)atof(pBufSectorCompati);
			if (dSectorCompati != g_dUnattendInfoVer) {
				MessageBox(hWnd, TEXT("���� ���� ���� XML ������ ȣȯ���� �ʽ��ϴ�."), TEXT("���� ���� ���� XML �׼��� ����"), MB_OK | MB_ICONERROR);
			} else {
				pTiXMLEle = 0;
				pTiXMLEle = TiXMLDoc.FirstChildElement("UnattendInfo")->FirstChildElement("Index");
				if (!pTiXMLEle) {
					MessageBox(hWnd, TEXT("���� ���� ���� XML�� ������ �ùٸ��� �ʽ��ϴ�.\nIndex ��尡 �����ϴ�."), TEXT("���� ���� ���� XML �׼��� ����"), MB_OK | MB_ICONERROR);
				} else {
					for (int idx = 0; idx <= g_nUnattendFiles; idx++) {
						//Attribution�� ���� ���� üũ�� ���� ��.
						for (pTiXMLEle; pTiXMLEle; pTiXMLEle = pTiXMLEle->NextSiblingElement()) {//Next ��� �˻� //�� ������ �ܰ��� Next()�Լ��� for���� ���� �� ����ǹǷ� ������ �ڵ尡 �ѹ� �� ����ȴ�.
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
	BOOL bRet = TRUE; //���� ���� ��� ���ϱ� ���� BOOL Ÿ������ ����
	TCHAR BufSearchPath[128] = { 0 };
	for (int i = 0; i < 127; i++) BufSearchPath[i] = SearchRoute[i];
	wcscat(BufSearchPath, TEXT("\\*.xml"));
	hFindFile = FindFirstFile(BufSearchPath, &FileData);
	if (hFindFile == INVALID_HANDLE_VALUE)	MessageBox(hWnd, TEXT("���� �˻�(*.xml)�� �����߽��ϴ�."), TEXT("���� �˻� ����"), MB_OK);
	while (hFindFile != INVALID_HANDLE_VALUE && bRet) {//���� �˻��� ���а� �ƴ� && bRet�� TRUE ��
		if ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {//�� ���� AND ������ ���� 0�̸� �˻��� ��ü�� �����̴�.
			if (_wcsicmp(FileData.cFileName, TEXT("Unattend_Info.xml")) == 0) {//��ҹ��� ���� �� ��
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

//���� �� ���ν���
INT_PTR CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{//���ν��� �Լ� �ȿ����� ����ϴ� ���� ����
	int wmId, wmEvent;
	//GDI �ʱ�ȭ
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
		 //STARTF_USEPOSITION :: dwX, dwY ��ġ ���
		 //STARTF_USESIZE     :: dwXSize, dwYSize ���
		 //Flags ���� �־����� �ʴ� si����ü ���� ���� ��������� ���μ����� ������ ���� �ʽ��ϴ�.
		 //si.dwX = 100;
		 //si.dwY = 100;
		 //si.dwXSize = 300;
		 //si.dwYSize = 300;  //dw ~ �� ��� �� ������ �ʽ��ϴ�.
		 //si.lpTitle = _T(" Child process! ");

	switch (message)
	{
		//if (g_bFinishWaitOfAutoUnattend){
		//	ShowWindow(hWnd, SW_SHOW);
		//	g_bFinishWaitOfAutoUnattend = false;
		//}

	case WM_INITDIALOG: {
		//�ڵ� �Ҵ�
		g_hWnd = hWnd;
		g_hComboxFirst = GetDlgItem(hWnd, IDC_COMBO1);
		g_hComboxUnattend = GetDlgItem(hWnd, IDC_COMBO2);
		//������ ����
		hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		//�ý��� �޴�
		AppendMenu(hSysMenu, MF_SEPARATOR, NULL, NULL); //���м� �߰�
		AppendMenu(hSysMenu, MF_STRING, CTM_SYSCMD, TEXT("��� ������Ʈ"));
		AppendMenu(hSysMenu, MF_STRING, CTM_SYSTASKMGR, TEXT("�۾� ������"));
		AppendMenu(hSysMenu, MF_STRING, CTM_SYSABOUT, TEXT("Sector ������Ʈ ����"));
		//���� ���� Ȯ��
			//AutoUnattend.xml üũ
		TCHAR BufA[64] = { 0 }; BufA[0] = g_pBufThisDir[0];
		wcscat(BufA, STR_AUTOUNATTEND);
		if (PathFileExists(BufA) == TRUE) {//���� ���μ����� ����̺꿡�� �˻�
			g_BufCreProcCmd[0] = g_pBufThisDir[0];
			wcscat(g_BufCreProcCmd, STR_CMDLINE_UNATTEND);
			wcscat(g_BufCreProcCmd, BufA);

			g_bCreProc_StateOfAutoSetup = CreateProcess(
				NULL, //������ ���μ����� �̸�, Ȯ����� �����ؾ��Ѵ�. (NULL�� �ְ� IpCommandLine�� ���� ����) ù��°�� ���޵Ǵ� ���ڸ� ���ؼ� ���������� �̸��� �����ϴ� ��쿡��
				//���� ���͸��� �������� ���������� ã�� �ȴ�. ������ ù��°�� NULL�� �ְ� �ι�° ���ڷ� ���޵Ǵ� ��쿡�� ǥ�� �˻���� ������� ���������� ã�´�.
				g_BufCreProcCmd, //argc / argv[] ���ڷ� ������ ���� �Ű�����. ǥ�� �˻� ��θ� �������� ã�´�.
				//1.���μ����� ���������� �����ϴ� Ǯ��
				//2.�������� ���μ����� ���� Ǯ��
				//3.windows�� �ý��� Ǯ��(system Directory)
				//4.windowsǮ�� (windows directory)
				//5.ȯ�溯�� PATH
				NULL,//���� �Ӽ��� �����ϴ� ����. NULL=�⺻��
				NULL, //�������� ���� �Ӽ��� �����ϴ� ����. NULL=�⺻��
				TRUE, //�θ����μ����� ��Ӱ����� �ڵ� ��� �� ���ΰ�
				0, //���� ��: CREATE_NEW_CONSOLE// ���μ����� Ư���� �������� �ɼ�. ��� ���Ұ�� 0 �Է�
				NULL, //ȯ�溯�� ����. NULL ���޽� �θ�� �ڽĿ��� ȯ�溯�� ����
				NULL,//�����ϴ� ���μ����� ���� ���͸�. NULL�� �ڽ���ġ = �θ���ġ
				&StartUpInfo, //STARTUPINFO ����ü ���� �ʱ�ȭ�� ������ �����͸� �Ű����� �� ����. STARTUPINFO ����ü �������� ���μ����� �Ӽ� ������ �����Ѵ�.
				&ProcInfo  //�����ϴ� ���μ����� ������ ��� ���� �Ű�����. PROCESS_INFORMATION ����ü ������ �ּҰ��� ����
			);

			if (g_bCreProc_StateOfAutoSetup) {

			} else {//���� ����
				MessageBox(hWnd, TEXT("\\sources\\setup.exe�� �������� ���߽��ϴ�."), TEXT("setup.exe ���� ����"), MB_OK | MB_ICONERROR);
			}
		} else {//��� ����̺꿡�� �˻�
			for (char ch = 65; ch <= 90; ch++) {//A~Z ����̺� �� //���߿� �õ� ���͸� ��� ��� ����
				TCHAR BufA[64] = { 0 }; BufA[0] = ch;
				wcscat(BufA, STR_AUTOUNATTEND);
				if (PathFileExists(BufA)) {
					g_BufCreProcCmd[0] = g_pBufThisDir[0];
					wcscat(g_BufCreProcCmd, STR_CMDLINE_UNATTEND);
					wcscat(g_BufCreProcCmd, BufA);
					g_bCreProc_StateOfAutoSetup = CreateProcess(NULL, g_BufCreProcCmd, NULL, NULL, TRUE, NULL, NULL, NULL, &StartUpInfo, &ProcInfo);
					if (g_bCreProc_StateOfAutoSetup) { // ���� ����
					} else {//���� ����
						MessageBox(hWnd, TEXT("\\sources\\setup.exe�� �������� ���߽��ϴ�."), TEXT("setup.exe ���� ����"), MB_OK | MB_ICONERROR);
					}
					break;
				}//if
			}//for
		}//if

			//ISO\sources\Unattend ���͸� ���� Ȯ��
		for (char ch = 65; ch <= 90; ch++) {//����̺� ����
			TCHAR BufPathUnattend[64] = { 0 }; BufPathUnattend[0] = ch;//����̺� ���� �Ҵ�
			wcscat(BufPathUnattend, STR_CMDLINE_UNATTEND3);
			if (PathFileExists(BufPathUnattend)) {//unattend ���� ����
				FindXMLFile(hWnd, BufPathUnattend, g_Ary2_FileData);
				if (g_bIsUnattendInfoXML) {
					TiXMLParseXML(hWnd, BufPathUnattend);
					BufferingFileDataToDesc();
					g_chDrvLtrOfUnattendDir = BufPathUnattend[0];
				}
				break;
			}//if
		}//for

		//�޺��ڽ� ����
			//ComBoxFirst
		strComboxFirst[0] = TEXT("Windows ��ġ");
		ComboBox_AddString(g_hComboxFirst, strComboxFirst[0]);
		ComboBox_SetCurSel(g_hComboxFirst, 0);

		//RecEnv ���� Ȯ��
		TCHAR BufSearchFilePath[32] = { 0 };
		BufSearchFilePath[0] = g_pBufThisDir[0];
		wcscat(BufSearchFilePath, STR_RECENV);
		if (PathFileExists(BufSearchFilePath)) {
			strComboxFirst[1] = TEXT("Windows ���� ���");
			ComboBox_AddString(g_hComboxFirst, strComboxFirst[1]);
		}
		//ComBoxUnattend
		if (g_bIsUnattendInfoXML) {
			ShowWindow(g_hComboxUnattend, SW_SHOW);
			TCHAR* strComboxUnattend[128] = { 0 };
			strComboxUnattend[0] = TEXT("���� ���� ��� �� ��");
			for (int j = 0; j <= g_nUnattendFiles - 1; j++) {
				strComboxUnattend[j + 1] = g_Ary2_Desc[j];
			}
			for (int i = 0; i <= g_nUnattendFiles; i++) {
				ComboBox_AddItemData(g_hComboxUnattend, strComboxUnattend[i]);
			}
			ComboBox_SetCurSel(g_hComboxUnattend, 0);
		}
		//AutoUnattend�� ���� setup.exe ���� ����
		if (g_bCreProc_StateOfAutoSetup) {
			WaitForSingleObject(ProcInfo.hProcess, INFINITE);
			//	g_bFinishWaitOfAutoUnattend = true;
		}

		//WM_CLOSE ����� Ȯ��
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
		switch (LOWORD(wParam)) {////WPARAM�� BN_CLICKED(HI WORD)�� �ش� ��Ʈ���� ID(LOW WORD)�� ����ǰ� LPARAM�� �ش� ��ư ��Ʈ���� �ڵ��� ����� ���·� ���޵˴ϴ�.
		case IDC_COMBO1://�۾� ���� �޺�
			switch (HIWORD(wParam)) {
			case CBN_SELENDOK:
				switch (SendMessage(g_hComboxFirst, CB_GETCURSEL, NULL, NULL)) {
				case 0://Windows ��ġ ����
					if (g_bIsUnattendInfoXML) ShowWindow(g_hComboxUnattend, SW_SHOW);
					g_bComboxIdx0 = true;
					break;
				case 1://Windows ���� ��� ����
					if (g_bIsUnattendInfoXML) ShowWindow(g_hComboxUnattend, SW_HIDE);
					g_bComboxIdx0 = false;
					break;
				}//switch
				InvalidateRect(hWnd, NULL, true);//Ŭ���̾�Ʈ ��ü �ٽ� �׸���
				break;
			}
			break;
		case IDC_COMBO2://���� ���� �޺� �ڽ�
			switch (HIWORD(wParam)) {
			case CBN_SELENDOK:
				g_nComboxUnattendCurIdx = SendMessage(g_hComboxUnattend, CB_GETCURSEL, NULL, NULL);
				break;
			}
			break;
		case IDC_BUTTON1://���� ��ư
			ZeroMemory(g_BufCreProcCmd, sizeof(g_BufCreProcCmd));
			switch (SendMessage(g_hComboxFirst, CB_GETCURSEL, NULL, NULL)) {
			case 0: {//Windows ��ġ
				if (!g_bIsUnattendInfoXML) {//UnattendInfo�� �������� ����
				Goto_ForceNotUnattendSetup:
					g_BufCreProcCmd[0] = g_pBufThisDir[0];
					wcscat(g_BufCreProcCmd, STR_CMDLINE);
					if (CreateProcess(0, g_BufCreProcCmd, 0, 0, TRUE, 0, 0, 0, &StartUpInfo, &ProcInfo)) {
						ShowWindow(hWnd, SW_HIDE);
						WaitForSingleObject(ProcInfo.hProcess, INFINITE);
						ShowWindow(hWnd, SW_SHOW);
					} else {
						MessageBox(hWnd, TEXT("setup.exe�� ������ �� �����ϴ�."), TEXT("setup.exe ���� ����"), MB_OK | MB_ICONERROR);
					}//CreateProcess
				} else {//UnattendInfo�� ���� ��.
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
						MessageBox(hWnd, TEXT("setup.exe�� ������ �� �����ϴ�."), TEXT("setup.exe ���� ����"), MB_OK | MB_ICONERROR);
					}//CreateProcess
				}//g_bIsUnattendInfoXML	
			}//Windows ��ġ
					break;
			case 1://Windows ���� ���
				g_BufCreProcCmd[0] = g_pBufThisDir[0];
				wcscat(g_BufCreProcCmd, STR_RECENV);
				if (CreateProcess(0, g_BufCreProcCmd, 0, 0, TRUE, 0, 0, 0, &StartUpInfo, &ProcInfo)) {
					ShowWindow(hWnd, SW_HIDE);
					WaitForSingleObject(ProcInfo.hProcess, INFINITE);
					ShowWindow(hWnd, SW_SHOW);
				} else {
					MessageBox(hWnd, TEXT("RecEnv.exe�� ������ �� �����ϴ�."), TEXT("RecEnv.exe ���� ����"), MB_OK | MB_ICONERROR);
				}
				break;
			case CB_ERR://����
				MessageBox(hWnd, TEXT("�޺� �ڽ� �ε��� ����"), TEXT("�޺� �ڽ� �ε��� ����"), MB_OK | MB_ICONERROR);
				break;
			}//���� ��ư ����ġ
			break;
		}//��ư ���� ����ġ
		break;
	case WM_SYSCOMMAND:
		wmId = LOWORD(wParam);
		switch (wmId)
		{
		case CTM_SYSABOUT:
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, AboutProc);
			break;
		case CTM_SYSCMD: {//�Լ�ȭ �� ��
			TCHAR BufFindPath[128] = { 0 };
			BufFindPath[0] = g_pBufThisDir[0];
			wcscat(BufFindPath, STR_PATH_CMD);
			if (PathFileExists(BufFindPath)) {
				CreateProcess(BufFindPath, 0, 0, 0, 0, 0, 0, 0, &StartUpInfo, &ProcInfo);
			} else {
				MessageBox(hWnd, TEXT("cmd.exe�� ������ �� �����ϴ�."), TEXT("cmd.exe ���� ����"), MB_OK | MB_ICONERROR);
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
				MessageBox(hWnd, TEXT("taskmgr.exe�� ������ �� �����ϴ�."), TEXT("taskmgr.exe ���� ����"), MB_OK | MB_ICONERROR);
			}
		}
							 break;
		}
		break;
	case WM_WINDOWPOSCHANGING: {
		//if (g_bCreProc_StateOfAutoSetup){//���� ��
		//	WINDOWPOS *WinPos = (WINDOWPOS*)lParam;
		//	WinPos->flags &= ~SWP_SHOWWINDOW;
		//}//���� ���̾�α� �ڽ� ���� �� ����� ���� �ִ� �ڵ忴���� WaitFor �Լ��� ���� ����� ȿ���� ������ �ʿ����. WM_COMMAND �� �ڿ� ShowWindow �Լ��� ȣ���
	}//case_WM_WINDOWPOSCHANGING
							   break;
	case WM_PAINT:
		hDC = BeginPaint(g_hWnd, &ps);
		hCurBMP = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));//BLUE ���
		hMemDC = CreateCompatibleDC(hDC);
		GetObject(hCurBMP, sizeof(BITMAP), &BMP);
		hOldBMP = (HBITMAP)SelectObject(hMemDC, hCurBMP); //�� BMP�� �ް� �⺻ BMP�� OLD�� ���
		BitBlt(hDC, 0, 0, ClientRect.right, ClientRect.bottom, hMemDC, 0, 0, SRCCOPY);

		hCurBMP = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));//win7logo 24.0.82
		GetObject(hCurBMP, sizeof(BITMAP), &BMP);
		SelectObject(hMemDC, hCurBMP);
		BitBlt(hDC, ((ClientRect.right - ClientRect.left) - BMP.bmWidth) / 2, ((ClientRect.bottom - ClientRect.top) - BMP.bmHeight) / 2 - 120, BMP.bmWidth, BMP.bmHeight, hMemDC, 0, 0, SRCCOPY);
		//	TransparentBlt(hDC, ((ClientRect.right - ClientRect.left) - BMP.bmWidth)/2, ((ClientRect.bottom - ClientRect.top) - BMP.bmHeight)/2-120, BMP.bmWidth, BMP.bmHeight, hMemDC, 0, 0, BMP.bmWidth, BMP.bmHeight, RGB(0, 0, 0));

			//�ؽ�Ʈ ���
		hFont = CreateFont(14, 0, 0, 0, 400, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("���� ���"));
		SelectObject(hDC, hFont);
		SetBkMode(hDC, TRANSPARENT);
		SetTextColor(hDC, RGB(255, 255, 255));
		TextOut(hDC, 15, ClientRect.bottom - 25, TEXT(STR_COPYRIGHT), strlen(STR_COPYRIGHT) - 2);
		hFont = CreateFont(20, 0, 0, 0, 500, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("���� ���"));
		SelectObject(hDC, hFont);
		TextOut(hDC, 160, (ClientRect.bottom - ClientRect.top) / 2 - 12, TEXT("�۾� ����: "), lstrlen(TEXT("�۾� ����: ")));
		if (g_bIsUnattendInfoXML && g_bComboxIdx0) TextOut(hDC, 125, (ClientRect.bottom - ClientRect.top) / 2 + 28, TEXT("���� ���� ����: "), lstrlen(TEXT("���� ���� ����: ")));
		//DC ����
		DeleteObject(hCurBMP); DeleteObject(hFont);
		DeleteDC(hMemDC); DeleteDC(hDC);
		EndPaint(g_hWnd, &ps);
		break;
	case WM_CLOSE:
		if (g_bPEMode) {
			if (MessageBox(hWnd, TEXT("Windows �غ� �����Ͻðڽ��ϱ�?\n��ǻ�Ͱ� �ٽ� ���õ˴ϴ�."), TEXT("Sector ����"), MB_ICONEXCLAMATION | MB_YESNO) == IDYES) {
				EndDialog(hWnd, NULL);
				return TRUE;
			}
		} else {
			EndDialog(hWnd, NULL);
			return TRUE;
		}
		break;
	}//�޽��� ����ġ
	return 0;
}//WndProc

INT_PTR CALLBACK AboutProc(HWND hDlgAbout, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message) {
	case WM_INITDIALOG: {
		//	g_hDlgAbout = hDlgAbout;
		//	hHOOK = SetWindowsHookEx(WH_KEYBOARD, KeyBDProc, 0, GetCurrentThreadId());
		HWND hLabelAbout = GetDlgItem(hDlgAbout, IDC_STATIC);
		SetWindowText(hLabelAbout, TEXT("\n   ���̼���(License):\n    �� ���α׷��� �����̸� �Ϲ����� ���� ����Ʈ���� ���̼���(Free Sotfware License)�� �����ϴ�.\n    Windows ������ �� �ΰ�� Microsoft Corporation�� ��� ��ǥ�Դϴ�.\n\n  �� ���α׷����� ������ �ܺ� ���̺귯���� ���ԵǾ� �ֽ��ϴ�.\n    TinyXML 2.6.2\n\n  �� ���α׷��� ������ ��� �� �÷������� ���ߵǾ����ϴ�.\n    Visual C++ 10.0 / Win32 API\n    Microsoft Windows 7 SP1 x64\n    Microsoft Visual Studio 2010 SP1 x86\n\n  ��� ����: http://windowsforum.kr\n"));
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
			MessageBox(g_hDlgAbout, TEXT("������"), 0, MB_OK);
			break;
	}//if_KeyDown
	return CallNextHookEx(hHOOK, nKeyCode, wParam, lParam);
}
*/

//WinMain �Լ�
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, WndProc);
	return 0;
}