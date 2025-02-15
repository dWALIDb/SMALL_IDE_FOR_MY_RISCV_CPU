#ifndef WORKAROUND__
#define UI__

#define MAX_READ_BUFFER_SIZE 64
#define MAX_WRITE_BUFFER_SIZE 64

//used to create files for read/write in WIN32 or POSIX 
void MakeFile(const char* path);

//OPENS FILE DIALOG
//GetOpenFileName in WIN32
//zenity in POSIX
//only one file can be selected
void BrowseFile(char* path,int buff_size);

//get the device path and the device owner (manufacturer) 
//usufull because open (POSIX) and CreateFile (WIN32) can take path to a device and they open it enabling read/write
//it appends ";" after each device every device path has its specific owner(name) at the same corresponding order
//";" was used as delimitter  because WIN32 API and RAYLIB API use this for their delimitter 
//when listing in dropboxes so this makes it easier
void GetDevices(char* device_paths_buff,char* owner_buff );

//this class opens/reads/writes and closes serial ports 
//POSIX (ttyUSB and ttyACM) are sepported
//WIN32 all devices that are lised by Win32_pnpentity class and have (COM) in their names
//COMMTIMOUTS and HANDLES for WIN32
//TERMIOS STRUCT for POSIX
class Serial_port{
public: 
bool ENABLE_READ=true;
bool ENABLE_WRITE=true;
bool STOP_READ=false;
char reciever_buff[MAX_READ_BUFFER_SIZE]={0};
char transmitter_buff[MAX_WRITE_BUFFER_SIZE]={0};
public :
    Serial_port();
    ~Serial_port();
int SetUpPort(const char* port_name);
int SetUpBaud(int baud_rate);
void ReadPort();
void WritePort();
void DestroyPort();
};

#endif