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
    // Define anchors
    Vector2 anchor00;            // ANCHOR ID:0
    Vector2 anchor01;            // ANCHOR ID:1
    Vector2 anchor02;            // ANCHOR ID:2
    Vector2 anchor03;            // ANCHOR ID:3
    
    // Define controls variables
    bool WindowBox000Active = true;            // WindowBox: WindowBox000
    bool Button005Pressed = false;            // Button: Button005
    bool Toggle006Pressed = false;            // LabelButton: Toggle006
    bool ToggleGroup007Pressed = false;            // ImageButtonEx: ToggleGroup007
    bool CheckBox008Checked = false;            // CheckBoxEx: CheckBox008
    bool ComboBox009Active = true;            // Toggle: ComboBox009
    int DropdownBox010Active= 0;            // ToggleGroup: DropdownBox010
    int Spinner011Active= 0;            // ComboBox: Spinner011
    bool ValueBox012EditMode = false;
    int ValueBox012Active = 0;            // DropdownBox: ValueBox012
    bool TextBox013EditMode = false;
    unsigned char TextBox013Text[64] = "SAMPLE TEXT";            // TextBox: TextBox013
    bool Slider014EditMode = false;
    unsigned char Slider014Text[64] = "";            // TextmultiBox: Slider014
    bool SliderBar015EditMode = false;
    int SliderBar015Value = 0;            // ValueBOx: SliderBar015
    bool ProgressBar016EditMode = false;
    int ProgressBar016Value = 0;            // Spinner: ProgressBar016
    float StatusBar017Value = 0.0f;            // SliderEx: StatusBar017
    float ListView018Value = 0.0f;            // SliderBarEx: ListView018
    float ColorPicker019Value = 0.0f;            // PrograssBarEx: ColorPicker019
    bool WindowBox021Active = true;            // WindowBox: WindowBox021

    // Define rectangles
    Rectangle layoutRecs[22];

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

    // Init anchors
    state.anchor00 = { 36, 12 };            // ANCHOR ID:0
    state.anchor01 = { 64, 27 };            // ANCHOR ID:1
    state.anchor02 = { 43, 100 };            // ANCHOR ID:2
    state.anchor03 = { 163, 321 };            // ANCHOR ID:3
    
    // Initilize controls variables
    state.WindowBox000Active = true;            // WindowBox: WindowBox000
    state.Button005Pressed = false;            // Button: Button005
    state.Toggle006Pressed = false;            // LabelButton: Toggle006
    state.ToggleGroup007Pressed = false;            // ImageButtonEx: ToggleGroup007
    state.CheckBox008Checked = false;            // CheckBoxEx: CheckBox008
    state.ComboBox009Active = true;            // Toggle: ComboBox009
    state.DropdownBox010Active= 0;            // ToggleGroup: DropdownBox010
    state.Spinner011Active= 0;            // ComboBox: Spinner011
    state.ValueBox012EditMode = false;
    state.ValueBox012Active = 0;            // DropdownBox: ValueBox012
    state.TextBox013EditMode = false;
    strcpy(state.TextBox013Text, "SAMPLE TEXT");            // TextBox: TextBox013
    state.Slider014EditMode = false;
    strcpy(state.Slider014Text, "");            // TextmultiBox: Slider014
    state.SliderBar015EditMode = false;
    state.SliderBar015Value = 0;            // ValueBOx: SliderBar015
    state.ProgressBar016EditMode = false;
    state.ProgressBar016Value = 0;            // Spinner: ProgressBar016
    state.StatusBar017Value = 0.0f;            // SliderEx: StatusBar017
    state.ListView018Value = 0.0f;            // SliderBarEx: ListView018
    state.ColorPicker019Value = 0.0f;            // PrograssBarEx: ColorPicker019
    state.WindowBox021Active = true;            // WindowBox: WindowBox021

    // Init controls rectangles
    state.layoutRecs[0] = (Rectangle){ anchor01.x + 0, anchor01.y + 0, 125, 50 }// WindowBox: WindowBox000
    state.layoutRecs[1] = (Rectangle){ anchor02.x + 0, anchor02.y + 0, 125, 30 }// GroupBox: GroupBox001
    state.layoutRecs[2] = (Rectangle){ 36, 147, 125, 25 }// Line: Line002
    state.layoutRecs[3] = (Rectangle){ 33, 172, 125, 35 }// Panel: Panel003
    state.layoutRecs[4] = (Rectangle){ 36, 217, 126, 25 }// Label: Label004
    state.layoutRecs[5] = (Rectangle){ 26, 257, 125, 30 }// Button: Button005
    state.layoutRecs[6] = (Rectangle){ 34, 300, 90, 25 }// LabelButton: Toggle006
    state.layoutRecs[7] = (Rectangle){ 44, 338, 125, 25 }// ImageButtonEx: ToggleGroup007
    state.layoutRecs[8] = (Rectangle){ 71, 374, 15, 15 }// CheckBoxEx: CheckBox008
    state.layoutRecs[9] = (Rectangle){ 22, 397, 125, 25 }// Toggle: ComboBox009
    state.layoutRecs[10] = (Rectangle){ 98, 425, 29, 25 }// ToggleGroup: DropdownBox010
    state.layoutRecs[11] = (Rectangle){ 21, 465, 125, 25 }// ComboBox: Spinner011
    state.layoutRecs[12] = (Rectangle){ 15, 497, 125, 25 }// DropdownBox: ValueBox012
    state.layoutRecs[13] = (Rectangle){ 18, 527, 125, 25 }// TextBox: TextBox013
    state.layoutRecs[14] = (Rectangle){ 95, 570, 125, 15 }// TextmultiBox: Slider014
    state.layoutRecs[15] = (Rectangle){ 93, 597, 125, 15 }// ValueBOx: SliderBar015
    state.layoutRecs[16] = (Rectangle){ 99, 623, 125, 15 }// Spinner: ProgressBar016
    state.layoutRecs[17] = (Rectangle){ 91, 662, 125, 25 }// SliderEx: StatusBar017
    state.layoutRecs[18] = (Rectangle){ 244, 57, 125, 27 }// SliderBarEx: ListView018
    state.layoutRecs[19] = (Rectangle){ 191, 130, 95, 28 }// PrograssBarEx: ColorPicker019
    state.layoutRecs[20] = (Rectangle){ 191, 239, 125, 30 }// StatusBar: DummyRec020
    state.layoutRecs[21] = (Rectangle){ anchor03.x + 131, anchor03.y + -36, 125, 50 }// WindowBox: WindowBox021

    // Custom variables initialization

    return state;
}

