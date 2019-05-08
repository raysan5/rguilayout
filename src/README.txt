# rGuiLayout v2.0

A simple and easy-to-use raygui layouts editor.

Useful for fast tools development. The perfect companion for rGuiStyler tool.

rGuiLayout comes in two flavours: ZERO (free version) and ONE (low-cost version).

## rGuiLayout ZERO features

 - +25 controls available to define your gui layout
 - Place controls visually and move/scale them freely
 - Snap to grid mode for maximum precission
 - Save/Load your layout as .rgl files
 - Supports image loading to be used as tracemap
 - Anchors mode support to link your controls
 - Controls text edition and name edition (for exported variables)
 - +10 predefined layout available for reference
 - Completely portable (single file)

## rGuiLayout ONE plus features

 - Powerful command line for batch conversion of rgl layouts
 - Import .rgs files to customize visual style (drag&drop)
 - Export layout directly as plain C code, ready to edit and compile
 
## rGuiLayout Usage

The tool is pretty intuitive to use, it mostly works with mouse and several key shortcuts that can be seen just pressing F1 key.
Use mouse wheel to navigate available controls or select them on right controls panel, place them and scale if required.

rGuiLayout allows loading .rgl layout files with drag&drop.

rGuiLayout ONE also allows loading .rgs style files created with rGuiStyler and export created layouts as C code.

Press LCTRL+ENTER to show code generation window, you can configure multiple export options to generate the code. To compile generated code, you need raygui v2.0 library (single-file header-only) and raylib v2.5 library.

rGuiLayout ONE comes with a powerful command line for layout to code generation. Just type on command line:

 > rguilayout.exe --help

## rGuiLayout License

Copyright (c) 2016-2019 raylib technologies ([@raylibtech](https://twitter.com/raylibtech)). All Rights Reserved.

In case you want a custom version of the tool, just get in touch: ray[at]raylibtech.com
