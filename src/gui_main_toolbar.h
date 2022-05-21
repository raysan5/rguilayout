/*******************************************************************************************
*
*   MainToolbar v1.0.0 - Main Toolbar
*
*   MODULE USAGE:
*       #define GUI_MAIN_TOOLBAR_IMPLEMENTATION
*       #include "gui_main_toolbar.h"
*
*       INIT: GuiMainToolbarState state = InitGuiMainToolbar();
*       DRAW: GuiMainToolbar(&state);
*
*   LICENSE: Propietary License
*
*   Copyright (c) 2019 raylib technologies (@raylibtech). All Rights Reserved.
*
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   This project is proprietary and confidential unless the owner allows
*   usage in any other form by expresely written permission.
*
**********************************************************************************************/

#include "raylib.h"

// WARNING: raygui implementation is expected to be defined before including this header

#ifndef GUI_MAIN_TOOLBAR_H
#define GUI_MAIN_TOOLBAR_H

typedef struct {
    Vector2 anchorMain;
    Vector2 anchorZoom;
    Vector2 anchorBack;
    Vector2 anchorChannels;
    Vector2 anchorHelpers;
    Vector2 anchorEmpty;
    Vector2 anchorFullscreen;
    
    bool btnOpenPressed;
    bool btnExportPressed;
    bool showInfoWindowActive;
    bool showEditWindowActive;
    bool btnHelpPressed;
    bool btnAboutPressed;
    bool centerPressed;
    float zoomValue;
    int scaleFilterActive;
    int channelsActive;
    int backgroundActive;
    bool helperGridActive;
    bool helperCursorActive;
    bool helperBoxActive;

    // Custom state variables (depend on development software)
    // NOTE: This variables should be added manually if required
    bool supportImage;
    bool supportInfoEdit;

} GuiMainToolbarState;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Internal Module Functions Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
GuiMainToolbarState InitGuiMainToolbar(void);
void GuiMainToolbar(GuiMainToolbarState *state);

#ifdef __cplusplus
}
#endif

#endif // GUI_MAIN_TOOLBAR_H

/***********************************************************************************
*
*   GUI_MAIN_TOOLBAR IMPLEMENTATION
*
************************************************************************************/
#if defined(GUI_MAIN_TOOLBAR_IMPLEMENTATION)

#include "raygui.h"

GuiMainToolbarState InitGuiMainToolbar(void)
{
    GuiMainToolbarState state = { 0 };

    state.anchorMain = (Vector2){ 0, 0 };
    state.anchorZoom = (Vector2){ 324, 0 };
    state.anchorBack = (Vector2){ 723, 0 };
    state.anchorChannels = (Vector2){ 882, 0 };
    state.anchorHelpers = (Vector2){ 1081, 0 };
    state.anchorEmpty = (Vector2){ 1080 + 150, 0 };
    state.anchorFullscreen = (Vector2){ 0, 0 };

    state.btnOpenPressed = false;
    state.btnExportPressed = false;
    state.showInfoWindowActive = true;
    state.showEditWindowActive = true;
    state.btnHelpPressed = false;
    state.btnAboutPressed = false;
    state.centerPressed = false;
    state.zoomValue = 1.0f;
    state.scaleFilterActive = 0;
    state.channelsActive = 0;
    state.backgroundActive = 0;
    state.helperGridActive = true;
    state.helperCursorActive = true;
    state.helperBoxActive = true;

    // Custom variables initialization
    state.supportImage = false;

    return state;
}

