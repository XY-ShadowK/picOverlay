/*
time: 2026/10/14 22:27:11
*/
#pragma once
#ifndef _PICTURES_HPP_
#define _PICTURES_HPP_

#include <memory>
#include <type_traits>
#include "png.h"
#include "zlib.h"

#include "include/bytes.hpp"
#include "include/buffer.hpp"

constexpr size_t PNG_BYTES_TO_CHECK = 8;

inline Buffer bufRead, bufWrite;

typedef struct tSelfPicInfo {
    // Byte colorType;
    Byte bitDepth;
    Byte numChannel;
    uint32_t width;
    uint32_t height;
    tSelfPicInfo() : bitDepth(0), numChannel(0), width(0), height(0) {}

    bool operator!=(const tSelfPicInfo& otherPicInfo) const noexcept {
        return bitDepth != otherPicInfo.bitDepth || numChannel != otherPicInfo.numChannel || width != otherPicInfo.width || height != otherPicInfo.height;
    }
    bool operator==(const tSelfPicInfo& otherPicInfo) const noexcept {
        return ! operator!=(otherPicInfo);
    }
}PicInfo;

class self_pic_data {
private:
    std::shared_ptr<PicInfo> picInfoPtr;
    bool isGray;
    bool hasAlpha;
    uint32_t sizeByte;
    PODList<Bytes>* degreeGray;
    PODList<Bytes>* colorRed;
    PODList<Bytes>* colorGreen;
    PODList<Bytes>* colorBlue;
    PODList<Bytes>* degreeAlpha;
    bool isStorageInit;
public:

    // Af + (Ab * (MAX - Af) + MAX / 2 ) / MAX
    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    static T calOverlayAlpha(const T& maxn, const T& halfMaxn, const T& alphaBack, const T& alphaFront) noexcept {
        if(alphaBack == maxn || alphaFront == maxn)
            return maxn;
        if(alphaBack == 0)
            return alphaFront;
        if(alphaFront == 0)
            return alphaBack;
        return alphaFront + (alphaBack * (maxn - alphaFront) + halfMaxn) / maxn;
    }
    // (Cf * Af * MAX + Cb * Ab * (MAX - Af) + (Ao * MAX) / 2) / (Ao * MAX)
    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    static T calOverlayColor(const T& maxn, const T& halfMaxn, const T& colorBack, const T& colorFront, const T& alphaBack, const T& alphaFront, const T& alphaOverlay) noexcept {
        if(alphaFront == maxn)
            return colorFront;
        if(alphaFront == 0)
            return colorBack;
        if(alphaBack == 0)
            return colorFront;
        // 使用Ao = MAX简化掉Ao / 2的计算
        if(alphaBack == maxn)
            return (colorFront * alphaFront + colorBack * (maxn - alphaFront) + halfMaxn) / maxn;
        const T aOMaxn = alphaOverlay * maxn, halfAOMaxn = aOMaxn >> 1;
        return (colorFront * alphaFront * maxn + colorBack * alphaBack * (maxn - alphaFront) + halfAOMaxn) / (aOMaxn);
    }
    template<typename T, bool hasCoefficientAlpha, typename = std::enable_if_t<std::is_integral_v<T>>>
    static void overlayColor(const uint32_t& depth, const uint32_t& numByte, const uint32_t& numColor, PODList<Bytes>** const& colorBackList, PODList<Bytes>** const& colorFrontList, PODList<Bytes>* const& alphaBackList, PODList<Bytes>* const& alphaFrontList, const uint64_t& coefficientAlpha = 0) noexcept {
        // 不检查空指针方便调试
        const uint32_t length = colorBackList[0]->size();
        Bytes* colorPixel = nullptr, *alphaPixel = nullptr;
        T maxn = (1 << depth) - 1, halfMaxn = maxn >> 1, alphaBack, alphaFront;
        for(int i = 0;i < length;i ++) {
            alphaPixel = alphaBackList->getChunk(i);
            alphaBack = alphaPixel->getValue<T>(numByte);
            alphaFront = alphaFrontList->getChunk(i)->getValue<T>(numByte);
            if constexpr (hasCoefficientAlpha)
                alphaFront = (alphaFront * coefficientAlpha + halfMaxn) / maxn;
            if(alphaFront == 0)
                continue;
            T&& alphaRes = calOverlayAlpha(maxn, halfMaxn, alphaBack, alphaFront);
            alphaPixel->copyBytes((Byte*) &alphaRes, numByte);
            for(int j = 0;j < numColor;j ++) {
                colorPixel = colorBackList[j]->getChunk(i);
                T&& colorRes = calOverlayColor(maxn, halfMaxn, colorPixel->getValue<T>(numByte), colorFrontList[j]->getChunk(i)->getValue<T>(numByte), alphaBack, alphaFront, alphaRes);
                colorPixel->copyBytes((Byte*) &colorRes, numByte);
            }
        }
    }
    template<typename T, bool hasCoefficientAlpha, typename = std::enable_if_t<std::is_integral_v<T>>>
    static void overlayColor(const uint32_t& depth, const uint32_t& numByte, const uint32_t& numColor, PODList<Bytes>** const& colorBackList, PODList<Bytes>** const& colorFrontList, PODList<Bytes>* const& alphaFrontList, const uint64_t& coefficientAlpha = 0) noexcept {
        // 不检查空指针方便调试
        const uint32_t length = colorBackList[0]->size();
        Bytes* colorPixel = nullptr;
        T maxn = (1 << depth) - 1, halfMaxn = maxn >> 1, alphaFront;
        for(int i = 0;i < length;i ++) {
            alphaFront = alphaFrontList->getChunk(i)->getValue<T>(numByte);
            if constexpr (hasCoefficientAlpha)
                alphaFront = (alphaFront * coefficientAlpha + halfMaxn) / maxn;
            if(alphaFront == 0)
                continue;
            for(int j = 0;j < numColor;j ++) {
                colorPixel = colorBackList[j]->getChunk(i);
                T&& colorRes = (colorFrontList[j]->getChunk(i)->getValue<T>(numByte) * alphaFront + colorPixel->getValue<T>(numByte) * (maxn - alphaFront) + halfMaxn) / maxn;
                colorPixel->copyBytes((Byte*) &colorRes, numByte);
            }
        }
    }

