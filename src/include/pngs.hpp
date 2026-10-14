/*
time: 2026/10/14 22:27:22
*/
#pragma once
#ifndef _PNGS_HPP_
#define _PNGS_HPP_

#include <string>

#include "include/file.h"
#include "include/pictures.hpp"

bool getPng(const std::string& filename, PngInfo*& pPngInfo);
bool setPng(const std::string& filename, PngInfo*& pPngInfo);
PicData* readPng(const std::string& filename);
bool writePng(const std::string& filename, PicData* pPicData, png_byte colorType = PNG_COLOR_TYPE_RGB_ALPHA);

#endif