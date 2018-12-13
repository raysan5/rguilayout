/*******************************************************************************************
*
*   raygui layout - TestTool
*
*   MODULE USAGE:
*       #define GUI_TESTTOOL_IMPLEMENTATION
*       #include "gui_testtool.h"
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

#ifndef TESTTOOL_H
#define TESTTOOL_H

typedef struct {
    // Define anchors
    Vector2 anchor01;    // ID:1 Name:anchor01
    Vector2 anchor02;    // ID:2 Name:anchor02
    // Define controls variables
    bool WindowBox000Active;    // WindowBox: WindowBox000
    bool Toggle006Active;    // Toggle: Toggle006
    int ToggleGroup007Count;
    const char *ToggleGroup007TextList[3];
    int ToggleGroup007Active;    // ToggleGroup: ToggleGroup007
    bool CheckBox008Checked;    // CheckBox: CheckBox008
    int ComboBox009Count;
    const char *ComboBox009TextList[3];
    int ComboBox009Active;    // ComboBox: ComboBox009
    int DropdownBox010Count;
    const char *DropdownBox010TextList[3];
    int DropdownBox010Active;
    bool DropdownBox010EditMode;    // DropdownBox: DropdownBox010
    int Spinner011Value;    // Spinner: Spinner011
    int ValueBox012Value;    // ValueBox: ValueBox012
    int TextBox013Size;
    unsigned char TextBox013Text[64];    // TextBox: TextBox013
    const float Slider014MinValue;
    const float Slider014MaxValue;
    float Slider014Value;    // Slider: Slider014
    const float SliderBar015MinValue;
    const float SliderBar015MaxValue;
    float SliderBar015Value;    // SliderBar: SliderBar015
    float ProgressBar016Value;    // ProgressBar: ProgressBar016
    char *StatusBar017Text;    // StatusBar: StatusBar017
    int ListView018Count;
    const char *ListView018TextList[3];
    int ListView018Active;
    bool ListView018EditMode;    // ListView: ListView018
    Color ColorPicker019Value;    // ColorPicker: ColorPicker019
    // Define controls rectangles
    Rectangle layoutRecs[21];
} GuiLayoutTestToolState;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
GuiLayoutTestToolState InitGuiLayoutTestTool(void);
void GuiLayoutTestTool (GuiLayoutTestToolState *state);

#ifdef __cplusplus
}
#endif

#endif // TESTTOOL_H

/***********************************************************************************
*
*   TESTTOOL IMPLEMENTATION
*
************************************************************************************/
#if defined(TESTTOOL_IMPLEMENTATION)

#include "raygui.h"

GuiLayoutTestToolState InitGuiLayoutTestTool(void)
{
    GuiLayoutTestToolState state = { 0 };
    // Init anchors
    state.anchor01 = { 81, 52 };    // ID:1 Name:anchor01
    state.anchor02 = { 79, 112 };    // ID:2 Name:anchor02
    // Init controls variables
    state.WindowBox000Active = true;    // WindowBox: WindowBox000
    state.Toggle006Active = true;    // Toggle: Toggle006
    state.ToggleGroup007Count = 3;
    strcpy(state.ToggleGroup007TextList[0],"ONE");
    strcpy(state.ToggleGroup007TextList[1],"TWO");
    strcpy(state.ToggleGroup007TextList[2],"THREE");
    state.ToggleGroup007Active = 0;    // ToggleGroup: ToggleGroup007
    state.CheckBox008Checked = false;    // CheckBox: CheckBox008
    state.ComboBox009Count = 3;
    strcpy(state.ComboBox009TextList[0],"ONE");
    strcpy(state.ComboBox009TextList[1],"TWO");
    strcpy(state.ComboBox009TextList[2],"THREE");
    state.ComboBox009Active = 0;    // ComboBox: ComboBox009
    state.DropdownBox010Count = 3;
    strcpy(state.DropdownBox010TextList[0],"ONE");
    strcpy(state.DropdownBox010TextList[1],"TWO");
    strcpy(state.DropdownBox010TextList[2],"THREE");
    state.DropdownBox010Active = 0;
    state.DropdownBox010EditMode = false;    // DropdownBox: DropdownBox010
    state.Spinner011Value = 0;    // Spinner: Spinner011
    state.ValueBox012Value = 0;    // ValueBox: ValueBox012
    state.TextBox013Size = 64;
    strcpy(state.TextBox013Text,"SAMPLE TEXT");    // TextBox: TextBox013
    state.Slider014MinValue = 0.0f;
    state.Slider014MaxValue = 100.0f;
    state.Slider014Value = 50.0f;    // Slider: Slider014
    state.SliderBar015MinValue = 0.0f;
    state.SliderBar015MaxValue = 100.0f;
    state.SliderBar015Value = 50.0f;    // SliderBar: SliderBar015
    state.ProgressBar016Value = 50.0f;    // ProgressBar: ProgressBar016
    state.*StatusBar017Text = "SAMPLE TEXT";    // StatusBar: StatusBar017
    state.ListView018Count = 3;
    strcpy(state.ListView018TextList[0],"ONE");
    strcpy(state.ListView018TextList[1],"TWO");
    strcpy(state.ListView018TextList[2],"THREE");
    state.ListView018Active = 0;
    state.ListView018EditMode = false;    // ListView: ListView018
    state.ColorPicker019Value;    // ColorPicker: ColorPicker019
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

void GuiLayoutTestTool (GuiLayoutTestToolState *state)
{
    // TODO: work in progress...
}

#endif // TESTTOOL_IMPLEMENTATION