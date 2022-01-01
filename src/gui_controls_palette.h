/*******************************************************************************************
*
*   ControlsPalette v1.0.0 - Controls Palette Panel
*
*   MODULE USAGE:
*       #define GUI_CONTROLS_PALETTE_IMPLEMENTATION
*       #include "gui_controls_palette.h"
*
*       INIT: GuiControlsPaletteState state = InitGuiControlsPalette();
*       DRAW: GuiControlsPalette(&state);
*
*   LICENSE: Propietary License
*
*   Copyright (c) 2018 raylib technologies (@raylibtech). All Rights Reserved.
*
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   This project is proprietary and confidential unless the owner allows
*   usage in any other form by expresely written permission.
*
**********************************************************************************************/

#include "raylib.h"

// WARNING: raygui implementation is expected to be defined before including this header
#undef RAYGUI_IMPLEMENTATION
#include "raygui.h"

#ifndef GUI_CONTROLS_PALETTE_H
#define GUI_CONTROLS_PALETTE_H

#define CONTROLS_PALETTE_COUNT  24

typedef struct GuiControlsPaletteState {
    Vector2 controlsAnchor;
    Vector2 containerScrollOffset;

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

    Rectangle scrollPanelBounds;

    Rectangle controlRecs[CONTROLS_PALETTE_COUNT];   // WARNING: Do not go out of bounds!

    // Custom state variables (depend on development software)
    // NOTE: This variables should be added manually if required
    int selectedControl;

} GuiControlsPaletteState;

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
GuiControlsPaletteState InitGuiControlsPalette(void);
void GuiControlsPalette(GuiControlsPaletteState *state);
void UpdateControlsPaletteRecs(GuiControlsPaletteState *state);

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

GuiControlsPaletteState InitGuiControlsPalette(void)
{
    GuiControlsPaletteState state = { 0 };

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

    // Custom variables initialization
    state.selectedControl = GUI_WINDOWBOX;

    return state;
}

void GuiControlsPalette(GuiControlsPaletteState *state)
{
    // Palette panel logic
    //----------------------------------------------------------------------------------------------
    for (int i = 0; i < CONTROLS_PALETTE_COUNT; i++)
    {
        if (CheckCollisionPointRec(GetMousePosition(), state->controlRecs[i]))
        {
            state->selectedControl = i;
            break;
        }
    }

    state->controlsAnchor = (Vector2){ state->scrollPanelBounds.x - 5, state->containerScrollOffset.y + state->scrollPanelBounds.y };

    UpdateControlsPaletteRecs(state);       // Update all controls rectangles (in case screen changed)
    //----------------------------------------------------------------------------------------------

    GuiScrollPanel(state->scrollPanelBounds, (Rectangle){ state->scrollPanelBounds.x, state->scrollPanelBounds.y, state->scrollPanelBounds.width - 10, 965 }, &state->containerScrollOffset);

    GuiLock();

    state->windowBoxActive = !GuiWindowBox(state->controlRecs[GUI_WINDOWBOX], "WindowBox");
    GuiGroupBox(state->controlRecs[GUI_GROUPBOX], "GroupBox");
    GuiLine(state->controlRecs[GUI_LINE], NULL);
    GuiPanel(state->controlRecs[GUI_PANEL]);
    GuiLabel(state->controlRecs[GUI_LABEL], "Label");
    state->buttonPressed = GuiButton(state->controlRecs[GUI_BUTTON], "Button");
    state->labelBtnPressed = GuiLabelButton(state->controlRecs[GUI_LABELBUTTON], "LabelButton");
    state->checkBoxChecked = GuiCheckBox(state->controlRecs[GUI_CHECKBOX], "", state->checkBoxChecked);
    state->toggleActive = GuiToggle(state->controlRecs[GUI_TOGGLE], "Toggle", state->toggleActive);
    state->toggleGroupActive = GuiToggleGroup((Rectangle) { state->controlRecs[GUI_TOGGLEGROUP].x, state->controlRecs[GUI_TOGGLEGROUP].y, 40, state->controlRecs[GUI_TOGGLEGROUP].height }, "ONE;TWO;THREE", state->toggleGroupActive);
    state->comboBoxActive = GuiComboBox(state->controlRecs[GUI_COMBOBOX], "ONE;TWO;THREE", state->comboBoxActive);
    if (GuiTextBox(state->controlRecs[GUI_TEXTBOX], state->textBoxText, 64, state->textBoxEditMode)) state->textBoxEditMode = !state->textBoxEditMode;
    if (GuiTextBoxMulti(state->controlRecs[GUI_TEXTBOXMULTI], state->multitextBoxText, 64, state->multitextBoxEditMode)) state->multitextBoxEditMode = !state->multitextBoxEditMode;
    if (GuiValueBox(state->controlRecs[GUI_VALUEBOX], NULL, &state->valueBoxValue, 0, 100, state->valueBoxEditMode)) state->valueBoxEditMode = !state->valueBoxEditMode;
    if (GuiSpinner(state->controlRecs[GUI_SPINNER], NULL, &state->spinnerValue, 0, 100, state->spinnerEditMode)) state->spinnerEditMode = !state->spinnerEditMode;
    state->sliderValue = GuiSlider(state->controlRecs[GUI_SLIDER], NULL, NULL, state->sliderValue, 0, 100);
    state->sliderBarValue = GuiSliderBar(state->controlRecs[GUI_SLIDERBAR], NULL, NULL, state->sliderBarValue, 0, 100);
    state->progressBarValue = GuiProgressBar(state->controlRecs[GUI_PROGRESSBAR], NULL, NULL, state->progressBarValue, 0, 100);
    GuiStatusBar(state->controlRecs[GUI_STATUSBAR], "StatusBar");
    GuiScrollPanel((Rectangle){state->controlRecs[GUI_SCROLLPANEL].x, state->controlRecs[GUI_SCROLLPANEL].y, state->controlRecs[GUI_SCROLLPANEL].width - state->scrollPanelBoundsOffset.x, state->controlRecs[GUI_SCROLLPANEL].height - state->scrollPanelBoundsOffset.y }, state->controlRecs[GUI_SCROLLPANEL], &state->scrollPanelScrollOffset);
    state->listViewActive = GuiListView(state->controlRecs[GUI_LISTVIEW], "ONE;TWO", &state->listViewScrollIndex, state->listViewActive );
    state->colorPickerValue = GuiColorPicker(state->controlRecs[GUI_COLORPICKER], state->colorPickerValue);
    GuiDummyRec(state->controlRecs[GUI_DUMMYREC], "DummyRec");
    if (GuiDropdownBox(state->controlRecs[GUI_DROPDOWNBOX], "ONE;TWO;THREE", &state->dropdownBoxActive, state->dropdownBoxEditMode)) state->dropdownBoxEditMode = !state->dropdownBoxEditMode;

    GuiUnlock();

    // Draw selected control rectangle
    DrawRectangleRec(state->controlRecs[state->selectedControl], Fade(RED, 0.2f));
    DrawRectangleLinesEx(state->controlRecs[state->selectedControl], 1, MAROON);
}

