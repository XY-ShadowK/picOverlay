#ifdef CMAKE_BUILD
#include "version.h"
#else
#include "compiler/version.h.in"
#endif

#include <iostream>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <map>
#include <bitset>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <cinttypes>

#include "Universe_Entry.hpp"
#include "cxxopts.hpp"

#include "include/pictures.hpp"
#include "include/pngs.hpp"

using std::string;
using std::to_string;
using std::stringstream;
using std::unordered_map;
using std::map;
using std::ifstream;
using std::getline;
using std::quoted;

const uint64_t MAX_PIXEL_DATA = 255;
const uint64_t MAX_INPUT_PIXEL_DATA = 255;
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

cxxopts::Options options("picOverlay", "图片叠加");
cxxopts::ParseResult settings;
string inPath, outPath;
unordered_map<string, PicData*> picDataStorage;
map<uint32_t, string> picLayers;
unordered_map<uint32_t, uint64_t> layerShow;
string lastPicArgs = "";
uint32_t numGeneratePics = 0;
ifstream fin;
stringstream sStream;

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
string num2leading0str(T num) {
    sStream.str("");
    sStream.clear();
    sStream << std::setw(2) << std::setfill('0') << num;
    return sStream.str();
}

void clearPicDataStorage() {
    unordered_map<string, PicData*>::iterator it = picDataStorage.begin();
    for(;it != picDataStorage.end();it ++) {
        delete it->second;
        it->second = nullptr;
    }
    picDataStorage.clear();
}

int parseArgs(int argc, char* const* const argv) {
    try {
        options
        .set_width(140)
        .add_options()
            ("i,input", "流程文件", cxxopts::value<string>()->default_value("./actions"), "ACTION_PROCESS_FILE")
            ("source", "图片读取源目录", cxxopts::value<string>()->default_value("."), "SOURCE_DIR")
            ("destination", "图片输出目标目录", cxxopts::value<string>()->default_value("."), "DESTINATION_DIR")
            ("buffer-x", "缓冲区宽度", cxxopts::value<size_t>()->default_value("3840"), "BUFFER_WIDTH")
            ("buffer-y", "缓冲区高度", cxxopts::value<size_t>()->default_value("3840"), "BUFFER_HEIGHT")
            ("custom-name", "启用生成文件名自定义")
            ("verbose", "显示详细信息")
            ("v,version", "打印当前版本")
            ("h,help", "打印帮助信息")
            // ("force-alpha", "强制启用alpha通道")
            // ("adam7", "启用隔行扫描（只对写入生效）")
            ;
        settings = options.parse(argc, argv);
        if(settings.count("help")) {
            printf(options.help().c_str());
            return 1;
        }
        if(settings.count("version"))
            printf("%s " APP_VERSION "\n", options.program().c_str());
        inPath = settings["source"].as<string>();
        outPath = settings["destination"].as<string>();
        if(settings.unmatched().size()) {
            fprintf(stderr, "%s can not recognize.\n", settings.unmatched()[0].c_str());
            return -1;
        }
    } catch (const cxxopts::exceptions::exception& e)
    {
        fprintf(stderr, "error parsing options: %s\n", e.what());
        return -1;
    }
    return 0;
}