    self_pic_data(const std::shared_ptr<PicInfo>& startPicInfoPtr = nullptr, const bool& gray = false, const bool& alpha = false) : picInfoPtr(startPicInfoPtr), isGray(gray), hasAlpha(alpha), sizeByte(picInfoPtr ? ((picInfoPtr->bitDepth / 8 + (picInfoPtr->bitDepth % 8 > 0))) : 0), degreeGray(nullptr), colorRed(nullptr), colorGreen(nullptr), colorBlue(nullptr), degreeAlpha(nullptr), isStorageInit(false) {}
    self_pic_data(const self_pic_data& otherPicData, const uint64_t& coefficientAlpha = 0) : picInfoPtr(nullptr), isGray(false), hasAlpha(false), sizeByte(0), degreeGray(nullptr), colorRed(nullptr), colorGreen(nullptr), colorBlue(nullptr), degreeAlpha(nullptr), isStorageInit(false) {
        copyData(otherPicData, coefficientAlpha);
    }
    ~self_pic_data() noexcept {
        clearStorage();
    }

    self_pic_data& operator=(const self_pic_data& otherPicData) {
        if(this != & otherPicData)
            copyData(otherPicData);
        return *this;
    }

    const std::shared_ptr<PicInfo>& getPicInfoPtr() const noexcept {
        return picInfoPtr;
    }
    bool getGray() const noexcept {
        return isGray;
    }
    bool getAlpha() const noexcept {
        return hasAlpha;
    }
    bool getStorageInit() const noexcept {
        return isStorageInit;
    }

