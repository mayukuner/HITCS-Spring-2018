#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include <WinSock2.h>
#include <fstream>
#pragma comment(lib,"ws2_32.lib")
#define SERVER_PORT 12340 //�˿ں�
#define SERVER_IP "0.0.0.0" //IP ��ַ
const int BUFFER_LENGTH = 1026; //��������С������̫���� UDP ������֡�а�����ӦС�� 1480 �ֽڣ�
const int SEND_WIND_SIZE = 10;//���ʹ��ڴ�СΪ 10��GBN ��Ӧ���� W + 1 <=N��W Ϊ���ʹ��ڴ�С��N Ϊ���кŸ�����
			      //����ȡ���к� 0...19 �� 20 ��
			      //��������ڴ�С��Ϊ 1����Ϊͣ-��Э��
const int RECV_WIND_SIZE = 10;
const int SEQ_SIZE = 20; //���кŵĸ������� 0~19 ���� 20 ��
			 //���ڷ������ݵ�һ���ֽ����ֵΪ 0�������ݻᷢ��ʧ��
			 //��˽��ն����к�Ϊ 1~20���뷢�Ͷ�һһ��Ӧ
BOOL ack[SEQ_SIZE];//�յ� ack �������Ӧ 0~19 �� ack
int curSeq;//��ǰ���ݰ��� seq
int curAck;//��ǰ�ȴ�ȷ�ϵ� ack
int totalSeq;//�յ��İ�������
int totalPacket;//��Ҫ���͵İ�����
		//************************************
		// Method: getCurTime
		// FullName: getCurTime
		// Access: public
		// Returns: void
		// Qualifier: ��ȡ��ǰϵͳʱ�䣬������� ptime ��
		// Parameter: char * ptime
		//************************************
void getCurTime(char *ptime) {
	char buffer[128];
	memset(buffer, 0, sizeof(buffer));
	time_t c_time;
	struct tm *p = new tm();
	time(&c_time);
	localtime_s(p, &c_time);
	sprintf_s(buffer, "%d/%d/%d %d:%d:%d",
		p->tm_year + 1900,
		p->tm_mon,
		p->tm_mday,
		p->tm_hour,
		p->tm_min,
		p->tm_sec);
	strcpy_s(ptime, sizeof(buffer), buffer);
}
//************************************
// Method: seqIsAvailable
// FullName: seqIsAvailable
// Access: public
// Returns: bool
// Qualifier: ��ǰ���к� curSeq �Ƿ����
//************************************
bool seqIsAvailable() {
	int step;
	step = curSeq - curAck;
	step = step >= 0 ? step : step + SEQ_SIZE;
	//���к��Ƿ��ڵ�ǰ���ʹ���֮��
	if (step >= SEND_WIND_SIZE) {
		return false;
	}
	if (ack[curSeq]) {
		return true;
	}
	return false;
}
//************************************
// Method: timeoutHandler
// FullName: timeoutHandler
// Access: public
// Returns: void
// Qualifier: ��ʱ�ش������������������ڵ�����֡��Ҫ�ش�
//************************************
void timeoutHandler() {
	printf("Timer out error.\n");
	int index;
	for (int i = 0; i< SEND_WIND_SIZE; ++i) {
		index = (i + curAck) % SEQ_SIZE;
		ack[index] = TRUE;
	}
	totalSeq -= SEND_WIND_SIZE;
	curSeq = curAck;
}
//************************************
// Method: ackHandler
// FullName: ackHandler
// Access: public
// Returns: void
// Qualifier: �յ� ack���ۻ�ȷ�ϣ�ȡ����֡�ĵ�һ���ֽ�
//���ڷ�������ʱ����һ���ֽڣ����кţ�Ϊ 0��ASCII��ʱ����ʧ�ܣ���˼�һ�ˣ��˴���Ҫ��һ��ԭ
// Parameter: char c
//************************************
void ackHandler(char c) {
	unsigned char index = (unsigned char)c - 1; //���кż�һ
	printf("Recv a ack of %d\n", index);
	if (curAck <= index) {
		for (int i = curAck; i <= index; ++i) {
			ack[i] = TRUE;
		}
		curAck = (index + 1) % SEQ_SIZE;
	}
	else {
		//ack ���������ֵ���ص��� curAck �����
		for (int i = curAck; i< SEQ_SIZE; ++i) {
			ack[i] = TRUE;
		}
		for (int i = 0; i <= index; ++i) {
			ack[i] = TRUE;
		}
		curAck = index + 1;
	}
}

