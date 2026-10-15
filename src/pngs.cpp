#include <iostream>
#include <cstdlib>

#include "include/pngs.hpp"

using std::cin;
using std::cerr;
using std::endl;
using std::string;

FILE* fpRead = nullptr;
FILE* fpWrite = nullptr;
char buf[9] = {0};

bool getPng(const string& filename, PngInfo*& pPngInfo)
{
    if(!(fpRead = cFileOpen(filename.c_str(), "rb"))) {
        cerr << filename << " 无法打开！错误：" << strerror(errno) << endl;
        return false;
    }
    if(fread(buf, 1, PNG_BYTES_TO_CHECK, fpRead) != PNG_BYTES_TO_CHECK) {
        cerr<<filename<<" 不是png图片！"<<endl;
        return false;
    }
    if(png_sig_cmp((png_const_bytep)buf, 0, PNG_BYTES_TO_CHECK)) {
        cerr<<filename<<" 不是png图片！"<<endl;
        return false;
    }
    pPngInfo = new PngInfo(png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr));
    if(!pPngInfo->getPStruct() || !pPngInfo->getPInfo()) {
        cerr<<"png结构体初始化错误！"<<endl;
        return false;
    }
    return true;
}
bool setPng(const string& filename, PngInfo*& pPngInfo) {
    if(!(fpWrite = cFileOpen(filename.c_str(), "wb"))) {
        cerr << filename << " 无法打开！错误：" << strerror(errno) << endl;
        return false;
    }
    pPngInfo = new PngInfo(png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr));
    if(!pPngInfo->getPStruct() || !pPngInfo->getPInfo()) {
        cerr<<"png结构体初始化错误！"<<endl;
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