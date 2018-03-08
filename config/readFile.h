#ifndef READFILE_H
#define READFILE_H

#include "singleton.h"
#include <iostream>
#include <string>


class ReadFile
{
public:
    ReadFile();
    const char *pName;
    int GetCurrentPath(char *pFileName);
    char* GetIniKeyString(char *title, char *key);
    int SetIniKeyString(char *title, char *key, char *value);
    int WriteIniKeyString(char *title,char *key, char *value);

    DECLARE_SINGLETON_CLASS(ReadFile)
};

typedef Singleton<ReadFile> SReadFile;

#endif // CONFIG_H
