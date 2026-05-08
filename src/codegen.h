/**********************************************************************************************
*
*   rGuiLayout - Code Generator
* 
*   CODEGEN SUPPORTED VARIABLES:
* 
*       > Layout general description variables
*           $(GUILAYOUT_NAME)
*               $(GUILAYOUT_NAME_PASCALCASE)
*               $(GUILAYOUT_NAME_UPPERCASE)
*               $(GUILAYOUT_NAME_LOWERCASE)
*           $(GUILAYOUT_VERSION)
*           $(GUILAYOUT_DESCRIPTION)
*           $(GUILAYOUT_COMPANY)
*           $(GUILAYOUT_WINDOW_WIDTH)
*           $(GUILAYOUT_WINDOW_HEIGHT)
*
*       > Layout C file (.c) data generation variables:
*           $(GUILAYOUT_FUNCTION_DECLARATION_C)
*           $(GUILAYOUT_FUNCTION_DEFINITION_C)
*           $(GUILAYOUT_INITIALIZATION_C)
*           $(GUILAYOUT_DRAWING_C)
*
*       > Layout Header file (.h) data generation variables:
*           $(GUILAYOUT_STRUCT_TYPE)
*           $(GUILAYOUT_FUNCTIONS_DECLARATION_H)
*           $(GUILAYOUT_FUNCTION_INITIALIZE_H)
*           $(GUILAYOUT_FUNCTION_DRAWING_H)
*
*   NOTE: Code generated requires raygui 3.5-dev
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

#include "rguilayout.h"

#ifndef CODEGEN_H
#define CODEGEN_H

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
char *GenLayoutCode(const char *buffer, GuiLayout *layout, Vector2 offset, GuiLayoutConfig config);

#ifdef __cplusplus
}
#endif

#endif // CODEGEN_H

/***********************************************************************************
*
*   CODEGEN_IMPLEMENTATION
*
************************************************************************************/
#if defined(CODEGEN_IMPLEMENTATION)

#define TABAPPEND(x, y, z)          { for (int t = 0; t < z; t++) TextAppend(x, "    ", y); }
#define ENDLINEAPPEND(x, y)         TextAppend(x, "\n", y);

//----------------------------------------------------------------------------------
// Global variables definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------

// .C Writting code functions (.c)
static void WriteFunctionsDeclarationC(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs);
static void WriteInitializationC(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs);
static void WriteDrawingC(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs);
static void WriteFunctionsDefinitionC(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs);

// .H Writting code functions (.h)
static void WriteStruct(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs);
static void WriteFunctionsDeclarationH(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs);
static void WriteFunctionInitializeH(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs);
static void WriteFunctionDrawingH(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs);

// Generic writting code functions (.c/.h)
static void WriteRectangleVariables(char *toolstr, int *pos, GuiLayoutControl control, bool exportAnchors, bool fullComments, const char *preText, int tabs, bool exportH);
static void WriteAnchors(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, bool define, bool initialize, const char *preText, int tabs);
static void WriteConstText(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs);
static void WriteControlsVariables(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, bool define, bool initialize, const char *preText, int tabs);
static void WriteControlsDrawing(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, const char *preText, int tabs);
static void WriteControlDraw(char *toolstr, int *pos, int index, GuiLayoutControl control, GuiLayoutConfig config, const char *preText);

