# rGuiLayout v2.1

A simple and easy-to-use raygui layouts editor.

Useful for fast tools development. The perfect companion for rGuiStyler tool.

rGuiLayout comes in two flavours: ZERO (free version) and ONE (low-cost version).

NOTE: This tool is itended to be used with raygui 2.6: https://github.com/raysan5/raygui

## rGuiLayout ZERO features

 - +25 controls available to define your gui layout
 - Place controls visually and move/scale them freely
 - Controls text edition and name edition (for exported variables)
 - Snap to grid mode for maximum precission
 - Anchors mode support to link your controls
 - Save/Load your layout as .rgl files
 - Generate ready-to-use C code (limited to 12 controls)
 - Tracemap image loading (.png) for reference
 - 8 sample layout provided for reference
 - Completely portable (single file)

## rGuiLayout ONE plus features

 - Powerful command line for batch conversion of rgl layouts
 - Import .rgs files to customize visual style (drag&drop)
 - Generate ready-to-use C code for layout (no controls limit)
 - Choose between different code generation templates
 
## rGuiLayout Usage

The tool is pretty intuitive to use, it mostly works with mouse and several key shortcuts that can be seen just pressing F1 key.
Use mouse wheel to navigate available controls or select them on right controls panel, place them and scale if required.

rGuiLayout allows loading .rgl layout files with drag&drop.

rGuiLayout ONE also allows loading .rgs style files created with rGuiStyler and export created layouts as C code with no controls limit.

Press LCTRL+ENTER to show code generation window, you can configure multiple export options to generate the code. 
To compile generated code, you need raygui v2.6 library (single-file header-only) and raylib v2.6-dev library.

rGuiLayout ONE comes with a powerful command line for layout to code generation.
To view all available operation just run on command line:

 > rguilayout.exe --help

## rGuiLayout Custom
 
Do you like this tool? Do you need any specific feature to be added? Maybe a custom export format? 
Maybe a custom gui theme? Or maybe a version for Linux, OSX, Android, Raspberry Pi or HTML5?
Just get in touch: ray@raylibtech.com

## rGuiStyler Issues & Feedback

You can report tool issues and feedback here: https://github.com/raylibtech/rtools  

## rGuiLayout License

rGuiLayout is closed source software but it's based on open-source technologies: 
 - raylib (github.com/raysan5/raylib)
 - raygui (github.com/raysan5/raygui).

rGuiLayout ZERO is free and rGuiLayout ONE is low-cost, with the addition of some useful features. 
In any case, consider it as a small donation to help the author keep working on software for games development.

Copyright (c) 2017-2019 raylib technologies (@raylibtech). All Rights Reserved.