void UpdateControlsPaletteRecs(GuiControlsPaletteState *state)
{
    state->controlRecs[GUI_WINDOWBOX] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 15, 125, 50 };
    state->controlRecs[GUI_GROUPBOX] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 75, 125, 30 };
    state->controlRecs[GUI_LINE] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 105, 125, 25 };
    state->controlRecs[GUI_PANEL] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 135, 125, 35 };
    state->controlRecs[GUI_LABEL] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 175, 126, 25 };
    state->controlRecs[GUI_BUTTON] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 205, 125, 30 };
    state->controlRecs[GUI_LABELBUTTON] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 240, 125, 30 };
    state->controlRecs[GUI_CHECKBOX] = (Rectangle){ state->controlsAnchor.x + 30, state->controlsAnchor.y + 280, 15, 15 };
    state->controlRecs[GUI_TOGGLE] = (Rectangle){ state->controlsAnchor.x + 55, state->controlsAnchor.y + 275, 90, 25 };
    state->controlRecs[GUI_TOGGLEGROUP] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 305, 125, 25 };
    state->controlRecs[GUI_COMBOBOX] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 335, 125, 25 };
    state->controlRecs[GUI_DROPDOWNBOX] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 365, 125, 25 };
    state->controlRecs[GUI_TEXTBOX] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 395, 125, 25 };
    state->controlRecs[GUI_TEXTBOXMULTI] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 425, 125, 75 };
    state->controlRecs[GUI_VALUEBOX] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 505, 125, 25 };
    state->controlRecs[GUI_SPINNER] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 535, 125, 25 };
    state->controlRecs[GUI_SLIDER] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 565, 125, 15 };
    state->controlRecs[GUI_SLIDERBAR] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 585, 125, 15 };
    state->controlRecs[GUI_PROGRESSBAR] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 605, 125, 15 };
    state->controlRecs[GUI_STATUSBAR] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 625, 125, 25 };
    state->controlRecs[GUI_SCROLLPANEL] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 655, 125, 75 };
    state->controlRecs[GUI_LISTVIEW] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 735, 125, 75 };
    state->controlRecs[GUI_COLORPICKER] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 815, 95, 95 };
    state->controlRecs[GUI_DUMMYREC] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 915, 125, 30 };
}

#endif // GUI_CONTROLS_PALETTE_IMPLEMENTATION