// Get controls specific texts functions
static char *GetControlRectangleText(int index, GuiLayoutControl control, bool defineRecs, bool exportAnchors,  const char *preText);
static char *GetScrollPanelContainerRecText(int index, GuiLayoutControl control, bool defineRecs, bool exportAnchors, const char *preText);
static char *GetControlTextParam(GuiLayoutControl control, bool defineText);
static char *GetControlNameParam(char *controlName, const char *preText);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Generate layout code string
// TODO: WARNING: layout is passed as value, probably not a good idea considering the size of the object
char *GenLayoutCode(const char *buffer, GuiLayout *layout, Vector2 offset, GuiLayoutConfig config)
{
    #define MAX_CODE_SIZE            1024*1024       // Max code size: 1MB
    #define MAX_VARIABLE_NAME_SIZE     64

    if (buffer == NULL) return NULL;

    char *toolstr = (char *)RL_CALLOC(MAX_CODE_SIZE, sizeof(char));
    const char *substr = NULL;

    int bufferPos = 0;
    int codePos = 0;
    int bufferLen = (int)strlen(buffer);

    // Offset all enabled anchors from reference window and offset
    for (int a = 1; a < MAX_ANCHOR_POINTS; a++)
    {
        if (layout->anchors[a].enabled)
        {
            layout->anchors[a].x -= (int)(layout->refWindow.x + offset.x);
            layout->anchors[a].y -= (int)(layout->refWindow.y + offset.y);
        }
    }

    // In case of controls with no anchor, offset must be applied to control position
    // NOTE: refWindow (aka anchor[0]) is already implicit!
    for (int i = 0; i < layout->controlCount; i++)
    {
        if (layout->controls[i].ap->id == 0)
        {
            layout->controls[i].rec.x -= offset.x;
            layout->controls[i].rec.y -= offset.y;
        }
    }

    int i = 0;
    for (i = 0; i < bufferLen; i++)
    {
        if ((buffer[i] == '$') && (buffer[i + 1] == '('))       // Detect variable to be replaced!
        {
            int spaceWidth = 0;
            for (int j = i-1; j >= 0; j--)
            {
                if (buffer[j] == ' ') spaceWidth++;
                else
                {
                    if (buffer[j] != '\n') spaceWidth = 0;
                    break;
                }
            }
            int tabs = (int)spaceWidth/4;

            substr = TextSubtext(buffer, bufferPos, i - bufferPos);

            strcpy(toolstr + codePos, substr);
            codePos = (int)strlen(toolstr);

            i += 2;
            bufferPos = i;

            for (int j = 0; j < MAX_VARIABLE_NAME_SIZE; j++)
            {
                if (buffer[i + j] == ')')
                {
                    substr = TextSubtext(buffer, i, j);

                    if (TextIsEqual(substr, "GUILAYOUT_NAME")) TextAppend(toolstr, config.name, &codePos);
                    else if (TextIsEqual(substr, "GUILAYOUT_NAME_UPPERCASE")) TextAppend(toolstr, TextToUpper(config.name), &codePos);
                    else if (TextIsEqual(substr, "GUILAYOUT_NAME_LOWERCASE")) TextAppend(toolstr, TextToLower(config.name), &codePos);
                    else if (TextIsEqual(substr, "GUILAYOUT_NAME_PASCALCASE")) TextAppend(toolstr, TextToPascal(config.name), &codePos);
                    else if (TextIsEqual(substr, "GUILAYOUT_VERSION")) TextAppend(toolstr, config.version, &codePos);
                    else if (TextIsEqual(substr, "GUILAYOUT_DESCRIPTION")) TextAppend(toolstr, config.description, &codePos);
                    else if (TextIsEqual(substr, "GUILAYOUT_COMPANY")) TextAppend(toolstr, config.company, &codePos);
                    else if (TextIsEqual(substr, "GUILAYOUT_WINDOW_WIDTH"))
                    {
                       if (layout->refWindow.width > 0) TextAppend(toolstr, TextFormat("%i", (int)layout->refWindow.width), &codePos);
                       else TextAppend(toolstr, TextFormat("%i", 800), &codePos);
                    }
                    else if (TextIsEqual(substr, "GUILAYOUT_WINDOW_HEIGHT"))
                    {
                       if (layout->refWindow.height > 0) TextAppend(toolstr, TextFormat("%i", (int)layout->refWindow.height), &codePos);
                       else TextAppend(toolstr, TextFormat("%i", 450), &codePos);
                    }

                    // C IMPLEMENTATION
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTION_DECLARATION_C")) WriteFunctionsDeclarationC(toolstr, &codePos, layout, config, tabs);
                    else if (TextIsEqual(substr, "GUILAYOUT_INITIALIZATION_C")) WriteInitializationC(toolstr, &codePos, layout, config, tabs);
                    else if (TextIsEqual(substr, "GUILAYOUT_DRAWING_C") && layout->controlCount > 0) WriteDrawingC(toolstr, &codePos, layout, config, tabs);
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTION_DEFINITION_C")) WriteFunctionsDefinitionC(toolstr, &codePos, layout, config, tabs);

                    // H IMPLEMENTATION
                    else if (TextIsEqual(substr, "GUILAYOUT_STRUCT_TYPE")) WriteStruct(toolstr, &codePos, layout, config, tabs);
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTIONS_DECLARATION_H")) WriteFunctionsDeclarationH(toolstr, &codePos, layout, config, tabs);
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTION_INITIALIZE_H")) WriteFunctionInitializeH(toolstr, &codePos, layout, config, tabs);
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTION_DRAWING_H") && layout->controlCount > 0) WriteFunctionDrawingH(toolstr, &codePos, layout, config, tabs);

                    bufferPos += (j + 1);

                    break;
                }
            }
        }
    }

    // Copy remaining text into toolstr
    strcpy(toolstr + codePos, buffer + bufferPos);

    // In case of controls with no anchor, offset must be applied to control position
    for (int i = 0; i < layout->controlCount; i++)
    {
        if (layout->controls[i].ap->id == 0)
        {
            layout->controls[i].rec.x += offset.y;
            layout->controls[i].rec.y += offset.y;
        }
    }

    // Restored all enabled anchors to reference window and offset
    for (int a = 1; a < MAX_ANCHOR_POINTS; a++)
    {
        if (layout->anchors[a].enabled)
        {
            layout->anchors[a].x += (int)(layout->refWindow.x + offset.x);
            layout->anchors[a].y += (int)(layout->refWindow.y + offset.y);
        }
    }

    return toolstr;
}

//----------------------------------------------------------------------------------
// .C specific writting code functions (.h)
//----------------------------------------------------------------------------------

// Write functions declaration code (.c)
static void WriteFunctionsDeclarationC(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs)
{
    if(config.exportButtonFunctions)
    {
        // Define required functions for calling
        int buttonsCount = 0;
        for (int i = 0; i < layout->controlCount; i++)
        {
            int type = layout->controls[i].type;
            if (type == GUI_BUTTON || type == GUI_LABELBUTTON)
            {
                buttonsCount++;
                TextAppend(toolstr, TextFormat("static void %s();", TextToPascal(layout->controls[i].name)), pos);
                if (config.fullComments)
                {
                    TABAPPEND(toolstr, pos, 4);
                    TextAppend(toolstr, TextFormat("// %s: %s logic", controlTypeName[layout->controls[i].type], layout->controls[i].name), pos);
                }
                ENDLINEAPPEND(toolstr, pos);
                TABAPPEND(toolstr, pos, tabs);
            }
        }
        if (buttonsCount > 0) *pos -= 1;
    }   
}

