/*******************************************************************************************
*
*   file_name v1.0 - tool description
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018 company name
*
**********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

//----------------------------------------------------------------------------------
// Controls Functions Declaration
//----------------------------------------------------------------------------------
static void GenerateCode();        // Button: GenerateCode logic


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //---------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "file_name");

    // file_name: controls initialization
    //----------------------------------------------------------------------------------
    // Anchor points
    Vector2 anchor01 = { 50, 50 };

    // WindowBox: generateWindow
    bool generateWindowActive = true;

    // Label: lblName
    const char *lblNameText = "Name:";

    // TextBox: toolName
    int toolNameSize = 64;
    char toolNameText[64] = "tool_name";

    // Label: lblVersion
    const char *lblVersionText = "Version:";

    // TextBox: toolVersion
    int toolVersionSize = 64;
    char toolVersionText[64] = "1.2.0-dev";

    // Label: lblWindowSize
    const char *lblWindowSizeText = "Window size:";

    // ValueBox: width
    int widthValue = 0;

    // Label: lblx
    const char *lblxText = "x";

    // ValueBox: height
    int heightValue = 0;

    // Label: lblCompany
    const char *lblCompanyText = "Company:";

    // TextBox: company
    int companySize = 64;
    char companyText[64] = "raysan5";

    // Label: lblDescription
    const char *lblDescriptionText = "Description:";

    // TextBox: toolDescription
    int toolDescriptionSize = 64;
    char toolDescriptionText[64] = "tool_description";

    // CheckBox: defineRecs
    bool defineRecsChecked = false;

    // CheckBox: defineTexts
    bool defineTextsChecked = false;

    // CheckBox: fullVariables
    bool fullVariablesChecked = false;

    // CheckBox: exportAnchors
    bool exportAnchorsChecked = false;

    // CheckBox: exportAnchor0
    bool exportAnchor0Checked = false;

    // CheckBox: fullComments
    bool fullCommentsChecked = false;

    // CheckBox: cropWindow
    bool cropWindowChecked = false;

    // Button: GenerateCode
    
    // Define controls rectangles
    Rectangle layoutRecs[21] = {
        (Rectangle){ anchor01.x + 0, anchor01.y + 0, 400, 225 }        // WindowBox: generateWindow
        (Rectangle){ anchor01.x + 10, anchor01.y + 35, 65, 25 }        // Label: lblName
        (Rectangle){ anchor01.x + 75, anchor01.y + 35, 135, 25 }        // TextBox: toolName
        (Rectangle){ anchor01.x + 225, anchor01.y + 35, 50, 25 }        // Label: lblVersion
        (Rectangle){ anchor01.x + 275, anchor01.y + 35, 115, 25 }        // TextBox: toolVersion
        (Rectangle){ anchor01.x + 10, anchor01.y + 65, 65, 25 }        // Label: lblWindowSize
        (Rectangle){ anchor01.x + 75, anchor01.y + 65, 60, 25 }        // ValueBox: width
        (Rectangle){ anchor01.x + 140, anchor01.y + 65, 10, 25 }        // Label: lblx
        (Rectangle){ anchor01.x + 150, anchor01.y + 65, 60, 25 }        // ValueBox: height
        (Rectangle){ anchor01.x + 225, anchor01.y + 65, 50, 25 }        // Label: lblCompany
        (Rectangle){ anchor01.x + 275, anchor01.y + 65, 115, 25 }        // TextBox: company
        (Rectangle){ anchor01.x + 10, anchor01.y + 95, 65, 25 }        // Label: lblDescription
        (Rectangle){ anchor01.x + 75, anchor01.y + 95, 315, 55 }        // TextBox: toolDescription
        (Rectangle){ anchor01.x + 10, anchor01.y + 160, 15, 15 }        // CheckBox: defineRecs
        (Rectangle){ anchor01.x + 10, anchor01.y + 180, 15, 15 }        // CheckBox: defineTexts
        (Rectangle){ anchor01.x + 10, anchor01.y + 200, 15, 15 }        // CheckBox: fullVariables
        (Rectangle){ anchor01.x + 140, anchor01.y + 160, 15, 15 }        // CheckBox: exportAnchors
        (Rectangle){ anchor01.x + 140, anchor01.y + 180, 15, 15 }        // CheckBox: exportAnchor0
        (Rectangle){ anchor01.x + 140, anchor01.y + 200, 15, 15 }        // CheckBox: fullComments
        (Rectangle){ anchor01.x + 275, anchor01.y + 160, 15, 15 }        // CheckBox: cropWindow
        (Rectangle){ anchor01.x + 275, anchor01.y + 185, 115, 30 }        // Button: GenerateCode
    };
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
            if (generateWindowActive)
            {
                generateWindowActive = !GuiWindowBox(layoutRecs[0], "Generate Code Options - layout");
            }
            GuiLabel(layoutRecs[1], lblNameText);
            GuiTextBox(layoutRecs[2], toolNameText, toolNameSize, true);
            GuiLabel(layoutRecs[3], lblVersionText);
            GuiTextBox(layoutRecs[4], toolVersionText, toolVersionSize, true);
            GuiLabel(layoutRecs[5], lblWindowSizeText);
            if (GuiValueBox(width, layoutRecs[6]Value, 0, 100, widthEditMode)) widthEditMode = !widthEditMode;
            GuiLabel(layoutRecs[7], lblxText);
            if (GuiValueBox(height, layoutRecs[8]Value, 0, 100, heightEditMode)) heightEditMode = !heightEditMode;
            GuiLabel(layoutRecs[9], lblCompanyText);
            GuiTextBox(layoutRecs[10], companyText, companySize, true);
            GuiLabel(layoutRecs[11], lblDescriptionText);
            GuiTextBox(layoutRecs[12], toolDescriptionText, toolDescriptionSize, true);
            defineRecsChecked = GuiCheckBoxEx(layoutRecs[13], defineRecsChecked, "Define Rectangles");
            defineTextsChecked = GuiCheckBoxEx(layoutRecs[14], defineTextsChecked, "Define text const");
            fullVariablesChecked = GuiCheckBoxEx(layoutRecs[15], fullVariablesChecked, "Full variables");
            exportAnchorsChecked = GuiCheckBoxEx(layoutRecs[16], exportAnchorsChecked, "Export anchors");
            exportAnchor0Checked = GuiCheckBoxEx(layoutRecs[17], exportAnchor0Checked, "Export anchor 0");
            fullCommentsChecked = GuiCheckBoxEx(layoutRecs[18], fullCommentsChecked, "Full comments");
            cropWindowChecked = GuiCheckBoxEx(layoutRecs[19], cropWindowChecked, "Crop to Window");
            if (GuiButton(layoutRecs[20], "Generate Code")) GenerateCode();
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
// Button: GenerateCode logic
static void GenerateCode()
{
    // TODO: Implement control logic
}


