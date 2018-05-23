#include <windows.h>
#include "sys.h"
#include "resource.h"
#include <dshow.h>
#include <gdiplus.h>  //GDI模块
//打开保存文件对话框  
#include<Commdlg.h>  
#pragma comment (lib, "Gdiplus.lib")	//加载静态库
extern BOOL CALLBACK  EnumWindowsProc(HWND hwnd, LPARAM lParam);
//声明一个结构体用于存储获取到的所有窗口类名  
#pragma comment(lib,"strmiids.lib")
typedef struct windows_class{
	char window_class_name[256];
	HWND win_hwnd;
	windows_class *next;

}windows_class;
//声明一个全局结构体  
windows_class *class_name;
//记录屏幕窗口类数量  
int num;
//创建directshow类
IGraphBuilder *pGraph = NULL;	//中间媒介类
IMediaControl *pControl = NULL;	//控制视频流类
IMediaEvent   *pEvent = NULL;	//事件通知类
IVideoWindow *pVidWin = NULL;	//窗口流类
using namespace Gdiplus;	//名字空间
GdiplusStartupInput StartupInput;	//GDI成员
ULONG_PTR m_gdiplusToken;	//GDI保护权限数据成员
HWND hWnd;	//progman
//初始化桌面函数
void SetWindowPack(){	//设置桌面
	//获取窗口句柄  
	hWnd = FindWindow("Progman", "Program Manager");
	if (hWnd == NULL){
		MessageBox(hWnd, "无法设置窗口", NULL, NULL);
		return;
	}
	//发送多屏消息  
	SendMessage(hWnd, 0x052c, 0, 0);
	//结构体初始化  
	class_name = (windows_class*)malloc(sizeof(windows_class));
	//枚举屏幕上所有窗口  
	EnumWindows(EnumWindowsProc, 0);
	//循环比对找到->WorkerW类  
	for (int i = 0; i<num; ++i){
		if (strncmp(class_name->window_class_name, "WorkerW", strlen(class_name->window_class_name)) == 0){//以有效字符比对，防止连同字符“0”等无效字符也一同包含在一起比对  
			HWND window_hwnd = FindWindowExA(class_name->win_hwnd, 0, "SHELLDLL_DefView", NULL);
			if (window_hwnd == NULL){   //无法获取句柄代表该workerw类窗口没有子窗口也就是获取到图标下面的WorkerW类窗口了  
				//直接关闭该窗口  
				SendMessage(class_name->win_hwnd, 16, 0, 0);
				break;
			}
			else{
				//获取成功看一下下一个窗口是不是Progman  
				class_name = class_name->next;
				if (class_name->window_class_name == "Progman"){
					HWND window_hwnd = FindWindowExA(class_name->win_hwnd, 0, "WorkerW", NULL);  //获取图标下面的WorkerW子窗口  
					if (window_hwnd == NULL){   //获取不到代表该窗口已经被关闭了  
						MessageBox(hWnd, "无法设置窗口", NULL, NULL);
						break;
					}
					else{   //结束窗口  
						SendMessage(window_hwnd, 16, 0, 0);
					}
				}
				else{//如果不是Progman就代表WorkerW类窗口的屏幕Z序列高于Progman，就说明获取到了WorkerW类窗口，直接关闭即可  
					SendMessage(class_name->win_hwnd, 16, 0, 0);
				}//注意WorkerW类是多屏消息产生的，是从Progman类分割下来的，在屏幕Z序列中会相邻在一起，所以不用担心next下一个类窗口不是Progman或者要删除的图标下面的WorkerW类  

			}
			//break;        //这行代码注释掉，如果上面的代码没有关闭图标下的WorkerW窗口说明不是真正的多屏消息产生的WorkerW窗口，让其继续循环下去，将屏幕所有窗口遍历一遍！  
		}
		class_name = class_name->next;
	}
}
//消息处理函数
void Set_Bk(HWND hwnd){
	HDC hdc = GetDC(hwnd);	//获取窗口绘图DC
	SetBkColor(hdc,RGB(255,255,255));
}	  //设置背景
void HandleEvent()
{
	
		long event_code, param1, param2;
		if (SUCCEEDED(pEvent->GetEvent(&event_code, &param1, &param2, 1)))
		{
			if (event_code == EC_COMPLETE)
			{
				//重新开始播放也可用g_pMediaSeeking的SetPositions来实现
				pControl->Stop();
				pControl->Run();
			}
			pEvent->FreeEventParams(event_code, param1, param2);
		}
	
}
BOOL vi_run = 0;
int IDtimer;
void CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime){
	HandleEvent();
}
//播放视频
void Play(){
	OPENFILENAME ofn = { 0 };
	TCHAR strFilename[MAX_PATH] = { 0 };//用于接收文件名  
	ofn.lStructSize = sizeof(OPENFILENAME);//结构体大小  
	ofn.hwndOwner = NULL;//拥有着窗口句柄，为NULL表示对话框是非模态的，实际应用中一般都要有这个句柄  
	ofn.lpstrFilter = TEXT(".avi\0*.avi\0\0\0");//设置过滤  
	ofn.nFilterIndex = 1;//过滤器索引  
	ofn.lpstrFile = strFilename;//接收返回的文件名，注意第一个字符需要为NULL  
	ofn.nMaxFile = sizeof(strFilename);//缓冲区长度  
	ofn.lpstrInitialDir = NULL;//初始目录为默认  
	ofn.lpstrTitle = TEXT("请选择一个文件");//使用系统默认标题留空即可  
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;//文件、目录必须存在，隐藏只读选项  
	if (GetOpenFileName(&ofn))	//此函数与GDI冲突
	{
		//打开完成相应事件..
	}
	else{
		MessageBox(NULL, "错误，没有选择视频文件..", NULL, NULL);
		return;
	}
	// 初始化DirectShow COM库.
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, "错误，无法初始化视频类库", NULL, NULL);
		return;
	}
	// 创建滤波器图表管理器
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
	if (FAILED(hr))
	{
		MessageBox(NULL, "错误 - 无法创建创建滤波器图表管理器", NULL, NULL);
		return;
	}
	// 查询媒体控制和媒体事件接口
	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	hr = pGraph->QueryInterface(IID_IVideoWindow, (void **)&pVidWin);
	//设置播放文件路径
	//由于编码的问题，所以我们需要将char转换成宽字符
	WCHAR wszClassName[256];
	memset(wszClassName, 0, sizeof(wszClassName));
	MultiByteToWideChar(CP_ACP, 0, strFilename, strlen(strFilename) + 1, wszClassName,
		sizeof(wszClassName) / sizeof(wszClassName[0]));
	//播放
	hr = pGraph->RenderFile(wszClassName, NULL);
	//设置非窗口播放模式
	pVidWin->put_Owner((OAHWND)hWnd);	//已经是分层透明窗口了直接定位到Progman窗口上绘图即可
	pVidWin->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	int nWidth = GetSystemMetrics(SM_CXSCREEN);  //屏幕宽度    
	int nHeight = GetSystemMetrics(SM_CYSCREEN); //屏幕高度
	pVidWin->SetWindowPosition(0, 0, nWidth, nHeight);	// 定位播放子窗口
	//播放
	if (SUCCEEDED(hr))
	{
		// 运行图表.
		hr = pControl->Run();
		vi_run = TRUE;
		if (SUCCEEDED(hr))
		{
			//等待回放结束事件.
			long evCode;
			pEvent->WaitForCompletion(NULL, &evCode);
			// 切记: 在实际应用当中,不能使用INFINITE标识, 因为它会不确定的阻塞程序
		}
	}
	//时钟检测是否播放完
	IDtimer = SetTimer(hWnd, 1, 1000, TimerProc);
	// 释放所有资源和关闭COM库
	pControl->Release();
	pEvent->Release();
	pGraph->Release();
	CoUninitialize();
	return;
}
//设置桌面背景
void Gdi_Set_Bk(HWND hwnd){
	HDC hDC = GetDC(hwnd);
	RECT rctA = { 20, 30, 180, 230 };
	LPRECT  rectDlg = &rctA;
	GetClientRect(hwnd, rectDlg);//获得窗体的大小
	int cxClient = rectDlg->right - rectDlg->left;  // 获得客户区宽度
	int cyClient = rectDlg->bottom - rectDlg->top;  // 获得客户区高度
	Image* pImage = Image::FromFile(L"1.jpg");
	Graphics graphics(hDC);
	graphics.DrawImage(pImage, 0, 0, cxClient, cyClient);
	DeleteDC(hDC);
	graphics.ReleaseHDC(hDC);
}
BOOL CALLBACK  EnumWindowsProc(HWND hwnd, LPARAM lParam)

