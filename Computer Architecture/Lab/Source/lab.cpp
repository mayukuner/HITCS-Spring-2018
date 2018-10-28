/*BEGIN_LEGAL
Intel Open Source License

Copyright (c) 2002-2017 Intel Corporation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
#include <iostream>
#include <fstream>
#include "pin.H"
#include "BranchPredictor.h"

ofstream OutFile;

// make it static to help the compiler optimize docount
//static UINT64 icount = 0;
static BranchPredictor* pre=new BranchPredictor();
static UINT32 lastAddress=0;
static UINT32 nextAddress=0;
static BOOL predict=false;
static BOOL FIRST=false;
static INS nextIns;
static INS lastIns;
static UINT32 correct=0;
static UINT32 fail=0;

// This function is called before every instruction is executed
VOID makePre(UINT32 addr) { pre->makePrediction((int)addr); }
VOID makeUp(UINT32 addr,BOOL pred,BOOL act) { pre->makeUpdate((int)addr,(bool)pred,(bool)act); }
VOID docount(BOOL pre,BOOL act){
    if(pre==act)    correct++;
    else fail++;
}


// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    nextAddress=INS_Address(ins);
    nextIns=ins;
    if(FIRST && INS_IsBranch(lastIns)){
        lastAddress=INS_Address(lastIns);
    		//cout<< INS_EffectiveAddressWidth(ins) <<"bits, Last PC:"<<lastAddress<<",";

    		if(INS_IsOriginal(lastIns) && nextAddress==(lastAddress+INS_Size(lastIns))) predict=false;
    		else predict=true;
    		//cout<<"Next PC:"<<nextAddress<<",Predict:"<<predict<<endl;

    		INS_InsertCall(ins,IPOINT_BEFORE,(AFUNPTR)(makePre),IARG_ADDRINT,lastAddress,IARG_END);

    		INS_InsertCall(ins,IPOINT_BEFORE,(AFUNPTR)(makeUp),IARG_ADDRINT,lastAddress,IARG_BOOL,pre->prediction[lastAddress%4000],IARG_BOOL,predict,IARG_END);
            INS_InsertCall(ins,IPOINT_BEFORE,(AFUNPTR)(docount),IARG_BOOL,pre->prediction[lastAddress%4000],IARG_BOOL,predict,IARG_END);


    }
    lastIns=nextIns;
    FIRST=true;
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "lab.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile <<"[" << 100*correct/(correct+fail) <<"\%] Total:" << correct+fail << ", Correct:" << correct <<", Fail:" << fail << endl;
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
