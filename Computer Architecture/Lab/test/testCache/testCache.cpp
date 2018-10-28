#include "CacheModel.h"

#include <time.h>
#include <stdlib.h>

int main(int argc,char* argv[]) {
	CacheModel* Cache;
	if(argc==1)
		Cache = new CacheModel(2, 256, 5);
	else
		Cache=new CacheModel(atoi(argv[1]),atoi(argv[2]),atoi(argv[3]));

	srand((unsigned)time(NULL));
	for (int i = 0; i < 100000; i++) {
		Cache->readReq(rand() % 300);
		Cache->writeReq(rand() % 300);
	}
	Cache->dumpResults();

	return 0;
}
