// g++ main.cpp -lraylib -lopengl32 -lgdi32 -mwindows -lwinmm -o main.exe

/*-------------------------------------------------------------------------------
RAYLIB AND WINDOWS API HAVE NAMING CONFLICTS SO WE CAN'T INCLUDE THEM IN THE SAME 
FILE THUS WE USE THE WIN32 API IN A DIFFERENT HEADER AND INCLUDE THAT HEADER IN THE 
RAYLIB FILE WE CAN'T USE NAME SPACES BECAUSE RAYLIB IS WRITTEN IN C :(
-------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <thread>

#include "libs\src\raylib.h"
#ifndef RAYGUI_IMPLEMENTATION
    #define RAYGUI_IMPLEMENTATION
#endif
#include ".\libs\src\raygui.h"

#include".\libs\workaround.hpp"

#include".\libs\assembler_class.hpp"

int main()
{
const char* serial_label=" SERIAL MONITOR ";
const char* serial_send=" SEND ";
const char* assembler_label=" ASSEMBLER ";

// only 6 ports available and 4 baud rates and 4 mif mem sizes
// this char* controls that
const char* ports="COM1;COM2;COM3;COM4;COM5;COM6";
const char* baud_rates="9600;19200;115200;256000";
const char* memsize="64;128;256;512;1024";

const int FONT_SIZE=25;
// booleans to control the serial monitor/assembler
bool autoscroll_enable=false;
bool settings=false;
bool functionality=false;
bool enable_line_feed=false;
// buffers that hold file paths,data that is recieved in buffer
char* DATA;
size_t DATA_size=64;
char input_file_path[256]={0};
char output_file_path[256]={0};
// variables for scroll panel of serial monitor
// and for indeces of drop down boxes
Vector2 scroll_offset;
Rectangle view_offset;
const int icon=ICON_TOOLS;
int baudrate_scroll_index=0;
int port_scroll_index=0;
int memsize_index=0;
bool baudrate_active_dropdown=0;
bool port_active_dropdown=0;
bool memsize_active_dropdown=0;

// dynamic memory alocation for the recieving buffer to get as much data
DATA=(char*)calloc(DATA_size,sizeof(char));
if (DATA==NULL)
{
    // can't continue if we can't allocate the buffer
    return 1;
}
DATA[DATA_size]=0;
    
    // assembler and serial port classes that handle the internal logic
    assembler asmblr;
    Serial_port port;
    // initialize indeces and the port to start 
    baudrate_scroll_index=0;
    port_scroll_index=4;
    int com_handle=0;
    com_handle=port.SetUpPort("COM5");
    
    if (com_handle==0)
    {
        port.SetUpBaud(9600);
    }
    // do the reading in a separate thread
    std::thread worker(&Serial_port::ReadPort,&port);
// ------------------------------------------------------------------------------
    InitWindow(900,600,"RISC V IDE");
    Image rayicon=LoadImage("pic.png");
    SetWindowIcon(rayicon);
    Font font_used=LoadFontEx("louis_george_cafe\\Louis George Cafe.ttf",FONT_SIZE,NULL,0);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    
    SetTargetFPS(30);
    
    GuiSetStyle(DEFAULT,TEXT_SIZE,FONT_SIZE);
    GuiSetFont(font_used);
    GuiSetStyle(DEFAULT,TEXT_LINE_SPACING,FONT_SIZE);
    
while (!WindowShouldClose())
{   
    //resize array if the length is not enough
    if(strlen(DATA)+strlen(port.READ_BUF)>DATA_size-1){
        DATA_size=DATA_size*2;
        DATA=(char*)realloc(DATA,DATA_size*sizeof(char));
        if (DATA==NULL)
        {
            break;
        }
    }else if(port.READ_BUF[0]!=0)
    {
        // read the data
        strcat(DATA,port.READ_BUF);
        port.READ_BUF[0]=0;
    }

    // getting the clipboard used to paste the path into the textboxes
    if (CheckCollisionPointRec(GetMousePosition(),Rectangle{GetScreenWidth()/2.0f-300,GetScreenHeight()-40.f,600,30}) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V))
    {
        strcat_s(port.WRITE_BUF,sizeof(port.WRITE_BUF),GetClipboardText());
    }
    if (CheckCollisionPointRec(GetMousePosition(),Rectangle{GetScreenWidth()/4.0f,150+FONT_SIZE+5,400.0f,FONT_SIZE}) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V))
    {
        strcat_s(input_file_path,sizeof(input_file_path),GetClipboardText());
    }
    if (CheckCollisionPointRec(GetMousePosition(),Rectangle{GetScreenWidth()/4.0f,220+FONT_SIZE+5,400.0f,FONT_SIZE}) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V))
    {
        strcat_s(output_file_path,sizeof(output_file_path),GetClipboardText());
    }
    

    BeginDrawing();
    ClearBackground(LIGHTGRAY);
    // control what we use assembler or serial port
    if (GuiLabelButton(Rectangle {(float)GetScreenWidth()-MeasureText(assembler_label,FONT_SIZE),10,(float)MeasureText(assembler_label,FONT_SIZE),20},assembler_label)) functionality=false;
    
    if (GuiLabelButton(Rectangle {10,10,(float)MeasureText(serial_label,FONT_SIZE),20},serial_label)) functionality=true;
    
    // SERIAL MONITOR FUNCTIONALITY
    if(functionality)
    {
        // this enables settings
        GuiSetIconScale(3);
        if(GuiLabelButton(Rectangle{10,45,48,48},"#140#") ||(IsKeyDown(KEY_LEFT_CONTROL)&&IsKeyPressed(KEY_M))) settings=!settings;
        GuiSetIconScale(1);
        // port status 
        switch (com_handle){  
        case 1 : DrawTextEx(font_used,"Disconnected",Vector2{GetScreenWidth()/2.0f-150,40},25,1.00f,RED);break;
        case 0 : DrawTextEx(font_used,"Connected Successfully",{GetScreenWidth()/2.0f-150,40},25,1.00f,DARKGREEN);break;
        default: break;
        }     
    
    GuiScrollPanel(Rectangle{10.0f,100.0f,(float)GetScreenWidth()-10.0f-10.0f,GetScreenHeight()-150.0f}, NULL,Rectangle{10.0f,100.0f,(float)MeasureTextEx(font_used,DATA,FONT_SIZE,1.0f).x+FONT_SIZE,(float)MeasureTextEx(font_used,DATA,FONT_SIZE,1.0f).y},&scroll_offset,&view_offset);
    // scroll panel's content  depends on the length and width of text
    // but it is clipped at a specific size 
    BeginScissorMode(10.0f,100.0f,(float)GetScreenWidth()-10.0f-10.0f,GetScreenHeight()-150.0f);
        DrawTextEx(font_used,DATA,Vector2{view_offset.x+scroll_offset.x+5,view_offset.y+scroll_offset.y},FONT_SIZE,1.0f,BLACK);
    EndScissorMode();

    //auto scroll_offset
    if (autoscroll_enable) scroll_offset.y=-MeasureTextEx(font_used,DATA,FONT_SIZE,1.0f).y;

    if (GuiTextBox(Rectangle{GetScreenWidth()/5.0f,GetScreenHeight()-10.f-FONT_SIZE,3*GetScreenWidth()/5.0f,FONT_SIZE},port.WRITE_BUF,MAX_WRITE_BUFFER_SIZE-1,CheckCollisionPointRec(GetMousePosition(),Rectangle{GetScreenWidth()/2.0f-300,GetScreenHeight()-40.f,600,30}))
    || GuiButton(Rectangle{4*GetScreenWidth()/5.0f,GetScreenHeight()-FONT_SIZE-10.0f,(float)MeasureText(serial_send,FONT_SIZE),FONT_SIZE},serial_send))
    {
        if(enable_line_feed) strcat(port.WRITE_BUF,"\n");
        // write to serial port
        port.WritePort();
        //reset the buffer for new input
        port.WRITE_BUF[0]=0;
    }
    if (GuiButton(Rectangle{30,GetScreenHeight()-FONT_SIZE-10.0f,100,FONT_SIZE},"Clear"))
    {
        // clearbuffer if user wants that :)
        DATA[0]=0;
    }
    
    }

    // SETTINGS FOR COMMUNICATION BAUDRATE,COMPORT ...
    if (settings && functionality)
    {
        GuiPanel(Rectangle{60,45,300,350},"SETTINGS");
        
        if(port_active_dropdown || baudrate_active_dropdown) GuiLock();
        // we can append new line automaticaly when sending/auto scrol/enabling comunication
        GuiCheckBox(Rectangle{80,261,FONT_SIZE,FONT_SIZE},"APPEND LINE FEED",&enable_line_feed);
        GuiCheckBox(Rectangle{80,230,FONT_SIZE,FONT_SIZE},"AUTOSCROLL",&autoscroll_enable);
        GuiCheckBox(Rectangle{80,291,FONT_SIZE,FONT_SIZE},"ENABLE WRITE",&port.ENABLE_WRITE);
        GuiCheckBox(Rectangle{80,321,FONT_SIZE,FONT_SIZE},"ENABLE READ",&port.ENABLE_READ);
        //show the buffer size in case it matters for user
        DrawTextEx(font_used,TextFormat("BUFFER SIZE: %d/%d",strlen(DATA),DATA_size),Vector2{80,350},FONT_SIZE,1,BLACK);

        if (GuiButton(Rectangle{80,180,250,FONT_SIZE},"VALIDATE PORT") && !baudrate_active_dropdown && !port_active_dropdown)
        {  
            //set the com port that the user sets up  
            if (com_handle==0)
            {
                port.DestroyPort();
            }
            int a=0;
            int b=0;
            const char** chosen_port=TextSplit(ports,';',&b);
            com_handle=port.SetUpPort(chosen_port[port_scroll_index]);
            const char** chosen_baud_rate=TextSplit(baud_rates,';',&a);
            port.SetUpBaud(TextToInteger(chosen_baud_rate[baudrate_scroll_index]));            
        }

        if(port_active_dropdown || baudrate_active_dropdown) GuiUnlock();
        // GuiDropDown must be last drown to cover other parameters that it may be dropped on
        GuiLabel(Rectangle {75,125,140,(float)font_used.baseSize+1},"COMPORT");
        GuiLabel(Rectangle {75,85,140,(float)font_used.baseSize+1},"BAUDRATE");
        
        
        if(baudrate_active_dropdown) GuiLock();
        if(GuiDropdownBox(Rectangle{190,125,140,(float)font_used.baseSize+1},ports,&port_scroll_index,port_active_dropdown)) {port_active_dropdown=!port_active_dropdown;baudrate_active_dropdown=false;}
        if(baudrate_active_dropdown) GuiUnlock();

        if(port_active_dropdown) GuiLock();
        if(GuiDropdownBox(Rectangle{190,85,140,(float)font_used.baseSize+1},baud_rates,&baudrate_scroll_index,baudrate_active_dropdown)) {baudrate_active_dropdown=!baudrate_active_dropdown;port_active_dropdown=false;}
        if(port_active_dropdown) GuiUnlock();
            
    }
    

    // assembler UI

    if(!functionality)
    {
        if (!asmblr.valid_code || !asmblr.valid_generation)
            DrawTextEx(font_used,asmblr.error_message.c_str(),Vector2{GetScreenWidth()/4.0f,500},FONT_SIZE,1.0f,RED);
        else 
            DrawTextEx(font_used,asmblr.error_message.c_str(),Vector2{GetScreenWidth()/4.0f,500},FONT_SIZE,1.0f,DARKGREEN);
        
        DrawTextEx(font_used,"Input File Path:",Vector2{GetScreenWidth()/4.0f,150},FONT_SIZE,1.0f,BLUE);
        GuiTextBox(Rectangle{GetScreenWidth()/4.0f,150+FONT_SIZE+5,400.0f,FONT_SIZE},input_file_path,256,CheckCollisionPointRec(GetMousePosition(),Rectangle{GetScreenWidth()/4.0f,150+FONT_SIZE+5,400.0f,FONT_SIZE}));
        if (GuiLabelButton(Rectangle{GetScreenWidth()/4.0f+405,150+FONT_SIZE+5,50,FONT_SIZE},"Browse"))
        {
            BrowseFile(input_file_path,256);
        }

        DrawTextEx(font_used,"Output File Path:",Vector2{GetScreenWidth()/4.0f,220},FONT_SIZE,1.0f,BLUE);
        GuiTextBox(Rectangle{GetScreenWidth()/4.0f,220+FONT_SIZE+5,400.0f,FONT_SIZE},output_file_path,256,CheckCollisionPointRec(GetMousePosition(),Rectangle{GetScreenWidth()/4.0f,220+FONT_SIZE+5,400.0f,FONT_SIZE}));
        if (GuiLabelButton(Rectangle{GetScreenWidth()/4.0f+405,220+FONT_SIZE+5,50,FONT_SIZE},"Browse"))
        {
            BrowseFile(output_file_path,256);
        }
    
    if (memsize_active_dropdown) GuiLock();

        if(GuiButton(Rectangle{GetScreenWidth()/4.0f,350,400,FONT_SIZE},"ASSEMBLE"))
        {    
            try
            {
                // input /output files must exist
                asmblr.assemble(input_file_path,output_file_path);
                if(asmblr.valid_code)
                asmblr.error_message="TERMINATED SUCCESSFULLY CONGRATULATIONS";
            }
            catch(...)
            {
                asmblr.error_message="ERROR AT LINE "+std::to_string(asmblr.Get_program_counter()/4+1);
                asmblr.valid_code=false;
            }
            
        }
        if(GuiButton(Rectangle{GetScreenWidth()/4.0f,390,400,FONT_SIZE},"GENERATE MIF"))
        {
            // Input is output of assembler and mif is in same directory
            char new_paths[4][256]={0};
            const char* working_path=GetDirectoryPath(output_file_path);
            try
            {
                for (int i = 0; i < 4; i++)
                {
                    strcpy(new_paths[i],working_path);
                    strcat(new_paths[i],"\\");
                    strcat(new_paths[i],GetFileNameWithoutExt(output_file_path));
                    strcat(new_paths[i],"_MIF");
                    strcat(new_paths[i],std::to_string(i).c_str());
                    strcat(new_paths[i],".mif");

                    if (!FileExists(new_paths[i]))
                    {
                        MakeFile(new_paths[i]);
                    }
                }
                int a=0;
                asmblr.generate_bytesFor_32bits(std::string(output_file_path),std::string(new_paths[0]),std::string(new_paths[1]),std::string(new_paths[2]),std::string(new_paths[3]),TextToInteger(TextSplit(memsize,';',&a)[memsize_index]));
                if(asmblr.valid_generation)
                asmblr.error_message="TERMINATED SUCCESSFULLY CONGRATULATIONS";
            }
            catch(...)
            {
                asmblr.valid_generation=false;
                asmblr.error_message="FAILED TO GENERATE MIF FILES";
            }
        }
    if (memsize_active_dropdown) GuiUnlock();    

    DrawTextEx(font_used,"Number of Adresses:",Vector2{GetScreenWidth()/4.0f,290},FONT_SIZE,1.0f,BLUE);
    if (GuiDropdownBox(Rectangle{GetScreenWidth()/4.0f+300,290,100,FONT_SIZE},memsize,&memsize_index,memsize_active_dropdown)) memsize_active_dropdown=!memsize_active_dropdown;

    }

    //debug garbage ;)
    // DrawText(TextFormat("sx:%.2f sy:%.2f",scroll_offset.x,scroll_offset.y),10+500,220,20,BLACK);
    // DrawText(TextFormat("vx:%.2f vy:%f",view_offset.x,view_offset.y),10+500,200,20,BLACK);
    // DrawText(TextFormat("vw:%.2f vh:%.2f",view_offset.width,view_offset.height),10+500,240,20,BLACK);
    // DrawText(TextFod dataLEN:rlen(DATA)),10+500,280,20,BLACK);
    // DrawText(TextFormat("index:%d ACTIVE:%d foc:%d" ,baudrate_scroll_index,baudrate_active_dropdown),10+500,320,20,BLACK);
    // DrawText(TextFormat("handle:%d" ,com_handle),10+500,320,20,BLACK);

    EndDrawing();
    
}
port.STOP_READ=true;
worker.join();

if (DATA!=NULL)
{
    free(DATA);
}
port.DestroyPort();
UnloadFont(font_used);
UnloadImage(rayicon);
CloseWindow();
return 0;
}
