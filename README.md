<img align="left" src="logo/rguilayout_256x256.png" width=256>

# rGuiLayout
A simple and easy-to-use raygui layouts editor. 

Useful for fast tools development. The perfect companion for [rGuiStyler](https://github.com/raysan5/rguistyler) tool.

<br>
<br>
<br>
<br>

## Features

 - **+25 controls available to define your imgui layout**
 - Place controls visually and move/scale them freely
 - Snap to grid mode for maximum precission
 - Save/Load your layout as **.rgl (text or binary mode)**
 - Import **.rgs** files ([rGuiStyler](https://github.com/raysan5/rguistyler)) to customize visual style (drag&drop)
 - **Export layout directly as plain C code, ready to edit and compile!**
 - Multiple code generation options to customize exported code
 - Supports image loading to be used as tracemap
 - Autovisible right controls panel for quick selection
 - Anchors mode support to link your controls
 - Controls text edition and name edition (for exported variables)
 - **+10 predefined layout available for reference**
 - **Completely portable (single file)**
 - Free and open source
 
## Usage
The tool is pretty intuitive to use, it mostly works with mouse and several key shortcuts that can be seen just pressing TAB key once program is open. Use mouse wheel to select desired control or RightMouseButton to open controls panel.

You can drag&drop .rgl files for layout loading and also .rgs files (rGuiStyler) for layout style loading. To export created layout as C code, just pres LCTRL+ENTER, you can configure multiple export options to generate the code.

To compile the code, you need raygui library (single-file header-only) and raylib v2.0 library, not yet released on itch.io but already available for early adopters on raylib GitHub, ready to be compiled.
 
## Screenshots

![rGuiLayout](screenshots/rguilayout_v100_shot03.png)

## License

rGuiLayout is **open source software**. rGuiLayout source code is licensed under an unmodified [zlib/libpng license](LICENSE).

Despite being open source and free for everybody willing to compile it, [consider a small donation](https://www.paypal.com/donate/?token=Oo3h2bHS9ux4lNzahXhmH3oaAqQpBeeMy2zNoH1lxHuD5Vdi1-B6XlPRlEyEzLmBb7rGwG) for the development efforts or [contributing to raylib patreon](https://www.patreon.com/raylib) to help the author keep working on open source software for games development.

*Copyright (c) 2016-2018 Ramon Santamaria ([@raysan5](https://twitter.com/raysan5))*
