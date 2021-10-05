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

#define CONTROLS_PALETTE_COUNT  26

typedef struct {
    Vector2 controlsAnchor;
    Vector2 containerScrollOffset;

    bool windowBoxActive;
    bool buttonPressed;
    bool labelBtnPressed;
    bool imageBtnPressed;
    bool toggleActive;
    bool checkBoxChecked;
    bool textBoxEditMode;
    unsigned char textBoxText[64];
    bool multitextBoxEditMode;
    unsigned char multitextBoxText[64];
    bool valueBoxEditMode;
    int valueBoxValue;
    bool spinnerEditMode;
    int spinnerValue;
    float sliderExValue;
    float sliderBarExValue;
    float progressBarExValue;
    Vector2 scrollPanelScrollOffset;
    Vector2 scrollPanelBoundsOffset;
    int listViewScrollIndex;
    int listViewActive;
    Color colorPickerValue;
    int toggleGroupActive;
    int comboBoxActive;
    bool dropdownBoxEditMode;
    int dropdownBoxActive;

    Rectangle layoutRecs[CONTROLS_PALETTE_COUNT];

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

    state.controlsAnchor = (Vector2){ GetScreenWidth() - 165, 0 };
    state.containerScrollOffset = (Vector2){ 0, 0 };

    state.windowBoxActive = true;
    state.buttonPressed = false;
    state.labelBtnPressed = false;
    state.imageBtnPressed = false;
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
    state.sliderExValue = 50.0f;
    state.sliderBarExValue = 40.0f;
    state.progressBarExValue = 60.0f;
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

    // Custom variables initialization
    state.selectedControl = 0;

    return state;
}

void GuiControlsPalette(GuiControlsPaletteState *state)
{
    // Palette panel logic
    //----------------------------------------------------------------------------------------------
    for (int i = 1; i < CONTROLS_PALETTE_COUNT; i++)
    {
        if (CheckCollisionPointRec(GetMousePosition(), state->layoutRecs[i]))
        {
            state->selectedControl = i - 1;
            break;
        }
    }

    state->controlsAnchor = (Vector2){ GetScreenWidth() - 165, state->containerScrollOffset.y };

    UpdateControlsPaletteRecs(state);
    //----------------------------------------------------------------------------------------------

    GuiScrollPanel(state->layoutRecs[0], (Rectangle){ state->layoutRecs[0].x, state->layoutRecs[0].y, state->layoutRecs[0].width - 10, 965 }, &state->containerScrollOffset);

    // NOTE: Not required because panel already fills full screen height
    //BeginScissorMode(view.x, view.y, view.width, view.height);

        GuiLock();

        state->windowBoxActive = !GuiWindowBox(state->layoutRecs[1], "WindowBox");
        GuiGroupBox(state->layoutRecs[2], "GroupBox");
        GuiLine(state->layoutRecs[3], NULL);
        GuiPanel(state->layoutRecs[4]);
        GuiLabel(state->layoutRecs[5], "Label");
        state->buttonPressed = GuiButton(state->layoutRecs[6], "Button");
        state->labelBtnPressed = GuiLabelButton(state->layoutRecs[7], "LabelButton");
        state->imageBtnPressed = GuiImageButtonEx(state->layoutRecs[8], "IM", GetFontDefault().texture, (Rectangle){ 0, 0, 1, 1 });
        state->checkBoxChecked = GuiCheckBox(state->layoutRecs[9], "", state->checkBoxChecked);
        state->toggleActive = GuiToggle(state->layoutRecs[10], "Toggle", state->toggleActive);
        state->toggleGroupActive = GuiToggleGroup(state->layoutRecs[11], "ONE;TWO;THREE", state->toggleGroupActive);
        state->comboBoxActive = GuiComboBox(state->layoutRecs[12], "ONE;TWO;THREE", state->comboBoxActive);
        if (GuiTextBox(state->layoutRecs[14], state->textBoxText, 64, state->textBoxEditMode)) state->textBoxEditMode = !state->textBoxEditMode;
        if (GuiTextBoxMulti(state->layoutRecs[15], state->multitextBoxText, 64, state->multitextBoxEditMode)) state->multitextBoxEditMode = !state->multitextBoxEditMode;
        if (GuiValueBox(state->layoutRecs[16], NULL, &state->valueBoxValue, 0, 100, state->valueBoxEditMode)) state->valueBoxEditMode = !state->valueBoxEditMode;
        if (GuiSpinner(state->layoutRecs[17], NULL, &state->spinnerValue, 0, 100, state->spinnerEditMode)) state->spinnerEditMode = !state->spinnerEditMode;
        state->sliderExValue = GuiSlider(state->layoutRecs[18], NULL, NULL, state->sliderExValue, 0, 100);
        state->sliderBarExValue = GuiSliderBar(state->layoutRecs[19], NULL, NULL, state->sliderBarExValue, 0, 100);
        state->progressBarExValue = GuiProgressBar(state->layoutRecs[20], NULL, NULL, state->progressBarExValue, 0, 100);
        GuiStatusBar(state->layoutRecs[21], "StatusBar");
        GuiScrollPanel((Rectangle){state->layoutRecs[22].x, state->layoutRecs[22].y, state->layoutRecs[22].width - state->scrollPanelBoundsOffset.x, state->layoutRecs[22].height - state->scrollPanelBoundsOffset.y }, state->layoutRecs[22], &state->scrollPanelScrollOffset);
        state->listViewActive = GuiListView(state->layoutRecs[23], "ONE;TWO", &state->listViewScrollIndex, state->listViewActive );
        state->colorPickerValue = GuiColorPicker(state->layoutRecs[24], state->colorPickerValue);
        GuiDummyRec(state->layoutRecs[25], "DummyRec");
        if (GuiDropdownBox(state->layoutRecs[13], "ONE;TWO;THREE", &state->dropdownBoxActive, state->dropdownBoxEditMode)) state->dropdownBoxEditMode = !state->dropdownBoxEditMode;

        GuiUnlock();

    //EndScissorMode();

    // Draw selected control rectangle
    DrawRectangleRec(state->layoutRecs[state->selectedControl + 1], Fade(RED, 0.2f));
    DrawRectangleLinesEx(state->layoutRecs[state->selectedControl + 1], 1, MAROON);
}