{
	//声明结构体  
	windows_class *enum_calss_name;
	//初始化  
	enum_calss_name = (windows_class*)malloc(sizeof(windows_class));
	//填充0到类名变量中  
	memset(enum_calss_name->window_class_name, 0, sizeof(enum_calss_name->window_class_name));
	//获取窗口类名  
	GetClassNameA(hwnd, enum_calss_name->window_class_name, sizeof(enum_calss_name->window_class_name));
	//获取窗口句柄  
	enum_calss_name->win_hwnd = hwnd;
	//递增类数量  
	num += 1;
	//链表形式存储  
	enum_calss_name->next = class_name;
	class_name = enum_calss_name;
	return TRUE;//这里必须返回TRUE,返回FALSE就不在枚举了  
}
//消息函数
LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	//判断消息ID
	switch (uMsg){
	case WM_PAINT:
		Gdi_Set_Bk(hwnd);
		break;
		case WM_COMMAND:	//菜单消息
			switch (LOWORD(wParam))
			{
			case ID_001:{	//菜单
							//画
							MessageBox(hwnd, "作者：周志豪\n简言：程序已经在github分布式版本控制系统中开源", "DD", NULL);
							break;
			}
			case ID_002:	//菜单处理事件，打开一个新的视频
				Gdi_Set_Bk(hwnd);
				Play();
				break;
			case ID_003:	//菜单处理事件，停止播放视频
				if (vi_run == FALSE){
					MessageBox(hwnd, "未检测到正在播放的视频，无法执行暂停命令ヾ(❀^ω^)ノﾞ", "DD", NULL);
				}
				else{
					pControl->Stop();
				}
				break;
			case ID_004:	//菜单处理事件，继续播放
				if (vi_run == FALSE){
					MessageBox(hwnd, "未检测到正在播放的视频，无法执行继续播放命令ヾ(❀^ω^)ノﾞ", "DD", NULL);
				}
				else{
					pControl->Run();
				}
				break;
			}
			break;
		       case WM_DESTROY:    // 窗口销毁消息
			           PostQuitMessage(0);   //  发送退出消息
			   return 0;
			 }
	     // 其他的消息调用缺省的消息处理程序
		     return DefWindowProc(hwnd, uMsg, wParam, lParam);
	
}
// 3、注册窗口类型
 BOOL RegisterWindow(LPCSTR lpcWndName, HINSTANCE hInstance)
 {
	      ATOM nAtom = 0;
		  // 构造创建窗口参数
		  WNDCLASS wndClass = { 0 };
		  wndClass.style = CS_HREDRAW | CS_VREDRAW;
	      wndClass.lpfnWndProc = WindowProc;      // 指向窗口过程函数
	      wndClass.cbClsExtra = 0;
	      wndClass.cbWndExtra = 0;
	      wndClass.hInstance = hInstance;
		  wndClass.hIcon = NULL;
	      wndClass.hCursor = NULL;
		  wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		  wndClass.lpszMenuName = (LPCSTR)ID_001;
	      wndClass.lpszClassName = lpcWndName;    // 注册的窗口名称，并非标题，以后创建窗口根据此注册的名称创建
		  nAtom = RegisterClass(&wndClass);
	      return TRUE;
	 }
 //创建窗口（lpClassName 一定是已经注册过的窗口类型）
  HWND CreateMyWindow(LPCTSTR lpClassName, HINSTANCE hInstance)
	  {
	      HWND hWnd = NULL;
	      // 创建窗口
		  hWnd = CreateWindow(lpClassName, "Interesting dynamic video", WS_OVERLAPPEDWINDOW^WS_THICKFRAME, 0, 0, 1000, 800, NULL, NULL, hInstance, NULL);
	      return hWnd;
  }
  //显示窗口
  void DisplayMyWnd(HWND hWnd)
	   {
			//获得屏幕尺寸

		   int scrWidth = GetSystemMetrics(SM_CXSCREEN);
		   int scrHeight = GetSystemMetrics(SM_CYSCREEN);
		   RECT rect;
		   GetWindowRect(hWnd, &rect);
	       ShowWindow(hWnd, SW_SHOW);
		   //重新设置rect里的值
		   rect.left = (scrWidth - rect.right) / 2;
		   rect.top = (scrHeight - rect.bottom) / 2;
		   //移动窗口到指定的位置
		   SetWindowPos(hWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);
	       UpdateWindow(hWnd);
	   }

  void doMessage()        // 消息循环处理函数
	   {
	      MSG msg = { 0 };
	       // 获取消息
		       while (GetMessage(&msg, NULL, 0, 0)) // 当接收到WM_QIUT消息时，GetMessage函数返回0，结束循环
		       {
		           DispatchMessage(&msg); // 派发消息，到WindowPro函数处理
		      }
	   }

 // 入口函数
int WINAPI WinMain(HINSTANCE hInstance,
	                      HINSTANCE hPrevInstance,
						  LPSTR lpCmdLine,
	                      int nShowCmd)
	  {
		  GdiplusStartup(&m_gdiplusToken, &StartupInput, NULL);
	      HWND hWnd = NULL;
	      LPCTSTR lpClassName = "MyWnd";  // 注册窗口的名称
	      RegisterWindow(lpClassName, hInstance);
	      hWnd = CreateMyWindow(lpClassName, hInstance);
		  DisplayMyWnd(hWnd);
		  SetWindowPack();	//初始化桌面
	      doMessage();
		 return 0;
	   }