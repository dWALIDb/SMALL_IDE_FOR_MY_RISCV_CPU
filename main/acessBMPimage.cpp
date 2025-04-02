// #include<iostream>
// #include<fstream>
#include<stdio.h>
#include<string.h>

int main(){
    FILE* fd=fopen("C:\\Users\\DELL\\Desktop\\learn\\c++\\expected.bmp","rb");
    unsigned char a[54]={0};
    fread(a,sizeof(char),54,fd);
    for(int i=0;i<54;i++)
    printf("%c\n",a[i]);
    //we cast the value stored at a[22],a[22],a[23],a[25] as an integer 
    //int is 4 bytes and char is a byte so we do conversion for address  a[22] till a[25]
    // this is a full int(4 bytes)
    //basically we converted 4 bytes starting from address of a[22] to integer little endian
    const int height=*(int *)&a[22];
    const int width=*(int *)&a[18];
    int size=*(int *)&a[34];
    printf("\nw:%d h:%d\n",width,height);
    printf("\nsize:%d\n",size);
    printf("\na[22]:%p,a[23]:%p,a[24]:%p,a[25]:%p\n",&a[22],&a[23],&a[24],&a[25]);
    printf("\na[22]:%x,a[23]:%x,[24]:%x,a[25]:%x\n",a[22],a[23],a[24],a[25]);
    printf("\n***************************************************************\n");
    
    FILE* fdw=fopen("C:\\Users\\DELL\\Desktop\\learn\\c++\\expected_vals.txt","wb");
    if (fdw==NULL)
    {
        return -1;
    }

    unsigned char read_pixels[3]={0};
    unsigned char pixel_data[height*width]={0};
    
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j <width ; j++)
        {
            //we dont need all the channels since we are working with gray scale
            //so we read the first channels of each pixel
            //pixels are stored from bottom left in this file format
            //this is taken care of by assigning each value to its corresponding cell
            fread(read_pixels,1,3,fd);
            pixel_data[32*(height-1-i)+(j)]=read_pixels[0];
        }
    }
    // the data is set up in a way such that it has 4 bytes in each line 
    // no spaces between the bytes 
    // the new line char must be delt with when reading the values because fread gets 
    // the whole line with the '\n' :)
    for (int i = 0; i < height*width; i++)
    {
        static unsigned char a[2]={0};
        if (i%4==0 && i!=0)
        {
            fwrite("\n",1,1,fdw);            
        }
        sprintf((char*)a,"%02x",pixel_data[i]);
        fwrite(&a,1,2,fdw);
    }

    fclose(fdw);
    fclose(fd);
    
    return 0;
}