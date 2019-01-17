/*******************************************************************************************
*
*   window_codegen v1.0.0 - Tool Description
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018 raylibtech
*
**********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

//----------------------------------------------------------------------------------
// Controls Functions Declaration
//----------------------------------------------------------------------------------
static void Button005();


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //---------------------------------------------------------------------------------------
    int screenWidth = 564;
    int screenHeight = 438;

    InitWindow(screenWidth, screenHeight, "window_codegen");

    // window_codegen: controls initialization
    //----------------------------------------------------------------------------------
    bool WindowBox000Active = true;
    bool Button005Pressed = false;
    bool LabelButton006Pressed = false;
    bool ImageButtonEx007Pressed = false;
    bool CheckBoxEx008Checked = false;
    bool Toggle009Active = true;
    int ToggleGroup010Active= 0;
    int ComboBox011Active= 0;
    bool DropdownBox012EditMode = false;
    int DropdownBox012Active = 0;
    bool TextBox013EditMode = false;
    unsigned char TextBox013Text[64] = "SAMPLE TEXT";
    bool TextmultiBox014EditMode = false;
    unsigned char TextmultiBox014Text[64] = "SAMPLE TEXT";
    bool ValueBOx015EditMode = false;
    int ValueBOx015Value = 0;
    bool Spinner016EditMode = false;
    int Spinner016Value = 0;
    float SliderEx017Value = 0.0f;
    float SliderBarEx018Value = 0.0f;
    float PrograssBarEx019Value = 0.0f;
    int ListView022ScrollIndex = 0;
    bool ListView022EditMode = false;
    int ListView022Active = 0;
    Color ColorPicker023Value;
    bool DropdownBox026EditMode = false;
    int DropdownBox026Active = 0;
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
static void Button005()
{
    // TODO: Implement control logic
}
