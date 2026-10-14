/*******************************************************************************************
*
*   rGuiLayout - Layout Code Generator
*
*   MODULE USAGE:
*       #define GUI_WINDOW_CODEGEN_IMPLEMENTATION
*       #include "gui_window_codegen.h"
*
*       INIT: GuiWindowCodegenState state = GUI_WINDOW_CODEGEN_STATE_DEFAULT;
*       DRAW: GuiWindowCodegen(&state);
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018-2026 raylib technologies (@raylibtech) / Ramon Santamaria (@raysan5)
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

    // Input data
    char *codeText;                 // Generated code string
    unsigned int codeHeight;        // Maximum height of code block (computed at drawing)
    Font codeFont;                  // Font used for text drawing

    char *customTemplate;           // Custom template loaded
    bool customTemplateLoaded;      // Custom template loaded flag

    // Control config
    bool windowActive;

    bool toolNameEditMode;
    char toolNameText[64];
    bool toolVersionEditMode;
    char toolVersionText[64];
    bool companyEditMode;
    char companyText[64];
    bool toolDescriptionEditMode;
    char toolDescriptionText[64];

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

} GuiWindowCodegenState;

// Default control state
#define GUI_WINDOW_CODEGEN_STATE_DEFAULT  { 0 }

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
int GuiWindowCodegen(Rectangle bounds, const char *text, GuiWindowCodegenState *state);

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

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
int GuiWindowCodegen(Rectangle bounds, const char *text, GuiWindowCodegenState *state)
{
    int result = 0;

    if (state->windowActive)
    {
        state->windowActive = !GuiWindowBox(bounds, text);

        GuiGroupBox((Rectangle){ bounds.x + 765, bounds.y + 40, 248, 178 }, "#101#Layout Info");
        GuiLabel((Rectangle){ bounds.x + 775, bounds.y + 58, 100, 24 }, "Name:");
        if (GuiTextBox((Rectangle){ bounds.x + 845, bounds.y + 58, 158, 24 }, state->toolNameText, 64, state->toolNameEditMode)) state->toolNameEditMode = !state->toolNameEditMode;
        GuiLabel((Rectangle){ bounds.x + 775, bounds.y + 88, 100, 24 }, "Version:");
        if (GuiTextBox((Rectangle){ bounds.x + 845, bounds.y + 88, 158, 24 }, state->toolVersionText, 64, state->toolVersionEditMode)) state->toolVersionEditMode = !state->toolVersionEditMode;
        GuiLabel((Rectangle){ bounds.x + 775, bounds.y + 118, 100, 24 }, "Company:");
        if (GuiTextBox((Rectangle){ bounds.x + 845, bounds.y + 118, 158, 24 }, state->companyText, 64, state->companyEditMode)) state->companyEditMode = !state->companyEditMode;
        GuiLabel((Rectangle){ bounds.x + 775, bounds.y + 148, 160, 24 }, "Short Description:");
        if (GuiTextBox((Rectangle){ bounds.x + 775, bounds.y + 172, 228, 28 }, state->toolDescriptionText, 64, state->toolDescriptionEditMode)) state->toolDescriptionEditMode = !state->toolDescriptionEditMode;
        
        GuiGroupBox((Rectangle){ bounds.x + 765, bounds.y + 236, 248, 226 }, "#142#Code Generation Options");
        GuiCheckBox((Rectangle){ bounds.x + 785, bounds.y + 292, 16, 16 }, "Export anchors", &state->exportAnchorsChecked);
        GuiCheckBox((Rectangle){ bounds.x + 785, bounds.y + 292 + 24, 16, 16 }, "Define Rectangles", &state->defineRecsChecked);
        GuiCheckBox((Rectangle){ bounds.x + 785, bounds.y + 292 + 48, 16, 16 }, "Define text as const", &state->defineTextsChecked);
        GuiCheckBox((Rectangle){ bounds.x + 785, bounds.y + 292 + 72, 16, 16 }, "Include detailed comments", &state->fullCommentsChecked);
        GuiCheckBox((Rectangle){ bounds.x + 785, bounds.y + 292 + 96, 16, 16 }, "Generate button functions", &state->genButtonFuncsChecked);

        if (state->codeTemplateActive != 2) GuiDisable();
        state->btnLoadCustomTemplatePressed = GuiButton((Rectangle){ bounds.x + 775, bounds.y + 292 + 128, 228, 24 }, state->customTemplateLoaded? "#9#Unload Custom Template" : "#5#Load Custom Template");
        GuiEnable();

        // Export generated code button
        state->btnExportCodePressed = GuiButton((Rectangle){ bounds.x + 765, bounds.y + 278 + 184 + 16, 248, 28 }, "#7#Export Generated Code");

        // Select desired code template to fill
        if (GuiDropdownBox((Rectangle){ bounds.x + 775, bounds.y + 256, 228, 24 }, 
                "STANDARD TEMPLATE (.c);PORTABLE TEMPLATE (.h); CUSTOM TEMPLATE (.c/.h)",
                &state->codeTemplateActive, state->codeTemplateEditMode)) state->codeTemplateEditMode = !state->codeTemplateEditMode;

        // Draw generated code
        Rectangle codePanel = { bounds.x + 10, bounds.y + 35, 745, 595 };
        Rectangle view = { 0 };
        GuiScrollPanel(codePanel, NULL, (Rectangle){ codePanel.x, codePanel.y, codePanel.width*2, (float)state->codeHeight }, &state->codePanelScrollOffset, &view);

        if (state->codeText != NULL)
        {
            BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);
                unsigned int linesCounter = 0;
                char *currentLine = state->codeText;

                while (currentLine)
                {
                    char *nextLine = strchr(currentLine, '\n');
                    if (nextLine) *nextLine = '\0'; // Temporaly terminating the current line

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

    return result;
}

#endif // GUI_WINDOW_CODEGEN_IMPLEMENTATION
