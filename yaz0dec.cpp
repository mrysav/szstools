//version 1.1 (20210524)
//by thakis
//Updated by Michael Hinrichs

#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;

typedef unsigned char u8;
typedef unsigned int u32;

u32 toDWORD(u32 d)
{
  u8 w1 = d & 0xFF;
  u8 w2 = (d >> 8) & 0xFF;
  u8 w3 = (d >> 16) & 0xFF;
  u8 w4 = d >> 24;
  return (w1 << 24) | (w2 << 16) | (w3 << 8) | w4;
}

struct Ret
{
  int srcPos, dstPos;
};

Ret decodeYaz0(u8* src, int srcSize, u8* dst, int uncompressedSize)
{
  Ret r = { 0, 0 };
  //int srcPlace = 0, dstPlace = 0; //current read/write positions
  
  u32 validBitCount = 0; //number of valid bits left in "code" byte
  u8 currCodeByte;
  while(r.dstPos < uncompressedSize)
  {
    //read new "code" byte if the current one is used up
    if(validBitCount == 0)
    {
      currCodeByte = src[r.srcPos];
      ++r.srcPos;
      validBitCount = 8;
    }
    
    if((currCodeByte & 0x80) != 0)
    {
      //straight copy
      dst[r.dstPos] = src[r.srcPos];
      r.dstPos++;
      r.srcPos++;
      //if(r.srcPos >= srcSize)
      //  return r;
    }
    else
    {
      //RLE part
      u8 byte1 = src[r.srcPos];
      u8 byte2 = src[r.srcPos + 1];
      r.srcPos += 2;
      //if(r.srcPos >= srcSize)
      //  return r;
      
      u32 dist = ((byte1 & 0xF) << 8) | byte2;
      u32 copySource = r.dstPos - (dist + 1);

      u32 numBytes = byte1 >> 4;
      if(numBytes == 0)
      {
        numBytes = src[r.srcPos] + 0x12;
        r.srcPos++;
        //if(r.srcPos >= srcSize)
        //  return r;
      }
      else
        numBytes += 2;

      //copy run
      for(int i = 0; i < numBytes; ++i)
      {
        dst[r.dstPos] = dst[copySource];
        copySource++;
        r.dstPos++;
      }
    }
    
    //use next bit from "code" byte
    currCodeByte <<= 1;
    validBitCount-=1;    
  }

  return r;
}

#pragma warning(disable : 4996)

