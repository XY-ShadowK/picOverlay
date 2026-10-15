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

**Command 2: `show layer opacity`**

**Command 3: `hide layer`**

**Command 4: `clear layer`**

**Command 5: `generate`**

Default `opacity`range: 0 ~ 255

### Simple Example

```
1920 1080 "D:/input pictures" "D/output pictures"
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

**Output:**

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
