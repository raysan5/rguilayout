/*******************************************************************************************
*
*   raygui layout - window_codegen
*
*   MODULE USAGE:
*       #define GUI_WINDOW_CODEGEN_IMPLEMENTATION
*       #include "gui_window_codegen.h"
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

#ifndef GUI_WINDOW_CODEGEN_H
#define GUI_WINDOW_CODEGEN_H

typedef struct {
    bool windowBoxActive = true;
    bool buttonPressed = false;
    bool labelBtnPressed = false;
    bool imageBtnPressed = false;
    bool toggleActive = true;
    bool checkBoxChecked = false;
    int toggleBtnActive= 0;
    int comboBoxActive= 0;
    bool dropdownBoxEditMode = false;
    int dropdownBoxActive = 0;
    bool textBoxEditMode = false;
    unsigned char textBoxText[64] = "TextBox";
    bool multitextBoxEditMode = false;
    unsigned char multitextBoxText[64] = "MultiTextBox";
    bool valueBoxEditMode = false;
    int valueBoxValue = 0;
    bool spinnerEditMode = false;
    int spinnerValue = 0;
    float sliderExValue = 0.0f;
    float sliderBarExValue = 0.0f;
    float progressBarExValue = 0.0f;
    bool listViewEditMode = false;
    int listViewActive = 0;
    Color colorPickerValue;

    // Custom state variables (depend on development software)
    // NOTE: This variables should be added manually if required

} GuiWindowCodegenState;

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
GuiWindowCodegenState InitGuiWindowCodegen(void);
void GuiWindowCodegen(GuiWindowCodegenState *state);

#ifdef __cplusplus
}
#endif

#endif // GUI_WINDOW_CODEGEN_H

/***********************************************************************************
*
*   GUI_WINDOW_CODEGEN IMPLEMENTATION
*
************************************************************************************/
#if defined(GUI_WINDOW_CODEGEN_IMPLEMENTATION)

#include "raygui.h"

GuiWindowCodegenState InitGuiWindowCodegen(void)
{
    GuiWindowCodegenState state = { 0 };

    state.windowBoxActive = true;
    state.buttonPressed = false;
    state.labelBtnPressed = false;
    state.imageBtnPressed = false;
    state.toggleActive = true;
    state.checkBoxChecked = false;
    state.toggleBtnActive= 0;
    state.comboBoxActive= 0;
    state.dropdownBoxEditMode = false;
    state.dropdownBoxActive = 0;
    state.textBoxEditMode = false;
    strcpy(state.textBoxText, "TextBox");
    state.multitextBoxEditMode = false;
    strcpy(state.multitextBoxText, "MultiTextBox");
    state.valueBoxEditMode = false;
    state.valueBoxValue = 0;
    state.spinnerEditMode = false;
    state.spinnerValue = 0;
    state.sliderExValue = 0.0f;
    state.sliderBarExValue = 0.0f;
    state.progressBarExValue = 0.0f;
    state.listViewEditMode = false;
    state.listViewActive = 0;
    state.colorPickerValue;

    // Custom variables initialization

    return state;
}

void GuiWindowCodegen(GuiWindowCodegenState *state)
{
    const char *toggleBtnTextList[3] = { "ONE", "TWO", "TRHEE" };
    const char *comboBoxTextList[3] = { "ONE", "TWO", "TRHEE" };
    const char *dropdownBoxTextList[3] = { "ONE", "TWO", "TRHEE" };
    const float sliderExMinValue = 0.0f;
    const float sliderExMaxValue = 100.0f;
    const float sliderBarExMinValue = 0.0f;
    const float sliderBarExMaxValue = 100.0f;
    const char *listViewTextList[3] = { "ONE", "TWO", "TRHEE" };
    
    if (state->dropdownBoxEditMode) GuiLock();

    if (state->windowBoxActive)
    {
        state->windowBoxActive = !GuiWindowBox((Rectangle){ 5, 5, 125, 50 }, "WindowBox");
        GuiPanel((Rectangle){ 0, 0, 135, 950 });
        GuiGroupBox((Rectangle){ 5, 70, 125, 30 }, "GroupBox");
        GuiLine((Rectangle){ 5, 105, 125, 25 }, 1);
        GuiPanel((Rectangle){ 5, 135, 125, 35 });
        GuiLabel((Rectangle){ 5, 175, 126, 25 }, "Label SAMPLE TEXT");
        if (GuiButton((Rectangle){ 5, 205, 125, 30 }, "Button")) button();
        
        
        state->toggleActive = GuiToggle((Rectangle){ 40, 275, 90, 25 }, "Toggle", state->toggleActive);
        state->checkBoxChecked = GuiCheckBox((Rectangle){ 15, 280, 15, 15 }, state->checkBoxChecked);
        state->toggleBtnActive = GuiToggleGroup((Rectangle){ 5, 305, 40, 25 }, toggleBtnTextList, state->3, state->toggleBtnActive);
        state->comboBoxActive = GuiComboBox((Rectangle){ 5, 335, 125, 25 }, comboBoxTextList, state->3, state->comboBoxActive);
        if (GuiTextBox((Rectangle){ 5, 395, 125, 25 }, state->textBoxText, state->64, state->textBoxEditMode)) state->textBoxEditMode = !state->textBoxEditMode;
        
        if (GuiValueBox((Rectangle){ 5, 505, 125, 25 }, state->valueBoxValue, 0, 100, state->valueBoxEditMode)) state->valueBoxEditMode = !state->valueBoxEditMode;
        if (GuiSpinner((Rectangle){ 5, 535, 125, 25 }, state->spinnerValue, 0, 100, 25, state->spinnerEditMode)) state->spinnerEditMode = !state->spinnerEditMode;
        state->sliderExValue = GuiSlider((Rectangle){ 5, 565, 125, 15 }, state->sliderExValue, 0, 100);
        state->sliderBarExValue = GuiSliderBar((Rectangle){ 5, 585, 125, 15 }, state->sliderBarExValue, 0, 100);
        state->progressBarExValue = GuiProgressBarEx((Rectangle){ 5, 605, 125, 15 }, state->progressBarExValue, 0, 100, true);
        GuiStatusBar((Rectangle){ 5, 625, 125, 25 }, state->statusBarText, 10);
        
        if (GuiListView((Rectangle){ 5, 735, 125, 75 }, listViewTextList, state->3, &state->listViewScrollIndex, &state->listViewActive, state->listViewEditMode)) state->listViewEditMode = !state->listViewEditMode;
        state->colorPickerValue = GuiColorPicker((Rectangle){ 5, 815, 95, 95 }, state->colorPickerValue);
        GuiDummyRec((Rectangle){ 5, 915, 125, 30 }, "DummyRec");
        if (GuiDropdownBox((Rectangle){ 5, 365, 125, 25 }, dropdownBoxTextList, state->3, &state->dropdownBoxActive, state->dropdownBoxEditMode)) state->dropdownBoxEditMode = !state->dropdownBoxEditMode;
    }
    
    GuiUnlock();
}

#endif // GUI_WINDOW_CODEGEN_IMPLEMENTATION
