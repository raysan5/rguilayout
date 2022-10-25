/*******************************************************************************************
*
*   rGuiLayout - Layout Code Generator Templates
*
*   DESCRIPTION:
*       Templates used to generate layout code files (.c/.h) replacing some variables
* 
*   CODEGEN SUPPORTED VARIABLES:
* 
*       > Layout general description variables
*           $(GUILAYOUT_NAME)
*               $(GUILAYOUT_NAME_PASCALCASE)
*               $(GUILAYOUT_NAME_UPPERCASE)
*               $(GUILAYOUT_NAME_LOWERCASE)
*           $(GUILAYOUT_VERSION)
*           $(GUILAYOUT_DESCRIPTION)
*           $(GUILAYOUT_COMPANY)
*           $(GUILAYOUT_WINDOW_WIDTH)
*           $(GUILAYOUT_WINDOW_HEIGHT)
*
*       > Layout C file (.c) data generation variables:
*           $(GUILAYOUT_FUNCTION_DECLARATION_C)
*           $(GUILAYOUT_FUNCTION_DEFINITION_C)
*           $(GUILAYOUT_INITIALIZATION_C)
*           $(GUILAYOUT_DRAWING_C)
*
*       > Layout Header file (.h) data generation variables:
*           $(GUILAYOUT_STRUCT_TYPE)
*           $(GUILAYOUT_FUNCTIONS_DECLARATION_H)
*           $(GUILAYOUT_FUNCTION_INITIALIZE_H)
*           $(GUILAYOUT_FUNCTION_DRAWING_H)
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018-2022 raylib technologies (@raylibtech) / Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

// Standard C file template
static const unsigned char *guiTemplateStandardCode = "\
/*******************************************************************************************\n\
*\n\
*   $(GUILAYOUT_NAME_PASCALCASE) v$(GUILAYOUT_VERSION) - $(GUILAYOUT_DESCRIPTION)\n\
*\n\
*   LICENSE: Propietary License\n\
*\n\
*   Copyright (c) 2022 $(GUILAYOUT_COMPANY). All Rights Reserved.\n\
*\n\
*   Unauthorized copying of this file, via any medium is strictly prohibited\n\
*   This project is proprietary and confidential unless the owner allows\n\
*   usage in any other form by expresely written permission.\n\
*\n\
**********************************************************************************************/\n\
\n\
#include \"raylib.h\"\n\
\n\
#define RAYGUI_IMPLEMENTATION\n\
#include \"raygui.h\"\n\
\n\
//----------------------------------------------------------------------------------\n\
// Controls Functions Declaration\n\
//----------------------------------------------------------------------------------\n\
$(GUILAYOUT_FUNCTION_DECLARATION_C)\n\
\n\
//------------------------------------------------------------------------------------\n\
// Program main entry point\n\
//------------------------------------------------------------------------------------\n\
int main()\n\
{\n\
    // Initialization\n\
    //---------------------------------------------------------------------------------------\n\
    int screenWidth = $(GUILAYOUT_WINDOW_WIDTH);\n\
    int screenHeight = $(GUILAYOUT_WINDOW_HEIGHT);\n\
\n\
    InitWindow(screenWidth, screenHeight, \"$(GUILAYOUT_NAME)\");\n\
\n\
    // $(GUILAYOUT_NAME): controls initialization\n\
    //----------------------------------------------------------------------------------\n\
    $(GUILAYOUT_INITIALIZATION_C)\n\
    //----------------------------------------------------------------------------------\n\
\n\
    SetTargetFPS(60);\n\
    //--------------------------------------------------------------------------------------\n\
\n\
    // Main game loop\n\
    while (!WindowShouldClose())    // Detect window close button or ESC key\n\
    {\n\
        // Update\n\
        //----------------------------------------------------------------------------------\n\
        // TODO: Implement required update logic\n\
        //----------------------------------------------------------------------------------\n\
\n\
        // Draw\n\
        //----------------------------------------------------------------------------------\n\
        BeginDrawing();\n\
\n\
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); \n\
\n\
            // raygui: controls drawing\n\
            //----------------------------------------------------------------------------------\n\
            $(GUILAYOUT_DRAWING_C)\n\
            //----------------------------------------------------------------------------------\n\
\n\
        EndDrawing();\n\
        //----------------------------------------------------------------------------------\n\
    }\n\
\n\
    // De-Initialization\n\
    //--------------------------------------------------------------------------------------\n\
    CloseWindow();        // Close window and OpenGL context\n\
    //--------------------------------------------------------------------------------------\n\
\n\
    return 0;\n\
}\n\
\n\
//------------------------------------------------------------------------------------\n\
// Controls Functions Definitions (local)\n\
//------------------------------------------------------------------------------------\n\
$(GUILAYOUT_FUNCTION_DEFINITION_C)\n\
";

// Header-only code file template
static const unsigned char *guiTemplateHeaderOnly = "\
/*******************************************************************************************\n\
*\n\
*   $(GUILAYOUT_NAME_PASCALCASE) v$(GUILAYOUT_VERSION) - $(GUILAYOUT_DESCRIPTION)\n\
*\n\
*   MODULE USAGE:\n\
*       #define GUI_$(GUILAYOUT_NAME_UPPERCASE)_IMPLEMENTATION\n\
*       #include \"gui_$(GUILAYOUT_NAME).h\"\n\
*\n\
*       INIT: Gui$(GUILAYOUT_NAME_PASCALCASE)State state = InitGui$(GUILAYOUT_NAME_PASCALCASE)();\n\
*       DRAW: Gui$(GUILAYOUT_NAME_PASCALCASE)(&state);\n\
*\n\
*   LICENSE: Propietary License\n\
*\n\
*   Copyright (c) 2022 $(GUILAYOUT_COMPANY). All Rights Reserved.\n\
*\n\
*   Unauthorized copying of this file, via any medium is strictly prohibited\n\
*   This project is proprietary and confidential unless the owner allows\n\
*   usage in any other form by expresely written permission.\n\
*\n\
**********************************************************************************************/\n\
\n\
#include \"raylib.h\"\n\
\n\
// WARNING: raygui implementation is expected to be defined before including this header\n\
#undef RAYGUI_IMPLEMENTATION\n\
#include \"raygui.h\"\n\
\n\
#include <string.h>     // Required for: strcpy()\n\
\n\
#ifndef GUI_$(GUILAYOUT_NAME_UPPERCASE)_H\n\
#define GUI_$(GUILAYOUT_NAME_UPPERCASE)_H\n\
\n\
$(GUILAYOUT_STRUCT_TYPE)\n\
\n\
#ifdef __cplusplus\n\
extern \"C\" {            // Prevents name mangling of functions\n\
#endif\n\
\n\
//----------------------------------------------------------------------------------\n\
// Defines and Macros\n\
//----------------------------------------------------------------------------------\n\
//...\n\
\n\
//----------------------------------------------------------------------------------\n\
// Types and Structures Definition\n\
//----------------------------------------------------------------------------------\n\
// ...\n\
\n\
//----------------------------------------------------------------------------------\n\
// Module Functions Declaration\n\
//----------------------------------------------------------------------------------\n\
$(GUILAYOUT_FUNCTIONS_DECLARATION_H)\n\
\n\
#ifdef __cplusplus\n\
}\n\
#endif\n\
\n\
#endif // GUI_$(GUILAYOUT_NAME_UPPERCASE)_H\n\
\n\
/***********************************************************************************\n\
*\n\
*   GUI_$(GUILAYOUT_NAME_UPPERCASE) IMPLEMENTATION\n\
*\n\
************************************************************************************/\n\
#if defined(GUI_$(GUILAYOUT_NAME_UPPERCASE)_IMPLEMENTATION)\n\
\n\
#include \"raygui.h\"\n\
\n\
//----------------------------------------------------------------------------------\n\
// Global Variables Definition\n\
//----------------------------------------------------------------------------------\n\
//...\n\
\n\
//----------------------------------------------------------------------------------\n\
// Internal Module Functions Definition\n\
//----------------------------------------------------------------------------------\n\
//...\n\
\n\
//----------------------------------------------------------------------------------\n\
// Module Functions Definition\n\
//----------------------------------------------------------------------------------\n\
$(GUILAYOUT_FUNCTION_INITIALIZE_H)\n\
\n\
$(GUILAYOUT_FUNCTION_DRAWING_H)\n\
\n\
#endif // GUI_$(GUILAYOUT_NAME_UPPERCASE)_IMPLEMENTATION\n\
";
