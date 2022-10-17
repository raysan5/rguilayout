/*******************************************************************************************
*
*   rGuiLayout - Controls Palette Panel
*
*   MODULE USAGE:
*       #define GUI_WINDOW_CONTROLS_PALETTE_IMPLEMENTATION
*       #include "gui_controls_palette.h"
*
*       INIT: GuiWindowControlsPaletteState state = InitGuiWindowControlsPalette();
*       DRAW: GuiWindowControlsPalette(&state);
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

#ifndef GUI_WINDOW_CONTROLS_PALETTE_H
#define GUI_WINDOW_CONTROLS_PALETTE_H

#define CONTROLS_PALETTE_COUNT  24

typedef struct GuiWindowControlsPaletteState {

    bool windowActive;
    Rectangle windowBounds;
    Vector2 panOffset;
    bool dragMode;
    bool supportDrag;

    Rectangle scrollPanelBounds;
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
    bool multitextBoxEditMode;              // GuiTextBoxMulti()
    unsigned char multitextBoxText[64];
    bool valueBoxEditMode;                  // GuiValueBox()
    int valueBoxValue;
    bool spinnerEditMode;                   // GuiSpinner()
    int spinnerValue;
    float sliderValue;                      // GuiSlider()
    float sliderBarValue;                   // GuiSliderbar()
    float progressBarValue;                 // GuiProgressBar()
    Vector2 scrollPanelScrollOffset;        // GuiScrollPanel()
    Vector2 scrollPanelBoundsOffset;
    int listViewScrollIndex;                // GuiListView()
    int listViewActive;
    Color colorPickerValue;                 // GuiColorPicker()
    int comboBoxActive;                     // GuiComboBox()
    bool dropdownBoxEditMode;               // GuiDropdownBox()
    int dropdownBoxActive;

} GuiWindowControlsPaletteState;

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
GuiWindowControlsPaletteState InitGuiWindowControlsPalette(void);
void GuiWindowControlsPalette(GuiWindowControlsPaletteState *state);
void UpdateControlsPaletteRecs(GuiWindowControlsPaletteState *state);

#ifdef __cplusplus
}
#endif

#endif // GUI_WINDOW_CONTROLS_PALETTE_H

/***********************************************************************************
*
*   GUI_CONTROLS_PALETTE IMPLEMENTATION
*
************************************************************************************/
#if defined(GUI_WINDOW_CONTROLS_PALETTE_IMPLEMENTATION)

#include "raygui.h"

