# picOverlay

[English](README.md) | 简体中文

批量生成多图层叠加的图片，可调整透明度，目前只支持png格式。

## 如何构建？

在根目录运行命令：

```Shell
mkdir build
cmake -B build
cmake --build ./build
```

对于MSVC编译器，将 `cmake --build ./build` 改为 `cmake --build ./build --config release`。

如果你的系统没有安装windowsRub、cxxopts、libpng、zlib等库，将 `cmake -B build` 改为如下

**POWERSHELL（Windows终端 非CMD）**

```PowerShell
cmake -B build `
	-DWINDOWS_RUB_INCLUDE="{你的 windowsRub include 目录}" `
	-DCXXOPTS_ROOT="{你的 cxxopts 根目录}" `
    -DPNG_INCLUDE_DIR="{你的 libpng include 目录}" `
    -DPNG_LIBRARY="{你的 libpng lib 文件}" `
    -DZLIB_INCLUDE_DIR="{你的 zlib include 目录}" `
    -DZLIB_LIBRARY="{你的 zlib lib 文件}"
```

**SHELL（LINUX终端）**

```Shell
cmake -B build \
	-DWINDOWS_RUB_INCLUDE="{你的 windowsRub include 目录}" \
	-DCXXOPTS_ROOT="{你的 cxxopts 根目录}" \
    -DPNG_INCLUDE_DIR="{你的 libpng include 目录}" \
    -DPNG_LIBRARY="{你的 libpng lib 文件}" \
    -DZLIB_INCLUDE_DIR="{你的 zlib include 目录}" \
    -DZLIB_LIBRARY="{你的 zlib lib 文件}"
```

## 如何使用？

1. 打开一个终端或者命令行
2. cd到可执行文件（windows下为.exe linux下无后缀）所在的目录
3. 运行 `picOverlay -h(或 --help)` 来得到帮助信息
   简单运行示例： `picOverlay -file=actions.txt`

### 选项列表：

* [`-i, --input`](OPTIONS-zh_CN.md##-i,---input) : 设置流程文件名
* [`--source`](OPTIONS-zh_CN.md##--source) : 设置图片读取目录
* [`--destination`](OPTIONS-zh_CN.md##--destination) : 设置图片生成目录
* [`--buffer-x`](OPTIONS-zh_CN.md##--buffer-x) : 设置缓冲区宽度
* [`--buffer-y`](OPTIONS-zh_CN.md##--buffer-y) : 设置缓冲区高度
* [`--custom-name`](OPTIONS-zh_CN.md##--custom-name) : 开启生成文件名自定义模式
* [`-h, --help`](OPTIONS-zh_CN.md##-h,---help) : 打印帮助信息

### 如何编写执行脚本

#### 每个命令一行：

**命令 1：**`set 图层编号 文件名`
设置文件到编号图层

**命令 2：**`show 图层编号 不透明度`
设置编号对应图层不透明度

**命令 3：**`hide 图层编号`
隐藏编号对应图层（等价于show 编号 0）

**命令 4：**`clear 图层编号`
清除编号对应图层

**命令 5：**`generate`
生成目标文件

默认`不透明度`范围：0 ~ 255

#### 简单样例

```
set 1 a.png
show 1 255
set 2 b.png
show 2 255
set 3 c.png
generate
set 2 d.png
generate
clear 2
show 3 255
generate
```

**输出：**

```
generate 1:
Layer 1 a.png Opacity : 255
Layer 2 b.png Opacity : 255
Layer 3 c.png Opacity : 0

generate 2:
Layer 1 a.png Opacity : 255
Layer 2 d.png Opacity : 255
Layer 3 c.png Opacity : 0

generate 3:
Layer 1 a.png Opacity : 255
Layer 3 c.png Opacity : 255
```

成功生成了01~03.png

支持中文路径，有问题反馈
