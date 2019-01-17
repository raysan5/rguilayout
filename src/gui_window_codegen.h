/*******************************************************************************************
*
*   raygui layout - WindowCodegen
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
    bool codeGenWindowActive;
    unsigned char toolNameText[64];
    bool toolNameEditMode;
    unsigned char toolVersionText[64];
    bool toolVersionEditMode;
    unsigned char companyText[64];
    bool companyEditMode;
    unsigned char toolDescriptionText[64];
    bool toolDescriptionEditMode;
    int codeTemplateActive;
    bool codeTemplateEditMode;
    bool exportAnchorsChecked;
    bool fullVariablesChecked;
    bool defineRecsChecked;
    bool defineTextsChecked;
    bool fullCommentsChecked;
    bool CheckBox019Checked;
    bool CheckBox021Checked;
    bool exportCodeButtonPressed;
    
    // Custom state variables (depend on development software)
    // NOTE: This variables should be added manually if required
    unsigned char *codeText;        // Generated code string
    unsigned int codeHeight;        // Generated code drawing size
    Vector2 codeOffset;             // Code drawing scroll panel offset
    
} GuiWindowCodegenState;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

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

    state.codeGenWindowActive = false;
    
    strcpy(state.toolNameText,"window_codegen");
    state.toolNameEditMode = false;
    strcpy(state.toolVersionText,"1.0.0");
    state.toolVersionEditMode = false;
    strcpy(state.companyText,"raylibtech");
    state.companyEditMode = false;
    strcpy(state.toolDescriptionText,"Tool Description");
    state.toolDescriptionEditMode = false;
    state.codeTemplateActive = 0;
    state.codeTemplateEditMode = false;
    state.exportAnchorsChecked = false;
    state.fullVariablesChecked = false;
    state.defineRecsChecked = false;
    state.defineTextsChecked = false;
    state.fullCommentsChecked = false;
    state.CheckBox019Checked = false;
    state.CheckBox021Checked = false;

    state.exportCodeButtonPressed = false; // TODO GEN
    
    // Custom variables initialization
    state.codeText = NULL;
    state.codeHeight = 0;
    state.codeOffset = (Vector2){ 0, 0 };
    
    return state;
}

void GuiWindowCodegen(GuiWindowCodegenState *state)
{
    const char *lblNameText = "Name:";
    const char *lblVersionText = "Version:";
    const char *lblCompanyText = "Company:";
    const char *lblDescriptionText = "Short Description:";
    const char *codeTemplateText = "STANDARD CODE FILE (.c);PORTABLE CODE FILE (.h);CUSTOM CODE FILE";
    
    // TODO: Additional const values (text and values)
    
    if (state->codeGenWindowActive)
    {
        if (state->codeTemplateEditMode) GuiLock();

        state->codeGenWindowActive = !GuiWindowBox((Rectangle){ 50, 50, 900, 640 }, "Code Generation Window");
        state->exportCodeButtonPressed = GuiButton((Rectangle){ 715, 605, 220, 30 }, "Export Generated Code");
        GuiDisable(); if (GuiButton((Rectangle){ 715, 645, 220, 30 }, "Execute Code")) {} GuiEnable();
        GuiGroupBox((Rectangle){ 715, 85, 220, 230 }, "Layout Info");
        GuiLabel((Rectangle){ 725, 95, 50, 25 }, lblNameText);
        if (GuiTextBox((Rectangle){ 775, 95, 150, 25 }, state->toolNameText, 64, state->toolNameEditMode)) state->toolNameEditMode = !state->toolNameEditMode;
        GuiLabel((Rectangle){ 725, 125, 50, 25 }, lblVersionText);
        if (GuiTextBox((Rectangle){ 775, 125, 150, 25 }, state->toolVersionText, 64, state->toolVersionEditMode)) state->toolVersionEditMode = !state->toolVersionEditMode;
        GuiLabel((Rectangle){ 725, 155, 50, 25 }, lblCompanyText);
        if (GuiTextBox((Rectangle){ 775, 155, 150, 25 }, state->companyText, 64, state->companyEditMode)) state->companyEditMode = !state->companyEditMode;
        GuiLabel((Rectangle){ 725, 185, 100, 25 }, lblDescriptionText);
        if (GuiTextBoxMulti((Rectangle){ 725, 205, 200, 100 }, state->toolDescriptionText, 64, state->toolDescriptionEditMode)) state->toolDescriptionEditMode = !state->toolDescriptionEditMode;
        GuiGroupBox((Rectangle){ 715, 330, 220, 160 }, "Code Generation Options");
        // TODO: checked state->
        state->exportAnchorsChecked = GuiCheckBox((Rectangle){ 735, 380, 15, 15 }, "Export anchors", state->exportAnchorsChecked);
        state->defineRecsChecked = GuiCheckBox((Rectangle){ 735, 420, 15, 15 }, "Define Rectangles", state->defineRecsChecked);
        state->defineTextsChecked = GuiCheckBox((Rectangle){ 735, 440, 15, 15 }, "Define text as const", state->defineTextsChecked);
        state->fullCommentsChecked = GuiCheckBox((Rectangle){ 735, 460, 15, 15 }, "Include detailed comments", state->fullCommentsChecked);
        GuiGroupBox((Rectangle){ 715, 505, 220, 65 }, "Gui Style Options");
        state->CheckBox019Checked = GuiCheckBox((Rectangle){ 735, 520, 15, 15 }, "Export gui style", state->CheckBox019Checked);
        state->CheckBox021Checked = GuiCheckBox((Rectangle){ 735, 540, 15, 15 }, "Embbed gui font", state->CheckBox021Checked);
        if (GuiDropdownBox((Rectangle){ 725, 345, 195, 25 }, codeTemplateText, &state->codeTemplateActive, state->codeTemplateEditMode)) state->codeTemplateEditMode = !state->codeTemplateEditMode;

        GuiUnlock();

        // Draw generated code
        if (state->codeText != NULL)
        {
            Rectangle codePanel = { 60, 85, 640, 590 };
            
            state->codeOffset = GuiScrollPanel(codePanel, (Rectangle){ codePanel.x, codePanel.y, codePanel.width, state->codeHeight }, state->codeOffset);
            
            BeginScissorMode(codePanel.x, codePanel.y, codePanel.width, codePanel.height);
                unsigned int linesCounter = 0;
                unsigned char *currentLine = state->codeText;
                
                while (currentLine)
                {
                    char *nextLine = strchr(currentLine, '\n');
                    if (nextLine) *nextLine = '\0';     // Temporaly terminating the current line
                    
                    // Only draw lines inside text panel
                    if (((state->codeOffset.y + 20*linesCounter) >= 0) && 
                        ((state->codeOffset.y + 20*linesCounter) < (codePanel.height - 2))) DrawText(currentLine, codePanel.x + 10, codePanel.y + state->codeOffset.y + 20*linesCounter, 10, DARKBLUE);
                    
                    if (nextLine) *nextLine = '\n';     // Restore newline-char, just to be tidy
                    currentLine = nextLine ? (nextLine + 1) : NULL;
                    
                    linesCounter++;
                }
            EndScissorMode();
            
            state->codeHeight = 20*linesCounter;
        }
    }
}

#endif // GUI_WINDOW_CODEGEN_IMPLEMENTATION
