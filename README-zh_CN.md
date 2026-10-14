# 如何构建？

在根目录运行命令：

```Shell
mkdir build
cd build
cmake ..
cmake --build .
```

对于MSVC编译器，将 `cmake --build .` 改为 `cmake --build . --config release`.

如果你的系统没有安装libpng或zlib库，将 `cmake ..` 改为如下

**POWERSHELL（Windows终端 非CMD）**

```PowerShell
cmake .. `
    -DPNG_INCLUDE_DIR="{your libpng include dir}" `
    -DPNG_LIBRARY="{your libpng lib dir}" `
    -DZLIB_INCLUDE_DIR="{your zlib include dir}" `
    -DZLIB_LIBRARY="{your zlib lib dir}"
```

**SHELL（LINUX终端）**

```Shell
cmake .. \
    -DPNG_INCLUDE_DIR="{your libpng include dir}" \
    -DPNG_LIBRARY="{your libpng lib dir}" \
    -DZLIB_INCLUDE_DIR="{your zlib include dir}" \
    -DZLIB_LIBRARY="{your zlib lib dir}"
```

# 如何使用？

1. 打开一个终端或者命令行
2. cd到exe在的目录
3. 运行 `picOverlay.exe /?(或者 /help)` 来得到运行命令格式
   简单运行距举例： `picOverlay.exe -file=actions.txt`

## 如何编写执行脚本

### 第一行：

```
缓冲区宽度 缓冲区高度 输入文件路径 输出文件路径
```

### 接下来的行：

**命令 1: `set 图层编号 文件名`**

设置文件到编号图层

**命令 2: `clear 图层编号`**

清除编号对应图层

**命令 3: `generate 文件名`**

生成目标文件

### 简单样例

```
1920 1080 "D:/输入 图片" "D/输出 图片"
set 1 a.png
set 2 b.png
set 3 c.png
generate 01.png
set 2 d.png
generate 02.png
clear 2
generate 03.png
```

**输出：**

```
generate: 01.png
Layer 1 a.png
Layer 2 b.png
Layer 3 c.png

generate: 02.png
Layer 1 a.png
Layer 2 d.png
Layer 3 c.png

generate: 03.png
Layer 1 a.png
Layer 3 c.png
```

成功生成了01~03.png

支持中文路径，但只测试了windows。。。
