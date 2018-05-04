#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include <string.h>
#include <tchar.h>
#include <mutex>
#include <map>
#include <ctime>
#include <string>
#include <queue>
#include <set>
#include <algorithm>
#include <fstream>
#pragma comment(lib,"Ws2_32.lib")
#define MAXSIZE 65507 //�������ݱ��ĵ���󳤶�
#define HTTP_PORT 80 //http �������˿�

//Http ��Ҫͷ������
struct HttpHeader {
	char method[4]; // POST ���� GET��ע����ЩΪ CONNECT����ʵ���ݲ�����
	char url[1024]; // ����� url
	char host[1024]; // Ŀ������
	char cookie[1024 * 10]; //cookie
	char time_stamp[100];
	HttpHeader() {
		ZeroMemory(this, sizeof(HttpHeader));
	}

};

BOOL InitSocket();
void ParseHttpHead(char *buffer, HttpHeader * httpHeader);
BOOL ConnectToServer(SOCKET *serverSocket, char *host);
unsigned int __stdcall ProxyThread(LPVOID lpParameter);
bool isForbidUrl(char*);
std::pair<std::string, std::string> transUrl(char*);

//������ز���
SOCKET ProxyServer;
sockaddr_in ProxyServerAddr;
const int ProxyPort = 8080;

//�����µ����Ӷ�ʹ�����߳̽��д������̵߳�Ƶ���Ĵ����������ر��˷���Դ
//����ʹ���̳߳ؼ�����߷�����Ч��
//const int ProxyThreadMaxNum = 20;
//HANDLE ProxyThreadHandle[ProxyThreadMaxNum] = {0};
//DWORD ProxyThreadDW[ProxyThreadMaxNum] = {0};
struct ProxyParam {
	SOCKET clientSocket;
	SOCKET serverSocket;
};

