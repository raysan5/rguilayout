/*******************************************************************************************
*
*   Main Toolbar
*
*   MODULE USAGE:
*       #define GUI_MAIN_TOOLBAR_IMPLEMENTATION
*       #include "gui_main_toolbar.h"
*
*       INIT: GuiMainToolbarState state = InitGuiMainToolbar();
*       DRAW: GuiMainToolbar(&state);
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2019-2022 raylib technologies (@raylibtech).
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

#include "raylib.h"

// WARNING: raygui implementation is expected to be defined before including this header

#ifndef GUI_MAIN_TOOLBAR_H
#define GUI_MAIN_TOOLBAR_H

typedef struct {

    // Anchors for panels
    Vector2 anchorFile;
    Vector2 anchorEdit;
    Vector2 anchorTools;
    Vector2 anchorVisuals;
    Vector2 anchorRight;
    
    // File options
    bool btnNewFilePressed;
    bool btnLoadFilePressed;
    bool btnSaveFilePressed;
    bool btnExportFilePressed;
    bool btnCloseFilePressed;

    // Editor options
    // EDIT: Controls: TextEdit (CTRL+T), NameEdit (CTRL+N), Delete (SUP), Duplicate (CTRL+D)
    // EDIT: Anchor-controls: Hide/Show (H), Show controls names (N - no control selected), Show controls order (ALT)
    // EDIT: SnapMode (S), Undo/Redo (CTRL+Z/CTRL+Y)

    // Tool options
    // TOOL: ControlPanel, Tracemap: Load, View, Alpha, Lock

    // Visual options
    bool showGridActive;
    int visualStyleActive;
    int prevVisualStyleActive;
    int languageActive;

    // Help options
    bool btnHelpPressed;
    bool btnAboutPressed;
    bool btnSponsorPressed;

    // Custom variables
    // TODO.

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

    // Anchors for panels
    state.anchorFile = (Vector2){ 0, 0 };
    state.anchorEdit = (Vector2){ state.anchorFile.x + 160 - 1, 0 };
    state.anchorTools = (Vector2){ state.anchorEdit.x + 188 - 1, 0 };
    state.anchorVisuals = (Vector2){ 0, 0 };    // Anchor right, depends on screen width
    state.anchorRight = (Vector2){ 0, 0 };      // Anchor right, depends on screen width

    // Project/File options
    state.btnNewFilePressed = false;
    state.btnLoadFilePressed = false;
    state.btnSaveFilePressed = false;
    state.btnExportFilePressed = false;
    state.btnCloseFilePressed = false;

    // Edit options
    //...

    // Tool options
    //...

    // Visuals options
    state.showGridActive = true;
    state.visualStyleActive = 0;
    state.prevVisualStyleActive = 0;
    state.languageActive = 0;

    // Info options
    state.btnHelpPressed = false;
    state.btnAboutPressed = false;
    state.btnSponsorPressed = false;

    // Custom variables
    // TODO.

    return state;
}

void GuiMainToolbar(GuiMainToolbarState *state)
{
    // Toolbar panels
    state->anchorRight.x = GetScreenWidth() - 104;       // Update right-anchor panel
    state->anchorVisuals.x = state->anchorRight.x - 224 + 1;    // Update right-anchor panel

    GuiPanel((Rectangle){ state->anchorFile.x, state->anchorFile.y, 160, 40 }, NULL);
    GuiPanel((Rectangle){ state->anchorEdit.x, state->anchorEdit.y, 188, 40 }, NULL);
    GuiPanel((Rectangle){ state->anchorTools.x, state->anchorTools.y, state->anchorVisuals.x - state->anchorTools.x + 1, 40 }, NULL);
    GuiPanel((Rectangle){ state->anchorVisuals.x, state->anchorVisuals.y, 224, 40 }, NULL);
    GuiPanel((Rectangle){ state->anchorRight.x, state->anchorRight.y, 104, 40 }, NULL);

    // Project/File options
    state->btnNewFilePressed = GuiButton((Rectangle){ state->anchorFile.x + 12, state->anchorFile.y + 8, 24, 24 }, "#8#");
    state->btnLoadFilePressed = GuiButton((Rectangle){ state->anchorFile.x + 12 + 24 + 4, state->anchorFile.y + 8, 24, 24 }, "#5#");
    state->btnSaveFilePressed = GuiButton((Rectangle){ state->anchorFile.x + 12 + 48 + 8, state->anchorFile.y + 8, 24, 24 }, "#6#");
    state->btnExportFilePressed = GuiButton((Rectangle){ state->anchorFile.x + 12 + 72 + 12, state->anchorFile.y + 8, 24, 24 }, "#7#");
    state->btnCloseFilePressed = GuiButton((Rectangle){ state->anchorFile.x + 12 + 96 + 16, state->anchorFile.y + 8, 24, 24 }, "#9#");

    // Edit options
    //...
    
    // Tool options
    //...
    
    // Visuals options
    state->showGridActive = GuiToggle((Rectangle){ state->anchorVisuals.x + 12, state->anchorVisuals.y + 8, 24, 24 }, "#97#", state->showGridActive);
    GuiLabel((Rectangle){ state->anchorVisuals.x + 12 + 32, state->anchorVisuals.y + 8, 60, 24 }, "Style:");
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_WIDTH, 40);
    state->visualStyleActive = GuiComboBox((Rectangle){ state->anchorVisuals.x + 8 + 48 + 32, state->anchorVisuals.y + 8, 120, 24 }, "Light;Jungle;Candy;Lavanda;Cyber;Terminal;Ashes;Bluish;Dark;Cherry;Sunny;Enefete", state->visualStyleActive);
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_WIDTH, 32);

    // Info options
    state->btnHelpPressed = GuiButton((Rectangle){ state->anchorRight.x + (GetScreenWidth() - state->anchorRight.x) - 12 - 72 - 8, state->anchorRight.y + 8, 24, 24 }, "#193#");
    state->btnAboutPressed = GuiButton((Rectangle){ state->anchorRight.x + (GetScreenWidth() - state->anchorRight.x) - 12 - 48 - 4, state->anchorRight.y + 8, 24, 24 }, "#191#");
    state->btnSponsorPressed = GuiButton((Rectangle){ state->anchorRight.x + (GetScreenWidth() - state->anchorRight.x) - 12 - 24, state->anchorRight.y + 8, 24, 24 }, "#186#");
}

#endif // GUI_MAIN_TOOLBAR_IMPLEMENTATION
