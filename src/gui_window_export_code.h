#include "raylib.h"

#ifndef GUI_WINDOW_EXPORT_CODE_H
#define GUI_WINDOW_EXPORT_CODE_H

typedef struct {
    // Main window properties
    bool active;
    Rectangle rec;
    // Name text
    int nameTextSize;  
    unsigned char nameText[32];    
    bool nameTextEdit;
    // Version text
    int versionTextSize;
    unsigned char versionText[16];
    bool versionTextEdit;
    // Window size values
    int windowWidth;
    bool windowWidthEdit;
    int windowHeight;
    bool windowHeightEdit;
    // Company text
    int companyTextSize;
    unsigned char companyText[32];
    bool companyTextEdit;
    // Description text
    int descriptionTextSize;
    unsigned char descriptionText[256];
    bool descriptionTextEdit;
    // Export options
    bool exportRects;
    bool exportText;
    bool exportAnchors;
    bool exportVariables;
    bool exportComments;
    bool cropToWindow;
    // Export button
    bool exportButtonPressed;
    
} GuiWindowExportCodeState;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
GuiWindowExportCodeState InitGuiWindowExportCode(void);
void GuiWindowExportCode (GuiWindowExportCodeState *state);

#ifdef __cplusplus
}
#endif

#endif // GUI_WINDOW_EXPORT_CODE_H

/***********************************************************************************
*
*   GUI_WINDOW_EXPORT_CODE IMPLEMENTATION
*
************************************************************************************/
#if defined(GUI_WINDOW_EXPORT_CODE_IMPLEMENTATION)

#include "raygui.h"

GuiWindowExportCodeState InitGuiWindowExportCode(void)
{
    GuiWindowExportCodeState state = { 0 };
    // Main window properties
    state.active = false;
    state.rec = (Rectangle){ GetScreenWidth()/2 - 200, GetScreenHeight()/2 - 112 , 400, 226 };
    // Name text
    state.nameTextSize = 32;
    strcpy(state.nameText, "LayoutName");
    state.nameTextEdit = false;
    // Version text
    state.versionTextSize = 16;
    strcpy(state.versionText, "1.0");
    state.versionTextEdit = false;
    // Window size values
    state.windowWidth = 0;
    state.windowWidthEdit = false;
    state.windowHeight = 0;
    state.windowHeightEdit = false;
    // Company text
    state.companyTextSize = 32;
    strcpy(state.companyText, "company name");
    state.companyTextEdit = false;
    // Description text
    state.descriptionTextSize = 256;
    strcpy(state.descriptionText, "layout description");
    state.descriptionTextEdit = false;
    // Export options    
    state.exportRects = false;
    state.exportText = false;
    state.exportAnchors = false;
    state.exportVariables = false;
    state.exportComments = false;
    state.cropToWindow = false;
    // Export button
    state.exportButtonPressed = false;
    
    return state;
}

// Gui export code
void GuiWindowExportCode(GuiWindowExportCodeState *state)
{
    if (state->active)
    {
        state->rec = (Rectangle){ GetScreenWidth()/2 - state->rec.width/2, GetScreenHeight()/2 - state->rec.height/2 , state->rec.width, state->rec.height };
        
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), 0.85f));
        // Main window
        state->active = !GuiWindowBox(state->rec, "Export Code Options");
        // Name text
        GuiLabel((Rectangle){state->rec.x + 10, state->rec.y + 35, 65, 25 }, "Name:");
        if (GuiTextBox((Rectangle){ state->rec.x + 75, state->rec.y + 35, 135, 25 }, state->nameText, state->nameTextSize, state->nameTextEdit)) state->nameTextEdit = !state->nameTextEdit;
        // Version text
        GuiLabel((Rectangle){state->rec.x + 225, state->rec.y + 35, 50, 25 }, "Version:");
        if (GuiTextBox((Rectangle){ state->rec.x + 275, state->rec.y + 35, 115, 25 }, state->versionText, state->versionTextSize, state->versionTextEdit)) state->versionTextEdit = !state->versionTextEdit;
        // Window size values
        GuiLabel((Rectangle){state->rec.x + 10, state->rec.y + 65, 65, 25 }, "Window size:");
        if (GuiValueBox((Rectangle){ state->rec.x + 75, state->rec.y + 65, 60, 25 }, &state->windowWidth, 400, 1000, state->windowWidthEdit)) state->windowWidthEdit = !state->windowWidthEdit;
        GuiLabel((Rectangle){state->rec.x + 140, state->rec.y + 65, 10, 25 }, "x:");
        if (GuiValueBox((Rectangle){ state->rec.x + 150, state->rec.y + 65, 60, 25 }, &state->windowHeight, 400, 1000, state->windowHeightEdit)) state->windowHeightEdit = !state->windowHeightEdit;
        // Company text
        GuiLabel((Rectangle){state->rec.x + 225, state->rec.y + 65, 50, 25 }, "Company:");
        if(GuiTextBox((Rectangle){ state->rec.x + 275, state->rec.y + 65, 115, 25 }, state->companyText, state->companyTextSize, state->companyTextEdit)) state->companyTextEdit = !state->companyTextEdit;
        // Description multitext
        GuiLabel((Rectangle){state->rec.x + 10, state->rec.y + 95, 65, 25 }, "Description:");
        if (GuiTextBoxMulti((Rectangle){ state->rec.x + 75, state->rec.y + 95, 315, 55 }, state->descriptionText, state->descriptionTextSize, state->descriptionTextEdit)) state->descriptionTextEdit = !state->descriptionTextEdit;
        // Export options
            // Define rectangles
            state->exportRects = GuiCheckBoxEx((Rectangle){ state->rec.x + 10, state->rec.y + 160, 15, 15 }, state->exportRects, "Define Rectangles");
            // Define text const
            state->exportText = GuiCheckBoxEx((Rectangle){ state->rec.x + 10, state->rec.y + 180, 15, 15 }, state->exportText, "Define text const");
            // Define Export anchors
            state->exportAnchors = GuiCheckBoxEx((Rectangle){ state->rec.x + 10, state->rec.y + 200, 15, 15 }, state->exportAnchors, "Export anchors");
            // Full variables
            state->exportVariables = GuiCheckBoxEx((Rectangle){ state->rec.x + 140, state->rec.y + 160, 15, 15 }, state->exportVariables, "Full variables");
            // Full comments
            state->exportComments = GuiCheckBoxEx((Rectangle){ state->rec.x + 140, state->rec.y + 180, 15, 15 }, state->exportComments, "Full comments");
            // Crop to window
            state->cropToWindow = GuiCheckBoxEx((Rectangle){ state->rec.x + 140, state->rec.y + 200, 15, 15 }, state->cropToWindow, "Crop to Window");
        // Export button
        state->exportButtonPressed = GuiButton((Rectangle){ state->rec.x + 275, state->rec.y + 185, 115, 30 }, "Export Code");  
    }      
}

#endif // GUI_WINDOW_EXPORT_CODE_IMPLEMENTATION