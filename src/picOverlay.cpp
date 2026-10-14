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

const unordered_map<string, uint32_t> options{
    {"-force_alpha", 0},
    {"-adam7", 1}
};
const png_byte colorTypes[2][2] = {
    {PNG_COLOR_TYPE_RGB, PNG_COLOR_TYPE_RGB_ALPHA},
    {PNG_COLOR_TYPE_GRAY, PNG_COLOR_TYPE_GRAY_ALPHA}
};
std::bitset<8> settings;
string inPath, outPath;
unordered_map<string, PicData*> picDataStorage;
const unordered_map<string, uint32_t> cmdCode{
    {"set", 0},
    {"clear", 1},
    {"generate", 2}
};
map<uint32_t, string> picLayers;
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
    return true;
}
void clearLayer(const uint32_t& layer) {
    picLayers.erase(layer);
}
bool generatePicAndSave(const string& filename) {
    if(picLayers.empty()) return false;
    map<uint32_t, string>::iterator itLayer = picLayers.begin();
    unordered_map<string, PicData*>::iterator itPic = picDataStorage.find(itLayer->second);
    if(itPic == picDataStorage.end()) return false;
    printf("\ngenerate: %s\nLayer %d : %s\n", filename.c_str(), itLayer->first, itLayer->second.c_str());
    PicData picData(*itPic->second);
    itLayer ++;
    for(;itLayer != picLayers.end();itLayer ++) {
        itPic = picDataStorage.find(itLayer->second);
        if(itPic == picDataStorage.end()) return false;
        printf("Layer %d : %s\n", itLayer->first, itLayer->second.c_str());
        picData.overlay(*itPic->second);
    }
    return writePng(outPath + '\\' + filename, &picData, colorTypes[picData.isGray][picData.hasAlpha]);
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
                ss >> t0 >> quoted(tempStr);
                if(!setLayer(tempStr, t0)) return -1;
                break;
            case 1 :
                ss >> t0;
                clearLayer(t0);
                break;
            case 2 :
                ss >> quoted(tempStr);
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