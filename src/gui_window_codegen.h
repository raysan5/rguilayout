/*******************************************************************************************
*
*   WindowCodegen v1.1 - Layout Code Generator
*
*   MODULE USAGE:
*       #define GUI_WINDOW_CODEGEN_IMPLEMENTATION
*       #include "gui_window_codegen.h"
*
*       INIT: GuiWindowCodegenState state = InitGuiWindowCodegen();
*       DRAW: GuiWindowCodegen(&state);
*
*   LICENSE: Propietary License
*
*   Copyright (c) 2018-2022 raylib technologies (@raylibtech). All Rights Reserved.
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

#ifndef GUI_WINDOW_CODEGEN_H
#define GUI_WINDOW_CODEGEN_H

typedef struct {
    Vector2 codegenAnchor;

    bool windowCodegenActive;
    bool toolNameEditMode;
    unsigned char toolNameText[64];
    bool toolVersionEditMode;
    unsigned char toolVersionText[64];
    bool companyEditMode;
    unsigned char companyText[64];
    bool toolDescriptionEditMode;
    unsigned char toolDescriptionText[64];
    bool codeTemplateEditMode;
    int codeTemplateActive;
    bool exportAnchorsChecked;
    bool defineRecsChecked;
    bool defineTextsChecked;
    bool fullCommentsChecked;
    bool generateButtonFunctionsChecked;
    bool guiExportStyleChecked;
    bool guiEmbedFontChecked;
    bool generateCodePressed;
    bool executeCodePressed;
    Vector2 codePanelScrollOffset;

    // Custom state variables (depend on development software)
    // NOTE: This variables should be added manually if required
    unsigned char *codeText;        // Generated code string
    unsigned int codeHeight;        // Generated code drawing size

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

    state.codegenAnchor = (Vector2){ 0, 0 };

    state.windowCodegenActive = false;
    state.toolNameEditMode = false;
    strcpy(state.toolNameText, "layout_name");
    state.toolVersionEditMode = false;
    strcpy(state.toolVersionText, "1.0.0");
    state.companyEditMode = false;
    strcpy(state.companyText, "raylib technologies");
    state.toolDescriptionEditMode = false;
    strcpy(state.toolDescriptionText, "Tool Description");
    state.codeTemplateEditMode = false;
    state.codeTemplateActive = 0;
    state.exportAnchorsChecked = false;
    state.defineRecsChecked = false;
    state.defineTextsChecked = false;
    state.fullCommentsChecked = false;
    state.generateButtonFunctionsChecked = false;
    state.guiExportStyleChecked = false;
    state.guiEmbedFontChecked = false;
    state.generateCodePressed = false;
    state.executeCodePressed = false;
    state.codePanelScrollOffset = (Vector2){ 0, 0 };

    // Custom variables initialization
    state.codeText = NULL;
    state.codeHeight = 0;

    return state;
}

void GuiWindowCodegen(GuiWindowCodegenState *state)
{
    if (state->windowCodegenActive)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), 0.8f));

        state->codegenAnchor = (Vector2){ GetScreenWidth()/2.0f - 450, GetScreenHeight()/2.0f - 320 };
        state->windowCodegenActive = !GuiWindowBox((Rectangle){ state->codegenAnchor.x + 0, state->codegenAnchor.y + 0, 900, 640 }, "#7#Code Generation Window");

        GuiGroupBox((Rectangle){ state->codegenAnchor.x + 665, state->codegenAnchor.y + 35, 220, 235 }, "Layout Info");
        GuiLabel((Rectangle){ state->codegenAnchor.x + 675, state->codegenAnchor.y + 45, 50, 25 }, "Name:");
        if (GuiTextBox((Rectangle){ state->codegenAnchor.x + 725, state->codegenAnchor.y + 45, 150, 25 }, state->toolNameText, 64, state->toolNameEditMode)) state->toolNameEditMode = !state->toolNameEditMode;
        GuiLabel((Rectangle){ state->codegenAnchor.x + 675, state->codegenAnchor.y + 75, 50, 25 }, "Version:");
        if (GuiTextBox((Rectangle){ state->codegenAnchor.x + 725, state->codegenAnchor.y + 75, 150, 25 }, state->toolVersionText, 64, state->toolVersionEditMode)) state->toolVersionEditMode = !state->toolVersionEditMode;
        GuiLabel((Rectangle){ state->codegenAnchor.x + 675, state->codegenAnchor.y + 105, 50, 25 }, "Company:");
        if (GuiTextBox((Rectangle){ state->codegenAnchor.x + 725, state->codegenAnchor.y + 105, 150, 25 }, state->companyText, 64, state->companyEditMode)) state->companyEditMode = !state->companyEditMode;
        GuiLabel((Rectangle){ state->codegenAnchor.x + 675, state->codegenAnchor.y + 135, 100, 25 }, "Short Description:");
        if (GuiTextBoxMulti((Rectangle){ state->codegenAnchor.x + 675, state->codegenAnchor.y + 160, 200, 100 }, state->toolDescriptionText, 64, state->toolDescriptionEditMode)) state->toolDescriptionEditMode = !state->toolDescriptionEditMode;
        
        GuiGroupBox((Rectangle){ state->codegenAnchor.x + 665, state->codegenAnchor.y + 285, 220, 153 }, "Code Generation Options");
        state->exportAnchorsChecked = GuiCheckBox((Rectangle){ state->codegenAnchor.x + 685, state->codegenAnchor.y + 335, 15, 15 }, "Export anchors", state->exportAnchorsChecked);
        state->defineRecsChecked = GuiCheckBox((Rectangle){ state->codegenAnchor.x + 685, state->codegenAnchor.y + 355, 15, 15 }, "Define Rectangles", state->defineRecsChecked);
        state->defineTextsChecked = GuiCheckBox((Rectangle){ state->codegenAnchor.x + 685, state->codegenAnchor.y + 375, 15, 15 }, "Define text as const", state->defineTextsChecked);
        state->fullCommentsChecked = GuiCheckBox((Rectangle){ state->codegenAnchor.x + 685, state->codegenAnchor.y + 395, 15, 15 }, "Include detailed comments", state->fullCommentsChecked);
        state->generateButtonFunctionsChecked = GuiCheckBox((Rectangle){ state->codegenAnchor.x + 685, state->codegenAnchor.y + 415, 15, 15 }, "Generate button functions", state->generateButtonFunctionsChecked);

        state->generateCodePressed = GuiButton((Rectangle){ state->codegenAnchor.x + 665, state->codegenAnchor.y + 455, 220, 30 }, "#7#Export Generated Code");

        // TODO: Add required fields to support a custom code template --> Requires documentation
        //if (GuiDropdownBox((Rectangle){ state->codegenAnchor.x + 675, state->codegenAnchor.y + 300, 200, 25 }, "STANDARD CODE FILE (.c);PORTABLE CODE FILE (.h);CUSTOM CODE FILE", &state->codeTemplateActive, state->codeTemplateEditMode)) state->codeTemplateEditMode = !state->codeTemplateEditMode;
        if (GuiDropdownBox((Rectangle){ state->codegenAnchor.x + 675, state->codegenAnchor.y + 300, 200, 25 }, "STANDARD CODE FILE (.c);PORTABLE CODE FILE (.h)", &state->codeTemplateActive, state->codeTemplateEditMode)) state->codeTemplateEditMode = !state->codeTemplateEditMode;
        GuiEnable();
        
        // Draw generated code
        if (state->codeText != NULL)
        {
            Rectangle codePanel = { state->codegenAnchor.x + 10, state->codegenAnchor.y + 35, 645, 595 };
            Rectangle view = GuiScrollPanel(codePanel, (Rectangle){ codePanel.x, codePanel.y, codePanel.width*2, (float)state->codeHeight }, &state->codePanelScrollOffset);

            BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);
                unsigned int linesCounter = 0;
                unsigned char *currentLine = state->codeText;

                while (currentLine)
                {
                    char *nextLine = strchr(currentLine, '\n');
                    if (nextLine) *nextLine = '\0';     // Temporaly terminating the current line

                    // Only draw lines inside text panel
                    if (((state->codePanelScrollOffset.y + 20*linesCounter) >= 0) &&
                        ((state->codePanelScrollOffset.y + 20*linesCounter) < (codePanel.height - 2)))
                    {
                        DrawText(currentLine, (int)codePanel.x + (int)state->codePanelScrollOffset.x + 10, (int)codePanel.y + (int)state->codePanelScrollOffset.y + 20*linesCounter + 8, 10, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
                    }

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