void GuiMainToolbar(GuiMainToolbarState *state)
{
    state->anchorFullscreen.x = (float)(GetScreenWidth() - 53);
    
    if (!state->supportImage) state->supportInfoEdit = false;
    
    GuiPanel((Rectangle){ state->anchorMain.x, state->anchorMain.y, 325, 48 });
    state->btnOpenPressed = GuiButton((Rectangle){ state->anchorMain.x + 16, state->anchorMain.y + 12, 24, 24 }, "#1#"); 

    if (!state->supportInfoEdit) GuiDisable();
    state->btnExportPressed = GuiButton((Rectangle){ state->anchorMain.x + 44, state->anchorMain.y + 12, 24, 24 }, "#7#");
    state->showInfoWindowActive = GuiToggle((Rectangle){ state->anchorMain.x + 86, state->anchorMain.y + 13, 22, 22 }, "#15#", state->showInfoWindowActive);
    state->showEditWindowActive = GuiToggle((Rectangle){ state->anchorMain.x + 110, state->anchorMain.y + 13, 22, 22 }, "#140#", state->showEditWindowActive);
    GuiEnable();
    
    state->btnHelpPressed = GuiButton((Rectangle){ state->anchorMain.x + 154, state->anchorMain.y + 12, 75, 24 }, "#193#HELP"); 
    state->btnAboutPressed = GuiButton((Rectangle){ state->anchorMain.x + 234, state->anchorMain.y + 12, 75, 24 }, "#191#ABOUT"); 
    
    GuiPanel((Rectangle){ state->anchorZoom.x, state->anchorZoom.y, 400, 48 });
    GuiPanel((Rectangle){ state->anchorBack.x, state->anchorBack.y, 160, 48 });
    GuiPanel((Rectangle){ state->anchorChannels.x, state->anchorChannels.y, 200, 48 });
    GuiPanel((Rectangle){ state->anchorHelpers.x, state->anchorHelpers.y, 150, 48 });

    if ((GetScreenWidth() - state->anchorEmpty.x - 52) > 0)
    {
        GuiPanel((Rectangle){ state->anchorEmpty.x, state->anchorEmpty.y, GetScreenWidth() - state->anchorEmpty.x - 52, 48 });
    }

    if (!state->supportImage) GuiDisable();
    state->centerPressed = GuiButton((Rectangle){ state->anchorZoom.x + 16, state->anchorZoom.y + 12, 24, 24 }, "#105#");
    state->zoomValue = GuiSliderBar((Rectangle){ state->anchorZoom.x + 86, state->anchorZoom.y + 19, 150, 12 }, "ZOOM:", TextFormat("%i%%", (int)(state->zoomValue*100.0f)), state->zoomValue, 0, 20.0f);
    
    int labelTextAlign = GuiGetStyle(LABEL, TEXT_ALIGNMENT);
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    state->scaleFilterActive = GuiToggleGroup((Rectangle){ state->anchorZoom.x + 340, state->anchorZoom.y + 14, 22, 22 }, "#34#;#35#", state->scaleFilterActive);
    GuiLabel((Rectangle){ state->anchorZoom.x + 340 - MeasureTextEx(GuiGetFont(), "Filter:", (float)GuiGetStyle(DEFAULT, TEXT_SIZE), (float)GuiGetStyle(DEFAULT, TEXT_SPACING)).x - (float)GuiGetStyle(SLIDER, TEXT_PADDING), state->anchorZoom.y + 17, 72, 18 }, "Filter:");
        
    GuiLabel((Rectangle){ state->anchorBack.x + 16, state->anchorBack.y + 16, 32, 18 }, "Back:");
    state->backgroundActive = GuiToggleGroup((Rectangle){ state->anchorBack.x + 50, state->anchorBack.y + 13, 22, 22 }, "#192#;;;", state->backgroundActive);
    GuiLabel((Rectangle){ state->anchorChannels.x + 70 - MeasureTextEx(GuiGetFont(), "Channels:", (float)GuiGetStyle(DEFAULT, TEXT_SIZE), (float)GuiGetStyle(DEFAULT, TEXT_SPACING)).x - (float)GuiGetStyle(SLIDER, TEXT_PADDING), state->anchorChannels.y + 16, 56, 18 }, "Channels:");
    state->channelsActive = GuiToggleGroup((Rectangle){ state->anchorChannels.x + 70, state->anchorChannels.y + 13, 22, 22 }, "*;R;G;B;A", state->channelsActive);
    GuiEnable();
    
    GuiLabel((Rectangle){ state->anchorHelpers.x + 65 - MeasureTextEx(GuiGetFont(), "Helpers:", (float)GuiGetStyle(DEFAULT, TEXT_SIZE), (float)GuiGetStyle(DEFAULT, TEXT_SPACING)).x - (float)GuiGetStyle(SLIDER, TEXT_PADDING), state->anchorHelpers.y + 15, 50, 18 }, "Helpers:");
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, labelTextAlign);
    
    state->helperGridActive = GuiToggle((Rectangle){ state->anchorHelpers.x + 65, state->anchorHelpers.y + 13, 22, 22 }, "#97#", state->helperGridActive);
    state->helperCursorActive = GuiToggle((Rectangle){ state->anchorHelpers.x + 89, state->anchorHelpers.y + 13, 22, 22 }, "#65#", state->helperCursorActive);
    state->helperBoxActive = GuiToggle((Rectangle){ state->anchorHelpers.x + 113, state->anchorHelpers.y + 13, 22, 22 }, "#38#", state->helperBoxActive);
    
    GuiPanel((Rectangle){ state->anchorFullscreen.x, state->anchorFullscreen.y, 53, 48 });
    state->fullScreenPressed = GuiButton((Rectangle){ state->anchorFullscreen.x + 15, state->anchorFullscreen.y + 12, 24, 24 }, "#53#");
    
    // Draw background colors
    DrawRectangleRec((Rectangle){ state->anchorBack.x + 50 + 2*(22 + GuiGetStyle(TOGGLE, GROUP_PADDING)) + 1, state->anchorBack.y + 14, 20, 20 }, BLACK);
    DrawRectangleRec((Rectangle){ state->anchorBack.x + 50 + 3*(22 + GuiGetStyle(TOGGLE, GROUP_PADDING)) + 1, state->anchorBack.y + 14, 20, 20 }, MAGENTA);

    // NOTE: Using 3 pixels check size (18/6)
    Rectangle bounds = { state->anchorBack.x + 50 + (23 + GuiGetStyle(TOGGLE, GROUP_PADDING)) + 1, state->anchorBack.y + 15, 18, 18 };
    for (int y = 0; y < bounds.height/3; y++)
    {
        for (int x = 0; x <  bounds.width/3; x++)
        {
            DrawRectangle((int)bounds.x + x*3, (int)bounds.y + y*3, 3, 3, ((x + y)%2)? LIGHTGRAY : RAYWHITE);
        }
    }
}

#endif // GUI_MAIN_TOOLBAR_IMPLEMENTATION
