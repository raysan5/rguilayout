/*******************************************************************************************
*
*   raygui layout - LayoutName
*
*   MODULE USAGE:
*       #define GUI_LAYOUTNAME_IMPLEMENTATION
*       #include "gui_layoutname.h"
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

#ifndef LAYOUTNAME_H
#define LAYOUTNAME_H

typedef struct {
    // Define anchors
    Vector2 anchor01;    // ID:1 Name:anchor01
    Vector2 anchor02;    // ID:2 Name:anchor02
    
    // Define controls variables
    bool WindowBox000Active;    // WindowBox: WindowBox000
    bool Toggle006Active;    // Toggle: Toggle006
    int ToggleGroup007Count;
    int ToggleGroup007Active;    // ToggleGroup: ToggleGroup007
    bool CheckBox008Checked;    // CheckBox: CheckBox008
    int ComboBox009Count;
    int ComboBox009Active;    // ComboBox: ComboBox009
    int DropdownBox010Count;
    int DropdownBox010Active;
    bool DropdownBox010EditMode;    // DropdownBox: DropdownBox010
    int Spinner011Value;
    bool Spinner011EditMode;    // Spinner: Spinner011
    int ValueBox012Value;
    bool ValueBox012EditMode;    // ValueBox: ValueBox012
    int TextBox013Size;
    unsigned char TextBox013Text[64];
    bool TextBox013EditMode;    // TextBox: TextBox013
    float Slider014Value;    // Slider: Slider014
    float SliderBar015Value;    // SliderBar: SliderBar015
    float ProgressBar016Value;    // ProgressBar: ProgressBar016
    char *StatusBar017Text;    // StatusBar: StatusBar017
    int ListView018Count;
    int ListView018Active;
    bool ListView018EditMode;    // ListView: ListView018
    Color ColorPicker019Value;    // ColorPicker: ColorPicker019

    // Define controls rectangles
    Rectangle layoutRecs[21];
} GuiLayoutLayoutNameState;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
GuiLayoutLayoutNameState InitGuiLayoutLayoutName(void);
void GuiLayoutLayoutName (GuiLayoutLayoutNameState *state);

#ifdef __cplusplus
}
#endif

#endif // LAYOUTNAME_H

/***********************************************************************************
*
*   LAYOUTNAME IMPLEMENTATION
*
************************************************************************************/
#if defined(LAYOUTNAME_IMPLEMENTATION)

#include "raygui.h"

GuiLayoutLayoutNameState InitGuiLayoutLayoutName(void)
{
    GuiLayoutLayoutNameState state = { 0 };

    // Init anchors
    state.anchor01 = { 81, 52 };    // ID:1 Name:anchor01
    state.anchor02 = { 79, 112 };    // ID:2 Name:anchor02
    
    // Init controls variables
    state.state.WindowBox000Active = true;    // WindowBox: WindowBox000
    state.state.Toggle006Active = true;    // Toggle: Toggle006
    state.state.ToggleGroup007Count = 3;
    state.ToggleGroup007Active = 0;    // ToggleGroup: ToggleGroup007
    state.state.CheckBox008Checked = false;    // CheckBox: CheckBox008
    state.state.ComboBox009Count = 3;
    state.ComboBox009Active = 0;    // ComboBox: ComboBox009
    state.state.DropdownBox010Count = 3;
    state.DropdownBox010Active = 0;
    state.DropdownBox010EditMode = false;    // DropdownBox: DropdownBox010
    state.state.Spinner011Value = 0;
    state.Spinner011EditMode = false;    // Spinner: Spinner011
    state.state.ValueBox012Value = 0;
    state.ValueBox012EditMode = false;    // ValueBox: ValueBox012
    state.state.TextBox013Size = 64;
    strcpy(state.TextBox013Text,"SAMPLE TEXT");
    state.TextBox013EditMode = false;    // TextBox: TextBox013
    state.state.Slider014Value = 50.0f;    // Slider: Slider014
    state.state.SliderBar015Value = 50.0f;    // SliderBar: SliderBar015
    state.state.ProgressBar016Value = 50.0f;    // ProgressBar: ProgressBar016
    state.strcpy(state.StatusBar017Text, "SAMPLE TEXT");    // StatusBar: StatusBar017
    state.state.ListView018Count = 3;
    state.ListView018Active = 0;
    state.ListView018EditMode = false;    // ListView: ListView018
    state.state.ColorPicker019Value;    // ColorPicker: ColorPicker019

    // Init controls rectangles
    state.layoutRecs[0] = (Rectangle){ anchor01.x + 0, anchor01.y + -1, 125, 50 }// WindowBox: WindowBox000
    state.layoutRecs[1] = (Rectangle){ anchor02.x + 0, anchor02.y + 0, 125, 30 }// GroupBox: GroupBox001
    state.layoutRecs[2] = (Rectangle){ 72, 159, 125, 25 }// Line: Line002
    state.layoutRecs[3] = (Rectangle){ 69, 184, 125, 35 }// Panel: Panel003
    state.layoutRecs[4] = (Rectangle){ 72, 229, 126, 25 }// Label: Label004
    state.layoutRecs[5] = (Rectangle){ 62, 269, 125, 30 }// Button: Button005
    state.layoutRecs[6] = (Rectangle){ 70, 312, 90, 25 }// Toggle: Toggle006
    state.layoutRecs[7] = (Rectangle){ 60, 348, 125, 25 }// ToggleGroup: ToggleGroup007
    state.layoutRecs[8] = (Rectangle){ 107, 386, 15, 15 }// CheckBox: CheckBox008
    state.layoutRecs[9] = (Rectangle){ 58, 409, 125, 25 }// ComboBox: ComboBox009
    state.layoutRecs[10] = (Rectangle){ 57, 442, 125, 25 }// DropdownBox: DropdownBox010
    state.layoutRecs[11] = (Rectangle){ 57, 477, 125, 25 }// Spinner: Spinner011
    state.layoutRecs[12] = (Rectangle){ 51, 509, 125, 25 }// ValueBox: ValueBox012
    state.layoutRecs[13] = (Rectangle){ 54, 539, 125, 25 }// TextBox: TextBox013
    state.layoutRecs[14] = (Rectangle){ 131, 582, 125, 15 }// Slider: Slider014
    state.layoutRecs[15] = (Rectangle){ 129, 609, 125, 15 }// SliderBar: SliderBar015
    state.layoutRecs[16] = (Rectangle){ 135, 635, 125, 15 }// ProgressBar: ProgressBar016
    state.layoutRecs[17] = (Rectangle){ 59, 665, 125, 25 }// StatusBar: StatusBar017
    state.layoutRecs[18] = (Rectangle){ 228, 54, 125, 75 }// ListView: ListView018
    state.layoutRecs[19] = (Rectangle){ 227, 142, 95, 95 }// ColorPicker: ColorPicker019
    state.layoutRecs[20] = (Rectangle){ 227, 251, 125, 30 }// DummyRec: DummyRec020
}

