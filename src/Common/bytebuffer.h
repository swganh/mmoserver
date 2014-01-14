//
//bytebuffer.h
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

#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#include <locale.h>

#define DATA_RESERVE 1024
#define PTR_BIN_SIZE 15


class bytebuffer
{
private:
    //private buffer functions
    template <typename T>
    T read();
    template <typename T>
    T peek();
    template <typename T>
    void append(T value);
    void ascii_to_unicode(wchar_t *destination,const char *source, int length);
    void unicode_to_ascii(char *destination,const wchar_t *source, int length);
    void cleanGarbage();
    char* createArray(unsigned int size);
    void resize(unsigned int size);

    //private buffer variables
    unsigned int _ReadPos; //initalize to 0. Increment on read.
    unsigned int _WritePos; //initialize to 0. Increment on write.
    char* _buffer;
    char** _ptrbin;
    unsigned int _bincount;
    unsigned int _capacity;
    unsigned int _bincapacity;


public:
    //other buffer functions
    bytebuffer();
    ~bytebuffer();
    unsigned int getSize(); //return _WritePos
    unsigned int getBytesLeft(); //return _WritePos - _ReadPos
    void popBytes(unsigned int size); //subtract size from end.
    char* getBuffer(); //return _buffer ptr.
    char* getBufferLeft(); //return _buffer+_ReadPos
    void clear();
    //read from buffer functions
    unsigned char readBYTE();
    unsigned short readSHORT();
    unsigned int readINT();
    unsigned long long readLONG();
    float readFLOAT();
    char* readASTRING();        //Ascii to Ascii
    wchar_t* readASTRINGtoUNICODE();     //Ascii to Unicode
    char* readUSTRINGtoASCII();        //Unicode to Ascii
    wchar_t* readUSTRING();     //Unicode to Unicode
    void readDATA(char *data,unsigned int length); //read raw data

    //write to buffer functions
    void writeBYTE(unsigned char data);
    void writeSHORT(unsigned short data);
    void writeINT(unsigned int data);
    void writeLONG(unsigned long long data);
    void writeFLOAT(float data);
    void writeASTRING(const char* string);    //Ascii to Ascii
    void writeASTRING(const wchar_t* string); //Unicode to Ascii
    void writeUSTRING(const char* string);    //Ascii to Unicode
    void writeUSTRING(const wchar_t* string); //Unicode to Unicode
    void writeDATA(const char *nData,unsigned int nLength); //write raw data
    void writeBUFFER(bytebuffer &data);
    void writeBUFFER(bytebuffer *data);

    char peekBYTE();
    unsigned short peekSHORT();
    unsigned int peekINT();
    unsigned long long peekLONG();

    char &operator[](int index);
};
#endif