// Write variables initialization code (.c)
static void WriteInitializationC(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs)
{
    // Const text
    if (config.defineTexts) WriteConstText(toolstr, pos, layout, config, tabs);

    // Anchors
    if (config.exportAnchors && (layout->anchorCount > 0)) WriteAnchors(toolstr, pos, layout, config, true, true, "", tabs);

    // Control variables
    if (layout->controlCount > 0) WriteControlsVariables(toolstr, pos, layout, config, true, true, "", tabs);

    // Rectangles
    if (config.defineRecs)
    {
        ENDLINEAPPEND(toolstr, pos);
        ENDLINEAPPEND(toolstr, pos);
        TABAPPEND(toolstr, pos, tabs);
        // Define controls rectangles
        if (config.fullComments)
        {
            TextAppend(toolstr, "// Define controls rectangles", pos);
            ENDLINEAPPEND(toolstr, pos);
            TABAPPEND(toolstr, pos, tabs);
        }
        TextAppend(toolstr, TextFormat("Rectangle layoutRecs[%i] = {", layout->controlCount), pos);
        ENDLINEAPPEND(toolstr, pos);

        for (int k = 0; k < layout->controlCount; k++)
        {
            TABAPPEND(toolstr, pos, tabs + 1);
            WriteRectangleVariables(toolstr, pos, layout->controls[k], config.exportAnchors, config.fullComments, "", tabs, false);
            ENDLINEAPPEND(toolstr, pos);
        }
        TABAPPEND(toolstr, pos, tabs);
        TextAppend(toolstr, "};", pos);
    }
}

// Write drawing code (.c)
static void WriteDrawingC(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs)
{
    if (layout->controlCount > 0) WriteControlsDrawing(toolstr, pos, layout, config, "", tabs);
}