GuiWindowControlsPaletteState InitGuiWindowControlsPalette(void)
{
    GuiWindowControlsPaletteState state = { 0 };

    state.windowActive = true;
    state.windowBounds = (Rectangle){ GetScreenWidth() - 180, 40, 180, GetScreenHeight() - 40 - 24 };
    state.selectedControl = GUI_WINDOWBOX;

    state.panOffset = (Vector2){ 0, 0 };
    state.dragMode = false;
    state.supportDrag = true;

    // Controls default states
    state.windowBoxActive = true;
    state.buttonPressed = false;
    state.labelBtnPressed = false;
    state.toggleActive = false;
    state.checkBoxChecked = false;
    state.textBoxEditMode = false;
    strcpy((char *)state.textBoxText, "TextBox");
    state.multitextBoxEditMode = false;
    strcpy((char *)state.multitextBoxText, "MultiTextBox");
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

void GuiWindowControlsPalette(GuiWindowControlsPaletteState *state)
{
    if (state->windowActive)
    {
        // Update window dragging
        //----------------------------------------------------------------------------------------
        if (state->supportDrag)
        {
            Vector2 mousePosition = GetMousePosition();

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                // Window can be dragged from the top window bar
                if (CheckCollisionPointRec(mousePosition, (Rectangle){ state->windowBounds.x, state->windowBounds.y, state->windowBounds.width, RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT }))
                {
                    state->dragMode = true;
                    state->panOffset.x = mousePosition.x - state->windowBounds.x;
                    state->panOffset.y = mousePosition.y - state->windowBounds.y;
                }
            }

            if (state->dragMode)
            {
                state->windowBounds.x = (mousePosition.x - state->panOffset.x);
                state->windowBounds.y = (mousePosition.y - state->panOffset.y);

                // Check screen limits to avoid moving out of screen
                if (state->windowBounds.x < 0) state->windowBounds.x = 0;
                else if (state->windowBounds.x > (GetScreenWidth() - state->windowBounds.width)) state->windowBounds.x = GetScreenWidth() - state->windowBounds.width;

                if (state->windowBounds.y < 40) state->windowBounds.y = 40;
                else if (state->windowBounds.y > (GetScreenHeight() - state->windowBounds.height - 24)) state->windowBounds.y = GetScreenHeight() - state->windowBounds.height - 24;

                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) state->dragMode = false;
            }
        }
        else state->windowBounds = (Rectangle){ GetScreenWidth() - 200, 60, 180, GetScreenHeight() - 60 - 30 };
        //----------------------------------------------------------------------------------------
        
        // Update selected control alette panel logic
        //----------------------------------------------------------------------------------------------
        for (int i = 0; i < CONTROLS_PALETTE_COUNT; i++)
        {
            if (CheckCollisionPointRec(GetMousePosition(), state->controlRecs[i]))
            {
                state->selectedControl = i;
                break;
            }
        }

        UpdateControlsPaletteRecs(state);       // Update all controls rectangles (in case screen changed)
        //----------------------------------------------------------------------------------------------

        // Draw window and controls
        //----------------------------------------------------------------------------------------
        //state->windowBounds = (Rectangle){ GetScreenWidth() - 200, 60, state->scrollPanelBounds.width, GetScreenHeight() - 60 - 30 };
        state->windowActive = !GuiWindowBox(state->windowBounds, "#101#Controls Palette");

        state->scrollPanelBounds = (Rectangle){ state->windowBounds.x, state->windowBounds.y + 24 - 1, state->windowBounds.width, state->windowBounds.height - 24 };
        Rectangle scissorRec = GuiScrollPanel(state->scrollPanelBounds, NULL, (Rectangle){ state->windowBounds.x, state->windowBounds.y, state->windowBounds.width - 12, 984 + 12 }, &state->containerScrollOffset);

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
            state->checkBoxChecked = GuiCheckBox(state->controlRecs[GUI_CHECKBOX], "", state->checkBoxChecked);
            state->toggleActive = GuiToggle(state->controlRecs[GUI_TOGGLE], "Toggle", state->toggleActive);
            state->toggleGroupActive = GuiToggleGroup((Rectangle) { state->controlRecs[GUI_TOGGLEGROUP].x, state->controlRecs[GUI_TOGGLEGROUP].y, (state->controlRecs[GUI_TOGGLEGROUP].width - GuiGetStyle(TOGGLE, GROUP_PADDING)*2.0f)/3.0f, state->controlRecs[GUI_TOGGLEGROUP].height }, "ONE;TWO;THREE", state->toggleGroupActive);
            state->comboBoxActive = GuiComboBox(state->controlRecs[GUI_COMBOBOX], "ONE;TWO;THREE", state->comboBoxActive);
            if (GuiTextBox(state->controlRecs[GUI_TEXTBOX], state->textBoxText, 64, state->textBoxEditMode)) state->textBoxEditMode = !state->textBoxEditMode;
            if (GuiTextBoxMulti(state->controlRecs[GUI_TEXTBOXMULTI], state->multitextBoxText, 64, state->multitextBoxEditMode)) state->multitextBoxEditMode = !state->multitextBoxEditMode;
            if (GuiValueBox(state->controlRecs[GUI_VALUEBOX], NULL, &state->valueBoxValue, 0, 100, state->valueBoxEditMode)) state->valueBoxEditMode = !state->valueBoxEditMode;
            if (GuiSpinner(state->controlRecs[GUI_SPINNER], NULL, &state->spinnerValue, 0, 100, state->spinnerEditMode)) state->spinnerEditMode = !state->spinnerEditMode;
            state->sliderValue = GuiSlider(state->controlRecs[GUI_SLIDER], NULL, NULL, state->sliderValue, 0, 100);
            state->sliderBarValue = GuiSliderBar(state->controlRecs[GUI_SLIDERBAR], NULL, NULL, state->sliderBarValue, 0, 100);
            state->progressBarValue = GuiProgressBar(state->controlRecs[GUI_PROGRESSBAR], NULL, NULL, state->progressBarValue, 0, 100);
            GuiStatusBar(state->controlRecs[GUI_STATUSBAR], "StatusBar");
            GuiScrollPanel((Rectangle){ state->controlRecs[GUI_SCROLLPANEL].x, state->controlRecs[GUI_SCROLLPANEL].y, state->controlRecs[GUI_SCROLLPANEL].width - state->scrollPanelBoundsOffset.x, state->controlRecs[GUI_SCROLLPANEL].height - state->scrollPanelBoundsOffset.y }, NULL, state->controlRecs[GUI_SCROLLPANEL], &state->scrollPanelScrollOffset);
            state->listViewActive = GuiListView(state->controlRecs[GUI_LISTVIEW], "ONE;TWO", &state->listViewScrollIndex, state->listViewActive);
            state->colorPickerValue = GuiColorPicker(state->controlRecs[GUI_COLORPICKER], NULL, state->colorPickerValue);
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

void UpdateControlsPaletteRecs(GuiWindowControlsPaletteState *state)
{
    state->controlRecs[GUI_WINDOWBOX] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 16, 120, 48 };
    state->controlRecs[GUI_GROUPBOX] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 72, 120, 32 };
    state->controlRecs[GUI_LINE] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 112, 120, 24 };
    state->controlRecs[GUI_PANEL] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 144, 120, 32 };
    state->controlRecs[GUI_LABEL] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 184, 120, 24 };
    state->controlRecs[GUI_BUTTON] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 216, 120, 24 };
    state->controlRecs[GUI_LABELBUTTON] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 248, 120, 24 };
    state->controlRecs[GUI_CHECKBOX] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 284, 16, 16 };      // Same line
    state->controlRecs[GUI_TOGGLE] = (Rectangle){ state->windowBounds.x + 44, state->windowBounds.y + state->containerScrollOffset.y + 24 + 280, 96, 24 };        // Same line
    state->controlRecs[GUI_TOGGLEGROUP] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 312, 120, 24 };
    state->controlRecs[GUI_COMBOBOX] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 344, 120, 24 };
    state->controlRecs[GUI_DROPDOWNBOX] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 376, 120, 24 };
    state->controlRecs[GUI_TEXTBOX] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 408, 120, 24 };
    state->controlRecs[GUI_TEXTBOXMULTI] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 440, 120, 72 };
    state->controlRecs[GUI_VALUEBOX] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 520, 120, 24 };
    state->controlRecs[GUI_SPINNER] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 552, 120, 24 };
    state->controlRecs[GUI_SLIDER] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 584, 120, 16 };
    state->controlRecs[GUI_SLIDERBAR] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 608, 120, 16 };
    state->controlRecs[GUI_PROGRESSBAR] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 632, 120, 16 };
    state->controlRecs[GUI_STATUSBAR] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 656, 120, 24 };
    state->controlRecs[GUI_SCROLLPANEL] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 688, 120, 72 };
    state->controlRecs[GUI_LISTVIEW] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 768, 120, 72 };
    state->controlRecs[GUI_COLORPICKER] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 848, 96, 96 };
    state->controlRecs[GUI_DUMMYREC] = (Rectangle){ state->windowBounds.x + 20, state->windowBounds.y + state->containerScrollOffset.y + 24 + 952, 120, 24 };
}

#endif // GUI_WINDOW_CONTROLS_PALETTE_IMPLEMENTATION