    void clearStorage() noexcept {
        delete degreeGray;
        delete colorRed;
        delete colorGreen;
        delete colorBlue;
        delete degreeAlpha;
        isStorageInit = false;
        degreeGray = nullptr;
        colorRed = nullptr;
        colorGreen = nullptr;
        colorBlue = nullptr;
        degreeAlpha = nullptr;
    }
    void copyData(const self_pic_data& otherPicData, const uint64_t& coefficientAlpha = 0) {
        uint32_t&& tempLength = otherPicData.picInfoPtr->width * otherPicData.picInfoPtr->height;
        if(!picInfoPtr || *picInfoPtr != *otherPicData.picInfoPtr) {
            picInfoPtr = otherPicData.picInfoPtr;
            isGray = otherPicData.isGray;
            hasAlpha = otherPicData.hasAlpha;
            sizeByte = otherPicData.sizeByte;
            clearStorage();
            initStorage(tempLength);
        }
        else if(!isStorageInit)
            initStorage(tempLength);
        if(isGray)
            *degreeGray = *otherPicData.degreeGray;
        else {
            *colorRed = *otherPicData.colorRed;
            *colorGreen = *otherPicData.colorGreen;
            *colorBlue = *otherPicData.colorBlue;
        }
        if(hasAlpha) {
            *degreeAlpha = *otherPicData.degreeAlpha;
            if(coefficientAlpha) {
                Bytes* alphaPixel = nullptr;
                uint64_t maxn = (1 << (uint8_t)picInfoPtr->bitDepth) - 1, halfMaxn = maxn >> 1;
                for(int i = 0;i < tempLength;i ++) {
                    alphaPixel = degreeAlpha->getChunk(i);
                    uint64_t&& alphaRes = (alphaPixel->getValue<uint64_t>(sizeByte) * coefficientAlpha + halfMaxn) / maxn;
                    alphaPixel->copyBytes((Byte*) &alphaRes, sizeByte);
                }
            }
        }
    }
    PODList<Bytes>** getStorage() const {
        if(!isStorageInit) return nullptr;
        PODList<Bytes>** pData = new PODList<Bytes>*[picInfoPtr->numChannel];
        int t = 0;
        if(isGray) pData[t ++] = degreeGray;
        else {
            pData[t ++] = colorRed;
            pData[t ++] = colorGreen;
            pData[t ++] = colorBlue;
        }
        if(hasAlpha) pData[t ++] = degreeAlpha;
        return pData;
    }
    PODList<Bytes>** initStorage(const uint32_t& length) {
        if(isGray) {
            degreeGray = new PODList<Bytes>(sizeByte, length);
        }
        else {
            colorRed = new PODList<Bytes>(sizeByte, length);
            colorGreen = new PODList<Bytes>(sizeByte, length);
            colorBlue = new PODList<Bytes>(sizeByte, length);
        }
        if(hasAlpha) {
            degreeAlpha = new PODList<Bytes>(sizeByte, length);
        }
        isStorageInit = true;
        return getStorage();
    }
    void loadData(Byte** image) {
        // 不考虑传入数据与info不符的情况
        uint32_t rowStart = 0, t;
        PODList<Bytes>** pData = initStorage(picInfoPtr->width * picInfoPtr->height);
        for (uint32_t i = 0;i < picInfoPtr->height;i ++) {
            t = 0;
            for (uint32_t j = 0;j < picInfoPtr->width;j ++) {
                for(uint32_t k = 0;k < picInfoPtr->numChannel;k ++) {
                    pData[k]->getChunk(rowStart + j)->copyBytes(image[i] + t, sizeByte);
                    t += sizeByte;
                }
            }
            rowStart += picInfoPtr->width;
        }
        delete[] pData;
        pData = nullptr;
    }
    void exportData(Byte** image) const {
        uint32_t rowStart = 0, t;
        PODList<Bytes>** pData = getStorage();
        if(!pData) throw std::logic_error("try to export data from null storage, void exportData(Byte** image) const");
        for (uint32_t i = 0;i < picInfoPtr->height;i ++) {
            t = 0;
            for (uint32_t j = 0;j < picInfoPtr->width;j ++) {
                for(uint32_t k = 0;k < picInfoPtr->numChannel;k ++) {
                    pData[k]->getChunk(rowStart + j)->sendBytes(image[i] + t, sizeByte);
                    t += sizeByte;
                }
            }
            rowStart += picInfoPtr->width;
        }
        delete[] pData;
        pData = nullptr;
    }
    bool overlay(const self_pic_data& otherPicData, const uint64_t& coefficientAlpha = 0) noexcept {
        // 自己叠加自己无意义，浪费性能
        if(this != &otherPicData) {
            if(isGray != otherPicData.isGray)
                return false;
            // 暂时不考虑不同大小的图片叠加
            if(*picInfoPtr != *otherPicData.picInfoPtr)
                return false;
            if(!otherPicData.hasAlpha) {
                copyData(otherPicData);
                return true;
            }
            if(hasAlpha) {
                if(coefficientAlpha)
                    overlayColor<uint64_t, true>(picInfoPtr->bitDepth, sizeByte, picInfoPtr->numChannel - 1, getStorage(), otherPicData.getStorage(), degreeAlpha, otherPicData.degreeAlpha, coefficientAlpha);
                else
                    overlayColor<uint64_t, false>(picInfoPtr->bitDepth, sizeByte, picInfoPtr->numChannel - 1, getStorage(), otherPicData.getStorage(), degreeAlpha, otherPicData.degreeAlpha, 0ULL);
            }
            else {
                if(coefficientAlpha)
                    overlayColor<uint64_t, true>(picInfoPtr->bitDepth, sizeByte, picInfoPtr->numChannel, getStorage(), otherPicData.getStorage(), otherPicData.degreeAlpha, coefficientAlpha);
                else
                    overlayColor<uint64_t, false>(picInfoPtr->bitDepth, sizeByte, picInfoPtr->numChannel, getStorage(), otherPicData.getStorage(), otherPicData.degreeAlpha, 0ULL);
            }
        }
        return true;
    }
};
using PicData = self_pic_data;

