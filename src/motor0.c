#define S_FUNCTION_NAME  motor0
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"
#include <windows.h>

static HANDLE hSerial1;
static DCB dcbSerialParams1 = {0};
static COMMTIMEOUTS timeouts1 = {0};

// 初始化串口1
void InitializeSerialPort1() {
    hSerial1 = CreateFile("COM9", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (hSerial1 == INVALID_HANDLE_VALUE) {
        mexErrMsgTxt("Error in opening serial port COM9");
    }
    
    dcbSerialParams1.DCBlength = sizeof(dcbSerialParams1);
    if (!GetCommState(hSerial1, &dcbSerialParams1)) {
        mexErrMsgTxt("Error in getting serial port state COM9");
    }

    dcbSerialParams1.BaudRate = 4000000; // 4M baud rate
    dcbSerialParams1.ByteSize = 8;
    dcbSerialParams1.StopBits = ONESTOPBIT;
    dcbSerialParams1.Parity = NOPARITY;

    if (!SetCommState(hSerial1, &dcbSerialParams1)) {
        mexErrMsgTxt("Error in setting serial port state COM9");
    }

    // 优化超时设置
    timeouts1.ReadIntervalTimeout = 1;
    timeouts1.ReadTotalTimeoutConstant = 1;
    timeouts1.ReadTotalTimeoutMultiplier = 1;
    timeouts1.WriteTotalTimeoutConstant = 1;
    timeouts1.WriteTotalTimeoutMultiplier = 1;

    if (!SetCommTimeouts(hSerial1, &timeouts1)) {
        mexErrMsgTxt("Error in setting serial port timeouts COM9");
    }
}

// 发送控制命令函数
void SendControlCommand1(unsigned char* command) {
    DWORD bytesWritten;
    if (!WriteFile(hSerial1, command, 17, &bytesWritten, NULL)) {
        mexErrMsgTxt("Error in writing to serial port COM9");
    }
}

// 接收反馈信息函数
void ReceiveFeedback1(unsigned char* feedback) {
    DWORD bytesRead;
    if (!ReadFile(hSerial1, feedback, 16, &bytesRead, NULL)) {
        mexErrMsgTxt("Error in reading from serial port COM9");
    }
}

// S-Function 的大小初始化
static void mdlInitializeSizes(SimStruct *S)
{
    ssSetNumSFcnParams(S, 0);
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
        return;
    }

    ssSetNumContStates(S, 0);
    ssSetNumDiscStates(S, 0);

    if (!ssSetNumInputPorts(S, 1)) return;
    ssSetInputPortWidth(S, 0, 17);
    ssSetInputPortDataType(S, 0, SS_UINT8);
    ssSetInputPortDirectFeedThrough(S, 0, 1);

    if (!ssSetNumOutputPorts(S, 1)) return;
    ssSetOutputPortWidth(S, 0, 16);
    ssSetOutputPortDataType(S, 0, SS_UINT8);

    ssSetNumSampleTimes(S, 1);

    ssSetOptions(S, SS_OPTION_WORKS_WITH_CODE_REUSE);
}

// S-Function 的采样时间初始化
static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, 0.001);
    ssSetOffsetTime(S, 0, 0.0);
}

// S-Function 的启动函数
#define MDL_START
#if defined(MDL_START)
static void mdlStart(SimStruct *S)
{
    InitializeSerialPort1();
}
#endif

// S-Function 的输出函数
static void mdlOutputs(SimStruct *S, int_T tid)
{
    InputUInt8PtrsType uPtrs = ssGetInputPortSignalPtrs(S, 0);
    uint8_T *y = (uint8_T *)ssGetOutputPortSignal(S, 0);

    unsigned char controlCommand[17];
    for (int i = 0; i < 17; i++) {
        controlCommand[i] = *uPtrs[i];
    }

    SendControlCommand1(controlCommand);

    unsigned char feedback[16];
    ReceiveFeedback1(feedback);

    for (int i = 0; i < 16; i++) {
        y[i] = feedback[i];
    }
}

// S-Function 的终止函数
#define MDL_TERMINATE
#if defined(MDL_TERMINATE)
static void mdlTerminate(SimStruct *S)
{
    CloseHandle(hSerial1);
}
#endif

// 必须的 S-Function 结束标记
#ifdef  MATLAB_MEX_FILE
#include "simulink.c"
#else
#include "cg_sfun.h"
#endif