// Write functions definition code (.c)
static void WriteFunctionsDefinitionC(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs)
{
    if(config.exportButtonFunctions)
    {
        for (int i = 0; i < layout->controlCount; i++)
        {
            int type = layout->controls[i].type;
            if (type == GUI_BUTTON || type == GUI_LABELBUTTON)
            {
                if (config.fullComments)
                {
                    TextAppend(toolstr, TextFormat("// %s: %s logic", controlTypeName[layout->controls[i].type], layout->controls[i].name), pos);
                    ENDLINEAPPEND(toolstr, pos);
                    TABAPPEND(toolstr, pos, tabs);
                }

                TextAppend(toolstr, TextFormat("static void %s()", TextToPascal(layout->controls[i].name)), pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
                TextAppend(toolstr, "{", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
                TextAppend(toolstr, "// TODO: Implement control logic", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
                TextAppend(toolstr, "}", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            }
        }
    }
}

//----------------------------------------------------------------------------------
// .H specific writting code functions (.h)
//----------------------------------------------------------------------------------

// Write state structure code (.h)
static void WriteStruct(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs)
{
    TABAPPEND(toolstr, pos, tabs);
    TextAppend(toolstr, "typedef struct {", pos);
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);

    // Write anchors variables (forced on .h)
    if (config.exportAnchors && (layout->anchorCount > 0)) WriteAnchors(toolstr, pos, layout, config, true, false, "", tabs + 1);

    // Write controls variables
    if (layout->controlCount > 0) WriteControlsVariables(toolstr, pos, layout, config, true, false, "", tabs + 1);

    // Export rectangles
    if (config.defineRecs)
    {
        ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);

        // Write rectangles
        if (config.fullComments)
        {
            TextAppend(toolstr, "// Define rectangles", pos);
            ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
        }

        TextAppend(toolstr, TextFormat("Rectangle layoutRecs[%i];", layout->controlCount), pos);
    }

    ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
    TextAppend(toolstr, "// Custom state variables (depend on development software)", pos);
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
    TextAppend(toolstr, "// NOTE: This variables should be added manually if required", pos);

    ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    TextAppend(toolstr, TextFormat("} Gui%sState;", TextToPascal(config.name)), pos);
}

// Write variables declaration code (.h)
static void WriteFunctionsDeclarationH(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs)
{
    TextAppend(toolstr, TextFormat("Gui%sState InitGui%s(void);", TextToPascal(config.name), TextToPascal(config.name)), pos);
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    TextAppend(toolstr, TextFormat("void Gui%s(Gui%sState *state);", TextToPascal(config.name), TextToPascal(config.name)), pos);    

    // Generate buttons functions declaration
    if(config.exportButtonFunctions)
    {
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
        WriteFunctionsDeclarationC(toolstr, pos, layout, config, tabs);
    }   
}

// Write initialization function code (.h)
static void WriteFunctionInitializeH(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs)
{
     // Export InitGuiLayout function definition
    TextAppend(toolstr, TextFormat("Gui%sState InitGui%s(void)", TextToPascal(config.name), TextToPascal(config.name)), pos);
    ENDLINEAPPEND(toolstr, pos);
    TextAppend(toolstr, "{", pos);
    ENDLINEAPPEND(toolstr, pos);
    TABAPPEND(toolstr, pos, tabs + 1);
    TextAppend(toolstr, TextFormat("Gui%sState state = { 0 };", TextToPascal(config.name)), pos);
    ENDLINEAPPEND(toolstr, pos);
    ENDLINEAPPEND(toolstr, pos);
    TABAPPEND(toolstr, pos, tabs + 1);

    // Init anchors (forced on .h)
    if (config.exportAnchors && (layout->anchorCount > 0)) WriteAnchors(toolstr, pos, layout, config, false, true, "state.", tabs + 1);

    // Init controls variables
    if (layout->controlCount > 0)
    {
        WriteControlsVariables(toolstr, pos, layout, config, false, true, "state.", tabs + 1);
    }

    // Define controls rectangles if required
    if (config.defineRecs)
    {
        ENDLINEAPPEND(toolstr, pos);
        ENDLINEAPPEND(toolstr, pos);
        TABAPPEND(toolstr, pos, tabs + 1);

        // Define controls rectangles
        if (config.fullComments)
        {
            TextAppend(toolstr, "// Init controls rectangles", pos);
            ENDLINEAPPEND(toolstr, pos);
            TABAPPEND(toolstr, pos, tabs + 1);
        }

        for (int k = 0; k < layout->controlCount; k++)
        {
            TextAppend(toolstr, TextFormat("state.layoutRecs[%i] = ", k), pos);
            WriteRectangleVariables(toolstr, pos, layout->controls[k], config.exportAnchors, config.fullComments, "state.", tabs, true);
            ENDLINEAPPEND(toolstr, pos);
            TABAPPEND(toolstr, pos, tabs + 1);
        }

        *pos -= (tabs + 1)*4 + 1;
    }

    ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
    TextAppend(toolstr, "// Custom variables initialization", pos);
    ENDLINEAPPEND(toolstr, pos);

    // Return gui state after defining all its variables
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
    TextAppend(toolstr, "return state;", pos);

    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    TextAppend(toolstr, "}", pos);
    
    // Generate buttons functions implementation
    if(config.exportButtonFunctions)
    {
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
        WriteFunctionsDefinitionC(toolstr, pos, layout, config, tabs);
    }  
}

// Write functions drawing code (.h)
static void WriteFunctionDrawingH(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs)
{
    // Export GuiLayout draw function
    TextAppend(toolstr, TextFormat("void Gui%s(Gui%sState *state)", TextToPascal(config.name), TextToPascal(config.name)), pos);
    ENDLINEAPPEND(toolstr, pos);
    TextAppend(toolstr, "{", pos);
    ENDLINEAPPEND(toolstr, pos);
    TABAPPEND(toolstr, pos, tabs + 1);

    // Const text
    if (config.defineTexts) WriteConstText(toolstr, pos, layout, config, tabs + 1);

    // Controls draw
    if (layout->controlCount > 0) WriteControlsDrawing(toolstr, pos, layout, config, "state->", tabs + 1);

    ENDLINEAPPEND(toolstr, pos);
    TextAppend(toolstr, "}", pos);
}

//----------------------------------------------------------------------------------
// Generic writting code functions (.c/.h)
//----------------------------------------------------------------------------------

// Write rectangle variables code (.c/.h)
static void WriteRectangleVariables(char *toolstr, int *pos, GuiLayoutControl control, bool exportAnchors, bool fullComments, const char *preText, int tabs, bool exportH)
{
    if (exportAnchors && control.ap->id > 0)
    {
        TextAppend(toolstr, TextFormat("(Rectangle){ %s%s.x + %i, %s%s.y + %i, %i, %i }", preText, control.ap->name, (int)control.rec.x, preText, control.ap->name, (int)control.rec.y, (int)control.rec.width, (int)control.rec.height), pos);
    }
    else
    {
        if (control.ap->id > 0) TextAppend(toolstr, TextFormat("(Rectangle){ %i, %i, %i, %i }", (int)control.rec.x + control.ap->x, (int)control.rec.y + control.ap->y, (int)control.rec.width, (int)control.rec.height), pos);
        else TextAppend(toolstr, TextFormat("(Rectangle){ %i, %i, %i, %i }", (int)control.rec.x - control.ap->x, (int)control.rec.y - control.ap->y, (int)control.rec.width, (int)control.rec.height), pos);
    }

    if (exportH) TextAppend(toolstr, ";", pos);
    else  TextAppend(toolstr, ",", pos);

    if (fullComments)
    {
        TABAPPEND(toolstr, pos, tabs);
        TextAppend(toolstr, TextFormat("// %s: %s",controlTypeName[control.type], control.name), pos);
    }
}

// Write anchors code (.c/.h)
static void WriteAnchors(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, bool define, bool initialize, const char *preText, int tabs)
{
    if (config.fullComments)
    {
        if (define) TextAppend(toolstr, "// Define anchors", pos);
        else if (initialize) TextAppend(toolstr, "// Init anchors", pos);
        ENDLINEAPPEND(toolstr, pos);
        TABAPPEND(toolstr, pos, tabs);
    }

    for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
    {
        GuiAnchorPoint anchor = layout->anchors[i];

        if (anchor.enabled)
        {
            if (define) TextAppend(toolstr, "Vector2 ", pos);
            else TextAppend(toolstr, TextFormat("%s", preText), pos);
            TextAppend(toolstr, TextFormat("%s", anchor.name), pos);
            if (initialize)
            {
                TextAppend(toolstr, " = ", pos);
                if(!define) TextAppend(toolstr, "(Vector2)", pos);
                TextAppend(toolstr, TextFormat("{ %i, %i }", (int)layout->anchors[i].x, (int)layout->anchors[i].y), pos);
            }
            TextAppend(toolstr, ";", pos);

            if (config.fullComments)
            {
                TABAPPEND(toolstr, pos, 3);
                TextAppend(toolstr, TextFormat("// ANCHOR ID:%i", anchor.id), pos);
            }

            ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
        }
    }
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
}

// Write controls "text as const" code (.c/.h)
static void WriteConstText(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, int tabs)
{
    // Const variables and define text
    if (config.fullComments)
    {
        TextAppend(toolstr, "// Const text", pos);
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    }

    bool drawConstText = false;
    for (int i = 0; i < layout->controlCount; i++)
    {
        int type = layout->controls[i].type;

        switch(type)
        {
            case GUI_WINDOWBOX:
            case GUI_GROUPBOX:
            case GUI_LABEL:
            case GUI_BUTTON:
            case GUI_LABELBUTTON:
            case GUI_CHECKBOX:
            case GUI_TOGGLE:
            case GUI_SLIDER:
            case GUI_SLIDERBAR:
            case GUI_PROGRESSBAR:
            case GUI_TOGGLEGROUP:
            case GUI_COMBOBOX:
            case GUI_DROPDOWNBOX:
            case GUI_LISTVIEW:
            case GUI_DUMMYREC:
            case GUI_STATUSBAR:
            case GUI_LINE:
            case GUI_PANEL:
            case GUI_VALUEBOX:
            case GUI_SPINNER:
            case GUI_SCROLLPANEL:
            case GUI_COLORPICKER:
            {
                // Skip constant text for elements with no text
                if (layout->controls[i].text[0] == '\0') continue;

                TextAppend(toolstr, TextFormat("const char *%sText = \"%s\";", layout->controls[i].name, layout->controls[i].text), pos);
                if (config.fullComments)
                {
                    TABAPPEND(toolstr, pos, 1);
                    TextAppend(toolstr, TextFormat("// %s: %s", TextToUpper(controlTypeName[layout->controls[i].type]), layout->controls[i].name), pos);
                }
                ENDLINEAPPEND(toolstr, pos);
                TABAPPEND(toolstr, pos, tabs);
                drawConstText = true;
                
            } break;
            default: break;
        }
    }

    if (drawConstText) ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
}

// Write controls variables code (.c/.h)
static void WriteControlsVariables(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, bool define, bool initialize, const char *preText, int tabs)
{
    if (config.fullComments)
    {
        if (define) TextAppend(toolstr, "// Define controls variables", pos);
        else if (initialize) TextAppend(toolstr, "// Initilize controls variables", pos);
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    }

    for (int i = 0; i < layout->controlCount; i++)
    {
        bool drawVariables = true;
        GuiLayoutControl control = layout->controls[i];

        switch (control.type)
        {
            case GUI_WINDOWBOX:
            {
                if (define) TextAppend(toolstr, "bool ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sActive", control.name), pos);
                if (initialize) TextAppend(toolstr, " = true", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_BUTTON:
            case GUI_LABELBUTTON:
            {
                if(!config.exportButtonFunctions)
                {
                    if (define) TextAppend(toolstr, "bool ", pos);
                    else TextAppend(toolstr, TextFormat("%s", preText), pos);
                    TextAppend(toolstr, TextFormat("%sPressed", control.name), pos);
                    if (initialize) TextAppend(toolstr, " = false", pos);
                    TextAppend(toolstr, ";", pos);
                }
                else drawVariables = false;
            } break;
            case GUI_CHECKBOX:
            {
                if (define) TextAppend(toolstr, "bool ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sChecked", control.name), pos);
                if (initialize) TextAppend(toolstr, " = false", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_TOGGLE:
            {
                if (define) TextAppend(toolstr, "bool ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sActive", control.name), pos);
                if (initialize) TextAppend(toolstr, " = true", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_TOGGLEGROUP:
            case GUI_COMBOBOX:
            {
                if (define) TextAppend(toolstr, "int ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sActive", control.name), pos);
                if (initialize) TextAppend(toolstr, " = 0", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_LISTVIEW:
            {
                if (define) TextAppend(toolstr, "int ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sScrollIndex", control.name), pos);
                if (initialize) TextAppend(toolstr, " = 0", pos);
                TextAppend(toolstr, ";", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
                
                if (define) TextAppend(toolstr, "int ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sActive", control.name), pos);
                if (initialize) TextAppend(toolstr, " = 0", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_DROPDOWNBOX:
            {
                if (define) TextAppend(toolstr, "bool ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sEditMode", control.name), pos);
                if (initialize) TextAppend(toolstr, " = false", pos);
                TextAppend(toolstr, ";", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);

                if (define) TextAppend(toolstr, "int ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sActive", control.name), pos);
                if (initialize) TextAppend(toolstr, " = 0", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_TEXTBOX:
            case GUI_TEXTBOXMULTI:
            {
                if (define) TextAppend(toolstr, "bool ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sEditMode", control.name), pos);
                if (initialize) TextAppend(toolstr, " = false", pos);
                TextAppend(toolstr, ";", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);

                if (define)
                {
                    TextAppend(toolstr, TextFormat("char %sText[%i]", control.name, MAX_CONTROL_TEXT_LENGTH), pos);
                    if (initialize) TextAppend(toolstr, TextFormat(" = \"%s\"", control.text), pos);
                }
                else if (initialize) TextAppend(toolstr, TextFormat("strcpy(%s%sText, \"%s\")", preText, control.name, control.text), pos);
                TextAppend(toolstr, ";", pos);

            } break;
            case GUI_VALUEBOX:
            case GUI_SPINNER:
            {
                if (define) TextAppend(toolstr, "bool ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sEditMode", control.name), pos);
                if (initialize) TextAppend(toolstr, " = false", pos);
                TextAppend(toolstr, ";", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);

                if (define) TextAppend(toolstr, "int ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sValue", control.name), pos);
                if (initialize) TextAppend(toolstr, " = 0", pos);
                TextAppend(toolstr, ";", pos);

            } break;
            case GUI_SLIDER:
            case GUI_SLIDERBAR:
            case GUI_PROGRESSBAR:
            {
                if (define) TextAppend(toolstr, "float ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sValue", control.name), pos);
                if (initialize) TextAppend(toolstr, " = 0.0f", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_COLORPICKER:
            {
                if (define) TextAppend(toolstr, "Color ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sValue", control.name), pos);
                if (initialize)
                {
                    TextAppend(toolstr, " = ", pos);
                    if (!define) TextAppend(toolstr, "(Color)", pos);
                    TextAppend(toolstr, "{ 0, 0, 0, 0 }", pos);
                }
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_SCROLLPANEL:
            {
                if (define) TextAppend(toolstr, "Rectangle ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sScrollView", control.name), pos);
                if (initialize)
                {
                    TextAppend(toolstr, " = ", pos);
                    if (!define) TextAppend(toolstr, "(Rectangle)", pos);
                    TextAppend(toolstr, "{ 0, 0, 0, 0 }", pos);
                }
                TextAppend(toolstr, ";", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);

                if (define) TextAppend(toolstr, "Vector2 ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sScrollOffset", control.name), pos);
                if (initialize)
                {
                    TextAppend(toolstr, " = ", pos);
                    if (!define) TextAppend(toolstr, "(Vector2)", pos);
                    TextAppend(toolstr, "{ 0, 0 }", pos);
                }
                TextAppend(toolstr, ";", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);

                if (define) TextAppend(toolstr, "Vector2 ", pos);
                else TextAppend(toolstr, TextFormat("%s", preText), pos);
                TextAppend(toolstr, TextFormat("%sBoundsOffset", control.name), pos);
                if (initialize)
                {
                    TextAppend(toolstr, " = ", pos);
                    if (!define) TextAppend(toolstr, "(Vector2)", pos);
                    TextAppend(toolstr, "{ 0, 0 }", pos);
                }
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_GROUPBOX:
            case GUI_LINE:
            case GUI_PANEL:
            case GUI_LABEL:
            case GUI_DUMMYREC:
            case GUI_STATUSBAR:
            default:
            {
                drawVariables = false;
            }break;
        }

        if (drawVariables)
        {
            if (config.fullComments)
            {
                TABAPPEND(toolstr, pos, 3);
                TextAppend(toolstr, TextFormat("// %s: %s", controlTypeName[layout->controls[i].type], layout->controls[i].name), pos);
            }
            ENDLINEAPPEND(toolstr, pos);
            TABAPPEND(toolstr, pos, tabs);
        }
    }
    *pos -= tabs*4 + 1; // Remove last \n\t
}

// Write controls drawing code (full block) (.c/.h)
static void WriteControlsDrawing(char *toolstr, int *pos, GuiLayout *layout, GuiLayoutConfig config, const char *preText, int tabs)
{
    if (config.fullComments)
    {
        TextAppend(toolstr, "// Draw controls", pos);
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    }

    bool *draw = (bool *)RL_CALLOC(layout->controlCount, sizeof(bool));
    for (int i = 0; i < layout->controlCount; i++) draw[i] = false;

    // If dropdown control exist, draw GuiLock condition
    bool dropDownExist = false;
    for (int i = 0; i < layout->controlCount; i++)
    {
        if (layout->controls[i].type == GUI_DROPDOWNBOX)
        {
            if (!dropDownExist)
            {
                dropDownExist = true;
                TextAppend(toolstr, "if (", pos);
            }
            else
            {
                TextAppend(toolstr, " || ", pos);
            }
            TextAppend(toolstr, TextFormat("%sEditMode", GetControlNameParam(layout->controls[i].name, preText)), pos);
        }
    }

    if (dropDownExist)
    {
        TextAppend(toolstr, ") GuiLock();", pos);
        ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    }

    // Draw GUI_WINDOWBOX
    for (int i = 0; i < layout->controlCount; i++)
    {
        if (!draw[i])
        {
            if (layout->controls[i].type == GUI_WINDOWBOX)
            {
                draw[i] = true;

                //char *rec = GetControlRectangleText(i, layout->controls[i], config.defineRecs, config.exportAnchors, preText);

                TextAppend(toolstr, TextFormat("if (%sActive)", GetControlNameParam(layout->controls[i].name, preText)), pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
                TextAppend(toolstr, "{", pos);

                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
                WriteControlDraw(toolstr, pos, i, layout->controls[i], config, preText); // Draw GUI_WINDOWBOX
                ENDLINEAPPEND(toolstr, pos);

                int windowAnchorID = layout->controls[i].ap->id;

                // Draw controls inside window
                for (int j = 0; j < layout->controlCount; j++)
                {
                    if (!draw[j] && i != j && layout->controls[j].type != GUI_WINDOWBOX  && layout->controls[j].type != GUI_DROPDOWNBOX)
                    {
                        if (windowAnchorID == layout->controls[j].ap->id)
                        {
                            draw[j] = true;

                            TABAPPEND(toolstr, pos, tabs + 1);
                            WriteControlDraw(toolstr, pos, j, layout->controls[j], config, preText);
                            ENDLINEAPPEND(toolstr, pos);
                        }
                    }
                }

                // Draw GUI_DROPDOWNBOX inside GUI_WINDOWBOX
                for (int j = 0; j < layout->controlCount; j++)
                {
                    if (!draw[j] && i != j && layout->controls[j].type == GUI_DROPDOWNBOX)
                    {
                        if (windowAnchorID == layout->controls[j].ap->id)
                        {
                            draw[j] = true;

                            TABAPPEND(toolstr, pos, tabs + 1);
                            WriteControlDraw(toolstr, pos, j, layout->controls[j], config, preText);
                            ENDLINEAPPEND(toolstr, pos);
                        }
                    }
                }
                TABAPPEND(toolstr, pos, tabs);
                TextAppend(toolstr, "}", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            }
        }
    }

    // Draw the rest of controls except dropdownbox
    for (int i = 0; i < layout->controlCount; i++)
    {
        if (!draw[i])
        {
            if (layout->controls[i].type != GUI_DROPDOWNBOX)
            {
                draw[i] = true;
                WriteControlDraw(toolstr, pos, i, layout->controls[i], config, preText);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            }
        }
    }

    // Draw GUI_DROPDOWNBOX
    for (int i = 0; i < layout->controlCount; i++)
    {
        if (!draw[i])
        {
            if (layout->controls[i].type == GUI_DROPDOWNBOX)
            {
                draw[i] = true;
                WriteControlDraw(toolstr, pos, i, layout->controls[i], config, preText);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            }
        }
    }

    if (dropDownExist)
    {
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
        TextAppend(toolstr, "GuiUnlock();", pos);
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    }

    *pos -= (tabs)*4 + 1; // Delete last tabs and \n

    RL_FREE(draw);
}

// Write control drawing code (individual controls) (.c/.h)
static void WriteControlDraw(char *toolstr, int *pos, int index, GuiLayoutControl control, GuiLayoutConfig config, const char *preText)
{
    char *rec = GetControlRectangleText(index, control, config.defineRecs, config.exportAnchors, preText);
    char *text = GetControlTextParam(control, config.defineTexts);
    char *name = GetControlNameParam(control.name, preText);

    // TODO: Define text for window, groupbox, buttons, toggles and dummyrecs
    switch (control.type)
    {
        case GUI_WINDOWBOX: TextAppend(toolstr, TextFormat("%sActive = !GuiWindowBox(%s, %s);", name, rec, text), pos); break;
        case GUI_GROUPBOX: TextAppend(toolstr, TextFormat("GuiGroupBox(%s, %s);", rec, text), pos); break;
        case GUI_LINE: TextAppend(toolstr, TextFormat("GuiLine(%s, %s);", rec, text), pos); break;
        case GUI_PANEL: TextAppend(toolstr, TextFormat("GuiPanel(%s, %s);", rec, text), pos); break;
        case GUI_LABEL: TextAppend(toolstr, TextFormat("GuiLabel(%s, %s);", rec, text), pos); break;
        case GUI_BUTTON: 
            if(!config.exportButtonFunctions) TextAppend(toolstr, TextFormat("%sPressed = GuiButton(%s, %s); ", name, rec, text), pos); 
            else TextAppend(toolstr, TextFormat("if (GuiButton(%s, %s)) %s(); ", rec, text, TextToPascal(control.name)), pos); 
            break;
        case GUI_LABELBUTTON: 
            if(!config.exportButtonFunctions) TextAppend(toolstr, TextFormat("%sPressed = GuiLabelButton(%s, %s);", name, rec, text), pos);
            else TextAppend(toolstr, TextFormat("if (GuiLabelButton(%s, %s)) %s();", rec, text, TextToPascal(control.name)), pos); 
            break;
        case GUI_CHECKBOX: TextAppend(toolstr, TextFormat("GuiCheckBox(%s, %s, &%sChecked);", rec, text, name), pos); break;
        case GUI_TOGGLE: TextAppend(toolstr, TextFormat("GuiToggle(%s, %s, &%sActive);", rec, text, name), pos); break;
        case GUI_TOGGLEGROUP:TextAppend(toolstr, TextFormat("GuiToggleGroup(%s, %s, &%sActive);", rec, text, name), pos); break;
        case GUI_COMBOBOX: TextAppend(toolstr, TextFormat("GuiComboBox(%s, %s, &%sActive);", rec, text, name), pos); break;
        case GUI_DROPDOWNBOX: TextAppend(toolstr, TextFormat("if (GuiDropdownBox(%s, %s, &%sActive, %sEditMode)) %sEditMode = !%sEditMode;", rec, text, name, name, name, name), pos); break;
        case GUI_TEXTBOX: TextAppend(toolstr, TextFormat("if (GuiTextBox(%s, %sText, %i, %sEditMode)) %sEditMode = !%sEditMode;", rec, name, MAX_CONTROL_TEXT_LENGTH, name, name, name), pos); break;
        case GUI_TEXTBOXMULTI: TextAppend(toolstr, TextFormat("if (GuiTextBoxMulti(%s, %sText, %i, %sEditMode)) %sEditMode = !%sEditMode;", rec, name, MAX_CONTROL_TEXT_LENGTH, name, name, name), pos); break;
        case GUI_VALUEBOX: TextAppend(toolstr, TextFormat("if (GuiValueBox(%s, %s, &%sValue, 0, 100, %sEditMode)) %sEditMode = !%sEditMode;", rec, text, name, name, name, name), pos); break;
        case GUI_SPINNER: TextAppend(toolstr, TextFormat("if (GuiSpinner(%s, %s, &%sValue, 0, 100, %sEditMode)) %sEditMode = !%sEditMode;", rec, text, name, name, name, name), pos); break;
        case GUI_SLIDER: TextAppend(toolstr, TextFormat("GuiSlider(%s, %s, NULL, &%sValue, 0, 100);", rec, text, name), pos); break;
        case GUI_SLIDERBAR: TextAppend(toolstr, TextFormat("GuiSliderBar(%s, %s, NULL, &%sValue, 0, 100);", rec, text, name), pos); break;
        case GUI_PROGRESSBAR: TextAppend(toolstr, TextFormat("GuiProgressBar(%s, %s, NULL, &%sValue, 0, 1);", rec, text, name), pos); break;
        case GUI_STATUSBAR: TextAppend(toolstr, TextFormat("GuiStatusBar(%s, %s);", rec, text), pos); break;
        case GUI_SCROLLPANEL:
        {
            char *containerRec = GetScrollPanelContainerRecText(index, control, config.defineRecs, config.exportAnchors, preText);
            TextAppend(toolstr, TextFormat("GuiScrollPanel(%s, %s, %s, &%sScrollOffset, &%sScrollView);", containerRec, text, rec, name, name), pos); break;
        }
        case GUI_LISTVIEW: TextAppend(toolstr, TextFormat("GuiListView(%s, %s, &%sScrollIndex, &%sActive);", rec, (text == NULL)? "null" : text, name, name), pos); break;
        case GUI_COLORPICKER: TextAppend(toolstr, TextFormat("GuiColorPicker(%s, %s, &%sValue);", rec, text, name), pos); break;
        case GUI_DUMMYREC: TextAppend(toolstr, TextFormat("GuiDummyRec(%s, %s);", rec, text), pos); break;
        default: break;
    }
}

// Get controls rectangle text
static char *GetControlRectangleText(int index, GuiLayoutControl control, bool defineRecs, bool exportAnchors, const char *preText)
{
    static char text[512];
    memset(text, 0, 512);

    if (defineRecs)
    {
        strcpy(text, TextFormat("%slayoutRecs[%i]", preText, index));
    }
    else
    {
        if (exportAnchors && control.ap->id > 0)
        {
            strcpy(text, TextFormat("(Rectangle){ %s%s.x + %i, %s%s.y + %i, %i, %i }", preText, control.ap->name, (int)control.rec.x, preText, control.ap->name, (int)control.rec.y, (int)control.rec.width, (int)control.rec.height));
        }
        else
        {
            if (control.ap->id > 0) strcpy(text, TextFormat("(Rectangle){ %i, %i, %i, %i }", (int)control.rec.x + control.ap->x, (int)control.rec.y + control.ap->y, (int)control.rec.width, (int)control.rec.height));
            else strcpy(text, TextFormat("(Rectangle){ %i, %i, %i, %i }", (int)control.rec.x - control.ap->x, (int)control.rec.y - control.ap->y, (int)control.rec.width, (int)control.rec.height));
        }
    }

    return text;
}

// Get scroll panel container rectangle text
static char *GetScrollPanelContainerRecText(int index, GuiLayoutControl control, bool defineRecs, bool exportAnchors, const char *preText)
{
    static char text[512];
    memset(text, 0, 512);

    if(defineRecs)
    {
        strcpy(text, TextFormat("(Rectangle){%slayoutRecs[%i].x, %slayoutRecs[%i].y, %slayoutRecs[%i].width - %s%sBoundsOffset.x, %slayoutRecs[%i].height - %s%sBoundsOffset.y }", preText, index, preText, index, preText, index, preText, control.name, preText, index, preText, control.name));
    }
    else
    {
        if (exportAnchors && control.ap->id > 0)
        {
            strcpy(text, TextFormat("(Rectangle){ %s%s.x + %i, %s%s.y + %i, %i - %s%sBoundsOffset.x, %i - %s%sBoundsOffset.y }", preText, control.ap->name, (int)control.rec.x, preText, control.ap->name, (int)control.rec.y, (int)control.rec.width, preText, control.name, (int)control.rec.height, preText, control.name));
        }
        else
        {
            // DOING
            if (control.ap->id > 0) strcpy(text, TextFormat("(Rectangle){ %i, %i, %i - %s%sBoundsOffset.x, %i - %s%sBoundsOffset.y }", (int)control.rec.x + control.ap->x, (int)control.rec.y + control.ap->y, (int)control.rec.width, preText, control.name, (int)control.rec.height, preText, control.name));
            else strcpy(text, TextFormat("(Rectangle){ %i, %i, %i - %s%sBoundsOffset.x, %i - %s%sBoundsOffset.y}", (int)control.rec.x - control.ap->x, (int)control.rec.y - control.ap->y, (int)control.rec.width, preText, control.name, (int)control.rec.height, preText, control.name));

        }
    }

    return text;
}

// Get controls parameters text
static char *GetControlTextParam(GuiLayoutControl control, bool defineText)
{
    static char text[512];
    memset(text, 0, 512);

    if (defineText) 
    {
        // Skip constant text for elements with no text
        if (control.text[0] == '\0') strcpy(text, "NULL");
        else strcpy(text, TextFormat("%sText", control.name));
    }
    else 
    {
        // NOTE: control.text will never be NULL
        if (control.text[0] == '\0') strcpy(text, "NULL");
        else strcpy(text, TextFormat("\"%s\"", control.text));
    }

    return text;
}

// Get controls name text
static char *GetControlNameParam(char *controlName, const char *preText)
{
    static char text[256];
    memset(text, 0, 256);

    strcpy(text, TextFormat("%s%s", preText, controlName));

    return text;
}

#endif // CODEGEN_IMPLEMENTATION