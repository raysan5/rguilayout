/*******************************************************************************************
*
*   rGuiLayout - Layout Code Generator
*
*   MODULE USAGE:
*       #define GUI_WINDOW_CODEGEN_IMPLEMENTATION
*       #include "gui_window_codegen.h"
*
*       INIT: GuiWindowCodegenState state = InitGuiWindowCodegen();
*       DRAW: GuiWindowCodegen(&state);
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018-2022 raylib technologies (@raylibtech) / Ramon Santamaria (@raysan5)
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

#ifndef GUI_WINDOW_CODEGEN_H
#define GUI_WINDOW_CODEGEN_H

typedef struct {

    bool windowActive;
    Rectangle windowBounds;

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
    bool genButtonFuncsChecked;

    bool btnExportCodePressed;
    //bool btnExecuteCodePressed;
    bool btnLoadCustomTemplatePressed;

    Vector2 codePanelScrollOffset;

    // Custom state variables
    unsigned char *codeText;        // Generated code string
    unsigned int codeHeight;        // Maximum height of code block (computed at drawing)
    Font codeFont;                  // Font used for text drawing

    unsigned char *customTemplate;  // Custom template loaded
    bool customTemplateLoaded;      // Custom template loaded flag

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

#include "font_gohufont.h"

GuiWindowCodegenState InitGuiWindowCodegen(void)
{
    GuiWindowCodegenState state = { 0 };

    state.windowActive = false;
    state.windowBounds = (Rectangle){ 0, 0, 1000, 640 };

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
    state.genButtonFuncsChecked = false;

    state.btnExportCodePressed = false;
    state.btnLoadCustomTemplatePressed = false;

    state.codePanelScrollOffset = (Vector2){ 0, 0 };

    // Custom variables initialization
    state.codeText = NULL;
    state.codeHeight = 0;
    state.codeFont = LoadFont_Gohufont();   // Font embedded (font_gohufont.h)

    state.customTemplate = NULL;            // Custom template loaded
    state.customTemplateLoaded = false;     // Custom template loaded flag

    return state;
}

void GuiWindowCodegen(GuiWindowCodegenState *state)
{
    if (state->windowActive)
    {
        state->windowBounds = (Rectangle){ GetScreenWidth()/2.0f - 1024/2, GetScreenHeight()/2.0f - 640/2, 1024, 640 };
        state->windowActive = !GuiWindowBox(state->windowBounds, "#7#Code Generation Window");

        GuiGroupBox((Rectangle){ state->windowBounds.x + 765, state->windowBounds.y + 40, 248, 178 }, "#101#Layout Info");
        GuiLabel((Rectangle){ state->windowBounds.x + 775, state->windowBounds.y + 58, 100, 24 }, "Name:");
        if (GuiTextBox((Rectangle){ state->windowBounds.x + 845, state->windowBounds.y + 58, 158, 24 }, state->toolNameText, 64, state->toolNameEditMode)) state->toolNameEditMode = !state->toolNameEditMode;
        GuiLabel((Rectangle){ state->windowBounds.x + 775, state->windowBounds.y + 88, 100, 24 }, "Version:");
        if (GuiTextBox((Rectangle){ state->windowBounds.x + 845, state->windowBounds.y + 88, 158, 24 }, state->toolVersionText, 64, state->toolVersionEditMode)) state->toolVersionEditMode = !state->toolVersionEditMode;
        GuiLabel((Rectangle){ state->windowBounds.x + 775, state->windowBounds.y + 118, 100, 24 }, "Company:");
        if (GuiTextBox((Rectangle){ state->windowBounds.x + 845, state->windowBounds.y + 118, 158, 24 }, state->companyText, 64, state->companyEditMode)) state->companyEditMode = !state->companyEditMode;
        GuiLabel((Rectangle){ state->windowBounds.x + 775, state->windowBounds.y + 148, 160, 24 }, "Short Description:");
        if (GuiTextBox((Rectangle){ state->windowBounds.x + 775, state->windowBounds.y + 172, 228, 28 }, state->toolDescriptionText, 64, state->toolDescriptionEditMode)) state->toolDescriptionEditMode = !state->toolDescriptionEditMode;
        
        GuiGroupBox((Rectangle){ state->windowBounds.x + 765, state->windowBounds.y + 236, 248, 226 }, "#142#Code Generation Options");
        GuiCheckBox((Rectangle){ state->windowBounds.x + 785, state->windowBounds.y + 292, 16, 16 }, "Export anchors", &state->exportAnchorsChecked);
        GuiCheckBox((Rectangle){ state->windowBounds.x + 785, state->windowBounds.y + 292 + 24, 16, 16 }, "Define Rectangles", &state->defineRecsChecked);
        GuiCheckBox((Rectangle){ state->windowBounds.x + 785, state->windowBounds.y + 292 + 48, 16, 16 }, "Define text as const", &state->defineTextsChecked);
        GuiCheckBox((Rectangle){ state->windowBounds.x + 785, state->windowBounds.y + 292 + 72, 16, 16 }, "Include detailed comments", &state->fullCommentsChecked);
        GuiCheckBox((Rectangle){ state->windowBounds.x + 785, state->windowBounds.y + 292 + 96, 16, 16 }, "Generate button functions", &state->genButtonFuncsChecked);

        if (state->codeTemplateActive != 2) GuiDisable();
        state->btnLoadCustomTemplatePressed = GuiButton((Rectangle){ state->windowBounds.x + 775, state->windowBounds.y + 292 + 128, 228, 24 }, state->customTemplateLoaded? "#9#Unload Custom Template" : "#5#Load Custom Template");
        GuiEnable();

        // Export generated code button
        state->btnExportCodePressed = GuiButton((Rectangle){ state->windowBounds.x + 765, state->windowBounds.y + 278 + 184 + 16, 248, 28 }, "#7#Export Generated Code");

        // Select desired code template to fill
        if (GuiDropdownBox((Rectangle){ state->windowBounds.x + 775, state->windowBounds.y + 256, 228, 24 }, 
                "STANDARD TEMPLATE (.c);PORTABLE TEMPLATE (.h); CUSTOM TEMPLATE (.c/.h)",
                &state->codeTemplateActive, state->codeTemplateEditMode)) state->codeTemplateEditMode = !state->codeTemplateEditMode;

        // Draw generated code
        Rectangle codePanel = { state->windowBounds.x + 10, state->windowBounds.y + 35, 745, 595 };
        Rectangle view = { 0 };
        GuiScrollPanel(codePanel, NULL, (Rectangle){ codePanel.x, codePanel.y, codePanel.width*2, (float)state->codeHeight }, &state->codePanelScrollOffset, &view);

        if (state->codeText != NULL)
        {
            BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);
                unsigned int linesCounter = 0;
                unsigned char *currentLine = state->codeText;

                while (currentLine)
                {
                    char *nextLine = strchr(currentLine, '\n');
                    if (nextLine) *nextLine = '\0';     // Temporaly terminating the current line

                    // Only draw lines inside text panel
                    if (((state->codePanelScrollOffset.y + 20*linesCounter) >= -40) &&
                        ((state->codePanelScrollOffset.y + 20*linesCounter) < (codePanel.height - 2)))
                    {
                        DrawTextEx(state->codeFont, currentLine, (Vector2) { codePanel.x + state->codePanelScrollOffset.x + 10, codePanel.y + state->codePanelScrollOffset.y + 20*linesCounter + 8 }, state->codeFont.baseSize, 1, GetColor(GuiGetStyle(TEXTBOX, TEXT_COLOR_NORMAL)));
                    }

                    if (nextLine) *nextLine = '\n';     // Restore newline-char, just to be tidy
                    currentLine = nextLine? (nextLine + 1) : NULL;

                    linesCounter++;
                }
            EndScissorMode();

            state->codeHeight = 20*linesCounter;
        }
    }
}

#endif // GUI_WINDOW_CODEGEN_IMPLEMENTATION