class self_png_info {
private:
    png_structp pStruct;
    png_infop pInfo;
    png_byte colorType;
    std::shared_ptr<PicInfo> picInfoPtr;
public:
    self_png_info() noexcept : pStruct(nullptr), pInfo(nullptr), colorType(0), picInfoPtr(nullptr) {}
    self_png_info(png_structp pPngStrcut) : pStruct(pPngStrcut), pInfo(png_create_info_struct(pStruct)), colorType(0), picInfoPtr(nullptr) {}
    ~self_png_info() {
        // 不需要判断指针非空，libpng内部做了判断
        png_destroy_read_struct(&pStruct, &pInfo, nullptr);
    }

    png_struct* const getPStruct() const noexcept {
        return pStruct;
    }
    const png_info* const getPInfo() const noexcept {
        return pInfo;
    }
    bool testGray() const noexcept {
        if(colorType & PNG_COLOR_MASK_COLOR)
            return false;
        return true;
    }
    bool testAlpha() const noexcept {
        if(colorType & PNG_COLOR_MASK_ALPHA)
            return true;
        return false;
    }
    void initIO(FILE* fp) {
        png_init_io(pStruct, fp);
    }
    void readInfo() {
        png_set_sig_bytes(pStruct, PNG_BYTES_TO_CHECK);
        // png_read_png(pStruct, pInfo, PNG_TRANSFORM_EXPAND, 0);
        png_read_info(pStruct, pInfo);
        png_set_expand(pStruct);
        png_read_update_info(pStruct, pInfo);
        picInfoPtr = std::make_shared<PicInfo>();
        colorType = png_get_color_type(pStruct, pInfo);
        picInfoPtr->bitDepth = png_get_bit_depth(pStruct, pInfo);
        picInfoPtr->width = png_get_image_width(pStruct, pInfo);
        picInfoPtr->height = png_get_image_height(pStruct, pInfo);
        picInfoPtr->numChannel = png_get_channels(pStruct, pInfo);
        // 转换tRNS透明为alpha通道 不需要 已经在png_set_expand()做了
        // if(png_get_valid(pStruct, pInfo, PNG_INFO_tRNS)) {
        //     png_set_tRNS_to_alpha(pStruct);
        //     png_read_update_info(pStruct, pInfo);
        // }
    }
    void setInfo() {
        png_set_IHDR(pStruct, pInfo, picInfoPtr->width, picInfoPtr->height, picInfoPtr->bitDepth, colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    }
    void setInfo(const std::shared_ptr<PicInfo>& startPicInfoPtr, png_byte tColorType = PNG_COLOR_TYPE_RGB_ALPHA) {
        picInfoPtr = startPicInfoPtr;
        colorType = tColorType;
        setInfo();
    }
    PicData* readData() const {
        PicData* pPicData = new PicData(picInfoPtr, testGray(), testAlpha());
        png_read_image(pStruct, bufRead.rows);
        pPicData->loadData(bufRead.rows);
        return pPicData;
    }
    void writeData(PicData* pPicData) const {
        png_write_info(pStruct, pInfo);
        pPicData->exportData(bufWrite.rows);
        png_write_image(pStruct, bufWrite.rows);
        png_write_end(pStruct, pInfo);
        return;
    }
};
using PngInfo = self_png_info;

inline void init_pic_buffer(uint32_t width, uint32_t height) {
    bufRead.resize(width, height);
    bufWrite.resize(width, height);
}

#endif