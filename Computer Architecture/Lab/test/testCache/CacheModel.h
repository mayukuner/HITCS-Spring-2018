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
		//关联度，行数，块大小
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
			//std::cout << "未命中" << address << std::endl;
			//return NULL;
		}
		else {
			for (int i = 0; i < Asso; i++) {
				if (cache[row][i].Tag == tag && cache[row][i].ValidBits>0) {
					cache[row][i].ValidBits++;
					//std::cout << "命中" <<address<<"（读）在"<<row << std::endl;
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
		//std::cout << "未命中（读）" << address << std::endl;
		MISS++;
		//调入缓存
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
					//std::cout << "命中" << address << "（写）在" << row << std::endl;
					HIT++;
					//此处写回主存
					//return cache[row][i].space;
					return (void*)1;
				}
				if (cache[row][i].ValidBits >= 0 && cache[row][i].ValidBits < lessUsedTime) {
					lessUsedTime = cache[row][i].ValidBits;
					lessUsed = i;
				}
			}
		}
		//std::cout << "未命中（写）" << address << std::endl;
		MISS++;
		//直接写，同时调入缓存
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

