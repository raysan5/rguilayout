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
    bool WindowBox000Active;
    bool Button005Pressed;
    bool LabelButton006Pressed;
    bool ImageButtonEx007Pressed;
    bool CheckBoxEx008Checked;
    bool Toggle009Active;
    int ToggleGroup010Active;
    int ComboBox011Active;
    bool DropdownBox012EditMode;
    int DropdownBox012Active;
    bool TextBox013EditMode;
    unsigned char TextBox013Text[64];
    bool TextmultiBox014EditMode;
    unsigned char TextmultiBox014Text[64];
    bool ValueBOx015EditMode;
    int ValueBOx015Value;
    bool Spinner016EditMode;
    int Spinner016Value;
    float SliderEx017Value;
    float SliderBarEx018Value;
    float PrograssBarEx019Value;
    int ListView022ScrollIndex;
    bool ListView022EditMode;
    int ListView022Active;
    Color ColorPicker023Value;
    bool DropdownBox026EditMode;
    int DropdownBox026Active;

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

    state.WindowBox000Active = true;
    state.Button005Pressed = false;
    state.LabelButton006Pressed = false;
    state.ImageButtonEx007Pressed = false;
    state.CheckBoxEx008Checked = false;
    state.Toggle009Active = true;
    state.ToggleGroup010Active= 0;
    state.ComboBox011Active= 0;
    state.DropdownBox012EditMode = false;
    state.DropdownBox012Active = 0;
    state.TextBox013EditMode = false;
    strcpy(state.TextBox013Text, "SAMPLE TEXT");
    state.TextmultiBox014EditMode = false;
    strcpy(state.TextmultiBox014Text, "SAMPLE TEXT");
    state.ValueBOx015EditMode = false;
    state.ValueBOx015Value = 0;
    state.Spinner016EditMode = false;
    state.Spinner016Value = 0;
    state.SliderEx017Value = 0.0f;
    state.SliderBarEx018Value = 0.0f;
    state.PrograssBarEx019Value = 0.0f;
    state.ListView022ScrollIndex = 0;
    state.ListView022EditMode = false;
    state.ListView022Active = 0;
    state.ColorPicker023Value;
    state.DropdownBox026EditMode = false;
    state.DropdownBox026Active = 0;

    // Custom variables initialization

    return state;
}

void GuiWindowCodegen(GuiWindowCodegenState *state)
{
    if (state->DropdownBox012EditMode || state->DropdownBox026EditMode) GuiLock();

    if (state->WindowBox000Active)
    {
        state->WindowBox000Active = !GuiWindowBox((Rectangle){ 55, 30, 125, 50 }, "SAMPLE TEXT");
        GuiLabel((Rectangle){ 56, 54, 126, 25 }, "SAMPLE TEXT");
        if (GuiDropdownBox((Rectangle){ 59, 71, 125, 25 }, "ONE;TWO;THREE", &state->DropdownBox026Active, state->DropdownBox026EditMode)) state->DropdownBox026EditMode = !state->DropdownBox026EditMode;
    }
    GuiGroupBox((Rectangle){ 195, 40, 125, 30 }, SAMPLE TEXT);
    GuiLine((Rectangle){ 294, 28, 125, 25 }, 1);
    GuiPanel((Rectangle){ 429, 23, 125, 35 });
    GuiLabel((Rectangle){ 14, 78, 126, 25 }, "SAMPLE TEXT");
    state->Button005Pressed = GuiButton((Rectangle){ 149, 73, 125, 30 }, "SAMPLE TEXT"); 
    state->LabelButton006Pressed = GuiLabelButton((Rectangle){ 299, 78, 125, 30 }, "SAMPLE TEXT");
    state->ImageButtonEx007Pressed = GuiImageButtonEx((Rectangle){ 429, 68, 125, 125 }, GetTextureDefault(), (Rectangle){ 0, 0, 1, 1 }, "SAMPLE TEXT");
    state->CheckBoxEx008Checked = GuiCheckBox((Rectangle){ 14, 138, 12, 12 }, "SAMPLE TEXT", state->CheckBoxEx008Checked);
    state->Toggle009Active = GuiToggle((Rectangle){ 149, 133, 90, 25 }, "SAMPLE TEXT", state->Toggle009Active);
    state->ToggleGroup010Active = GuiToggleGroup((Rectangle){ 259, 138, 50, 30 }, "ONE;TWO;THREE;HAHA", state->ToggleGroup010Active);
    state->ComboBox011Active = GuiComboBox((Rectangle){ 9, 178, 125, 25 }, "ONE;TWO;THREE", state->ComboBox011Active);
    if (GuiTextBox((Rectangle){ 294, 178, 125, 25 }, state->TextBox013Text, 64, state->TextBox013EditMode)) state->TextBox013EditMode = !state->TextBox013EditMode;
    if (GuiTextBoxMulti((Rectangle){ 14, 223, 125, 75 }, state->TextmultiBox014Text, 64, state->TextmultiBox014EditMode)) state->TextmultiBox014EditMode = !state->TextmultiBox014EditMode;
    if (GuiValueBox((Rectangle){ 154, 228, 125, 25 }, &state->ValueBOx015Value, 0, 100, state->ValueBOx015EditMode)) state->ValueBOx015EditMode = !state->ValueBOx015EditMode;
    if (GuiSpinner((Rectangle){ 289, 228, 125, 25 }, &state->Spinner016Value, 0, 100, 25, state->Spinner016EditMode)) state->Spinner016EditMode = !state->Spinner016EditMode;
    state->SliderEx017Value = GuiSliderEx((Rectangle){ 229, 263, 125, 15 }, "", state->SliderEx017Value, 0, 100, true);
    state->SliderBarEx018Value = GuiSliderBarEx((Rectangle){ 229, 288, 125, 15 }, "", state->SliderBarEx018Value, 0, 100, true);
    state->PrograssBarEx019Value = GuiProgressBarEx((Rectangle){ 224, 318, 125, 15 }, state->PrograssBarEx019Value, 0, 1, true);
    GuiStatusBar((Rectangle){ 424, 228, 125, 25 }, , 15);
    //TODO: GUI_SCROLLPANEL no avaible yet
    if (GuiListView((Rectangle){ 424, 343, 125, 75 }, "", &state->ListView022Active, &state->ListView022ScrollIndex, state->ListView022EditMode)) state->ListView022EditMode = !state->ListView022EditMode;
    state->ColorPicker023Value = GuiColorPicker((Rectangle){ 14, 313, 95, 95 }, state->ColorPicker023Value);
    GuiDummyRec((Rectangle){ 219, 353, 125, 30 }, );
    if (GuiDropdownBox((Rectangle){ 149, 178, 125, 25 }, "ONE;TWO;THREE", &state->DropdownBox012Active, state->DropdownBox012EditMode)) state->DropdownBox012EditMode = !state->DropdownBox012EditMode;
    
    GuiUnlock();
}

#endif // GUI_WINDOW_CODEGEN_IMPLEMENTATION
