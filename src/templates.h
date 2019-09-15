
static const unsigned char *guiTemplateStandardCode = "\
/*******************************************************************************************\n\
*\n\
*   $(GUILAYOUT_NAME_PASCALCASE) v$(GUILAYOUT_VERSION) - $(GUILAYOUT_DESCRIPTION)\n\
*\n\
*   LICENSE: Propietary License\n\
*\n\
*   Copyright (c) 2019 $(GUILAYOUT_COMPANY). All Rights Reserved.\n\
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
*   Copyright (c) 2019 $(GUILAYOUT_COMPANY). All Rights Reserved.\n\
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
