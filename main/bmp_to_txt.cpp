#include<stdio.h>
#include<string.h>

int main(int argc,char** argv){
    if (argc!=3)
    {
        printf("\nPlease provide the source and destination of the files.\n");
        printf("\nprovide as follows <destination> <source>\n");
        return -1;
    }
    
    FILE* fd=fopen(argv[2],"rb");
    if (fd==NULL)
    {
        printf("File at %s was not found",argv[2]);
        return -1;
    }
    FILE* fdw=fopen(argv[1],"wb");
    if (fdw==NULL)
    {
        printf("File at %s was not found",argv[1]);
        return -1;
    }
    unsigned char a[54]={0};
    //read the header of the image
    fread(a,sizeof(char),54,fd);
    //we cast the value stored at a[22],a[22],a[23],a[25] as an integer 
    //int is 4 bytes and char is a byte so we do conversion for address  a[22] till a[25]
    // this is a full int(4 bytes)
    //basically we converted 4 bytes starting from address of a[22] to integer little endian
    const int height=*(int *)&a[22];
    const int width=*(int *)&a[18];
    int size=*(int *)&a[34];
    printf("\nIMAGE WIDTH:%d\tIMAGE HEIGHT:%d\n",width,height);
    printf("\n\tIMAGE SIZE:%d\n",size);
    printf("\n***************************************************************\n");
    printf("\nNOTE: Pictures are assumed to have multiple of 4 in each row example (32x32)\n");
    printf("        Picture is assumed to be in 24-bit BMP file Format\n");
    printf("\n***************************************************************\n");
    
   //extracting the bits
    unsigned char read_pixels[3]={0};
    unsigned char pixel_data[height*width]={0};
    
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j <width ; j++)
        {
            //the pixels are stored as Blue then Green then Red
            //conversion from RGB to Gray Scale using weighted average
            //it accounts for how the the human eye responds to each color
            fread(read_pixels,1,3,fd);
            pixel_data[32*(height-1-i)+(j)]=(unsigned char)0.299*read_pixels[2]+0.587*read_pixels[1]+0.114*read_pixels[0];
        }
    }
    //data puts the byte of the pixel in the first row following up with 3 rows of zeroes
    //this was designed for word addressable memory where assembler reads 4 lines comtaining bytes and assigns it to a word in memory
    // the new line char must be delt with when reading the values because fread gets 
    // the whole line with the '\n' :)
    unsigned char b[8]={0};
    for (int i = 0; i < height*width; i++)
    {   //convert from decimal to binary then ascii letters (0 or 1) to set up the bits
        //example 5 is 00000101 we need to get the string for that :)
        for (int i = 0; i < 8; i++)
            if (pixel_data[i]<<i & 0x80) b[i]=0x31;
            else b[i]=0x30;
        fwrite(&b,sizeof(unsigned char),sizeof b,fdw);
        fwrite("\n00000000\n00000000\n00000000\n",sizeof(unsigned char),sizeof ("\n00000000\n00000000\n00000000\n") -1,fdw);
    }

    fclose(fdw);
    fclose(fd);
    
    return 0;
}