void GuiLayoutLayoutName (GuiLayoutLayoutNameState *state)
{
    // Const variables
    const char *Label004Text = "SAMPLE TEXT";    // Label: Label004
    const char *ToggleGroup007TextList[3] = { "ONE", "TWO", "TRHEE" };    // ToggleGroup: ToggleGroup007
    const char *ComboBox009TextList[3] = { "ONE", "TWO", "TRHEE" };    // ComboBox: ComboBox009
    const char *DropdownBox010TextList[3] = { "ONE", "TWO", "TRHEE" };    // DropdownBox: DropdownBox010
    const float Slider014MinValue = 0.0f;
    const float Slider014MaxValue = 100.0f;    // Slider: Slider014
    const float SliderBar015MinValue = 0.0f;
    const float SliderBar015MaxValue = 100.0f;    // SliderBar: SliderBar015
    const char *ListView018TextList[3] = { "ONE", "TWO", "TRHEE" };    // ListView: ListView018
    
    // Draw controls
    if (state->DropdownBox010EditMode) GuiLock();

    if (state->WindowBox000Active)
    {
        state->WindowBox000Active = !GuiWindowBox(state->layoutRecs[0], "SAMPLE TEXT");
    }
    GuiGroupBox(state->layoutRecs[1], "SAMPLE TEXT");
    GuiLine(state->layoutRecs[2], 1);
    GuiPanel(state->layoutRecs[3]);
    GuiLabel(state->layoutRecs[4], Label004Text);
    if (GuiButton(state->layoutRecs[5], "SAMPLE TEXT")) Button005();
    state->Toggle006Active = GuiToggle(state->layoutRecs[6], "SAMPLE TEXT", state->Toggle006Active);
    state->ToggleGroup007Active = GuiToggleGroup(state->layoutRecs[7], ToggleGroup007TextList, state->ToggleGroup007Count, state->ToggleGroup007Active);
    state->CheckBox008Checked = GuiCheckBox(state->layoutRecs[8], state->CheckBox008Checked);
    state->ComboBox009Active = GuiComboBox(state->layoutRecs[9], ComboBox009TextList, state->ComboBox009Count, state->ComboBox009Active);
    if (GuiSpinner(state->layoutRecs[11], state->Spinner011Value, 0, 100, 25, state->Spinner011EditMode)) state->Spinner011EditMode = !state->Spinner011EditMode;
    if (GuiValueBox(state->layoutRecs[12], state->ValueBox012Value, 0, 100, state->ValueBox012EditMode)) state->ValueBox012EditMode = !state->ValueBox012EditMode;
    if (GuiTextBox(state->layoutRecs[13], state->TextBox013Text, state->TextBox013Size, state->TextBox013EditMode)) state->TextBox013EditMode = !state->TextBox013EditMode;
    state->Slider014Value = GuiSlider(state->layoutRecs[14], state->Slider014Value, Slider014MinValue, Slider014MaxValue);
    state->SliderBar015Value = GuiSliderBar(state->layoutRecs[15], state->SliderBar015Value, SliderBar015MinValue, SliderBar015MaxValue);
    state->ProgressBar016Value = GuiProgressBarEx(state->layoutRecs[16], state->ProgressBar016Value, 0, 100, true);
    GuiStatusBar(state->layoutRecs[17], state->StatusBar017Text, 10);
    if (GuiListView(state->layoutRecs[18], ListView018TextList, state->ListView018Count, &state->ListView018ScrollIndex, &state->ListView018Active, state->ListView018EditMode)) state->ListView018EditMode = !state->ListView018EditMode;
    state->ColorPicker019Value = GuiColorPicker(state->layoutRecs[19], state->ColorPicker019Value);
    GuiDummyRec(state->layoutRecs[20], "SAMPLE TEXT");
    if (GuiDropdownBox(state->layoutRecs[10], DropdownBox010TextList, state->DropdownBox010Count, &state->DropdownBox010Active, state->DropdownBox010EditMode)) state->DropdownBox010EditMode = !state->DropdownBox010EditMode;
    
    GuiUnlock();
}

#endif // LAYOUTNAME_IMPLEMENTATION