void GuiWindowCodegen(GuiWindowCodegenState *state)
{
    // Const variables
    const char *Label004Text = "SAMPLE TEXT";    // Label: Label004
    const char *DropdownBox010TextList[3] = { "ONE", "TWO", "TRHEE" };    // ToggleGroup: DropdownBox010
    const char *Spinner011TextList[3] = { "ONE", "TWO", "TRHEE" };    // ComboBox: Spinner011
    const char *ValueBox012TextList[3] = { "ONE", "TWO", "TRHEE" };    // DropdownBox: ValueBox012
    const float StatusBar017MinValue = 0.0f;
    const float StatusBar017MaxValue = 100.0f;    // SliderEx: StatusBar017
    const float ListView018MinValue = 0.0f;
    const float ListView018MaxValue = 100.0f;    // SliderBarEx: ListView018
    
    // Draw controls
    if (state->ValueBox012EditMode) GuiLock();

    if (state->WindowBox000Active)
    {
        state->WindowBox000Active = !GuiWindowBox(state->layoutRecs[0], "SAMPLE TEXT");
    }
    if (state->WindowBox021Active)
    {
        state->WindowBox021Active = !GuiWindowBox(state->layoutRecs[21], "SAMPLE TEXT");
    }
    GuiGroupBox(state->layoutRecs[1], "SAMPLE TEXT");
    GuiLine(state->layoutRecs[2], 1);
    GuiPanel(state->layoutRecs[3]);
    GuiLabel(state->layoutRecs[4], Label004Text);
    if (GuiButton(state->layoutRecs[5], "SAMPLE TEXT")) Button005();
    
    
    state->CheckBox008Checked = GuiCheckBox(state->layoutRecs[8], state->CheckBox008Checked);
    state->ComboBox009Active = GuiToggle(state->layoutRecs[9], "", state->ComboBox009Active);
    state->DropdownBox010Active = GuiToggleGroup(state->layoutRecs[10], DropdownBox010TextList, state->DropdownBox010Count, state->DropdownBox010Active);
    state->Spinner011Active = GuiComboBox(state->layoutRecs[11], Spinner011TextList, state->Spinner011Count, state->Spinner011Active);
    if (GuiTextBox(state->layoutRecs[13], state->TextBox013Text, state->TextBox013Size, state->TextBox013EditMode)) state->TextBox013EditMode = !state->TextBox013EditMode;
    
    if (GuiValueBox(state->layoutRecs[15], state->SliderBar015Value, 0, 100, state->SliderBar015EditMode)) state->SliderBar015EditMode = !state->SliderBar015EditMode;
    if (GuiSpinner(state->layoutRecs[16], state->ProgressBar016Value, 0, 100, 25, state->ProgressBar016EditMode)) state->ProgressBar016EditMode = !state->ProgressBar016EditMode;
    state->StatusBar017Value = GuiSliderEx(state->layoutRecs[17], state->StatusBar017Value, StatusBar017MinValue, StatusBar017MaxValue, "SAMPLE TEXT", true);
    state->ListView018Value = GuiSliderBar(state->layoutRecs[18], state->ListView018Value, ListView018MinValue, ListView018MaxValue);
    state->ColorPicker019Value = GuiProgressBarEx(state->layoutRecs[19], state->ColorPicker019Value, 0, 100, true);
    GuiStatusBar(state->layoutRecs[20], state->DummyRec020Text, 10);
    if (GuiDropdownBox(state->layoutRecs[12], ValueBox012TextList, state->ValueBox012Count, &state->ValueBox012Active, state->ValueBox012EditMode)) state->ValueBox012EditMode = !state->ValueBox012EditMode;
    
    GuiUnlock();
}

#endif // GUI_WINDOW_CODEGEN_IMPLEMENTATION