int _tmain(int argc, _TCHAR* argv[])
{
	printf("�����������������\n");
	printf("��ʼ��...\n");
	if (!InitSocket()) {
		printf("socket ��ʼ��ʧ��\n");
		return -1;
	}

	printf("����������������У������˿� %d\n", ProxyPort);
	SOCKET acceptSocket = INVALID_SOCKET;
	ProxyParam *lpProxyParam;
	HANDLE hThread;
	DWORD dwThreadID;

	//������������ϼ���
	while (true) {
		acceptSocket = accept(ProxyServer, NULL, NULL);
		lpProxyParam = new ProxyParam;
		if (lpProxyParam == NULL) {
			continue;
		}

		lpProxyParam->clientSocket = acceptSocket;
		hThread = (HANDLE)_beginthreadex(NULL, 0,
			&ProxyThread, (LPVOID)lpProxyParam, 0, 0);
		CloseHandle(hThread);
		Sleep(200);
	}

	closesocket(ProxyServer);
	WSACleanup();
	return 0;
}
//************************************
// Method: InitSocket
// FullName: InitSocket
// Access: public
// Returns: BOOL
// Qualifier: ��ʼ���׽���
//************************************
BOOL InitSocket() {
	//�����׽��ֿ⣨���룩
	WORD wVersionRequested;
	WSADATA wsaData;
	//�׽��ּ���ʱ������ʾ
	int err;
	//�汾 2.2
	wVersionRequested = MAKEWORD(2, 2);
	//���� dll �ļ� Scoket ��
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		//�Ҳ��� winsock.dll
		printf("���� winsock ʧ�ܣ��������Ϊ: %d\n", WSAGetLastError());
		return FALSE;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("�����ҵ���ȷ�� winsock �汾\n");
		WSACleanup();
		return FALSE;
	}
	ProxyServer = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == ProxyServer) {
		printf("�����׽���ʧ�ܣ��������Ϊ��%d\n", WSAGetLastError());
		return FALSE;
	}
	ProxyServerAddr.sin_family = AF_INET;
	ProxyServerAddr.sin_port = htons(ProxyPort);
	ProxyServerAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(ProxyServer, (SOCKADDR*)&ProxyServerAddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		printf("���׽���ʧ��\n");
		return FALSE;
	}
	if (listen(ProxyServer, SOMAXCONN) == SOCKET_ERROR) {
		printf("�����˿�%d ʧ��", ProxyPort);
		return FALSE;
	}
	return TRUE;
}
//************************************
// Method: addCache
//************************************
#define CACHE_SIZE 100
std::mutex cacheMutex;
std::map<std::string, time_t> time_stamp;
std::map<std::string, std::string> cache;
std::set<std::pair<time_t, std::string> > queue;
void __stdcall addCache(const char* request, const char* response) {
	if (cache.count(request) == 1) {
		cache.erase(request);
		queue.erase(std::make_pair(time_stamp[request], request));
		time_stamp.erase(request);
	}
	while ((int)cache.size() > CACHE_SIZE) {
		auto t = *queue.begin();
		queue.erase(t);
		time_stamp.erase(t.second);
		cache.erase(t.second);
	}
	cache[request] = response;
	time_stamp[request] = time(NULL);
	queue.insert(std::make_pair(time_stamp[request], request));
}
//************************************
// Method: ProxyThread
// FullName: ProxyThread
// Access: public
// Returns: unsigned int __stdcall
// Qualifier: �߳�ִ�к���
// Parameter: LPVOID lpParameter
//************************************
unsigned int __stdcall ProxyThread(LPVOID lpParameter) {
	char Buffer[MAXSIZE];
	char *CacheBuffer;
	ZeroMemory(Buffer, MAXSIZE);
	SOCKADDR_IN clientAddr;
	int length = sizeof(SOCKADDR_IN);
	int recvSize;
	int ret;
	HttpHeader* httpHeader;
	int len;
	recvSize = recv(((ProxyParam*)lpParameter)->clientSocket, Buffer, MAXSIZE, 0);
	if (recvSize <= 0) {
		goto error;
	}
	puts("-----------");
	printf("Buffer:\n%s\n\n", Buffer);
	len = strlen(Buffer);
	for (int i = 1; i <= 10; i++) {
		printf("%c %d\n", Buffer[len - i], (int)Buffer[len - i]);
	}
	puts("-----------");
	httpHeader = new HttpHeader();
	CacheBuffer = new char[recvSize + 1];
	ZeroMemory(CacheBuffer, recvSize + 1);
	memcpy(CacheBuffer, Buffer, recvSize);
	ParseHttpHead(CacheBuffer, httpHeader);

	if (isForbidUrl(httpHeader->host)) {
		std::string tmp = "HTTP/1.1 403 Forbidden\r\n\
Date: Sat, 31 Dec 2005 23:59:59 GMT\r\n\
Content-Type: text/html; charset = ISO - 8859 - 1\r\n\
Content-Length: 122\r\n\
\r\n\
<html>\r\n\
<head>\r\n\
<title>Wrong Homepage</title>\r\n\
</head>\r\n\
<body>\r\n\
sorry for that\r\n\
</body>\r\n\
</html>";
		printf("tmp:%s\n", tmp.c_str());
		ret = send(((ProxyParam *)lpParameter)->clientSocket, tmp.c_str(),
			tmp.size() + 1, 0);
		goto error;
	}

	{
		auto p = transUrl(httpHeader->host);
		if (p.first != "") {
			printf("\nfake request:\n%s\n", p.second.c_str());
			std::ifstream file(p.second, std::ifstream::in);
			file.read(Buffer, sizeof(Buffer));
			printf("Fishing Buffer:\n%s\n", Buffer);
			//��Ŀ����������ص�����ֱ��ת�����ͻ���
			ret = send(((ProxyParam*)lpParameter)->clientSocket, Buffer, sizeof(Buffer), 0);
			goto error;
		}
	}

	// ����: �費��Ҫ��[]
	delete CacheBuffer;
	if (!ConnectToServer(&((ProxyParam*)lpParameter)->serverSocket, httpHeader->host)) {
		goto error;
	}
	printf("������������ %s �ɹ�\n", httpHeader->host);
	printf("Cache size:%d\n", cache.size());
	if (cache.count(Buffer)) {
		printf("Found Cache!\n");
		cacheMutex.lock();
		std::string request(Buffer);
		std::string request0(Buffer);
		std::string modified("If-Modified-Since: ");
		char *word = new char[1024];
		tm t;
		gmtime_s(&t, &time_stamp[Buffer]);
		ZeroMemory(word, 1024);
		strftime(word, 1024, "%a, %d %b %Y %X GMT", &t);
		word[strlen(word)] = '\r';
		word[strlen(word)] = '\n';
		modified.append(word);
		request.insert(request.find_first_of("\r\n") + 2, modified.c_str());
		delete word;
		printf("\n---%d---\nNew request after inserted if-modified-since: %s\n\n", request.find_first_of("\r\n"), request.c_str());
		//���ͻ��˷��͵� HTTP ���ݱ���ֱ��ת����Ŀ�������
		ret = send(((ProxyParam *)lpParameter)->serverSocket, request.c_str(), request.size() + 1, 0);
		//�ȴ�Ŀ���������������
		recvSize = recv(((ProxyParam*)lpParameter)->serverSocket, Buffer, MAXSIZE, 0);
		if (recvSize <= 0) {
			goto error;
		}
		std::string response(Buffer);
		printf("----Response----\n%s\n--------\n", response.c_str());
		int pos = response.find("304");
		if (pos != -1 && pos < response.find_first_of("\r\n")) {
			printf("\nNo need to re-request!", pos, response.find_first_of("\r\n"), response[pos], response[pos + 1], response[pos + 2]);
			int t = 0;
			while (response[t] != '\r') {
				putchar(response[t++]);
			}
			putchar('\n');
			ret = send(((ProxyParam*)lpParameter)->clientSocket, cache[request0].c_str(), cache[request0].size() + 1, 0);
		}
		else {
			printf("\nNeed to re-request!\n");
			//��Ŀ����������ص�����ֱ��ת�����ͻ���
			ret = send(((ProxyParam*)lpParameter)->clientSocket, Buffer, sizeof(Buffer), 0);
		}
		cacheMutex.unlock();
	}
	else {
		char *requestBuffer = new char[MAXSIZE + 1];
		ZeroMemory(requestBuffer, sizeof(Buffer));
		memcpy(requestBuffer, Buffer, strlen(Buffer));
		//���ͻ��˷��͵� HTTP ���ݱ���ֱ��ת����Ŀ�������
		ret = send(((ProxyParam *)lpParameter)->serverSocket, Buffer, strlen(Buffer) + 1, 0);
		//�ȴ�Ŀ���������������
		recvSize = recv(((ProxyParam*)lpParameter)->serverSocket, Buffer, MAXSIZE, 0);
		if (recvSize <= 0) {
			goto error;
		}
		cacheMutex.lock();
		addCache(requestBuffer, Buffer);
		// printf("\n-------------\nrequestBuffer:%s\n--------------\n", requestBuffer);
		//��Ŀ����������ص�����ֱ��ת�����ͻ���
		ret = send(((ProxyParam*)lpParameter)->clientSocket, Buffer, sizeof(Buffer), 0);
		cacheMutex.unlock();
	}

	//������
error:
	printf("�ر��׽���\n");
	Sleep(200);
	closesocket(((ProxyParam*)lpParameter)->clientSocket);
	closesocket(((ProxyParam*)lpParameter)->serverSocket);
	delete lpParameter;
	_endthreadex(0);
	return 0;
}
//************************************
// Method: ParseHttpHead
// FullName: ParseHttpHead
// Access: public
// Returns: void
// Qualifier: ���� TCP �����е� HTTP ͷ��
// Parameter: char * buffer
// Parameter: HttpHeader * httpHeader
//************************************
void ParseHttpHead(char *buffer, HttpHeader * httpHeader) {
	char *p;
	char *ptr;
	const char * delim = "\r\n";
	p = strtok_s(buffer, delim, &ptr);//��ȡ��һ��
	if (p[0] == 'G') {//GET ��ʽ
		memcpy(httpHeader->method, "GET", 3);
		memcpy(httpHeader->url, &p[4], strlen(p) - 13);
	}
	else if (p[0] == 'P') {//POST ��ʽ
		memcpy(httpHeader->method, "POST", 4);
		memcpy(httpHeader->url, &p[5], strlen(p) - 14);
	}
	printf("%s\n", httpHeader->url);
	p = strtok_s(NULL, delim, &ptr);
	while (p) {
		switch (p[0]) {
		case 'H'://Host
			memcpy(httpHeader->host, &p[6], strlen(p) - 6);
			break;
		case 'C'://Cookie
			if (strlen(p) > 8) {
				char header[8];
				ZeroMemory(header, sizeof(header));
				memcpy(header, p, 6);
				if (!strcmp(header, "Cookie")) {
					memcpy(httpHeader->cookie, &p[8], strlen(p) - 8);
				}
			}
			break;
		default:
			break;
		}
		p = strtok_s(NULL, delim, &ptr);
	}
}
//************************************
// Method: ConnectToServer
// FullName: ConnectToServer
// Access: public
// Returns: BOOL
// Qualifier: ������������Ŀ��������׽��֣�������
// Parameter: SOCKET * serverSocket
// Parameter: char * host
//************************************
BOOL ConnectToServer(SOCKET *serverSocket, char *host) {
	printf("host:%s\n", host);
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(HTTP_PORT);
	HOSTENT *hostent = gethostbyname(host);
	if (!hostent) {
		return FALSE;
	}
	in_addr Inaddr = *((in_addr*)*hostent->h_addr_list);
	serverAddr.sin_addr.s_addr = inet_addr(inet_ntoa(Inaddr));
	*serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (*serverSocket == INVALID_SOCKET) {
		return FALSE;
	}
	if (connect(*serverSocket, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		closesocket(*serverSocket);
		return FALSE;
	}
	return TRUE;
}



bool isForbidUrl(char *s) {
	std::string t(s);
	static std::vector<std::string> forbidUrl = {
		"jwts.hit.edu.cn"
	};
	for (auto url : forbidUrl) {
		if (t.find(url) == 0) {
			return true;
		}
	}
	return false;
}

std::pair<std::string, std::string> transUrl(char *s) {
	static std::map<std::string, std::pair<std::string, std::string> > urlDict = {
		{ "qioqio.top", std::make_pair("blog.qwertier.cn", "Text.txt") }
	};
	if (urlDict.count(s) != 0) {
		return urlDict[s];
	}
	else {
		return std::make_pair("", "");
	}
}