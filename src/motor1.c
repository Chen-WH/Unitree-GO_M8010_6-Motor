#define S_FUNCTION_NAME  motor1
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"
#include <windows.h>
#include <stdio.h>

static HANDLE hSerial2;
static DCB dcbSerialParams2 = {0};
static COMMTIMEOUTS timeouts2 = {0};

// 初始化串口2
void InitializeSerialPort2() {
    hSerial2 = CreateFile("COM2", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (hSerial2 == INVALID_HANDLE_VALUE) {
        DWORD dwError = GetLastError();
        char errorMsg[256];
        sprintf(errorMsg, "Error in opening serial port COM2, Error Code: %lu", dwError);
        mexErrMsgTxt(errorMsg);
    }
    
    dcbSerialParams2.DCBlength = sizeof(dcbSerialParams2);
    if (!GetCommState(hSerial2, &dcbSerialParams2)) {
        DWORD dwError = GetLastError();
        char errorMsg[256];
        sprintf(errorMsg, "Error in getting serial port state COM2, Error Code: %lu", dwError);
        mexErrMsgTxt(errorMsg);
    }

    dcbSerialParams2.BaudRate = 4000000; // 4M baud rate
    dcbSerialParams2.ByteSize = 8;
    dcbSerialParams2.StopBits = ONESTOPBIT;
    dcbSerialParams2.Parity = NOPARITY;

    if (!SetCommState(hSerial2, &dcbSerialParams2)) {
        DWORD dwError = GetLastError();
        char errorMsg[256];
        sprintf(errorMsg, "Error in setting serial port state COM2, Error Code: %lu", dwError);
        mexErrMsgTxt(errorMsg);
    }

    // 优化超时设置
    timeouts2.ReadIntervalTimeout = 1;
    timeouts2.ReadTotalTimeoutConstant = 1;
    timeouts2.ReadTotalTimeoutMultiplier = 1;
    timeouts2.WriteTotalTimeoutConstant = 1;
    timeouts2.WriteTotalTimeoutMultiplier = 1;

    if (!SetCommTimeouts(hSerial2, &timeouts2)) {
        DWORD dwError = GetLastError();
        char errorMsg[256];
        sprintf(errorMsg, "Error in setting serial port timeouts COM2, Error Code: %lu", dwError);
        mexErrMsgTxt(errorMsg);
    }
}

// 发送控制命令函数
void SendControlCommand2(unsigned char* command) {
    DWORD bytesWritten;
    if (!WriteFile(hSerial2, command, 17, &bytesWritten, NULL)) {
        DWORD dwError = GetLastError();
        char errorMsg[256];
        sprintf(errorMsg, "Error in writing to serial port COM2, Error Code: %lu", dwError);
        mexErrMsgTxt(errorMsg);
    }
}

// 接收反馈信息函数
void ReceiveFeedback2(unsigned char* feedback) {
    DWORD bytesRead;
    if (!ReadFile(hSerial2, feedback, 16, &bytesRead, NULL)) {
        DWORD dwError = GetLastError();
        char errorMsg[256];
        sprintf(errorMsg, "Error in reading from serial port COM2, Error Code: %lu", dwError);
        mexErrMsgTxt(errorMsg);
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
    InitializeSerialPort2();
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

    SendControlCommand2(controlCommand);

    unsigned char feedback[16];
    ReceiveFeedback2(feedback);

    for (int i = 0; i < 16; i++) {
        y[i] = feedback[i];
    }
}

// S-Function 的终止函数
#define MDL_TERMINATE
#if defined(MDL_TERMINATE)
static void mdlTerminate(SimStruct *S)
{
    CloseHandle(hSerial2);
}
#endif

// 必须的 S-Function 结束标记
#ifdef  MATLAB_MEX_FILE
#include "simulink.c"
#else
#include "cg_sfun.h"
#endif