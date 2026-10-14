/*
time: 2026/10/14 22:26:42
*/
#pragma once
#ifndef _BUFFER_HPP_
#define _BUFFER_HPP_

#include <cstdint>
#include "zlib.h"

class Buffer {
public:
    uint32_t width;
    uint32_t height;
    Byte** rows;

    Buffer(uint32_t w = 0, uint32_t h = 0) : width(w), height(h), rows(nullptr) {
        reassign();
    }
    void reassignOnlyWidth() {
        // 防止内存泄漏
        if(!(width && height))
            return;
        Byte* raw = new Byte[height * width];
        for(int i = 0;i < height;i ++) {
            rows[i] = raw;
            raw += width;
        }
    }
    // void reassignOnlyHeight(uint32_t oldHeight) {
    //     Byte** temp = rows;
    //     rows = new Byte*[height];
    //     for(int i = 0;i < oldHeight;i ++)
    //         rows[i] = temp[i];
    //     for(int i = oldHeight;i < height;i ++)
    //         rows[i] = new Byte[width];
    //     delete[] temp;
    //     temp = nullptr;
    // }
    void reassign() {
        delete[] rows;
        rows = new Byte*[height];
        reassignOnlyWidth();
    }
    void resize(uint32_t w, uint32_t h) {
        if(w > width || h > height) {
            width = w;
            if(h > height) {
                height = h;
                reassign();
            }
            else reassignOnlyWidth();
            return;
        }
    }
};

#endif