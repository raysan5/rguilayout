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
*   Copyright (c) 2019-2022 raylib technologies (@raylibtech) / Ramon Santamaria (@raysan5)
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
    bool btnUndoPressed;                // Undo last action recorded
    bool btnRedoPressed;                // Redo last action recorded
    bool snapModeActive;                // Toggle snap to grid mode
    //bool editionModeActive;             // Toggle control creation/selection mode

    // Selected control options
    bool btnEditTextPressed;            // Edit control text
    bool btnEditNamePressed;            // Edit control name
    bool btnDuplicateControlPressed;    // Duplicate control
    bool btnUnlinkControlPressed;       // Unlink control from anchor

    bool btnDeleteControlPressed;       // Delete control

    // Selected anchor options
    bool btnEditAnchorNamePressed;      // Edit anchor name
    bool hideAnchorControlsActive;      // Toggle Hide/Show anchor linked controls
    bool btnUnlinkAnchorControlsPressed;   // Unlink all controls linked to selected anchor
    bool btnDeleteAnchorPressed;        // Delete selected anchor

    // Selected tracemap options
    bool btnLoadTracemapPressed;        // Load tracemap image
    bool hideTracemapActive;            // Hide/show tracemap image
    bool lockTracemapActive;            // Lock/unlock tracemap image
    bool btnDeleteTracemapPressed;      // Unload tracemap image
    float tracemapAlphaValue;           // Adjust tracemap opacity

    // Visual options
    bool showControlRecsActive;         // Toggle all controls rectangles drawing
    bool showControlNamesActive;        // Toggle all controls names
    bool showControlOrderActive;        // Toggle all control drawing order (layers)
    bool showControlPanelActive;        // Toggle control panel window
    bool showGridActive;                // Show/hide work grid

    int visualStyleActive;
    int prevVisualStyleActive;
    int languageActive;

    // Help options
    bool btnHelpPressed;
    bool btnAboutPressed;
    bool btnSponsorPressed;

    // Custom variables
    // NOTE: Required to enable/disable some toolbar elements
    int controlSelected;
    int anchorSelected;
    bool tracemapLoaded;

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
    state.anchorTools = (Vector2){ state.anchorEdit.x + 112 - 1, 0 };
    state.anchorVisuals = (Vector2){ 0, 0 };    // Anchor right, depends on screen width
    state.anchorRight = (Vector2){ 0, 0 };      // Anchor right, depends on screen width

    // Project/File options
    state.btnNewFilePressed = false;
    state.btnLoadFilePressed = false;
    state.btnSaveFilePressed = false;
    state.btnExportFilePressed = false;
    state.btnCloseFilePressed = false;

    // Editor options
    state.btnUndoPressed = false;               // Undo last action recorded
    state.btnRedoPressed = false;               // Redo last action recorded
    state.snapModeActive = true;                // Toggle snap to grid mode

    // Selected control edit options
    state.btnEditTextPressed = false;           // Edit control text
    state.btnEditNamePressed = false;           // Edit control name
    state.btnDuplicateControlPressed = false;   // Duplicate control
    state.btnUnlinkControlPressed = false;      // Unlink control from anchor
    state.btnDeleteControlPressed = false;      // Delete selected control
    
    // Selected anchor options
    state.btnEditAnchorNamePressed = false;     // Edit anchor name
    state.hideAnchorControlsActive = false;     // Toggle Hide/Show anchor linked controls
    state.btnUnlinkAnchorControlsPressed = false;  // Unlink all controls linked to selected anchor
    state.btnDeleteAnchorPressed = false;       // Delete selected anchor

    // Selected tracemap options
    state.btnLoadTracemapPressed = false;       // Load tracemap image
    state.hideTracemapActive = true;            // Hide/show tracemap image
    state.lockTracemapActive = false;           // Lock/unlock tracemap image
    state.tracemapAlphaValue = 0.7f;            // Adjust tracemap opacity
    state.btnDeleteTracemapPressed = false;     // Unload tracemap image

    // Visuals options
    state.showControlRecsActive = false;        // Show control rectangles
    state.showControlNamesActive = false;       // Show controls names
    state.showControlOrderActive = false;       // Show controls drawing order
    state.showControlPanelActive = true;        // Toggle control panel window
    state.showGridActive = true;                // Show/hide work grid

    state.visualStyleActive = 0;
    state.prevVisualStyleActive = 0;
    state.languageActive = 0;

    // Info options
    state.btnHelpPressed = false;
    state.btnAboutPressed = false;
    state.btnSponsorPressed = false;

    // Custom variables
    state.controlSelected = -1;
    state.anchorSelected = -1;
    state.tracemapLoaded = false;

    return state;
}

