# How to build?

Run command in the root dir:

```Shell
mkdir build
cd build
cmake ..
cmake --build .
```

For MSVC, change `cmake --build .` to `cmake --build . --config release`.

If your system does not have libpng or(and) zlib installed, change `cmake ..` to below.

**POWERSHELL**

```PowerShell
cmake .. `
    -DPNG_INCLUDE_DIR="{your libpng include dir}" `
    -DPNG_LIBRARY="{your libpng lib dir}" `
    -DZLIB_INCLUDE_DIR="{your zlib include dir}" `
    -DZLIB_LIBRARY="{your zlib lib dir}"
```

**SHELL**

```Shell
cmake .. \
    -DPNG_INCLUDE_DIR="{your libpng include dir}" \
    -DPNG_LIBRARY="{your libpng lib dir}" \
    -DZLIB_INCLUDE_DIR="{your zlib include dir}" \
    -DZLIB_LIBRARY="{your zlib lib dir}"
```

# How to use?

1. Open shell/cmd/...
2. change dir to exe in
3. run picOverlay.exe /?(or /help) to get run command format
   simple running eg: `picOverlay.exe -file=actions.txt`

## How to write action script?

### First Line:

```
bufferWidth bufferHeight inputPath outputPath
```

### Lines between:

**Command 1: `set layer filename`**

**Command 2: `clear layer`**

**Command 3: `generate filename`**

### Simple Example

```
1920 1080 "D:/input pictures" "D/output pictures"
set 1 a.png
set 2 b.png
set 3 c.png
generate 01.png
set 2 d.png
generate 02.png
clear 2
generate 03.png
```

**Output:**

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
