/*
time: 2026/10/14 21:50:50
*/
#pragma once
#ifndef _FILE_H_
#define _FILE_H_

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
#elif defined(__linux__)
    
#else
    #error Need compile in Windows or Linux Platform!
#endif

inline FILE* cFileOpen(const char* filename, const char* mode) {
    if(strlen(mode) > 3) return NULL;
#ifdef _WIN32
    int fnWideLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename, -1, NULL, 0);
    if (fnWideLen <= 0) return NULL;
    wchar_t* wFilename = (wchar_t*)malloc(fnWideLen * sizeof(wchar_t));
    if (!wFilename) return NULL;
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wFilename, fnWideLen);

    wchar_t wMode[4] = {0};
    for (unsigned int i = 0; i < 3 && mode[i];i ++)
        wMode[i] = (wchar_t)(unsigned char)mode[i];
    
    FILE* fp = NULL;
    fp = _wfopen(wFilename, wMode);
    free(wFilename);
    return fp;
#elif defined(__linux__)
    return fopen(filename, mode);
#else
    #error Need compile in Windows or Linux Platform!
#endif
}

#endif