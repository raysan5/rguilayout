/*******************************************************************************************
*
*   rGuiLayout - Controls Palette
*
*   MODULE USAGE:
*       #define GUI_CONTROLS_PALETTE_IMPLEMENTATION
*       #include "gui_controls_palette.h"
*
*       INIT: GuiControlsPaletteState state = GUI_CONTROLS_PALETTE_STATE_DEFAULT;
*       DRAW: GuiControlsPalette(&state);
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018-2026 raylib technologies (@raylibtech) / Ramon Santamaria (@raysan5)
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

#ifndef GUI_CONTROLS_PALETTE_H
#define GUI_CONTROLS_PALETTE_H

#define GUI_CONTROLS_PALETTE_COUNT  24

typedef struct GuiControlsPaletteState {

    bool panelActive;

    // Config options
    int selectedControl;
    Rectangle controlRecs[GUI_CONTROLS_PALETTE_COUNT]; // WARNING: Do not go out of bounds!
    Vector2 containerScrollOffset;

    // Controls contained variables
    bool windowBoxActive;                   // GuiWindowBox()
    bool buttonPressed;                     // GuiButton()
    bool labelBtnPressed;                   // GuiLabelButton()
    bool toggleActive;                      // GuiToggle()
    int toggleGroupActive;                  // GuiToggleGroup()
    bool checkBoxChecked;                   // GuiCheckBox()
    bool textBoxEditMode;                   // GuiTextBox()
    char textBoxText[64];
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

} GuiControlsPaletteState;

// Default control state
#define GUI_CONTROLS_PALETTE_STATE_DEFAULT \
    { \
        .selectedControl = GUI_WINDOWBOX, \
        .windowBoxActive = true, \
        .sliderValue = 50.0f, \
        .sliderBarValue = 40.0f, \
        .progressBarValue = 60.0f, \
        .colorPickerValue = RED \
    }

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void GuiControlsPalette(Rectangle bounds, const char *text, GuiControlsPaletteState *state);

#ifdef __cplusplus
}
#endif

#endif // GUI_CONTROLS_PALETTE_H

/***********************************************************************************
*
*   GUI_CONTROLS_PALETTE IMPLEMENTATION
*
************************************************************************************/
#if defined(GUI_CONTROLS_PALETTE_IMPLEMENTATION)

#include "raygui.h"

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void GuiControlsPalette(Rectangle bounds, const char *text, GuiControlsPaletteState *state)
{
    if (state->panelActive)
    {       
        // Update selected control palette panel logic
        //----------------------------------------------------------------------------------------------
        if (!GuiIsLocked())
        {
            for (int i = 0; i < GUI_CONTROLS_PALETTE_COUNT; i++)
            {
                if (CheckCollisionPointRec(GetMousePosition(), state->controlRecs[i]))
                {
                    state->selectedControl = i;
                    break;
                }
            }

            // Update controls palette rectangles (in case bounds change)
            state->controlRecs[GUI_WINDOWBOX] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 16, 120, 48 };
            state->controlRecs[GUI_GROUPBOX] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 76, 120, 32 };
            state->controlRecs[GUI_LINE] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 116, 120, 16 };
            state->controlRecs[GUI_PANEL] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 140, 120, 36 };
            state->controlRecs[GUI_LABEL] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 184, 120, 24 };
            state->controlRecs[GUI_BUTTON] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 216, 120, 24 };
            state->controlRecs[GUI_LABELBUTTON] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 248, 120, 24 };
            state->controlRecs[GUI_CHECKBOX] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 280, 24, 24 };      // Same line
            state->controlRecs[GUI_TOGGLE] = (Rectangle){ bounds.x + 52, bounds.y + state->containerScrollOffset.y + 24 + 280, 88, 24 };        // Same line
            state->controlRecs[GUI_TOGGLEGROUP] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 312, 120, 24 };
            state->controlRecs[GUI_COMBOBOX] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 344, 120, 24 };
            state->controlRecs[GUI_DROPDOWNBOX] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 376, 120, 24 };
            state->controlRecs[GUI_TEXTBOX] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 408, 120, 24 };
            //state->controlRecs[GUI_TEXTBOXMULTI] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 440, 120, 72 };
            state->controlRecs[GUI_VALUEBOX] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 440, 120, 24 };
            state->controlRecs[GUI_SPINNER] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 472, 120, 24 };
            state->controlRecs[GUI_SLIDER] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 512, 120, 16 };
            state->controlRecs[GUI_SLIDERBAR] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 544, 120, 16 };
            state->controlRecs[GUI_PROGRESSBAR] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 576, 120, 16 };
            state->controlRecs[GUI_STATUSBAR] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 608, 120, 24 };
            state->controlRecs[GUI_SCROLLPANEL] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 640, 120, 72 };
            state->controlRecs[GUI_LISTVIEW] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 720, 120, 72 };
            state->controlRecs[GUI_COLORPICKER] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 800, 96, 96 };
            state->controlRecs[GUI_DUMMYREC] = (Rectangle){ bounds.x + 20, bounds.y + state->containerScrollOffset.y + 24 + 904, 120, 24 };
        }
        //----------------------------------------------------------------------------------------------

        // Draw window and controls
        //----------------------------------------------------------------------------------------
        Rectangle scissorRec = { 0 };
        GuiScrollPanel(bounds, text, (Rectangle){ bounds.x, bounds.y, bounds.width - 16, 944 }, // WARNING: Hardcoded content height!
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

#endif // GUI_WINDOW_CONTROLS_PALETTE_IMPLEMENTATION
