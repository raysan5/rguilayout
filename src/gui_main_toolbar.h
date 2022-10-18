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
    bool btnUndoPressed;                // Shortcut: CTRL+Z
    bool btnRedoPressed;                // Shortcut: CTRL+Y

    // Selected control edit options
    bool btnEditTextPressed;            // Edit control text, Shortcut: CTRL+T
    bool btnEditNamePressed;            // Edit control name, Shortcut: CTRL+N
    bool btnDeleteControlPressed;       // Delecte control, Shortcut: DELETE
    bool btnDuplicateControlPressed;    // Ducplicate control, Shortcut: CTRL+D
    bool btnResizeControlPressed;       // Resize control to closest snap, Shortcut: CTRL+R

    // Tool options
    bool snapModeActive;                // Shortcut: SHIFT+S

    bool hideAnchorControlsActive;      // Toggle Hide/Show, Shortcut: CTRL+H
    bool showControlNamesActive;        // Show controls names, Shortcut: N - no control selected
    bool showControlOrderActive;        // Show controls drawing order, Shortcut: ALT (Use mouse wheel to reorder)
    bool showControlPanelActive;        // Toggle control panel window

    bool btnLoadTracemapPressed;        // Load tracemap image
    // TOOL: Tracemap: Load, View, Alpha, Lock

    // Visual options
    bool showGridActive;                // Show grid
    int visualStyleActive;
    int prevVisualStyleActive;
    int languageActive;

    // Help options
    bool btnHelpPressed;
    bool btnAboutPressed;
    bool btnSponsorPressed;

    // Custom variables
    // NOTE: Required to enable/disable some toolbar elements
    bool controlSelected;
    bool anchorSelected;
    bool tracemapSelected;

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
    state.anchorTools = (Vector2){ state.anchorEdit.x + 300 - 1, 0 };
    state.anchorVisuals = (Vector2){ 0, 0 };    // Anchor right, depends on screen width
    state.anchorRight = (Vector2){ 0, 0 };      // Anchor right, depends on screen width

    // Project/File options
    state.btnNewFilePressed = false;
    state.btnLoadFilePressed = false;
    state.btnSaveFilePressed = false;
    state.btnExportFilePressed = false;
    state.btnCloseFilePressed = false;

    // Editor options
    state.btnUndoPressed = false;                // Shortcut: CTRL+Z
    state.btnRedoPressed = false;                // Shortcut: CTRL+Y

    // Selected control edit options
    state.btnEditTextPressed = false;            // Shortcut: CTRL+T
    state.btnEditNamePressed = false;            // Shortcut: CTRL+N
    state.btnDeleteControlPressed = false;       // Shortcut: DELETE
    state.btnDuplicateControlPressed = false;    // Shortcut: CTRL+D
    state.btnResizeControlPressed = false;       // Shortcut: CTRL+R

    // Tool options
    state.snapModeActive = false;                // Shortcut: SHIFT+S

    state.hideAnchorControlsActive = false;      // Toggle Hide/Show, Shortcut: CTRL+H
    state.showControlNamesActive = false;        // Show controls names, Shortcut: N - no control selected
    state.showControlOrderActive = false;        // Show controls drawing order, Shortcut: ALT (Use mouse wheel to reorder)
    state.showControlPanelActive = true;         // Toggle control panel window

    state.btnLoadTracemapPressed = false;        // Load tracemap image

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
    state.controlSelected = false;
    state.anchorSelected = false;
    state.tracemapSelected = false;

    return state;
}

