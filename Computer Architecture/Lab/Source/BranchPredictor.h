#pragma once
#include <iostream>

using namespace std;

class BranchPredictor {
public:
	/*------data------*/	//33KB MAX
	//*
	//*
	//*
	//*
	/*----------------*/

	BranchPredictor();

	bool makePrediction(int address);
	int makeUpdate(int address,bool pred,bool act);
};

BranchPredictor::BranchPredictor() {}

/*
*int address：	预测地址

*RETURN 预测结果
*/
bool BranchPredictor::makePrediction(int address) {
	/*------------------edit-----------------------*/
}


/*
*int address：	预测地址
*bool pred：	prediction，预测结果
*bool act：		actually，实际运行结果
*/
int BranchPredictor::makeUpdate(int address, bool pred, bool act) {	
	/*------------------edit-----------------------*/
	return 0;
}
