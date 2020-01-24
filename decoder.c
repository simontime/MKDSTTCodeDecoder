#include <stdio.h>
#include <stdlib.h>

#include "strings.h"

const char *base32LUT = "S7LCX3JZE8FG4HBKWN52YPA6RTU9VMDQ";

unsigned char decodeBase32Character(char c)
{
    /* Look for the character in the custom base32 table */
    for (unsigned char i = 0; i < 32; i++)
    {
        if (c == base32LUT[i])
            return i;
    }
    
    return -1; // Shouldn't ever happen
}

void decodeCodeString(const char *inStr, unsigned char *outCode)
{
    unsigned char filteredCode[16];
    unsigned char *fp = filteredCode;
    
    for (;;)
    {
        char ch = *inStr;
        
        if (!ch || fp - filteredCode > sizeof(filteredCode))
            break;
        
        /* Allowed characters */
        if ((ch >= '2' && ch <= '9') || 
            (ch >= 'A' && ch <= 'H') ||
            (ch >= 'J' && ch <= 'N') ||
            (ch >= 'P' && ch <= 'Z'))
        {
            *fp++ = decodeBase32Character(ch);
        }
        
        inStr++;
    }
    
    /* Extremely crude base32 decoding */
    outCode[0] = filteredCode[0]  << 3 | filteredCode[1]  >> 2;
    outCode[1] = filteredCode[1]  << 6 | filteredCode[2]  << 1 | filteredCode[3]  >> 4;
    outCode[2] = filteredCode[3]  << 4 | filteredCode[4]  >> 1;
    outCode[3] = filteredCode[4]  << 7 | filteredCode[5]  << 2 | filteredCode[6]  >> 3;
    outCode[4] = filteredCode[6]  << 5 | filteredCode[7];
    outCode[5] = filteredCode[8]  << 3 | filteredCode[9]  >> 2;
    outCode[6] = filteredCode[9]  << 6 | filteredCode[10] << 1 | filteredCode[11] >> 4;
    outCode[7] = filteredCode[11] << 4 | filteredCode[12] >> 1;
    outCode[8] = filteredCode[12] << 7 | filteredCode[13] << 2 | filteredCode[14] >> 3;
    outCode[9] = filteredCode[14] << 5 | filteredCode[15];
}

void decryptCode(unsigned char *code)
{
    /* XOR "crypto" */ 
    for (int i = 1; i < 10; i++)
        code[i - 1] ^= code[i];
    
    code[9] ^= 0xC3;
}

unsigned short calcChecksum(unsigned char *data, int len)
{
    unsigned short sum = 0;
    
    /* Appears to just be a standard CRC16 algorithm */
    for (int i = 0; i < len; i++)
    {
        unsigned char ch = *data++;

        for (int j = 0; j < 8; j++)
        {
            if (sum & 0x8000)
                sum = (sum << 1) ^ 0x1021;
            else
                sum <<= 1;
            
            if (ch & 0x80)
                sum ^= 1;
            
            ch <<= 1;
        }            
    }
    
    return sum;
}

int main()
{
    char buf[32];
    unsigned char outcode[10];
    unsigned short checksum, calcsum;
    unsigned int stats, time, course, kartch;
    
    printf("Enter code: ");
    gets(buf);
    puts("");
    
    decodeCodeString(buf, outcode);
    decryptCode(outcode);
    
    checksum   = outcode[8] << 8 | outcode[9];
    outcode[8] = outcode[9] = 0;    
    calcsum    = calcChecksum(outcode, 10);
    stats      = *(unsigned int *)outcode;
    
    time   = stats >> 14;
    course = (stats >> 9) % 32;
    kartch = stats % 0x200;
    
    printf("Time:      %d:%02d:%03d\n",
        time / 60000, (time / 1000) % 60, time % 1000);
    printf("Course:    %s\n", courseNames[course]);
    printf("Character: %s\n", characterNames[kartch / 37]);
    printf("Kart:      %s\n", kartNames[kartch % 37]);
    wprintf(L"Player:    %c%c\n",
        *(unsigned short *)&outcode[4], *(unsigned short *)&outcode[6]);
    printf("Valid?     %s\n", checksum == calcsum ? "Yes" : "No");
    
    puts("\nPress any key to exit.");
    getche();
    
    return 0;
}