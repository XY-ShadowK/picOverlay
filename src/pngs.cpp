#include <iostream>
#include <cstdlib>

#include "errorUtils.hpp"

#include "include/pngs.hpp"

using std::string;
using errorUtils::errorCode2String;
using errorUtils::SysGetLastError;

static FILE* fpRead = nullptr;
static FILE* fpWrite = nullptr;
static char buf[9] = {0};

bool getPng(const string& filename, PngInfo*& pPngInfo)
{
    if(!(fpRead = cFileOpen(filename.c_str(), "rb"))) {
        fprintf(stderr, "%s 无法打开！ %s\n", filename.c_str(), errorCode2String(SysGetLastError()).c_str());
        return false;
    }
    if(fread(buf, 1, PNG_BYTES_TO_CHECK, fpRead) != PNG_BYTES_TO_CHECK ||
    png_sig_cmp((png_const_bytep)buf, 0, PNG_BYTES_TO_CHECK)) {
        fprintf(stderr, "%s 不是png图片！\n", filename.c_str());
        return false;
    }
    pPngInfo = new PngInfo(png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr));
    if(!pPngInfo->getPStruct() || !pPngInfo->getPInfo()) {
        fprintf(stderr, "png结构体初始化错误！\n");
        return false;
    }
    return true;
}
bool setPng(const string& filename, PngInfo*& pPngInfo) {
    if(!(fpWrite = cFileOpen(filename.c_str(), "wb"))) {
        fprintf(stderr, "%s 无法打开！ %s\n", filename.c_str(), errorCode2String(SysGetLastError()).c_str());
        return false;
    }
    pPngInfo = new PngInfo(png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr));
    if(!pPngInfo->getPStruct() || !pPngInfo->getPInfo()) {
        fprintf(stderr, "png结构体初始化错误！\n");
        return false;
    }
    return true;
}

PicData* readPng(const string& filename) {
    PngInfo* pPngInfo = nullptr;
    if(!getPng(filename, pPngInfo) || setjmp(png_jmpbuf(pPngInfo->getPStruct()))) {
        if(fpRead) {
            fclose(fpRead);
            fpRead = nullptr;
        }
        delete pPngInfo;
        pPngInfo = nullptr;
        return nullptr;
    }
    pPngInfo->initIO(fpRead);
    pPngInfo->readInfo();
    PicData*&& pPicData = pPngInfo->readData();
    if(fpRead) {
        fclose(fpRead);
        fpRead = nullptr;
    }
    return pPicData;
}
bool writePng(const string& filename, PicData* pPicData, png_byte colorType) {
    PngInfo* pPngInfo = nullptr;
    if(!setPng(filename, pPngInfo) || setjmp(png_jmpbuf(pPngInfo->getPStruct()))) {
        if(fpWrite) {
            fclose(fpWrite);
            fpWrite = nullptr;
        }
        delete pPngInfo;
        pPngInfo = nullptr;
        return false;
    }
    pPngInfo->initIO(fpWrite);
    pPngInfo->setInfo(pPicData->getPicInfoPtr(), colorType);
    pPngInfo->writeData(pPicData);
    if(fpWrite) {
        fclose(fpWrite);
        fpWrite = nullptr;
    }
    return true;
}