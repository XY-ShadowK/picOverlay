/*
time: 2026/10/14 22:26:51
*/
#pragma once
#ifndef _BYTES_HPP_
#define _BYTES_HPP_

#include <type_traits>
#include <string.h>

#include "png.h"
#include "zlib.h"

class self_bytes {
protected:
    Byte* bytes;
public:
    self_bytes() noexcept : bytes(nullptr) {}
    // 无法动态绑定
    self_bytes(Byte*&& newBytes) noexcept : bytes(newBytes) {}
    self_bytes(self_bytes&& otherBytes) noexcept : bytes(otherBytes.bytes) {
        otherBytes.setBytes(nullptr);
    }
    self_bytes(const self_bytes& otherBytes) = delete;
    // self_bytes(const self_bytes& otherBytes) noexcept {
    //     copyBytes(otherBytes.bytes);
    // }
    virtual ~self_bytes() {
        // delete[] bytes;
        setBytes(nullptr);
    }

    // virtual self_bytes& operator=(Byte*&& newBytes) {
    //     delete[] bytes;
    //     // 移动赋值不需要触发更新
    //     bytes = newBytes;
    //     return *this;
    // }
    // 注意本类不持有bytes，只是使用
    virtual self_bytes& operator=(self_bytes&& otherBytes) noexcept {
        if(this != &otherBytes) {
            // operator=(std::move(otherBytes.bytes));
            setBytes(otherBytes.bytes);
            otherBytes.setBytes(nullptr);
        }
        return *this;
    }
    self_bytes& operator=(const self_bytes& otherBytes) = delete;
    // self_bytes& operator=(const self_bytes& otherBytes) noexcept {
    //     if(this != &otherBytes)
    //         copyBytes(otherBytes.bytes);
    //     return *this;
    // }
    void copyBytes(const Byte* const& bytesCopy, size_t num) noexcept {
        memcpy(bytes, bytesCopy, num);
    }
    void sendBytes(Byte* const& bytesTarget, size_t num) const noexcept {
        memcpy(bytesTarget, bytes, num);
    }
    virtual void setBytes(Byte* const& bytesTarget) noexcept {
        bytes = bytesTarget;
    }
    const Byte* const& getBytes() const noexcept {
        return bytes;
    }
    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    T getValue(const uint8_t& numByte) const noexcept {
        T value = 0;
        memcpy(&value, bytes, numByte);
        return value;
    }
};
using Bytes = self_bytes;

template<typename T, typename = std::enable_if_t<std::is_base_of_v<Bytes, T>>>
class self_pod_list : public Bytes {
private:
    uint32_t sizeChunk;
    uint32_t length;
    bool isOwner;
    T* chunks;
    void copyPOD(const self_pod_list& otherList) noexcept {
        sizeChunk = otherList.sizeChunk;
        length = otherList.length;
    }
public:
    self_pod_list() = delete;
    self_pod_list(Byte*&& newBytes) = delete;
    self_pod_list(uint32_t sizeCk, uint32_t l) : self_pod_list(sizeCk, l, new Byte[sizeCk * l]) {}
    self_pod_list(uint32_t sizeCk, uint32_t l, Byte*&& newBytes) noexcept : Bytes(std::move(newBytes)), sizeChunk(sizeCk), length(l), isOwner(true), chunks(new T[l]) {
        remap();
    }
    self_pod_list(self_pod_list&& otherList) noexcept : Bytes(std::move(otherList)), sizeChunk(otherList.sizeChunk), length(otherList.length), isOwner(otherList.isOwner), chunks(otherList.chunks) {
        otherList.chunks = nullptr;
    }
    self_pod_list(const self_pod_list& otherList) : Bytes(new Byte[otherList.sizeChunk * otherList.length]), sizeChunk(otherList.sizeChunk), length(otherList.length), isOwner(true), chunks(new T[otherList.length]) {
        copyBytes(otherList.bytes, sizeChunk * length);
        remap();
    }
    ~self_pod_list() {
        delete[] chunks;
        chunks = nullptr;
        selfResetBytes();
    }

    // self_pod_list& operator=(Byte*&& otherBytes) = delete;
    // self_pod_list& operator=(Byte*&& otherBytes) {
    //     throw std::logic_error("forbidden action : self_pod_list& operator=(Byte*&& otherBytes)");
    // }
    self_pod_list& operator=(self_pod_list&& otherList) {
        if(this != &otherList) {
            Bytes::operator=(std::move(otherList));
            copyPOD(otherList);
            delete[] chunks;
            chunks = otherList.chunks;
            otherList.chunks = nullptr;
        }
        return *this;
    }
    self_pod_list& operator=(const self_pod_list& otherList) {
        if(this != &otherList) {
            copyPOD(otherList);
            delete[] chunks;
            selfResetBytes();
            chunks = new T[length];
            uint32_t&& temp = sizeChunk * length;
            setBytes(new Byte[temp]);
            copyBytes(otherList.bytes, temp);
        }
        return *this;
    }
    void setBytes(Byte* const& bytesTarget) noexcept {
        Bytes::setBytes(bytesTarget);
        remap();
    }
    // 无法通过设置虚函数来禁止，因为本类实质是列表，本类对象getValue得到的值无意义
    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    T getValue() = delete;
    uint32_t size() const noexcept {
        return length;
    }
    void remap() noexcept {
        if(bytes) {
            Byte* temp = bytes;
            for(int i = 0;i < length;i ++) {
                chunks[i].setBytes(temp);
                temp += sizeChunk;
            }
        }
        else
            for(int i = 0;i < length;i ++)
                chunks[i].setBytes(nullptr);
    }
    void selfResetBytes() {
        if(isOwner)
            delete[] bytes;
        bytes = nullptr;
    }
    T* getChunk(uint32_t pos) const noexcept {
        return chunks + pos;
    }
};
template<typename T, typename = std::enable_if_t<std::is_base_of_v<Bytes, T>>>
using PODList = self_pod_list<T>;

#endif