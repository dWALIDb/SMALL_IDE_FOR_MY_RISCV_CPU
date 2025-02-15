#include "workaround.hpp"
//#include <thread>
#include <stdio.h>
#include <string.h>

// #define _WIN32_
// #define _POSIX_

Serial_port::~Serial_port(){
    this->ENABLE_READ=false;
    this->ENABLE_WRITE=false;
    DestroyPort();
}

Serial_port::Serial_port(){
    this->ENABLE_READ=false;
    this->ENABLE_WRITE=false;
    this->STOP_READ=false;
}

#if defined(_WIN32_)
#include<windows.h>
#include<stdio.h>
HANDLE com;

void MakeFile(const char* path){
    HANDLE d=CreateFileA(path,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_ALWAYS,0,NULL);
    if (d==INVALID_HANDLE_VALUE)
    {
        return;
    }
    CloseHandle(d);
}

void GetDevices(char* device_paths_buff,char* owner_buff){
    device_paths_buff[0]=0;
    owner_buff[0]=0;
    char devices[256]={0};
    //we set up a query on cmd
    //this scripts lists serial devices that have (COM) in their names
    FILE* cmd=_popen("wmic path Win32_pnpentity get name |findstr \"COM\"","r");
    if (cmd==NULL)
    {
        return;
    }
    fread(devices,sizeof(char),sizeof(devices),cmd);
    // results are like this 
    // device name (COM*) in each line 
    if(devices[0]==0) return;
    char* line=strtok(devices,"\n");
    while(line!=NULL)
    {
        //sometimes the string returned by read has spaces at the end 
        //thus we stop the string after the ) in the result of our query 
        //format of query is as follows 
        //<MANUFACTURER NAME> (COM*) <SPACES THAT WE ARE DISCARDING>
        line[strlen(line)-strlen(strrchr(line,')')+1)]=0;
        strcat(owner_buff,line);
        //strrchr returns the delimiter too :) 
        line=strrchr(line,'(');
        //discard the first character by starting string at the next char
        line++;
        //discard last char because it is a newline char
        line[strlen(line)-1]=0;
        strcat(device_paths_buff,line);
        line=strtok(NULL,"\n");
        if (line!=NULL)
        {
            strcat(owner_buff,";");
            strcat(device_paths_buff,";");
        }
     
    }
    _pclose(cmd);
}

void BrowseFile(char* path,int buff_size){
    //we set the window name and no flags were set so only one file can be used
    OPENFILENAMEA o={sizeof(OPENFILENAME)};
    o.lpstrFilter="\0";
    o.nMaxFile=buff_size;
    o.lpstrTitle="Chose File To Assemble";
    o.lpstrFile=path;
    
    GetOpenFileNameA(&o);
}

