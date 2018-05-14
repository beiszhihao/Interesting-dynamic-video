#include <windows.h>
#include "sys.h"
#include "resource.h"
#include <dshow.h>
#include <gdiplus.h>  //GDIģ��
//�򿪱����ļ��Ի���  
#include<Commdlg.h>  
#pragma comment (lib, "Gdiplus.lib")	//���ؾ�̬��
extern BOOL CALLBACK  EnumWindowsProc(HWND hwnd, LPARAM lParam);
//����һ���ṹ�����ڴ洢��ȡ�������д�������  
#pragma comment(lib,"strmiids.lib")
typedef struct windows_class{
	char window_class_name[256];
	HWND win_hwnd;
	windows_class *next;

}windows_class;
//����һ��ȫ�ֽṹ��  
windows_class *class_name;
//��¼��Ļ����������  
int num;
//����directshow��
IGraphBuilder *pGraph = NULL;	//�м�ý����
IMediaControl *pControl = NULL;	//������Ƶ����
IMediaEvent   *pEvent = NULL;	//�¼�֪ͨ��
IVideoWindow *pVidWin = NULL;	//��������
using namespace Gdiplus;	//���ֿռ�
GdiplusStartupInput StartupInput;	//GDI��Ա
ULONG_PTR m_gdiplusToken;	//GDI����Ȩ�����ݳ�Ա
HWND hWnd;	//progman
//��ʼ�����溯��
void SetWindowPack(){	//��������
	//��ȡ���ھ��  
	hWnd = FindWindow("Progman", "Program Manager");
	if (hWnd == NULL){
		MessageBox(hWnd, "�޷����ô���", NULL, NULL);
		return;
	}
	//���Ͷ�����Ϣ  
	SendMessage(hWnd, 0x052c, 0, 0);
	//�ṹ���ʼ��  
	class_name = (windows_class*)malloc(sizeof(windows_class));
	//ö����Ļ�����д���  
	EnumWindows(EnumWindowsProc, 0);
	//ѭ���ȶ��ҵ�->WorkerW��  
	for (int i = 0; i<num; ++i){
		if (strncmp(class_name->window_class_name, "WorkerW", strlen(class_name->window_class_name)) == 0){//����Ч�ַ��ȶԣ���ֹ��ͬ�ַ���0������Ч�ַ�Ҳһͬ������һ��ȶ�  
			HWND window_hwnd = FindWindowExA(class_name->win_hwnd, 0, "SHELLDLL_DefView", NULL);
			if (window_hwnd == NULL){   //�޷���ȡ��������workerw�ര��û���Ӵ���Ҳ���ǻ�ȡ��ͼ�������WorkerW�ര����  
				//ֱ�ӹرոô���  
				SendMessage(class_name->win_hwnd, 16, 0, 0);
				break;
			}
			else{
				//��ȡ�ɹ���һ����һ�������ǲ���Progman  
				class_name = class_name->next;
				if (class_name->window_class_name == "Progman"){
					HWND window_hwnd = FindWindowExA(class_name->win_hwnd, 0, "WorkerW", NULL);  //��ȡͼ�������WorkerW�Ӵ���  
					if (window_hwnd == NULL){   //��ȡ��������ô����Ѿ����ر���  
						MessageBox(hWnd, "�޷����ô���", NULL, NULL);
						break;
					}
					else{   //��������  
						SendMessage(window_hwnd, 16, 0, 0);
					}
				}
				else{//�������Progman�ʹ���WorkerW�ര�ڵ���ĻZ���и���Progman����˵����ȡ����WorkerW�ര�ڣ�ֱ�ӹرռ���  
					SendMessage(class_name->win_hwnd, 16, 0, 0);
				}//ע��WorkerW���Ƕ�����Ϣ�����ģ��Ǵ�Progman��ָ������ģ�����ĻZ�����л�������һ�����Բ��õ���next��һ���ര�ڲ���Progman����Ҫɾ����ͼ�������WorkerW��  

			}
			//break;        //���д���ע�͵����������Ĵ���û�йر�ͼ���µ�WorkerW����˵�����������Ķ�����Ϣ������WorkerW���ڣ��������ѭ����ȥ������Ļ���д��ڱ���һ�飡  
		}
		class_name = class_name->next;
	}
}
//��Ϣ������
void Set_Bk(HWND hwnd){
	HDC hdc = GetDC(hwnd);	//��ȡ���ڻ�ͼDC
	SetBkColor(hdc,RGB(255,255,255));
}	  //���ñ���
//������Ƶ
void Play(){
	//�Ի�����ʽ��ȡҪ���ŵ��ļ�·��
	OPENFILENAME ofn = { 0 };
	TCHAR strFilename[MAX_PATH] = { 0 };//���ڽ����ļ���  
	ofn.lStructSize = sizeof(OPENFILENAME);//�ṹ���С  
	ofn.hwndOwner = NULL;//ӵ���Ŵ��ھ����ΪNULL��ʾ�Ի����Ƿ�ģ̬�ģ�ʵ��Ӧ����һ�㶼Ҫ��������  
	ofn.lpstrFilter = TEXT(".avi\0*.avi\0\0\0");//���ù���  
	ofn.nFilterIndex = 1;//����������  
	ofn.lpstrFile = strFilename;//���շ��ص��ļ�����ע���һ���ַ���ҪΪNULL  
	ofn.nMaxFile = sizeof(strFilename);//����������  
	ofn.lpstrInitialDir = NULL;//��ʼĿ¼ΪĬ��  
	ofn.lpstrTitle = TEXT("��ѡ��һ���ļ�");//ʹ��ϵͳĬ�ϱ������ռ���  
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;//�ļ���Ŀ¼������ڣ�����ֻ��ѡ��  
	if (GetOpenFileName(&ofn))
	{
		//�������Ӧ�¼�..
	}
	else{
		MessageBox(NULL, "����û��ѡ����Ƶ�ļ�..", NULL, NULL);
		return;
	}
	// ��ʼ��DirectShow COM��.
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, "�����޷���ʼ����Ƶ���", NULL, NULL);
		return;
	}
	// �����˲���ͼ�������
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
	if (FAILED(hr))
	{
		MessageBox(NULL, "���� - �޷����������˲���ͼ�������", NULL, NULL);
		return;
	}
	// ��ѯý����ƺ�ý���¼��ӿ�
	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	hr = pGraph->QueryInterface(IID_IVideoWindow, (void **)&pVidWin);
	//���ò����ļ�·��
	//���ڱ�������⣬����������Ҫ��charת���ɿ��ַ�
	WCHAR wszClassName[256];
	memset(wszClassName, 0, sizeof(wszClassName));
	MultiByteToWideChar(CP_ACP, 0, strFilename, strlen(strFilename) + 1, wszClassName,
		sizeof(wszClassName) / sizeof(wszClassName[0]));
	//����
	hr = pGraph->RenderFile(wszClassName, NULL);
	//���÷Ǵ��ڲ���ģʽ
	pVidWin->put_Owner((OAHWND)hWnd);	//�Ѿ��Ƿֲ�͸��������ֱ�Ӷ�λ��Progman�����ϻ�ͼ����
	pVidWin->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	int nWidth = GetSystemMetrics(SM_CXSCREEN);  //��Ļ���    
	int nHeight = GetSystemMetrics(SM_CYSCREEN); //��Ļ�߶�
	pVidWin->SetWindowPosition(0, 0, nWidth, nHeight);	// ��λ�����Ӵ���
	//����
	if (SUCCEEDED(hr))
	{
		// ����ͼ��.
		hr = pControl->Run();
		if (SUCCEEDED(hr))
		{
			//�ȴ��طŽ����¼�.
			long evCode;
			pEvent->WaitForCompletion(NULL, &evCode);
			// �м�: ��ʵ��Ӧ�õ���,����ʹ��INFINITE��ʶ, ��Ϊ���᲻ȷ������������
		}
	}
	// �ͷ�������Դ�͹ر�COM��
	pControl->Release();
	pEvent->Release();
	pGraph->Release();
	CoUninitialize();
	return;
}
//�������汳��
void Gdi_Set_Bk(HWND hwnd){
	//GDI+��Դ��ʼ��
	GdiplusStartup(&m_gdiplusToken, &StartupInput, NULL);
	PAINTSTRUCT paintstruct;
	paintstruct.fErase = TRUE;
	HDC hDC = GetDC(hwnd);
	RECT rctA = { 20, 30, 180, 230 };
	LPRECT  rectDlg = &rctA;
	GetClientRect(hwnd, rectDlg);//��ô���Ĵ�С
	int cxClient = rectDlg->right - rectDlg->left;  // ��ÿͻ������
	int cyClient = rectDlg->bottom - rectDlg->top;  // ��ÿͻ����߶�
	Image* pImage = Image::FromFile(L"1.jpg");
	Graphics graphics(hDC);
	Bitmap memBitmap(cxClient, cyClient);
	graphics.DrawImage(pImage, 0, 0, cxClient, cyClient);
	DeleteDC(hDC);
	graphics.ReleaseHDC(hDC);
}
BOOL CALLBACK  EnumWindowsProc(HWND hwnd, LPARAM lParam)

