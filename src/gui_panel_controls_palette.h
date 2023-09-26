/*******************************************************************************************
*
*   rGuiLayout - Controls Palette Panel
*
*   MODULE USAGE:
*       #define GUI_PANEL_CONTROLS_PALETTE_IMPLEMENTATION
*       #include "gui_panel_palette.h"
*
*       INIT: GuiPanelControlsPaletteState state = InitGuiPanelControlsPalette();
*       DRAW: GuiPanelControlsPalette(&state);
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018-2022 raylib technologies (@raylibtech) / Ramon Santamaria (@raysan5)
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
#undef RAYGUI_IMPLEMENTATION
#include "raygui.h"

#ifndef GUI_PANEL_CONTROLS_PALETTE_H
#define GUI_PANEL_CONTROLS_PALETTE_H

#define CONTROLS_PALETTE_COUNT  24

typedef struct GuiPanelControlsPaletteState {

    bool panelActive;
    Rectangle panelBounds;
    Vector2 panOffset;
    bool dragMode;
    bool supportDrag;

    Vector2 containerScrollOffset;

    Rectangle controlRecs[CONTROLS_PALETTE_COUNT];   // WARNING: Do not go out of bounds!
    int selectedControl;

    // Controls contained variables
    bool windowBoxActive;                   // GuiWindowBox()
    bool buttonPressed;                     // GuiButton()
    bool labelBtnPressed;                   // GuiLabelButton()
    bool toggleActive;                      // GuiToggle()
    int toggleGroupActive;                  // GuiToggleGroup()
    bool checkBoxChecked;                   // GuiCheckBox()
    bool textBoxEditMode;                   // GuiTextBox()
    unsigned char textBoxText[64];
    bool valueBoxEditMode;                  // GuiValueBox()
    int valueBoxValue;
    bool spinnerEditMode;                   // GuiSpinner()
    int spinnerValue;
    float sliderValue;                      // GuiSlider()
    float sliderBarValue;                   // GuiSliderbar()
    float progressBarValue;                 // GuiProgressBar()
    Vector2 scrollPanelScrollOffset;        // GuiScrollPanel()
    Rectangle scrollPanelView;
    Vector2 scrollPanelBoundsOffset;
    int listViewScrollIndex;                // GuiListView()
    int listViewActive;
    Color colorPickerValue;                 // GuiColorPicker()
    int comboBoxActive;                     // GuiComboBox()
    bool dropdownBoxEditMode;               // GuiDropdownBox()
    int dropdownBoxActive;

} GuiPanelControlsPaletteState;

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
GuiPanelControlsPaletteState InitGuiPanelControlsPalette(void);
void GuiPanelControlsPalette(GuiPanelControlsPaletteState *state);
void UpdateControlsPaletteRecs(GuiPanelControlsPaletteState *state);

#ifdef __cplusplus
}
#endif

#endif // GUI_PANEL_CONTROLS_PALETTE_H

/***********************************************************************************
*
*   GUI_PANEL_CONTROLS_PALETTE IMPLEMENTATION
*
************************************************************************************/
#if defined(GUI_PANEL_CONTROLS_PALETTE_IMPLEMENTATION)

#include "raygui.h"

GuiPanelControlsPaletteState InitGuiPanelControlsPalette(void)
{
    GuiPanelControlsPaletteState state = { 0 };

    state.panelActive = true;
    state.panelBounds = (Rectangle){ GetScreenWidth() - 168, 40, 168, GetScreenHeight() - 40 - 24 };
    state.selectedControl = GUI_WINDOWBOX;

    state.panOffset = (Vector2){ 0, 0 };
    state.dragMode = false;
    state.supportDrag = false;

    // Controls default states
    state.windowBoxActive = true;
    state.buttonPressed = false;
    state.labelBtnPressed = false;
    state.toggleActive = false;
    state.checkBoxChecked = false;
    state.textBoxEditMode = false;
    strcpy((char *)state.textBoxText, "TextBox");
    state.valueBoxEditMode = false;
    state.valueBoxValue = 0;
    state.spinnerEditMode = false;
    state.spinnerValue = 0;
    state.sliderValue = 50.0f;
    state.sliderBarValue = 40.0f;
    state.progressBarValue = 60.0f;
    state.scrollPanelScrollOffset = (Vector2){ 0, 0 };
    state.scrollPanelBoundsOffset = (Vector2){ 0, 0 };
    state.listViewScrollIndex = 0;
    state.listViewActive = 0;
    state.colorPickerValue = RED;
    state.toggleGroupActive = -1;
    state.comboBoxActive = 0;
    state.dropdownBoxEditMode = false;
    state.dropdownBoxActive = 0;

    UpdateControlsPaletteRecs(&state);

    return state;
}