void GuiMainToolbar(GuiMainToolbarState *state)
{
    // Toolbar panels
    state->anchorRight.x = GetScreenWidth() - 104;       // Update right-anchor panel
    state->anchorVisuals.x = state->anchorRight.x - 324 + 1;    // Update right-anchor panel

    GuiPanel((Rectangle){ state->anchorFile.x, state->anchorFile.y, 160, 40 }, NULL);
    GuiPanel((Rectangle){ state->anchorEdit.x, state->anchorEdit.y, 112, 40 }, NULL);
    GuiPanel((Rectangle){ state->anchorTools.x, state->anchorTools.y, state->anchorVisuals.x - state->anchorTools.x + 1, 40 }, NULL);
    GuiPanel((Rectangle){ state->anchorVisuals.x, state->anchorVisuals.y, 324, 40 }, NULL);
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
    state->snapModeActive = GuiToggle((Rectangle){ state->anchorEdit.x + 12 + 48 + 16, state->anchorEdit.y + 8, 24, 24 }, "#50#", state->snapModeActive);

    // Selected control options
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiLabel((Rectangle){ state->anchorTools.x, state->anchorTools.y + 8, 64, 24 }, "Control:");
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    if (state->controlSelected == -1) GuiDisable();
    state->btnEditTextPressed = GuiButton((Rectangle){ state->anchorTools.x + 68, state->anchorTools.y + 8, 24, 24 }, "#30#");
    state->btnEditNamePressed = GuiButton((Rectangle){ state->anchorTools.x + 92 + 4, state->anchorTools.y + 8, 24, 24 }, "#31#");
    state->btnDuplicateControlPressed = GuiButton((Rectangle){ state->anchorTools.x + 116 + 8, state->anchorTools.y + 8, 24, 24 }, "#16#");
    state->btnUnlinkControlPressed = GuiButton((Rectangle){ state->anchorTools.x + 140 + 12, state->anchorTools.y + 8, 24, 24 }, "#175#");
    state->btnDeleteControlPressed = GuiButton((Rectangle){ state->anchorTools.x + 164 + 16, state->anchorTools.y + 8, 24, 24 }, "#143#");
    GuiEnable();

    // Selected anchor options
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiLabel((Rectangle){ state->anchorTools.x + 202, state->anchorTools.y + 8, 64, 24 }, "Anchor:");
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    if (state->anchorSelected == -1) GuiDisable();
    state->btnEditAnchorNamePressed = GuiButton((Rectangle){ state->anchorTools.x + 198 + 72, state->anchorTools.y + 8, 24, 24 }, "#31#");
    state->hideAnchorControlsActive = GuiToggle((Rectangle){ state->anchorTools.x + 198 + 72 + 24 + 4, state->anchorTools.y + 8, 24, 24 }, state->hideAnchorControlsActive? "#45#" : "#44#", state->hideAnchorControlsActive);
    state->btnUnlinkAnchorControlsPressed = GuiButton((Rectangle){ state->anchorTools.x + 198 + 72 + 48 + 8, state->anchorTools.y + 8, 24, 24 }, "#175#");
    state->btnDeleteAnchorPressed = GuiButton((Rectangle){ state->anchorTools.x + 198 + 72 + 72 + 12, state->anchorTools.y + 8, 24, 24 }, "#143#");
    GuiEnable();
    
    // Selected tracemap options
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiLabel((Rectangle){ state->anchorTools.x + 390, state->anchorTools.y + 8, 64, 24 }, "Tracemap:");
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    state->btnLoadTracemapPressed = GuiButton((Rectangle){ state->anchorTools.x + 390 + 70, state->anchorTools.y + 8, 24, 24 }, "#12#");
    if (!state->tracemapLoaded) GuiDisable();
    state->hideTracemapActive =  GuiToggle((Rectangle){ state->anchorTools.x + 390 + 70 + 24 + 4, state->anchorTools.y + 8, 24, 24 }, state->hideTracemapActive? "#45#" : "#44#", state->hideTracemapActive);
    state->lockTracemapActive =  GuiToggle((Rectangle){ state->anchorTools.x + 390 + 70 + 48 + 8, state->anchorTools.y + 8, 24, 24 }, state->lockTracemapActive? "#137#" : "#138#", state->lockTracemapActive);
    state->btnDeleteTracemapPressed = GuiButton((Rectangle){ state->anchorTools.x + 390 + 70 + 72 + 12, state->anchorTools.y + 8, 24, 24 }, "#143#");
    //state->tracemapAlphaValue = 0.8f;            // TODO: Adjust tracemap opacity
    GuiEnable();

    // Draw separation line rectangle for convenience
    DrawRectangle(state->anchorTools.x + 380 + 72 + 96 + 35, state->anchorTools.y, 1, 40, GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)));

    // Visuals options
    state->showControlRecsActive = GuiToggle((Rectangle){ state->anchorVisuals.x + 12 , state->anchorVisuals.y + 8, 24, 24 }, "#98#", state->showControlRecsActive);
    state->showControlNamesActive = GuiToggle((Rectangle){ state->anchorVisuals.x + 12 + 24 + 4 , state->anchorVisuals.y + 8, 24, 24 }, "#214#", state->showControlNamesActive);
    state->showControlOrderActive = GuiToggle((Rectangle){ state->anchorVisuals.x + 12 + 48 + 8, state->anchorVisuals.y + 8, 24, 24 }, "#197#", state->showControlOrderActive);
    state->showControlPanelActive = GuiToggle((Rectangle){ state->anchorVisuals.x + 12 + 72 + 12, state->anchorVisuals.y + 8, 24, 24 }, "#101#", state->showControlPanelActive);
    state->showGridActive = GuiToggle((Rectangle){ state->anchorVisuals.x + 12 + 96 + 16, state->anchorVisuals.y + 8, 24, 24 }, "#97#", state->showGridActive);
    
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiLabel((Rectangle){ state->anchorVisuals.x + 134, state->anchorVisuals.y + 8, 60, 24 }, "Style:");
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_WIDTH, 40);
    state->visualStyleActive = GuiComboBox((Rectangle){ state->anchorVisuals.x + 148 + 48, state->anchorVisuals.y + 8, 120, 24 }, "Light;Jungle;Candy;Lavanda;Cyber;Terminal;Ashes;Bluish;Dark;Cherry;Sunny;Enefete", state->visualStyleActive);
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_WIDTH, 32);

    // Info options
    state->btnHelpPressed = GuiButton((Rectangle){ state->anchorRight.x + (GetScreenWidth() - state->anchorRight.x) - 12 - 72 - 8, state->anchorRight.y + 8, 24, 24 }, "#193#");
    state->btnAboutPressed = GuiButton((Rectangle){ state->anchorRight.x + (GetScreenWidth() - state->anchorRight.x) - 12 - 48 - 4, state->anchorRight.y + 8, 24, 24 }, "#191#");
    state->btnSponsorPressed = GuiButton((Rectangle){ state->anchorRight.x + (GetScreenWidth() - state->anchorRight.x) - 12 - 24, state->anchorRight.y + 8, 24, 24 }, "#186#");
}

#endif // GUI_MAIN_TOOLBAR_IMPLEMENTATION
