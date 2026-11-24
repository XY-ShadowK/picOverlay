# picOverlay

English | [简体中文](README-zh_CN.md)

Batch generate images with multiple layers overlayed, with adjustable transparency, currently only supports PNG format.

## How to build?

Run command in the root dir:

```Shell
mkdir build
cmake -B build
cmake --build ./build
```

For MSVC, change `cmake --build ./build` to `cmake --build ./build --config release`.

If your system does not have windowsRub, cxxopts, libpng, zlib installed, change `cmake -B build` to below.

**POWERSHELL**

```PowerShell
cmake -B build `
	-DWINDOWS_RUB_INCLUDE="{your windowsRub include dir}" `
	-DCXXOPTS_ROOT="{your cxxopts root dir}" `
    -DPNG_INCLUDE_DIR="{your libpng include dir}" `
    -DPNG_LIBRARY="{your libpng lib dir}" `
    -DZLIB_INCLUDE_DIR="{your zlib include dir}" `
    -DZLIB_LIBRARY="{your zlib lib dir}"
```

**SHELL**

```Shell
cmake -B build \
	-DWINDOWS_RUB_INCLUDE="{your windowsRub include dir}" \
	-DCXXOPTS_ROOT="{your cxxopts root dir}" \
    -DPNG_INCLUDE_DIR="{your libpng include dir}" \
    -DPNG_LIBRARY="{your libpng lib dir}" \
    -DZLIB_INCLUDE_DIR="{your zlib include dir}" \
    -DZLIB_LIBRARY="{your zlib lib dir}"
```

## How to use?

1. Open shell/cmd/...
2. Change dir to executable file in
3. Run `picOverlay -h(or --help)` to get help information
   Simple Running e.g. : `picOverlay -file=actions.txt`

### Option List:

* [`-i, --input`](OPTIONS.md##-i,---input) : Set action process filename
* [`--source`](OPTIONS.md##--source) : Set read directory
* [`--destination`](OPTIONS.md##--destination) : Set output directory
* [`--buffer-x`](OPTIONS.md##--buffer-x) : Set buffer width
* [`--buffer-y`](OPTIONS.md##--buffer-y) : Set buffer height
* [`--custom-name`](OPTIONS.md##--custom-name) : Turn on filename custom mode
* [`--verbose`](OPTIONS.md##--verbose) : Enable print details
* [`-v, -version`](OPTIONS.md##-v,---version) : Print version information
* [`-h, --help`](OPTIONS.md##-h,---help) : Print help information

### How to write action script?

#### One command per line:

**Command 1: `set layer filename`**

**Command 2: `show layer opacity`**

**Command 3: `hide layer`**

**Command 4: `clear layer`**

**Command 5: `generate`**

Default `opacity`range: 0 ~ 255

#### Simple Example

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