void setLayerPicName(const uint32_t& layer, const string& picName) {
    // if(picLayers.find(layer) == picLayers.end())
    //     picLayers.emplace(layer, picName);
    // else
    //     picLayers[layer] = picName;
    map<uint32_t, string>::iterator hint = picLayers.lower_bound(layer);
    if(hint == picLayers.end() || picLayers.key_comp()(layer, hint->first))
        picLayers.emplace_hint(hint, layer, picName);
    else
        hint->second = picName;
}
bool setLayerPicData(const uint32_t& layer, const string& filename) {
    auto [it, inserted] = picDataStorage.try_emplace(filename, nullptr);
    if(inserted) {
        PicData* pPicData = readPng(inPath + '/' + filename);
        if(pPicData)
            it->second = pPicData;
        else
            return false;
    }
    return true;
}
bool setLayer(const uint32_t& layer, const string& filename) {
    if(!setLayerPicData(layer, filename))
        return false;
    setLayerPicName(layer, filename);
    // 已存在不透明度时，不改变原值
    layerShow.try_emplace(layer, 0);
    return true;
}
bool showLayer(const uint32_t& layer, const uint8_t& alpha) {
    // 检查是正确的，但为了速度，假设不会传入不存在的layer
    // if(layerShow.find(layer) == layerShow.end())
    //     return false;
    if constexpr (MAX_PIXEL_DATA == MAX_INPUT_PIXEL_DATA)
        layerShow[layer] = alpha;
    else
        layerShow[layer] = alpha * MAX_PIXEL_DATA / MAX_INPUT_PIXEL_DATA;
    return true;
}
bool hideLayer(const uint32_t& layer) {
    const unordered_map<uint32_t, uint64_t>::iterator& it = layerShow.find(layer);
    if(it == layerShow.end())
        return false;
    // layerShow[layer] = 0;
    it->second = 0;
    return true;
}
void clearLayer(const uint32_t& layer) {
    picLayers.erase(layer);
    // 不用清除图层显示情况
}
bool generatePicAndSave(const string& filename = num2leading0str(numGeneratePics) + ".png") {
    if(picLayers.empty()) return false;
    map<uint32_t, string>::iterator itLayer = picLayers.begin();
    printf("\ngenerate %u:\n", ++ numGeneratePics);
    string nowPicArgs = "";
    for(;itLayer != picLayers.end();itLayer ++) {
        const unordered_map<uint32_t, uint64_t>::const_iterator& itShow = layerShow.find(itLayer->first);
        if(itShow->second)
            nowPicArgs += to_string(itLayer->first) + '*' + itLayer->second + '*' + to_string(itShow->second) + " ? ";
    }
    if(nowPicArgs == lastPicArgs) {
        printf("same as the previous one.\n");
        numGeneratePics --;
        return false;
    }
    PicData picData;
    for(itLayer = picLayers.begin();itLayer != picLayers.end();itLayer ++) {
        const unordered_map<uint32_t, uint64_t>::const_iterator& itShow = layerShow.find(itLayer->first);
        if(itShow->second) {
            const unordered_map<string, PicData*>::const_iterator& itPic = picDataStorage.find(itLayer->second);
            if(itPic == picDataStorage.end()) return false;
            printf("Layer %u : %s Opacity : %llu\n", itLayer->first, itLayer->second.c_str(), itShow->second);
            if(picData.getStorageInit())
                picData.overlay(*itPic->second, itShow->second);
            else
                picData.copyData(*itPic->second, itShow->second);
        }
        else
            printf("Layer %u : %s Opacity : 0\n", itLayer->first, itLayer->second.c_str());
    }
    if(picData.getStorageInit()) {
        bool res = writePng(outPath + '/' + filename, &picData, colorTypes[picData.getGray()][picData.getAlpha()]);
        if(res) 
            lastPicArgs = nowPicArgs;
        else printf("failed!\n");
        return res;
    }
    numGeneratePics --;
    return false;
}

int resolve() {
    if(!fin.is_open()) {
        if(settings.count("version"))
            return 0;
        return -1;
    }
    printf("\n");

    string line, command, tempStr;
    stringstream ss;
    uint32_t t0, t1;
    unordered_map<string, uint32_t>::const_iterator it;
    init_pic_buffer(settings["buffer-x"].as<size_t>() * 8, settings["buffer-y"].as<size_t>() * 8);
    while(getline(fin, line)) {
        ss.str(line);
        ss.clear();
        if(settings["verbose"].as<bool>())
            printf("%s\n", line.c_str());
        ss >> command;
        const unordered_map<string, uint32_t>::const_iterator& it = cmdCode.find(command);
        if(it == cmdCode.end()) {
            fprintf(stderr, "参数错误！\n");
            return -1;
        }
        switch(it->second) {
            case 0 :
                ss >> t0 >> quoted(tempStr, '"', '\0');
                if(!setLayer(t0, tempStr)) return -1;
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
                if(settings["custom-name"].as<bool>()) {
                    ss >> quoted(tempStr, '"', '\0');
                    generatePicAndSave(tempStr);
                }
                else generatePicAndSave();
                break;
        }
    }
    return 0;
}

int resolveClear() {
    int ret = 0;
    ret = resolve();
    clearPicDataStorage();
    fin.close();
    return ret;
}

UNIVERSE_MAIN
    int ret = 0;
    if((ret = parseArgs(argc, argv)) == 0) {
        fin.open(settings["input"].as<string>(), std::ios::in);
        UNIVERSE_MAIN_RETURN(resolveClear());
    }
    if(ret < 0) {
        UNIVERSE_MAIN_RETURN(ret);
    }
    UNIVERSE_MAIN_RETURN(0);
UNIVERSE_MAINEND