void Serial_port::DestroyPort(){    
    this->ENABLE_READ=false;
    this->ENABLE_WRITE=false;
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
    t.ReadIntervalTimeout=1;//TIME BETWEEN RECEPTION OF 2 CHARACTERS IN ms  80  10
    t.ReadTotalTimeoutConstant=10;//100  10
    t.ReadTotalTimeoutMultiplier=1;//10  10
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
    this->ENABLE_READ=false;
    this->ENABLE_WRITE=false;
    //we open the serial port for read/write 
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



void Serial_port::ReadPort(){

    DWORD BYTES_READ;

    while (!this->STOP_READ)
    {
        if (this->ENABLE_READ)
        {
            ReadFile(com,this->reciever_buff,MAX_READ_BUFFER_SIZE,&BYTES_READ,NULL);
            this->reciever_buff[BYTES_READ]=0;
            PurgeComm(com,PURGE_RXCLEAR);
        } else
        {
            Sleep(250);
            this->reciever_buff[0]=0;
            PurgeComm(com,PURGE_RXCLEAR);
        }
    }
}

void Serial_port::WritePort(){

    DWORD BYTES_WRITE;
    if(this->ENABLE_WRITE)
    {
        WriteFile(com,this->transmitter_buff,strlen(this->transmitter_buff),&BYTES_WRITE,NULL);
        PurgeComm(com,PURGE_TXCLEAR);
    }
}

#elif defined(_POSIX_)
#include<unistd.h>
#include<fcntl.h>
#include<termios.h>

struct termios tty={0};
int com;
//must be created separately because when i use the split text in raylib 
//text gets overwritten, so can't use multiple text splits :(
void MakeFile(const char* path){
    int d=open(path,O_CREAT);
    if (d==-1)
    {
        return;
    }
    close(d);
}

void BrowseFile(char* path,int buff_size){
    FILE* cmd= popen("zenity --file-selection","r");
    
    if(cmd==NULL){return;}
    
    fgets(path,buff_size,cmd);
    int l=strlen(path);
    if (l>0)
    {
        //take away the newline char that fgets appends on end of string
        path[l-1]=0;
    }

    pclose(cmd);
}
// this function resets the operands and generates serial device's paths that are connected
// and gives the owned device's paths according to the 
void GetDevices(char* device_paths_buff,char* owner_buff )
{
    device_paths_buff[0]=0;
    owner_buff[0]=0;
    FILE* usb;
    char ports[256]={0} ;
    char vendor[256]={0};
    char command[256]={0};
    //this one lists the available serial ports typicaly ttyUSB or ttyACM 
    // and for errors it forwards them to /dev/null meaning they wont be printed
    usb=popen("ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null","r");
    if(usb == NULL)   
        return;
    fread(ports,sizeof(char),sizeof(ports),usb);
    if (ports[0]==0)
        return;
    char *line;
    line=strtok(ports,"\n");
    strcat(device_paths_buff,line);
    // strcat(device_paths_buff,";");
    while (line!=NULL)
    {
        // we build the script depending on the ttyUSB or ttyACM
        // that where listed by ls command eariler
        // it gives some more usefull info about the devices
        strcat(command,"udevadm info --name=");
        strcat(command,line);
        // grep gets the vendor name as it was stored in the USB ID in linux kernel
        // and we extract the name from the system data base
        strcat(command," --query=property | grep ID_VENDOR_FROM_DATABASE | cut -d= -f2");
        usb=popen(command,"r");
        if(usb == NULL)   
        {
            device_paths_buff[0]=0;
            owner_buff[0]=0;
            return;
        }

        fread(vendor,sizeof(char),sizeof(vendor),usb);
        strcat(owner_buff,vendor);
        //fread appends the newline char at the end so we get rid of it by setting that last element("\n") to 0 
        owner_buff[strlen(owner_buff)-1]=0;
        // if there are more devices append ; delimitter 
        //reset the command buffer to buid a new script
        command[0]=0;
        // look for the next delimiter
        line=strtok(NULL,"\n");
        if (line!=NULL)
        {
            strcat(device_paths_buff,";");   
            strcat(device_paths_buff,line);   
            //for the owner buffer we want the ; delimiter to be placed
            //but only if there are more strings to process, else no need
            strcat(owner_buff,";");   
        }        
    }

    printf("%s",device_paths_buff);
    printf("%s",owner_buff);

    pclose(usb);

}

void Serial_port::DestroyPort(){
    this->ENABLE_READ=false;
    this->ENABLE_WRITE=false;
    close(com);
}

int Serial_port::SetUpBaud(int baud_rate)
{
    this->ENABLE_READ=false;
    this->ENABLE_WRITE=false;
    int choice;
     switch(baud_rate){
    //these are the fastest baudrates that termois can offer depending on linux kernel
      case 9600: choice=B9600;break;
      case 19200:choice=B19200;break;
      case 38400:choice=B38400;break;
      default:break;
    }

     if(cfsetspeed(&tty,choice)==-1)return 1;
     
     this->ENABLE_READ=true;
     this->ENABLE_WRITE=true;
     return 0;
}

int Serial_port::SetUpPort(const char* port_name)
{  
     this->ENABLE_READ=false;
     this->ENABLE_WRITE=false;
    //makes the device not contoling terminal
     com=open(port_name,O_RDWR|O_NONBLOCK|O_NOCTTY); 
    //no input/output processing 
     tty.c_iflag=0;
     tty.c_oflag=0;
    //we want no parity 8 data bits non canonical mode , no modem control, 1 stop bit
     tty.c_cflag &= ~(CSIZE|CSTOPB|PARENB|ICANON);
     tty.c_cflag |= (CS8|CLOCAL|CREAD);
    //since we have non canonical mode we have to setup timing VMIN for minimum number of bytes and VTIME for timeouts
     tty.c_cc[VTIME]=0;//1 tenth of a second is largest delay between to consicultive bytes
     tty.c_cc[VMIN]=MAX_READ_BUFFER_SIZE;
    //set attributes immediately
     if(tcsetattr(com,TCSANOW,&tty)==-1) return 1;
    
     tcflush(com,TCIOFLUSH);
     this->ENABLE_READ=true;
     this->ENABLE_WRITE=true;
    //  printf("created: %d",com);
return 0;
}

void Serial_port::ReadPort(){

    int BYTES_READ;
    while (!this->STOP_READ)
    {
        if (this->ENABLE_READ)
        {
            usleep(250*1000);
            BYTES_READ=read(com,this->reciever_buff,MAX_READ_BUFFER_SIZE);
            if (BYTES_READ>-1)
            {
                this->reciever_buff[BYTES_READ]=0;
                tcflush(com,TCIFLUSH);
            }
            
        } else
        {
            usleep(500*1000);
            //this->reciever_buff[0]=0;
            //tcflush(com,TCIFLUSH);
        }
    }
}

void Serial_port::WritePort(){

    int BYTES_WRITE;
    if(this->ENABLE_WRITE)
    {
	    BYTES_WRITE=write(com,this->transmitter_buff,strlen(this->transmitter_buff));
        if(BYTES_WRITE>-1) tcflush(com,TCOFLUSH);
    }
}



#endif