#pragma once
#include<iostream>
#include<math.h>

class CacheData {
public:
	void* space;

	char ValidBits;
	char Tag;
	//void* Payload;
};

class CacheModel {
public:
	CacheData** cache= NULL;
	int NumRows;
	int Asso;

	int HIT;
	int MISS;

	CacheModel(int associativityParam,int logNumRowsParam,int logBlockSizeParam) {
		//�����ȣ����������С
		NumRows = logNumRowsParam;
		Asso = associativityParam;
		HIT = 0; MISS = 0;

		cache= new CacheData*[logNumRowsParam];
		for (int x = 0; x < logNumRowsParam; x++) {
			cache[x] = new CacheData[associativityParam];
			for (int y = 0; y < associativityParam; y++) {
				cache[x][y].space = malloc(pow(2,logBlockSizeParam)/8);
				cache[x][y].Tag = 0;
				cache[x][y].ValidBits = 0;
			}
		}

	};
	~CacheModel() {};

	void* readReq(int address) {
		char tag = address%256;
		int row = address%NumRows;
		int lessUsedTime = 65535;
		int lessUsed = 0;

		if (!cache[row]) {
			//std::cout << "δ����" << address << std::endl;
			//return NULL;
		}
		else {
			for (int i = 0; i < Asso; i++) {
				if (cache[row][i].Tag == tag && cache[row][i].ValidBits>0) {
					cache[row][i].ValidBits++;
					//std::cout << "����" <<address<<"��������"<<row << std::endl;
					HIT++;
					//return cache[row][i].space;
					return (void*)1;
				}
				if (cache[row][i].ValidBits >= 0 && cache[row][i].ValidBits < lessUsedTime) {
					lessUsedTime = cache[row][i].ValidBits;
					lessUsed = i;
				}
			}
		}
		//std::cout << "δ���У�����" << address << std::endl;
		MISS++;
		//���뻺��
		cache[row][lessUsed].ValidBits = 1;
		cache[row][lessUsed].Tag = tag;
		return NULL;
	};

	void* writeReq(int address) {
		char tag = address % 256;
		int row = address%NumRows;
		int lessUsedTime = 65535;
		int lessUsed = 0;

		if (!cache[row]) {
			
		}
		else {
			for (int i = 0; i < Asso; i++) {
				if (cache[row][i].Tag == tag && cache[row][i].ValidBits>0) {
					cache[row][i].ValidBits++;
					//std::cout << "����" << address << "��д����" << row << std::endl;
					HIT++;
					//�˴�д������
					//return cache[row][i].space;
					return (void*)1;
				}
				if (cache[row][i].ValidBits >= 0 && cache[row][i].ValidBits < lessUsedTime) {
					lessUsedTime = cache[row][i].ValidBits;
					lessUsed = i;
				}
			}
		}
		//std::cout << "δ���У�д��" << address << std::endl;
		MISS++;
		//ֱ��д��ͬʱ���뻺��
		cache[row][lessUsed].ValidBits = 1;
		cache[row][lessUsed].Tag = tag;
		return NULL;
	};
	void dumpResults() {
		std::cout << "HIT:" << HIT << ",MISS:" << MISS << std::endl;
		HIT = 0; MISS = 0;
	};

	void getPhysicalPageNumber();
};