void GuiPanelControlsPalette(GuiPanelControlsPaletteState *state)
{
    if (state->panelActive)
    {
        // Update ScrollPanel bounds in case window is resized
        state->panelBounds.width = ((GetScreenHeight() - 64) < (944 + 12))? 170 : 158;     // Check if scroll bar is needed
        state->panelBounds.y = 40;
        state->panelBounds.height = GetScreenHeight() - 24 - 40 + 2;

        // Update window dragging
        //----------------------------------------------------------------------------------------
        if (state->supportDrag)
        {
            // NOTE: Only drag on X-axis supported

            Vector2 mousePosition = GetMousePosition();

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                // Window can be dragged from the top window bar
                if (CheckCollisionPointRec(mousePosition, (Rectangle){ state->panelBounds.x, state->panelBounds.y, state->panelBounds.width, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT }))
                {
                    state->dragMode = true;
                    state->panOffset.x = mousePosition.x - state->panelBounds.x;
                    //state->panOffset.y = mousePosition.y - state->panelBounds.y;
                }
            }

            if (state->dragMode)
            {
                state->panelBounds.x = (mousePosition.x - state->panOffset.x);
                //state->panelBounds.y = (mousePosition.y - state->panOffset.y);

                // Check screen limits to avoid moving out of screen
                if (state->panelBounds.x < 0) state->panelBounds.x = 0;
                else if (state->panelBounds.x > (GetScreenWidth() - state->panelBounds.width)) state->panelBounds.x = GetScreenWidth() - state->panelBounds.width;
                
                //if (state->panelBounds.y < 40) state->panelBounds.y = 40;
                //else if (state->panelBounds.y > (GetScreenHeight() - state->panelBounds.height - 24 - 2)) state->panelBounds.y = GetScreenHeight() - state->panelBounds.height - 24;

                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) state->dragMode = false;
            }
        }
        else state->panelBounds.x = GetScreenWidth() - state->panelBounds.width;
        //----------------------------------------------------------------------------------------
        
        // Update selected control palette panel logic
        //----------------------------------------------------------------------------------------------
        if (!GuiIsLocked())
        {
            for (int i = 0; i < CONTROLS_PALETTE_COUNT; i++)
            {
                if (CheckCollisionPointRec(GetMousePosition(), state->controlRecs[i]))
                {
                    state->selectedControl = i;
                    break;
                }
            }

            UpdateControlsPaletteRecs(state);       // Update all controls rectangles (in case screen changed)
        }
        //----------------------------------------------------------------------------------------------

        // Draw window and controls
        //----------------------------------------------------------------------------------------
        Rectangle scissorRec = { 0 };
        GuiScrollPanel(state->panelBounds, "#101#Controls Palette", 
            (Rectangle){ state->panelBounds.x, state->panelBounds.y, state->panelBounds.width - 16, 944 }, // WARNING: Hardcoded content height!
            &state->containerScrollOffset, &scissorRec);

        // Limit drawing to scroll panel bounds
        // WARNING: It requires a batch processing and restart
        BeginScissorMode(scissorRec.x, scissorRec.y, scissorRec.width, scissorRec.height);

            GuiLock();

            state->windowBoxActive = !GuiWindowBox(state->controlRecs[GUI_WINDOWBOX], "WindowBox");
            GuiGroupBox(state->controlRecs[GUI_GROUPBOX], "GroupBox");
            GuiLine(state->controlRecs[GUI_LINE], NULL);
            GuiPanel(state->controlRecs[GUI_PANEL], NULL);
            GuiLabel(state->controlRecs[GUI_LABEL], "Label");
            state->buttonPressed = GuiButton(state->controlRecs[GUI_BUTTON], "Button");
            state->labelBtnPressed = GuiLabelButton(state->controlRecs[GUI_LABELBUTTON], "LabelButton");
            GuiCheckBox(state->controlRecs[GUI_CHECKBOX], "", &state->checkBoxChecked);
            GuiToggle(state->controlRecs[GUI_TOGGLE], "Toggle", &state->toggleActive);
            GuiToggleGroup((Rectangle) { state->controlRecs[GUI_TOGGLEGROUP].x, state->controlRecs[GUI_TOGGLEGROUP].y, (state->controlRecs[GUI_TOGGLEGROUP].width - GuiGetStyle(TOGGLE, GROUP_PADDING)*2.0f)/3.0f, state->controlRecs[GUI_TOGGLEGROUP].height }, "ONE;TWO;THREE", &state->toggleGroupActive);
            GuiComboBox(state->controlRecs[GUI_COMBOBOX], "ONE;TWO;THREE", &state->comboBoxActive);
            if (GuiTextBox(state->controlRecs[GUI_TEXTBOX], state->textBoxText, 64, state->textBoxEditMode)) state->textBoxEditMode = !state->textBoxEditMode;
            if (GuiValueBox(state->controlRecs[GUI_VALUEBOX], NULL, &state->valueBoxValue, 0, 100, state->valueBoxEditMode)) state->valueBoxEditMode = !state->valueBoxEditMode;
            if (GuiSpinner(state->controlRecs[GUI_SPINNER], NULL, &state->spinnerValue, 0, 100, state->spinnerEditMode)) state->spinnerEditMode = !state->spinnerEditMode;
            GuiSlider(state->controlRecs[GUI_SLIDER], NULL, NULL, &state->sliderValue, 0, 100);
            GuiSliderBar(state->controlRecs[GUI_SLIDERBAR], NULL, NULL, &state->sliderBarValue, 0, 100);
            GuiProgressBar(state->controlRecs[GUI_PROGRESSBAR], NULL, NULL, &state->progressBarValue, 0, 100);
            GuiStatusBar(state->controlRecs[GUI_STATUSBAR], "StatusBar");
            GuiScrollPanel(state->controlRecs[GUI_SCROLLPANEL], NULL, state->controlRecs[GUI_SCROLLPANEL], &state->scrollPanelScrollOffset, &state->scrollPanelView);
            GuiListView(state->controlRecs[GUI_LISTVIEW], "ONE;TWO", &state->listViewScrollIndex, &state->listViewActive);
            GuiColorPicker(state->controlRecs[GUI_COLORPICKER], NULL, &state->colorPickerValue);
            GuiDummyRec(state->controlRecs[GUI_DUMMYREC], "DummyRec");
            if (GuiDropdownBox(state->controlRecs[GUI_DROPDOWNBOX], "ONE;TWO;THREE", &state->dropdownBoxActive, state->dropdownBoxEditMode)) state->dropdownBoxEditMode = !state->dropdownBoxEditMode;

            // Draw selected control rectangle
            DrawRectangleRec(state->controlRecs[state->selectedControl], Fade(RED, 0.2f));
            DrawRectangleLinesEx(state->controlRecs[state->selectedControl], 1, MAROON);

            GuiUnlock();

        EndScissorMode();
        //----------------------------------------------------------------------------------------
    }
}