void GuiMainToolbar(GuiMainToolbarState *state)
{
    // Toolbar panels
    state->anchorRight.x = GetScreenWidth() - 104;       // Update right-anchor panel
    state->anchorVisuals.x = state->anchorRight.x - 224 + 1;    // Update right-anchor panel

    GuiPanel((Rectangle){ state->anchorFile.x, state->anchorFile.y, 160, 40 }, NULL);
    GuiPanel((Rectangle){ state->anchorEdit.x, state->anchorEdit.y, 300, 40 }, NULL);
    GuiPanel((Rectangle){ state->anchorTools.x, state->anchorTools.y, state->anchorVisuals.x - state->anchorTools.x + 1, 40 }, NULL);
    GuiPanel((Rectangle){ state->anchorVisuals.x, state->anchorVisuals.y, 224, 40 }, NULL);
    GuiPanel((Rectangle){ state->anchorRight.x, state->anchorRight.y, 104, 40 }, NULL);

    // Project/File options
    state->btnNewFilePressed = GuiButton((Rectangle){ state->anchorFile.x + 12, state->anchorFile.y + 8, 24, 24 }, "#8#");
    state->btnLoadFilePressed = GuiButton((Rectangle){ state->anchorFile.x + 12 + 24 + 4, state->anchorFile.y + 8, 24, 24 }, "#5#");
    state->btnSaveFilePressed = GuiButton((Rectangle){ state->anchorFile.x + 12 + 48 + 8, state->anchorFile.y + 8, 24, 24 }, "#6#");
    state->btnExportFilePressed = GuiButton((Rectangle){ state->anchorFile.x + 12 + 72 + 12, state->anchorFile.y + 8, 24, 24 }, "#7#");
    state->btnCloseFilePressed = GuiButton((Rectangle){ state->anchorFile.x + 12 + 96 + 16, state->anchorFile.y + 8, 24, 24 }, "#9#");

    // Editor options
    state->btnUndoPressed = GuiButton((Rectangle){ state->anchorEdit.x + 12, state->anchorEdit.y + 8, 24, 24 }, "#72#");  
    state->btnRedoPressed = GuiButton((Rectangle){ state->anchorEdit.x + 12 + 24 + 4, state->anchorEdit.y + 8, 24, 24 }, "#73#");

    if (!state->controlSelected) GuiDisable();
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiLabel((Rectangle){ state->anchorEdit.x + 12 + 48 + 16, state->anchorEdit.y + 8, 64, 24 }, "Control:");
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    state->btnEditTextPressed = GuiButton((Rectangle){ state->anchorEdit.x + 60 + 72 + 16, state->anchorEdit.y + 8, 24, 24 }, "#30#");
    state->btnEditNamePressed = GuiButton((Rectangle){ state->anchorEdit.x + 60 + 96 + 20, state->anchorEdit.y + 8, 24, 24 }, "#31#");
    state->btnDeleteControlPressed = GuiButton((Rectangle){ state->anchorEdit.x + 60 + 120 + 24, state->anchorEdit.y + 8, 24, 24 }, "#143#");
    state->btnDuplicateControlPressed = GuiButton((Rectangle){ state->anchorEdit.x + 60 + 144 + 28, state->anchorEdit.y + 8, 24, 24 }, "#16#");
    state->btnResizeControlPressed = GuiButton((Rectangle){ state->anchorEdit.x + 60 + 168 + 32, state->anchorEdit.y + 8, 24, 24 }, "#33#");
    GuiEnable();

    // Tool options
    state->snapModeActive = GuiToggle((Rectangle){ state->anchorTools.x + 12, state->anchorTools.y + 8, 24, 24 }, "#50#", state->snapModeActive);

    if (!state->anchorSelected) GuiDisable();
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiLabel((Rectangle){ state->anchorTools.x + 12 + 24 + 8, state->anchorTools.y + 8, 64, 24 }, "Anchor:");
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    state->hideAnchorControlsActive = GuiToggle((Rectangle){ state->anchorTools.x + 80 + 24 + 8, state->anchorTools.y + 8, 24, 24 }, state->hideAnchorControlsActive? "#45#" : "#44#", state->hideAnchorControlsActive);
    GuiEnable();
    
    if (state->controlSelected) GuiDisable();
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiLabel((Rectangle){ state->anchorTools.x + 88 + 48 + 8, state->anchorTools.y + 8, 64, 24 }, "Layout:");
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    state->showControlNamesActive = GuiToggle((Rectangle){ state->anchorTools.x + 148 + 48 + 16 , state->anchorTools.y + 8, 24, 24 }, "#214#", state->showControlNamesActive);
    state->showControlOrderActive = GuiToggle((Rectangle){ state->anchorTools.x + 148 + 72 + 20, state->anchorTools.y + 8, 24, 24 }, "#197#", state->showControlOrderActive);
    GuiEnable();
    state->showControlPanelActive = GuiToggle((Rectangle){ state->anchorTools.x + 148 + 96 + 24, state->anchorTools.y + 8, 24, 24 }, "#101#", state->showControlPanelActive);

    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiLabel((Rectangle){ state->anchorTools.x + 148 + 132 + 36, state->anchorTools.y + 8, 64, 24 }, "Tracemap:");
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    state->btnLoadTracemapPressed = GuiButton((Rectangle){ state->anchorTools.x + 148 + 200 + 36, state->anchorTools.y + 8, 24, 24 }, "#12#");

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
