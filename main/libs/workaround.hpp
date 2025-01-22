#ifndef WORKAROUND__
#define UI__

#define MAX_READ_BUFFER_SIZE 64
#define MAX_WRITE_BUFFER_SIZE 64

void MakeFile(const char* path);

void BrowseFile(char* path,int buff_size);

class Serial_port{
public: 
bool ENABLE_READ=true;
bool ENABLE_WRITE=true;
bool STOP_READ=false;
char READ_BUF[MAX_READ_BUFFER_SIZE]={0};
char WRITE_BUF[MAX_WRITE_BUFFER_SIZE]={0};
public:
    Serial_port();
    ~Serial_port();
int SetUpPort(const char* port_name);
int SetUpBaud(int baud_rate);
void ReadPort();
void WritePort();
void DestroyPort();
};

#endif
