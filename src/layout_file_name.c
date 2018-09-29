/*******************************************************************************************
*
*   layout_file_name - tool description
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018 raylib technologies
*
**********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

//----------------------------------------------------------------------------------
// Controls Functions Declaration
//----------------------------------------------------------------------------------
static void ExportImage();        // Button: ExportImage logic
static void ImportCharset();        // Button: ImportCharset logic

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //---------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "layout_file_name");

    // layout_file_name: controls initialization
    //----------------------------------------------------------------------------------

    bool WindowBox033Active = true;
    int algorythmActive = 0;
    const char *algorythmTextList[3] = { "ONE", "TWO", "THREE" };
    int heuristicsActive = 0;
    const char *heuristicsTextList[3] = { "ONE", "TWO", "THREE" };
    int paddingValue = 0;
    bool trimSpritesChecked = false;
    int widthValue = 0;
    int heightValue = 0;
    bool forcePOTChecked = false;
    bool forceSquaredChecked = false;
    int minCharValue = 0;
    int maxCharValue = 0;
    bool generateSDFChecked = false;
    int charHeightValue = 0;
    int Spinner033Value = 0;
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
            if (WindowBox033Active)
            {
                WindowBox033Active = !GuiWindowBox((Rectangle){ 50, 50, 230, 510 }, "ATLAS SETTINGS");
            }
            GuiGroupBox((Rectangle){ 60, 90, 210, 150 }, "Packing");
            GuiLabel((Rectangle){ 70, 105, 60, 25 }, "Algorythm:");
            algorythmActive = GuiComboBox((Rectangle){ 130, 105, 130, 25 }, algorythmTextList, 3, algorythmActive);
            GuiLabel((Rectangle){ 70, 140, 60, 25 }, "Heuristics:");
            heuristicsActive = GuiComboBox((Rectangle){ 130, 140, 130, 25 }, heuristicsTextList, 3, heuristicsActive);
            GuiLabel((Rectangle){ 70, 175, 60, 25 }, "Padding:");
            paddingValue = GuiSpinner((Rectangle){ 130, 175, 130, 25 }, paddingValue, 100, 25);
            trimSpritesChecked = GuiCheckBoxEx((Rectangle){ 175, 215, 15, 15 }, trimSpritesChecked, "Trim sprites");
            GuiGroupBox((Rectangle){ 60, 250, 210, 120 }, "Texture");
            GuiLabel((Rectangle){ 70, 265, 30, 25 }, "Size:");
            widthValue = GuiValueBox((Rectangle){ 115, 265, 65, 25 }, widthValue, 100);
            GuiLabel((Rectangle){ 185, 265, 10, 25 }, "x");
            heightValue = GuiValueBox((Rectangle){ 195, 265, 65, 25 }, heightValue, 100);
            forcePOTChecked = GuiCheckBoxEx((Rectangle){ 70, 300, 15, 15 }, forcePOTChecked, "Force POT");
            forceSquaredChecked = GuiCheckBoxEx((Rectangle){ 165, 300, 15, 15 }, forceSquaredChecked, "Force Square");
            if (GuiButton((Rectangle){ 70, 325, 190, 30 }, "Export Image")) ExportImage(); 

            GuiGroupBox((Rectangle){ 60, 380, 210, 170 }, "Font");
            GuiLabel((Rectangle){ 70, 390, 100, 25 }, "Char size (Height):");
            GuiLabel((Rectangle){ 230, 390, 31, 25 }, "pixels");
            GuiLabel((Rectangle){ 70, 415, 100, 25 }, "Characters Range:");
            minCharValue = GuiSpinner((Rectangle){ 70, 440, 85, 25 }, minCharValue, 100, 25);
            GuiLabel((Rectangle){ 160, 440, 15, 25 }, "to");
            maxCharValue = GuiSpinner((Rectangle){ 175, 440, 85, 25 }, maxCharValue, 100, 25);
            if (GuiButton((Rectangle){ 70, 475, 190, 30 }, "Import Charset")) ImportCharset(); 

            generateSDFChecked = GuiCheckBoxEx((Rectangle){ 70, 520, 15, 15 }, generateSDFChecked, "Generate SDF");
            charHeightValue = GuiValueBox((Rectangle){ 175, 390, 50, 25 }, charHeightValue, 100);
            Spinner033Value = GuiSpinner((Rectangle){ 175, 515, 85, 25 }, Spinner033Value, 100, 25);
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
// Button: ExportImage logic
static void ExportImage()
{
    // TODO: Implement control logic
}

// Button: ImportCharset logic
static void ImportCharset()
{
    // TODO: Implement control logic
}