void decodeAll(u8 * src, int srcSize, char* srcName)
{
  int readBytes = 0;

  while(readBytes < srcSize)
  {
    //search yaz0 block
    while(readBytes + 3 < srcSize && (src[readBytes] != 'Y' || src[readBytes + 1] != 'a' || src[readBytes + 2] != 'z' || src[readBytes + 3] != '0'))
      ++readBytes;

    if(readBytes + 3 >= srcSize)
      return; //nothing left to decode

    readBytes += 4;

    std::string fullname(srcName);
    string rawname = fullname.substr(0, fullname.find_last_of("."));

    char dstName[300];
    if (src[readBytes + 13] == 'S' && src[readBytes + 14] == 'A' && src[readBytes + 15] == 'R' && src[readBytes + 16] == 'C')
      sprintf_s(dstName, "%s.sarc", rawname.c_str());
    else if (src[readBytes + 13] == 'F' && src[readBytes + 14] == 'R' && src[readBytes + 15] == 'E' && src[readBytes + 16] == 'S')
      sprintf_s(dstName, "%s.fres", rawname.c_str());
    else if (src[readBytes + 13] == 'R' && src[readBytes + 14] == 'A' && src[readBytes + 15] == 'R' && src[readBytes + 16] == 'C')
      sprintf_s(dstName, "%s.rarc", rawname.c_str());
    else if (src[readBytes + 13] == 'N' && src[readBytes + 14] == 'A' && src[readBytes + 15] == 'R' && src[readBytes + 16] == 'C')//Super Mario 3D Land
      sprintf_s(dstName, "%s.narc", rawname.c_str());
    else if (src[readBytes + 13] == 'U' && src[readBytes + 15] == '8' && src[readBytes + 16] == '-')//Mario Kart Wii
      sprintf_s(dstName, "%s.u8", rawname.c_str());
    else if (src[readBytes + 13] == 2 && src[readBytes + 14] == 2 && src[readBytes + 15] == 0 && src[readBytes + 16] == 0)//Mario Kart 8 Deluxe
      sprintf_s(dstName, "%s.kcl", rawname.c_str());
    else if (src[readBytes + 13] == 'B' && src[readBytes + 14] == 'N' && src[readBytes + 15] == 'T' && src[readBytes + 16] == 'X')//Mario Kart 8 Deluxe
      sprintf_s(dstName, "%s.bntx", rawname.c_str());
    else if (src[readBytes + 13] == 'M' && src[readBytes + 14] == 's' && src[readBytes + 15] == 'g' && src[readBytes + 16] == 'S' && src[readBytes + 17] == 't' && src[readBytes + 18] == 'd' && src[readBytes + 19] == 'B' && src[readBytes + 20] == 'n')//Animal Crossing New Horizons
      sprintf_s(dstName, "%s.msbt", rawname.c_str());
    else if (src[readBytes + 13] == 'F' && src[readBytes + 14] == 'F' && src[readBytes + 15] == 'N' && src[readBytes + 16] == 'T')//New Super Mario Bros U
      sprintf_s(dstName, "%s.ffnt", rawname.c_str());
    else if (src[readBytes + 13] == 'B' && src[readBytes + 14] == 'R' && src[readBytes + 15] == 'E' && src[readBytes + 16] == 'S')//Mario Kart Wii
      sprintf_s(dstName, "%s.bres", rawname.c_str());
    else if (src[readBytes + 13] == 'A' && src[readBytes + 14] == 'A' && src[readBytes + 15] == '_' && src[readBytes + 16] == '<' && src[readBytes + 17] == 'b' && src[readBytes + 18] == 's' && src[readBytes + 19] == 't' && src[readBytes + 20] == ' ')//Super Mario Galaxy 1 & 2
      sprintf_s(dstName, "%s.baa", rawname.c_str());
    //else if (src[readBytes + 13] == 'C' && src[readBytes + 14] == 'R' && src[readBytes + 15] == 'A' && src[readBytes + 16] == 'R')//in Super Mario All Stars, Super Mario Galaxy's RARC files are in little endian. Keep this commented until it can be reversed
      //sprintf_s(dstName, "%s.crar", rawname.c_str(), readBytes - 4);
    //else if (src[readBytes + 13] == '<' && src[readBytes + 14] == '_' && src[readBytes + 15] == 'A' && src[readBytes + 16] == 'A' && src[readBytes + 17] == ' ' && src[readBytes + 18] == 't' && src[readBytes + 19] == 's' && src[readBytes + 20] == 'b')//in Super Mario All Stars, Super Mario Galaxy's BAA files are in little endian. Keep this commented until it can be reversed
      //sprintf_s(dstName, "%s.baa", rawname.c_str(), readBytes - 8);
    else if (fullname.find(".szs") == std::string::npos)
    {
        printf("no .szs extention detected");
        std::string newOldFile(srcName);
        strcat(srcName, ".szs");
        rename(newOldFile.c_str(), srcName);
        sprintf_s(dstName, "%s", fullname.c_str());
    }
    else
    {
      sprintf_s(dstName, "%s", rawname.c_str());
    }
    FILE* DataFile;
    if((DataFile=fopen(dstName,"wb"))==NULL)
      exit(-1);
    printf("Writing %s\n", dstName);

    u32 Size=toDWORD(*(u32*)(src + readBytes));
    printf("Writing 0x%X bytes\n",Size);
    u8* Dst=(u8 *) malloc(Size+0x1000);

    readBytes += 12; //4 byte size, 8 byte unused

    Ret r = decodeYaz0(src + readBytes, srcSize - readBytes, Dst, Size);
    readBytes += r.srcPos;
    printf("Read 0x%X bytes from input\n", readBytes);

    fwrite(Dst,r.dstPos,1,DataFile);
    free(Dst);
    fclose(DataFile);
  }
}

int main(int argc, char* argv[])
{
  if(argc < 2)
    return EXIT_FAILURE;

  FILE* inFile = fopen(argv[1], "rb");
  if(inFile == NULL)
    return EXIT_FAILURE;

  fseek(inFile, 0, SEEK_END);
  int size = ftell(inFile);
  fseek(inFile, 0, SEEK_SET);

  printf("input file size: 0x%X\n", size);

  u8* buff = new u8[size];

  fread(buff, 1, size, inFile);

  fclose(inFile);

  decodeAll(buff, size, argv[1]);
  delete [] buff;

  return EXIT_SUCCESS;
}