inline bool inRange(int x, int l, int r) {
    return l <= x && x <= r;
}
void receiveSR(SOCKET &sockServer, SOCKADDR_IN &addrClient) {
    //�����׽���Ϊ������ģʽ
    int iMode = 0; //1����������0������
    ioctlsocket(sockServer, FIONBIO, (u_long FAR*) &iMode);//����������
    char* buffer = new char[BUFFER_LENGTH];
    char** window = new char*[RECV_WIND_SIZE];
    for (int i = 0; i < RECV_WIND_SIZE; i++) {
        window[i] = new char[BUFFER_LENGTH];
        window[i][0] = '\0';
    }
    int length;
    int recvBase = 1, recvSize;
    int stage = 0;
    while (true) {
        recvSize =
            recvfrom(sockServer, buffer, BUFFER_LENGTH, 0, ((SOCKADDR*)&addrClient), &length);
        printf("Data received from the client: %s\n", buffer);
        switch (stage) {
        case 0://�ȴ����ֽ׶�
            unsigned char u_code = (unsigned char)buffer[0];
            if (u_code == 205)
            {
                printf("Ready for file transmission\n");
                buffer[0] = 200;
                buffer[1] = '\0';
                sendto(sockServer, buffer, 2, 0,
                    (SOCKADDR*)&addrClient, sizeof(SOCKADDR));
                stage = 1;
            }
            break;
        case 1://���ݴ���׶�
            unsigned char seq = buffer[0];
            if (seq == 0 && strcmp(buffer + 1, "finished") == 0) {
                printf("Transmission finished!");
                stage = 2;
                goto end;
            }
            printf("recv a packet with a seq of %d\n", seq);
            if (inRange(seq, recvBase - RECV_WIND_SIZE, recvBase + RECV_WIND_SIZE - 1)) {
                sendto(sockServer, buffer, 2, 0,
                    (SOCKADDR*)&addrClient, sizeof(SOCKADDR));
                printf("send an ack of %d\n", (unsigned char)buffer[0]);
            }
            if (inRange(seq, recvBase, recvBase + RECV_WIND_SIZE - 1)) {
                int window_seq = seq % RECV_WIND_SIZE;
                if (window[window_seq][0] == 0) {
                    memcpy(window[window_seq], buffer + 1, BUFFER_LENGTH - 1);
                }
                while (window[recvBase%RECV_WIND_SIZE][0]) {
                    window[recvBase%RECV_WIND_SIZE][0] = 0;
                    printf("Confirmed packet %d: %s\n", recvBase, window[recvBase%RECV_WIND_SIZE]);
                    recvBase++;
                }
            }
        }
	
    }

end:
    //�����׽���Ϊ������ģʽ
    int iMode = 1; //1����������0������
    ioctlsocket(sockServer, FIONBIO, (u_long FAR*) &iMode);//����������

    //�ڴ����
    for (int i = 0; i < RECV_WIND_SIZE; i++)
        delete[] window[i];
    delete[] buffer;
    delete[] window;
}
//������
int main(int argc, char* argv[])
{
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
		printf("WSAStartup failed with error: %d\n", err);
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("Could not find a usable version of Winsock.dll\n");
		WSACleanup();
	}
	else {
		printf("The Winsock 2.2 dll was found okay\n");
	}
	SOCKET sockServer = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//�����׽���Ϊ������ģʽ
	int iMode = 1; //1����������0������
	ioctlsocket(sockServer, FIONBIO, (u_long FAR*) &iMode);//����������
	SOCKADDR_IN addrServer; //��������ַ
				//addrServer.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
	addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//���߾���
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(SERVER_PORT);
	err = bind(sockServer, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
	if (err) {
		err = GetLastError();
		printf("Could not bind the port %d for socket.Error code is %d\n", SERVER_PORT, err);
		WSACleanup();
		return -1;
	}
	SOCKADDR_IN addrClient; //�ͻ��˵�ַ
	int length = sizeof(SOCKADDR);
	char buffer[BUFFER_LENGTH]; //���ݷ��ͽ��ջ�����
	ZeroMemory(buffer, sizeof(buffer));
	//���������ݶ����ڴ�
	std::ifstream icin;
	icin.open("test.txt");
	char data[1024 * 113];
	ZeroMemory(data, sizeof(data));
	icin.read(data, 1024 * 113);
	icin.close();
	totalPacket = 30; //sizeof(data) / 1024;
	printf("Total Packet:%d\n", totalPacket);
	int recvSize;
	for (int i = 0; i < SEQ_SIZE; ++i) {
		ack[i] = TRUE;
	}
	while (true) {
		//���������գ���û���յ����ݣ�����ֵΪ-1
		recvSize =
			recvfrom(sockServer, buffer, BUFFER_LENGTH, 0, ((SOCKADDR*)&addrClient), &length);
		if (recvSize < 0) {
			Sleep(200);
			continue;
		}
		printf("recv from client: %s\n", buffer);
		if (strcmp(buffer, "-time") == 0) {
			getCurTime(buffer);
		}
		else if (strcmp(buffer, "-quit") == 0) {
			strcpy_s(buffer, strlen("Good bye!") + 1, "Good bye!");
		}
		else if (strcmp(buffer, "-testgbn") == 0) {
			//���� gbn ���Խ׶�
			//���� server��server ���� 0 ״̬���� client ���� 205 ״̬�루server���� 1 ״̬��
			//server �ȴ� client �ظ� 200 ״̬�룬����յ���server ���� 2 ״̬������ʼ�����ļ���������ʱ�ȴ�ֱ����ʱ\
			            							//���ļ�����׶Σ�server ���ʹ��ڴ�С��Ϊ
			ZeroMemory(buffer, sizeof(buffer));
			int recvSize;
			int waitCount = 0;
			printf("Begin to test GBN protocol,please don't abort the process\n");
			//������һ�����ֽ׶�
			//���ȷ�������ͻ��˷���һ�� 205 ��С��״̬�루���Լ�����ģ���ʾ������׼�����ˣ����Է�������
			//�ͻ����յ� 205 ֮��ظ�һ�� 200 ��С��״̬�룬��ʾ�ͻ���׼�����ˣ����Խ���������
			//�������յ� 200 ״̬��֮�󣬾Ϳ�ʼʹ�� GBN ����������
			printf("Shake hands stage\n");
			int stage = 0;
			bool runFlag = true;
			while (runFlag) {
				switch (stage) {
				case 0://���� 205 �׶�
					buffer[0] = 205;
					sendto(sockServer, buffer, strlen(buffer) + 1, 0,
						(SOCKADDR*)&addrClient, sizeof(SOCKADDR));
					Sleep(100);
					stage = 1;
					break;
				case 1://�ȴ����� 200 �׶Σ�û���յ��������+1����ʱ������˴Ρ����ӡ����ȴ��ӵ�һ����ʼ
					recvSize =
						recvfrom(sockServer, buffer, BUFFER_LENGTH, 0, ((SOCKADDR*)&addrClient), &length);
					if (recvSize < 0) {
						++waitCount;
						if (waitCount > 20) {
							runFlag = false;
							printf("Timeout error\n");
							break;
						}
						Sleep(500);
						continue;
					}
					else {
						if ((unsigned char)buffer[0] == 200) {
							printf("Begin a file transfer\n");
							printf("File size is %dB, each packet is 1024B and packet total num is %d\n", sizeof(data), totalPacket);
							curSeq = 0;
							curAck = 0;
							totalSeq = 0;
							waitCount = 0;
							stage = 2;
						}
					}
					break;
				case 2://���ݴ���׶�
					if (seqIsAvailable() && totalSeq < totalPacket) {
						//���͸��ͻ��˵����кŴ� 1 ��ʼ
						buffer[0] = curSeq + 1;
						ack[curSeq] = FALSE;
						//���ݷ��͵Ĺ�����Ӧ���ж��Ƿ������
						//Ϊ�򻯹��̴˴���δʵ��
						memcpy(&buffer[1], data + 1024 * totalSeq, 1024);
						printf("send a packet with a seq of %d\n", curSeq);
						sendto(sockServer, buffer, BUFFER_LENGTH, 0,
							(SOCKADDR*)&addrClient, sizeof(SOCKADDR));
						++curSeq;
						curSeq %= SEQ_SIZE;
						++totalSeq;
						Sleep(500);
					}
					//�ȴ� Ack����û���յ����򷵻�ֵΪ-1��������+1
					recvSize =
						recvfrom(sockServer, buffer, BUFFER_LENGTH, 0, ((SOCKADDR*)&addrClient), &length);
					if (recvSize < 0) {
						waitCount++;
						//20 �εȴ� ack ��ʱ�ش�
						if (waitCount > 20)
						{
							timeoutHandler();
							waitCount = 0;
						}
					}
					else {
						//�յ� ack
						ackHandler(buffer[0]);
						if (curAck == curSeq && totalPacket == totalSeq) {
							printf("Transmission finished!\n");
							buffer[0] = 0;
							strcpy(buffer + 1, "finished");
							runFlag = false;
						}
						waitCount = 0;
					}
					Sleep(500);
					break;
				}
			}
		}
        else if (strcmp(buffer, "-testsr") == 0) {
            receiveSR(sockServer, addrClient);
        }
		sendto(sockServer, buffer, BUFFER_LENGTH, 0, (SOCKADDR*)&addrClient,
			sizeof(SOCKADDR));
		Sleep(500);
	}
	//�ر��׽��֣�ж�ؿ�
	closesocket(sockServer);
	WSACleanup();
	return 0;
}