void UpdateControlsPaletteRecs(GuiPanelControlsPaletteState *state)
{
    state->controlRecs[GUI_WINDOWBOX] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 16, 120, 48 };
    state->controlRecs[GUI_GROUPBOX] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 76, 120, 32 };
    state->controlRecs[GUI_LINE] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 116, 120, 16 };
    state->controlRecs[GUI_PANEL] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 140, 120, 36 };
    state->controlRecs[GUI_LABEL] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 184, 120, 24 };
    state->controlRecs[GUI_BUTTON] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 216, 120, 24 };
    state->controlRecs[GUI_LABELBUTTON] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 248, 120, 24 };
    state->controlRecs[GUI_CHECKBOX] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 280, 24, 24 };      // Same line
    state->controlRecs[GUI_TOGGLE] = (Rectangle){ state->panelBounds.x + 52, state->panelBounds.y + state->containerScrollOffset.y + 24 + 280, 88, 24 };        // Same line
    state->controlRecs[GUI_TOGGLEGROUP] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 312, 120, 24 };
    state->controlRecs[GUI_COMBOBOX] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 344, 120, 24 };
    state->controlRecs[GUI_DROPDOWNBOX] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 376, 120, 24 };
    state->controlRecs[GUI_TEXTBOX] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 408, 120, 24 };
    //state->controlRecs[GUI_TEXTBOXMULTI] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 440, 120, 72 };
    state->controlRecs[GUI_VALUEBOX] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 440, 120, 24 };
    state->controlRecs[GUI_SPINNER] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 472, 120, 24 };
    state->controlRecs[GUI_SLIDER] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 512, 120, 16 };
    state->controlRecs[GUI_SLIDERBAR] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 544, 120, 16 };
    state->controlRecs[GUI_PROGRESSBAR] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 576, 120, 16 };
    state->controlRecs[GUI_STATUSBAR] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 608, 120, 24 };
    state->controlRecs[GUI_SCROLLPANEL] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 640, 120, 72 };
    state->controlRecs[GUI_LISTVIEW] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 720, 120, 72 };
    state->controlRecs[GUI_COLORPICKER] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 800, 96, 96 };
    state->controlRecs[GUI_DUMMYREC] = (Rectangle){ state->panelBounds.x + 20, state->panelBounds.y + state->containerScrollOffset.y + 24 + 904, 120, 24 };
}

#endif // GUI_WINDOW_CONTROLS_PALETTE_IMPLEMENTATION
