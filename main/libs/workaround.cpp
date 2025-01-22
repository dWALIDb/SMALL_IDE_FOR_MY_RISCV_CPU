#include "workaround.hpp"
#include<windows.h>
#include<thread>
#include<stdio.h>
HANDLE com;
//must be created separately because when i use the split text in raylib 
//text gets overwritten, so can't use multiple text splits :(
void MakeFile(const char* path){
    HANDLE d=CreateFileA(path,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_ALWAYS,0,NULL);
    if (d==INVALID_HANDLE_VALUE)
    {
        return;
    }
    CloseHandle(d);
}

void BrowseFile(char* path,int buff_size){
    OPENFILENAMEA o={sizeof(OPENFILENAME)};
    o.lpstrFilter="\0";
    o.nMaxFile=buff_size;
    o.lpstrTitle="Chose File To Assemble";
    o.lpstrFile=path;
    GetOpenFileNameA(&o);
}

void Serial_port::DestroyPort(){
CloseHandle(com);
}

int Serial_port::SetUpBaud(int baud_rate)
{
        this->ENABLE_READ=false;
        this->ENABLE_WRITE=false;
    DCB d={0};
    d.DCBlength=sizeof(d);
    if(GetCommState(com,&d))
    {
        d.BaudRate=baud_rate;
        d.ByteSize=DATABITS_8;
        d.Parity=NOPARITY;
        d.StopBits=ONESTOPBIT;
    SetCommState(com,&d);
    }else {
        return 1;}
    // ----------------------------------------------------
    // TIMEOUTS FOLLOW THIS FORMULA
    // Timeout = (MULTIPLIER * number_of_bytes) + CONSTANT
    // ----------------------------------------------------
    COMMTIMEOUTS t={0};
    t.ReadIntervalTimeout=10;//TIME BETWEEN RECEPTION OF 2 CHARACTERS IN ms  80  10
    t.ReadTotalTimeoutConstant=10;//100  10
    t.ReadTotalTimeoutMultiplier=2;//10  10
    t.WriteTotalTimeoutConstant=5;//100  10
    t.WriteTotalTimeoutMultiplier=1;//10  10
    SetCommTimeouts(com,&t);
    SetCommMask(com,EV_RXCHAR);
    PurgeComm(com,PURGE_RXCLEAR| PURGE_TXCLEAR);
    this->ENABLE_READ=true;
    this->ENABLE_WRITE=true;
    return 0;
}

int Serial_port::SetUpPort(const char* port_name)
{  
    DestroyPort();
    this->ENABLE_READ=false;
    this->ENABLE_WRITE=false;
com=CreateFileA(port_name,GENERIC_READ|GENERIC_WRITE,0,NULL,
OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

if (com==INVALID_HANDLE_VALUE)
{
    return 1;
}
this->ENABLE_READ=true;
this->ENABLE_WRITE=true;
return 0;
}

Serial_port::Serial_port(){
    this->ENABLE_READ=false;
    this->ENABLE_WRITE=false;
    if (SetUpPort("COM5")==0 && SetUpBaud(CBR_9600)==0)
    {
        this->ENABLE_READ=true;
        this->ENABLE_WRITE=true;
    }
}

Serial_port::~Serial_port(){
    DestroyPort();
}

void Serial_port::ReadPort(){

    DWORD BYTES_READ;
    DWORD d;
    while (!this->STOP_READ)
    {
        if (this->ENABLE_READ)
        {
            ReadFile(com,this->READ_BUF,MAX_READ_BUFFER_SIZE,&BYTES_READ,NULL);
            READ_BUF[BYTES_READ]=0;
            PurgeComm(com,PURGE_RXCLEAR);
        } 
    }
}
void Serial_port::WritePort(){

    DWORD BYTES_WRITE;
    if(this->ENABLE_WRITE)
    {
        WriteFile(com,this->WRITE_BUF,strlen(this->WRITE_BUF),&BYTES_WRITE,NULL);
        PurgeComm(com,PURGE_TXCLEAR);
    }
}