void UpdateControlsPaletteRecs(GuiControlsPaletteState *state)
{
    state->layoutRecs[0] = (Rectangle){ GetScreenWidth() - 160, 0, 160, GetScreenHeight() };

    state->layoutRecs[1] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 5, 125, 50 };
    state->layoutRecs[2] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 70, 125, 30 };
    state->layoutRecs[3] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 105, 125, 25 };
    state->layoutRecs[4] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 135, 125, 35 };
    state->layoutRecs[5] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 175, 126, 25 };
    state->layoutRecs[6] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 205, 125, 30 };
    state->layoutRecs[7] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 240, 90, 30 };
    state->layoutRecs[8] = (Rectangle){ state->controlsAnchor.x + 115, state->controlsAnchor.y + 240, 30, 30 };
    state->layoutRecs[9] = (Rectangle){ state->controlsAnchor.x + 30, state->controlsAnchor.y + 280, 15, 15 };
    state->layoutRecs[10] = (Rectangle){ state->controlsAnchor.x + 55, state->controlsAnchor.y + 275, 90, 25 };
    state->layoutRecs[11] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 305, 40, 25 };
    state->layoutRecs[12] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 335, 125, 25 };
    state->layoutRecs[13] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 365, 125, 25 };
    state->layoutRecs[14] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 395, 125, 25 };
    state->layoutRecs[15] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 425, 125, 75 };
    state->layoutRecs[16] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 505, 125, 25 };
    state->layoutRecs[17] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 535, 125, 25 };
    state->layoutRecs[18] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 565, 125, 15 };
    state->layoutRecs[19] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 585, 125, 15 };
    state->layoutRecs[20] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 605, 125, 15 };
    state->layoutRecs[21] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 625, 125, 25 };
    state->layoutRecs[22] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 655, 125, 75 };
    state->layoutRecs[23] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 735, 125, 75 };
    state->layoutRecs[24] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 815, 95, 95 };
    state->layoutRecs[25] = (Rectangle){ state->controlsAnchor.x + 20, state->controlsAnchor.y + 915, 125, 30 };
}

#endif // GUI_CONTROLS_PALETTE_IMPLEMENTATION
