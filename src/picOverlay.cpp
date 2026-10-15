#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <bitset>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "include/pictures.hpp"
#include "include/pngs.hpp"

using std::cin;
using std::cerr;
using std::endl;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::map;
using std::ifstream;
using std::getline;
using std::quoted;

const uint64_t MAX_PIXEL_DATA = 255;
const uint64_t MAX_INPUT_PIXEL_DATA = 255;
const unordered_map<string, uint32_t> options{
    {"-force_alpha", 0},
    {"-adam7", 1}
};
const png_byte colorTypes[2][2] = {
    {PNG_COLOR_TYPE_RGB, PNG_COLOR_TYPE_RGB_ALPHA},
    {PNG_COLOR_TYPE_GRAY, PNG_COLOR_TYPE_GRAY_ALPHA}
};
const unordered_map<string, uint32_t> cmdCode{
    {"set", 0},
    {"show", 1},
    {"hide", 2},
    {"clear", 3},
    {"generate", 4}
};
std::bitset<8> settings;
string inPath, outPath;
unordered_map<string, PicData*> picDataStorage;
map<uint32_t, string> picLayers;
unordered_map<uint32_t, uint64_t> layerShow;
ifstream fin;

void printHelp() {
    printf("picOverlay -file=[filename]\n\n");
    // printf("-force_alpha 强制启用alpha通道\n");
    // printf("-adam7 启用隔行扫描（只对写入生效）\n");
}

void setLayer(const uint32_t& layer, const string& picName) {
    if(picLayers.find(layer) == picLayers.end())
        picLayers.emplace(layer, picName);
    else
        picLayers[layer] = picName;
}
bool setLayer(const string& filename, const uint32_t& layer) {
    unordered_map<string, PicData*>::const_iterator it;
    if((it = picDataStorage.find(filename)) == picDataStorage.end()) {
        PicData* pPicData = readPng(inPath + '\\' + filename);
        if(pPicData)
            picDataStorage.emplace(filename, pPicData);
        else
            exit(-1);
    }
    setLayer(layer, filename);
    // 已存在不透明度时，不改变原值
    layerShow.emplace(layer, 0);
    return true;
}
bool showLayer(const uint32_t& layer, const uint8_t& alpha) {
    if(layerShow.find(layer) == layerShow.end())
        return false;
    if constexpr (MAX_PIXEL_DATA == MAX_INPUT_PIXEL_DATA)
        layerShow[layer] = alpha;
    else
        layerShow[layer] = alpha * MAX_PIXEL_DATA / MAX_INPUT_PIXEL_DATA;
    return true;
}
bool hideLayer(const uint32_t& layer) {
    if(layerShow.find(layer) == layerShow.end())
        return false;
    layerShow[layer] = 0;
    return true;
}
void clearLayer(const uint32_t& layer) {
    picLayers.erase(layer);
    // 不用清除图层显示情况
}
bool generatePicAndSave(const string& filename) {
    if(picLayers.empty()) return false;
    map<uint32_t, string>::iterator itLayer = picLayers.begin();
    unordered_map<string, PicData*>::iterator itPic = picDataStorage.find(itLayer->second);
    printf("\ngenerate: %s\n", filename.c_str());
    PicData picData;
    unordered_map<uint32_t, uint64_t>::iterator itShow;
    for(;!picData.getStorageInit();itLayer ++) {
        itShow = layerShow.find(itLayer->first);
        if(itShow->second) {
            itPic = picDataStorage.find(itLayer->second);
            if(itPic == picDataStorage.end()) return false;
            printf("Layer %u : %s Opacity : %llu\n", itLayer->first, itLayer->second.c_str(), itShow->second);
            picData.copyData(*itPic->second, itShow->second);
            itLayer ++;
            break;
        }
        printf("Layer %u : %s Opacity : 0\n", itLayer->first, itLayer->second.c_str());
    }
    for(;itLayer != picLayers.end();itLayer ++) {
        itShow = layerShow.find(itLayer->first);
        if(itShow->second) {
            itPic = picDataStorage.find(itLayer->second);
            if(itPic == picDataStorage.end()) return false;
            if(itShow->second) {
                printf("Layer %u : %s Opacity : %llu\n", itLayer->first, itLayer->second.c_str(), itShow->second);
                picData.overlay(*itPic->second, itShow->second);
            }
        }
        else
            printf("Layer %u : %s Opacity : 0\n", itLayer->first, itLayer->second.c_str());
    }
    return writePng(outPath + '\\' + filename, &picData, colorTypes[picData.getGray()][picData.getAlpha()]);
}

int resolve() {
    if(!fin.is_open()) return -1;
    string line, command, tempStr;
    stringstream ss;
    uint32_t t0, t1;
    unordered_map<string, uint32_t>::const_iterator it;
    getline(fin, line);
    ss.str(line);
    ss >> t0 >> t1 >> quoted(inPath, '"', '\0') >> quoted(outPath, '"', '\0');
    init_pic_buffer(t0 * 8, t1 * 8);
    while(getline(fin, line)) {
        ss.str(line);
        ss.clear();
        ss >> command;
        if((it = cmdCode.find(command)) == cmdCode.end()) {
            cerr<<"参数错误！"<<endl;
            return -1;
        }
        switch(it->second) {
            case 0 :
                ss >> t0 >> quoted(tempStr, '"', '\0');
                if(!setLayer(tempStr, t0)) return -1;
                break;
            case 1 :
                ss >> t0 >> t1;
                if(!showLayer(t0, t1)) return -1;
                break;
            case 2 :
                ss >> t0;
                if(!hideLayer(t0)) return -1;
                break;
            case 3 :
                ss >> t0;
                clearLayer(t0);
                break;
            case 4 :
                ss >> quoted(tempStr, '"', '\0');
                generatePicAndSave(tempStr);
                break;
        }
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if(argc <= 0) printHelp();
    if(!strcmp(argv[1], "/?") || !strcmp(argv[1], "/help")) {
        printHelp();
        return 0;
    }
    unordered_map<string, uint32_t>::const_iterator it;
    for(int i = 1;i < argc;i ++) {
        if((it = options.find(argv[i])) == options.end()) {
            if(strncmp(argv[i], "-file=", 6)) {
                cerr<<"参数错误！"<<endl;
                return -1;
            }
            fin.open(argv[i] + 6, std::ios::in);
            continue;
        }
        settings.set(it->second);
    }
    return resolve();
}