#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <bitset>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <type_traits>

#include "windowsRub.hpp"
#include "cxxopts.hpp"

#include "include/pictures.hpp"
#include "include/pngs.hpp"

using std::cin;
using std::cerr;
using std::endl;
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
// const unordered_map<string, uint32_t> options{
//     {"-custom_name", 0},
//     {"-force_alpha", 1},
//     {"-adam7", 2}
// };
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

void parseArgs(int argc, char* argv[]) {
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
            ("h,help", "打印帮助信息")
            // ("force-alpha", "强制启用alpha通道")
            // ("adam7", "启用隔行扫描（只对写入生效）")
            ;
        settings = options.parse(argc, argv);
        if(settings.count("help")) {
            printf(options.help().c_str());
            exit(0);
        }
        inPath = settings["source"].as<string>();
        outPath = settings["destination"].as<string>();
        if(settings.unmatched().size()) {
            cerr<<settings.unmatched()[0]<<endl;
            exit(-1);
        }
    } catch (const cxxopts::exceptions::exception& e)
    {
        cerr<<"error parsing options: "<<e.what()<<endl;
        exit(-1);
    }
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
        PicData* pPicData = readPng(inPath + '/' + filename);
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
    // if(layerShow.find(layer) == layerShow.end())
    //     return false;
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
bool generatePicAndSave(const string& filename = num2leading0str(numGeneratePics) + ".png") {
    if(picLayers.empty()) return false;
    map<uint32_t, string>::iterator itLayer = picLayers.begin();
    unordered_map<uint32_t, uint64_t>::iterator itShow;
    printf("\ngenerate %u:\n", ++ numGeneratePics);
    string nowPicArgs = "";
    for(;itLayer != picLayers.end();itLayer ++) {
        itShow = layerShow.find(itLayer->first);
        if(itShow->second)
            nowPicArgs += to_string(itLayer->first) + '*' + itLayer->second + '*' + to_string(itShow->second) + " ? ";
    }
    if(nowPicArgs == lastPicArgs) {
        printf("same as the previous one.\n");
        numGeneratePics --;
        return false;
    }
    unordered_map<string, PicData*>::iterator itPic;
    PicData picData;
    for(itLayer = picLayers.begin();itLayer != picLayers.end();itLayer ++) {
        itShow = layerShow.find(itLayer->first);
        if(itShow->second) {
            itPic = picDataStorage.find(itLayer->second);
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
    if(!fin.is_open()) return -1;
    string line, command, tempStr;
    stringstream ss;
    uint32_t t0, t1;
    unordered_map<string, uint32_t>::const_iterator it;
    init_pic_buffer(settings["buffer-x"].as<size_t>() * 8, settings["buffer-y"].as<size_t>() * 8);
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
                if(settings["custom_name"].as<bool>()) {
                    ss >> quoted(tempStr, '"', '\0');
                    generatePicAndSave(tempStr);
                }
                else generatePicAndSave();
                break;
        }
    }
    return 0;
}

UNIVERSE_MAIN
    parseArgs(argc, argv);
    fin.open(settings["input"].as<string>(), std::ios::in);
    return resolve();
UNIVERSE_MAINEND