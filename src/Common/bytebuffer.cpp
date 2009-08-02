//
//bytebuffer.CPP
//
//Copyright(C) 2006  Eric Dyoniziak
//
// This code is used to create a dynamic buffer for reading and writing binary
// data. It has pre-built read and write functions for making specific operations
// needed for packet creation in SWG, however it is easily expandable for
// any generic data type and for any situation where data serialization is needed.
//
// This program is free software; you can redistribute it and/or modify
// it freely. You must however leave this copyright notice in its original form.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
#include "bytebuffer.h"
#include <iostream>

//-------------------------------------------------------------------------------------------------------------------
bytebuffer::bytebuffer()
{
    _ReadPos = 0;
    _WritePos = 0;
    _bincount = 0;
    _buffer = new char[DATA_RESERVE];
    _ptrbin = new char*[PTR_BIN_SIZE];
    _bincapacity = PTR_BIN_SIZE;
    _capacity = DATA_RESERVE;
}
//-------------------------------------------------------------------------------------------------------------------
bytebuffer::~bytebuffer()
{
    cleanGarbage();
    delete [] _buffer;
    delete [] _ptrbin;
}
//-------------------------------------------------------------------------------------------------------------------
unsigned int bytebuffer::getSize()
{
    return _WritePos;
}
//-------------------------------------------------------------------------------------------------------------------
char* bytebuffer::getBuffer()
{
    char* ret = createArray(_WritePos);
    memcpy(ret,_buffer,_WritePos);
    return ret;
}
//-------------------------------------------------------------------------------------------------------------------
unsigned char bytebuffer::readBYTE()
{
    return read<unsigned char>();
}
//-------------------------------------------------------------------------------------------------------------------
unsigned short bytebuffer::readSHORT()
{
    return read<unsigned short>();
}
//-------------------------------------------------------------------------------------------------------------------
unsigned int bytebuffer::readINT()
{
    return read<unsigned int>();
}
//-------------------------------------------------------------------------------------------------------------------
unsigned long long bytebuffer::readLONG()
{
    return read<unsigned long long>();
}
//-------------------------------------------------------------------------------------------------------------------
float bytebuffer::readFLOAT()
{
    return read<float>();
}
//-------------------------------------------------------------------------------------------------------------------
char* bytebuffer::readASTRING()
{
    unsigned short size = readSHORT();
    char* ret = createArray(size+1);
    readDATA(ret,size);
    ret[size] = 0;
    return ret;
}
//-------------------------------------------------------------------------------------------------------------------
wchar_t* bytebuffer::readASTRINGtoUNICODE()
{
    unsigned short size = readSHORT();
    char* tempArray = new char[size];
    readDATA(tempArray,size);
    wchar_t* ret = (wchar_t*)createArray(size * 2 +2);
    ascii_to_unicode(ret,tempArray,size);
    delete [] tempArray;
    ret[size] = 0;
    return ret;
}
//-------------------------------------------------------------------------------------------------------------------
char* bytebuffer::readUSTRINGtoASCII()
{
    unsigned int size = readINT();
    wchar_t* tempArray = new wchar_t[size];
    readDATA((char*)tempArray,size * 2);
    char* ret = createArray(size + 1);
    unicode_to_ascii(ret,tempArray,size);
    delete [] tempArray;
    ret[size] = 0;
    return ret;
}
//-------------------------------------------------------------------------------------------------------------------
wchar_t* bytebuffer::readUSTRING()
{
    unsigned int size = readINT();
    wchar_t* ret = (wchar_t*)createArray(size * 2 + 2);
    readDATA((char*)ret,size * 2);
    ret[size] = 0;
    return ret;
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::readDATA(char *data,unsigned int length)
{
    if((_ReadPos + length) > _WritePos)
    {
        std::cout << "Fatal Error[0]: Read Beyond End of Buffer.\nPress Enter to Close Program...";
        std::cin.get();
        exit(1);
    }
    else
    {
        for(unsigned int i = 0;i < length;i++)
        {
            data[i] = _buffer[_ReadPos];
            _ReadPos++;
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::writeBYTE(unsigned char data)
{
    append<unsigned char>(data);
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::writeSHORT(unsigned short data)
{
    append<unsigned short>(data);
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::writeINT(unsigned int data)
{
    append<unsigned int>(data);
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::writeLONG(unsigned long long data)
{
    append<unsigned long long>(data);
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::writeFLOAT(float data)
{
    append<float>(data);
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::writeASTRING(const char* string)
{
    unsigned int size;
    for(size = 0;string[size] != '\0';size++)
        ;
    writeSHORT(size);
    writeDATA(string,size);
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::writeASTRING(const wchar_t* string)
{
    unsigned int size;
    for(size = 0;string[size] != 0;size++)
        ;
    char* new_string = new char[size];
    unicode_to_ascii(new_string,string,size);
    writeSHORT(size);
    writeDATA(new_string,size);
    delete [] new_string;
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::writeUSTRING(const char* string)
{
    unsigned int size;
    for(size = 0;string[size] != '\0';size++)
        ;
    wchar_t* new_string = new wchar_t[size];
    ascii_to_unicode(new_string,string,size);
    writeINT(size);
    writeDATA((char*)new_string,size *2);
    delete [] new_string;
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::writeUSTRING(const wchar_t* string)
{
    unsigned int size;
    for(size = 0;string[size] != 0;size++)
        ;
    writeINT(size);
    writeDATA((char*)string,size *2);
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::writeDATA(const char *nData,unsigned int nLength)
{
    while(_WritePos + nLength > _capacity)
        resize(_capacity * 3);
    for(unsigned int i = 0;i < nLength;i++)
    {
        _buffer[_WritePos] = nData[i];
        _WritePos++;
    }
}
//-------------------------------------------------------------------------------------------------------------------
template <typename T>
T bytebuffer::read()
{
    unsigned int Tsize = sizeof(T);
    if(_ReadPos + Tsize > _WritePos)
    {
        std::cout << "Fatal Error[1]: Read Beyond End of Buffer.\nPress Enter to Close Program...";
        std::cin.get();
        exit(1);
    }
    else
    {
        T ret =  *((T*)&_buffer[_ReadPos]);
        _ReadPos += Tsize;
        return ret;
    }
}
//-------------------------------------------------------------------------------------------------------------------
template <typename T>
void bytebuffer::append(T value)
{
    unsigned int Tsize = sizeof(T);
    while(_WritePos + Tsize > _capacity)
        resize(_capacity * 3);

    *(T*)(&_buffer[_WritePos]) = value;
    _WritePos+= Tsize;
}
//-------------------------------------------------------------------------------------------------------------------
template <typename T>
T bytebuffer::peek()
{
    unsigned int Tsize = sizeof(T);
    if(_ReadPos + Tsize > _WritePos)
    {
        std::cout << "Fatal Error: Read Beyond End of Buffer.\nPress Enter to Close Program...";
        std::cin.get();
        exit(1);
    }
    else
    {
        T ret =  *((T*)&_buffer[_ReadPos]);
        return ret;
    }
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::ascii_to_unicode(wchar_t *destination,const char *source, int length)
{
    for(int i=0; i < length; i++)
        destination[i] = source[i];
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::unicode_to_ascii(char *destination,const wchar_t *source, int length)
{
    for(int i=0; i < length; i++)
        destination[i] = static_cast<char>(source[i]);
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::writeBUFFER(bytebuffer &data)
{
    writeDATA(data.getBuffer(),data.getSize());
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::writeBUFFER(bytebuffer *data)
{
    writeDATA(data->getBuffer(),data->getSize());
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::cleanGarbage()
{
    if(_bincount)
    {
        for(unsigned int i = 0;i < _bincount;i++)
            delete [] _ptrbin[i];
        _bincount = 0;
    }
}
//-------------------------------------------------------------------------------------------------------------------
char* bytebuffer::createArray(unsigned int size)
{
    if(_bincount == _bincapacity)
    {
        char **new_buffer = new char*[_bincapacity * 3];
        _bincapacity *= 3;
        for(unsigned int i=0;i < _bincount;i++)
            new_buffer[i] = _ptrbin[i];
        delete [] _ptrbin;
        _ptrbin= new_buffer;
    }
    _ptrbin[_bincount] = new char[size];
    _bincount++;
    return _ptrbin[_bincount-1];
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::resize(unsigned int size)
{
    if(size <= _capacity)
    {
        std::cout << "Error: Cannot resize bytebuffer when current buffer is greater than or equal to new size.\nPress Enter to Close Program...";
        std::cin.get();
        exit(1);
    }

    char *new_buffer = new char[size];
    _capacity = size;
    for(unsigned int i=0;i < _WritePos;i++)
    {
        new_buffer[i] = _buffer[i];
    }
    delete [] _buffer;
    _buffer = new_buffer;
}
//-------------------------------------------------------------------------------------------------------------------
char bytebuffer::peekBYTE()
{
    return peek<char>();
}
//-------------------------------------------------------------------------------------------------------------------
unsigned short bytebuffer::peekSHORT()
{
    return peek<unsigned short>();
}
//-------------------------------------------------------------------------------------------------------------------
unsigned int bytebuffer::peekINT()
{
    return peek<unsigned int>();
}
//-------------------------------------------------------------------------------------------------------------------
unsigned long long bytebuffer::peekLONG()
{
    return peek<unsigned long long>();
}
//-------------------------------------------------------------------------------------------------------------------
char &bytebuffer::operator[](int index)
{
    return _buffer[index];
}
//-------------------------------------------------------------------------------------------------------------------
unsigned int bytebuffer::getBytesLeft()
{
    return _WritePos - _ReadPos;
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::popBytes(unsigned int size)
{
    if(size <= _WritePos)
        _WritePos -= size;
    else
    {
        std::cout << "Error: Could not execute popBytes(). removal size was greater than current size.\n";
        std::cin.get();
        exit(1);
    }
}
//-------------------------------------------------------------------------------------------------------------------
char* bytebuffer::getBufferLeft()
{
    unsigned int size = _WritePos - _ReadPos;
    char* ret = createArray(size);
    memcpy(ret,_buffer+_ReadPos,size);
    return ret;
}
//-------------------------------------------------------------------------------------------------------------------
void bytebuffer::clear()
{
    delete [] _buffer;
    _ReadPos = 0;
    _WritePos = 0;
    _buffer = new char[DATA_RESERVE];
    _capacity = DATA_RESERVE;
}
//-------------------------------------------------------------------------------------------------------------------
