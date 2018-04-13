#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <dshow.h>
//�򿪱����ļ��Ի���  
#include<Commdlg.h>  
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
int main(){
	/*��������*/
	HWND hwndDOS = GetForegroundWindow();
	ShowWindow(hwndDOS, SW_HIDE);
	//��ȡ���ھ��  
	HWND hWnd = FindWindow(_T("Progman"), _T("Program Manager"));
	if (hWnd == NULL){
		printf("�޷���ȡ������");
		getchar();
		return 0;
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
						printf("�ô����Ѿ����ر�..");
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
	//������һ���Ϳ��Խ������Ƶ����Ƕ�뵽Progman�ര�ڵ����ˣ�Ƕ��֮��Ҳ���ᱻ�ڵ�����Ϊ�ڵ���WorkerW�����Ѿ����ر��ˣ�  
	//��ʼʹ��direct show������Ƶ������direct show���ɵ���ƵǶ�뵽���汳����
	//����directshow��
	IGraphBuilder *pGraph = NULL;	//�м�ý����
	IMediaControl *pControl = NULL;	//������Ƶ����
	IMediaEvent   *pEvent = NULL;	//�¼�֪ͨ��
	IVideoWindow *pVidWin = NULL;	//��������
	// ��ʼ��DirectShow COM��.
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		printf("�����޷���ʼ����ɳ�ʼ��������");
		ShowWindow(hwndDOS, SW_SHOW);
		getchar();
		return -1;
	}
	// �����˲���ͼ�������
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
	if (FAILED(hr))
	{
		printf("���� - �޷����������˲���ͼ�������");
		ShowWindow(hwndDOS, SW_SHOW);
		getchar();
		return -2;
	}
	// ��ѯý����ƺ�ý���¼��ӿ�
	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	hr = pGraph->QueryInterface(IID_IVideoWindow, (void **)&pVidWin);
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
		printf("����û��ѡ����Ƶ�ļ�..");
		ShowWindow(hwndDOS, SW_SHOW);
		getchar();
		return -3;
	}
	//���ò����ļ�·��
	hr = pGraph->RenderFile(strFilename, NULL);
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
			pEvent->WaitForCompletion(INFINITE, &evCode);
			// �м�: ��ʵ��Ӧ�õ���,����ʹ��INFINITE��ʶ, ��Ϊ���᲻ȷ������������
		}
	}
	// �ͷ�������Դ�͹ر�COM��
	pControl->Release();
	pEvent->Release();
	pGraph->Release();
	CoUninitialize();
	return -1;
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