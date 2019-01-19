/*******************************************************************************************
*
*   $(GUILAYOUT_NAME) v$(GUILAYOUT_VERSION) - $(GUILAYOUT_DESCRIPTION)
*
*   LICENSE: Propietary License
*
*   Copyright (c) 2018 raylib technologies (@raylibtech). All Rights Reserved.
*
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   This project is proprietary and confidential unless the owner allows
*   usage in any other form by expresely written permission.
*
**********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

//----------------------------------------------------------------------------------
// Controls Functions Declaration
//----------------------------------------------------------------------------------
$(GUILAYOUT_FUNCTION_DECLARATION_C)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //---------------------------------------------------------------------------------------
    int screenWidth = $(GUILAYOUT_WINDOW_WIDTH);
    int screenHeight = $(GUILAYOUT_WINDOW_HEIGHT);

    InitWindow(screenWidth, screenHeight, "$(GUILAYOUT_NAME)");

    // $(GUILAYOUT_NAME): controls initialization
    //----------------------------------------------------------------------------------
    $(GUILAYOUT_INITIALIZATION_C)
    //----------------------------------------------------------------------------------

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Implement required update logic
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(GetColor(style[DEFAULT_BACKGROUND_COLOR]));

            // raygui: controls drawing
            //----------------------------------------------------------------------------------
            $(GUILAYOUT_DRAWING_C)
            //----------------------------------------------------------------------------------

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Controls Functions Definitions (local)
//------------------------------------------------------------------------------------
$(GUILAYOUT_FUNCTION_DEFINITION_C)