#include<stdio.h>
#include<string.h>

//generate data from a file that has 4 bytes perline
//make it such that it has 8 bits perline (split the data from words to bytes per line)
int main(){

    FILE* fd=fopen("C:\\Users\\DELL\\Desktop\\learn\\c++\\expected_vals.txt","r");
    FILE* fd1=fopen("C:\\Users\\DELL\\Desktop\\learn\\c++\\expected_vals_bits.txt","w");
    if (fd==NULL || fd1==NULL)
    {
        return -1;
    }
    unsigned char a[8]={0};
    unsigned char b[4]={0};
    int stripe=0;
    while(fread(a,1,2,fd))
    {
        if (stripe%8==0 && stripe !=0)
        {
            // each eight chars we have a \n
            //avoid new line char
            fread(a,1,1,fd);
            stripe=0;
        }
        stripe+=2;
        for (int i = 0; i < 2; i++)
        {
            a[i]>='A' ? (a[i]>='a'? a[i]-=0x57 : a[i]-=0x37) :a[i]-=0x30;
            for (int j = 0; j <4; j++)
            {
                //get the bit and convert to ascii 
                b[3-j]= 0!=(a[i] & (1<<j) );
                b[3-j]+= 0x30;
            }
            fwrite(b,1,4,fd1);
        }
        fwrite("\n00000000\n00000000\n00000000\n",1,28,fd1);
    }       
    return 0;
}