{
	//�����ṹ��  
	windows_class *enum_calss_name;
	//��ʼ��  
	enum_calss_name = (windows_class*)malloc(sizeof(windows_class));
	//���0������������  
	memset(enum_calss_name->window_class_name, 0, sizeof(enum_calss_name->window_class_name));
	//��ȡ��������  
	GetClassNameA(hwnd, enum_calss_name->window_class_name, sizeof(enum_calss_name->window_class_name));
	//��ȡ���ھ��  
	enum_calss_name->win_hwnd = hwnd;
	//����������  
	num += 1;
	//������ʽ�洢  
	enum_calss_name->next = class_name;
	class_name = enum_calss_name;
	return TRUE;//������뷵��TRUE,����FALSE�Ͳ���ö����  
}
//��Ϣ����
LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	//�ж���ϢID
	switch (uMsg){
		case WM_COMMAND:	//�˵���Ϣ
			switch (LOWORD(wParam))
			{
			case ID_001:{	//�˵�
							//��
							MessageBox(hwnd, "���ߣ���־��\n���ԣ������Ѿ���github�ֲ�ʽ�汾����ϵͳ�п�Դ", "DD", NULL);
							break;
			}
			case ID_002:	//�˵������¼�����һ���µ���Ƶ
				Play();
				break;
			}
			break;
		       case WM_DESTROY:    // ����������Ϣ
			           PostQuitMessage(0);   //  �����˳���Ϣ
			   return 0;
			 }
	     // ��������Ϣ����ȱʡ����Ϣ�������
		     return DefWindowProc(hwnd, uMsg, wParam, lParam);
	
}
// 3��ע�ᴰ������
 BOOL RegisterWindow(LPCSTR lpcWndName, HINSTANCE hInstance)
 {
	      ATOM nAtom = 0;
		  // ���촴�����ڲ���
		  WNDCLASS wndClass = { 0 };
		  wndClass.style = CS_HREDRAW | CS_VREDRAW;
	      wndClass.lpfnWndProc = WindowProc;      // ָ�򴰿ڹ��̺���
	      wndClass.cbClsExtra = 0;
	      wndClass.cbWndExtra = 0;
	      wndClass.hInstance = hInstance;
		  wndClass.hIcon = NULL;
	      wndClass.hCursor = NULL;
		 // wndClass.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));;
		  wndClass.lpszMenuName = (LPCSTR)ID_001;
	      wndClass.lpszClassName = lpcWndName;    // ע��Ĵ������ƣ����Ǳ��⣬�Ժ󴴽����ڸ��ݴ�ע������ƴ���
		  nAtom = RegisterClass(&wndClass);
	      return TRUE;
	 }
 //�������ڣ�lpClassName һ�����Ѿ�ע����Ĵ������ͣ�
  HWND CreateMyWindow(LPCTSTR lpClassName, HINSTANCE hInstance)
	  {
	      HWND hWnd = NULL;
	      // ��������
		  hWnd = CreateWindow(lpClassName, "Interesting dynamic video",  WS_OVERLAPPED | WS_SYSMENU, 0, 0, 1000, 800, NULL, NULL, hInstance, NULL);
	      return hWnd;
  }
  //��ʾ����
  void DisplayMyWnd(HWND hWnd)
	   {
			//�����Ļ�ߴ�

		   int scrWidth = GetSystemMetrics(SM_CXSCREEN);
		   int scrHeight = GetSystemMetrics(SM_CYSCREEN);
		   RECT rect;
		   GetWindowRect(hWnd, &rect);
	       ShowWindow(hWnd, SW_SHOW);
		   //��������rect���ֵ
		   rect.left = (scrWidth - rect.right) / 2;
		   rect.top = (scrHeight - rect.bottom) / 2;
		   //�ƶ����ڵ�ָ����λ��
		   Gdi_Set_Bk(hWnd);
		   SetWindowPos(hWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);
	       UpdateWindow(hWnd);

	   }

  void doMessage()        // ��Ϣѭ��������
	   {
	      MSG msg = { 0 };
	       // ��ȡ��Ϣ
		       while (GetMessage(&msg, NULL, 0, 0)) // �����յ�WM_QIUT��Ϣʱ��GetMessage��������0������ѭ��
		       {
		           DispatchMessage(&msg); // �ɷ���Ϣ����WindowPro��������
		      }
	   }

 // ��ں���
int WINAPI WinMain(HINSTANCE hInstance,
	                      HINSTANCE hPrevInstance,
						  LPSTR lpCmdLine,
	                      int nShowCmd)
	  {
	      HWND hWnd = NULL;
	      LPCTSTR lpClassName = "MyWnd";  // ע�ᴰ�ڵ�����
	      RegisterWindow(lpClassName, hInstance);
	      hWnd = CreateMyWindow(lpClassName, hInstance);
		  DisplayMyWnd(hWnd);
		  SetWindowPack();	//��ʼ������
	      doMessage();
		 return 0;
	   }