#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <dshow.h>
//打开保存文件对话框  
#include<Commdlg.h>  
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
int main(){
	/*隐藏自身*/
	HWND hwndDOS = GetForegroundWindow();
	ShowWindow(hwndDOS, SW_HIDE);
	//获取窗口句柄  
	HWND hWnd = FindWindow(_T("Progman"), _T("Program Manager"));
	if (hWnd == NULL){
		printf("无法获取桌面句柄");
		getchar();
		return 0;
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
						printf("该窗口已经被关闭..");
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
	//到了这一步就可以将你的视频窗口嵌入到Progman类窗口当中了，嵌入之后也不会被遮挡，因为遮挡的WorkerW窗口已经被关闭了！  
	//开始使用direct show播放视频，并将direct show生成的视频嵌入到桌面背景层
	//创建directshow类
	IGraphBuilder *pGraph = NULL;	//中间媒介类
	IMediaControl *pControl = NULL;	//控制视频流类
	IMediaEvent   *pEvent = NULL;	//事件通知类
	IVideoWindow *pVidWin = NULL;	//窗口流类
	// 初始化DirectShow COM库.
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		printf("错误：无法初始化完成初始化操作！");
		ShowWindow(hwndDOS, SW_SHOW);
		getchar();
		return -1;
	}
	// 创建滤波器图表管理器
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
	if (FAILED(hr))
	{
		printf("错误 - 无法创建创建滤波器图表管理器");
		ShowWindow(hwndDOS, SW_SHOW);
		getchar();
		return -2;
	}
	// 查询媒体控制和媒体事件接口
	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	hr = pGraph->QueryInterface(IID_IVideoWindow, (void **)&pVidWin);
	//对话框形式获取要播放的文件路径
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
	if (GetOpenFileName(&ofn))
	{
		//打开完成相应事件..
	}
	else{
		printf("错误，没有选择视频文件..");
		ShowWindow(hwndDOS, SW_SHOW);
		getchar();
		return -3;
	}
	//设置播放文件路径
	hr = pGraph->RenderFile(strFilename, NULL);
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
		if (SUCCEEDED(hr))
		{
			//等待回放结束事件.
			long evCode;
			pEvent->WaitForCompletion(INFINITE, &evCode);
			// 切记: 在实际应用当中,不能使用INFINITE标识, 因为它会不确定的阻塞程序
		}
	}
	// 释放所有资源和关闭COM库
	pControl->Release();
	pEvent->Release();
	pGraph->Release();
	CoUninitialize();
	return -1;
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