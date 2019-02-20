/**********************************************************************************************
*
*   rGuiLayout v2.0 - code generator functions
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

#include "rguilayout.h"

#ifndef CODEGEN_H
#define CODEGEN_H

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
unsigned char *GenerateLayoutCode(const unsigned char *buffer, GuiLayout layout, GuiLayoutConfig config);

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
static void WriteFunctionsDeclarationC(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs);
static void WriteInitializationC(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs);
static void WriteDrawingC(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs);
static void WriteFunctionsDefinitionC(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs);

// .H Writting code functions (.h)
static void WriteStruct(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs);
static void WriteFunctionsDeclarationH(unsigned char *toolstr, int *pos, GuiLayoutConfig config, int tabs);
static void WriteFunctionInitializeH(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs);
static void WriteFunctionDrawingH(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs);

// Generic writting code functions (.c/.h)
static void WriteRectangleVariables(unsigned char *toolstr, int *pos, GuiControl control, bool exportAnchors, bool fullComments, const char *preText, int tabs, bool exportH);
static void WriteAnchors(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, bool define, bool initialize, const char* preText, int tabs);
static void WriteConstText(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs);
static void WriteControlsVariables(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, bool define, bool initialize, const char *preText, int tabs);
static void WriteControlsDrawing(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, const char *preText, int tabs);
static void WriteControlDraw(unsigned char *toolstr, int *pos, int index, GuiControl control, GuiLayoutConfig config, const char *preText);

// Get controls specific texts functions
static char *GetControlRectangleText(int index, GuiControl control, bool defineRecs, bool exportAnchors,  const char *preText);
static char *GetScrollPanelContainerRecText(int index, GuiControl control, bool defineRecs, bool exportAnchors, const char *preText);
static char *GetControlTextParam(GuiControl control, bool defineText);
static char *GetControlNameParam(char *controlName, const char *preText);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Generate layout code string
unsigned char *GenerateLayoutCode(const unsigned char *buffer, GuiLayout layout, GuiLayoutConfig config)
{
    #define MAX_CODE_SIZE            1024*512
    #define MAX_VARIABLE_NAME_SIZE   64

    unsigned char *toolstr = (unsigned char *)calloc(MAX_CODE_SIZE, sizeof(unsigned char));
    unsigned const char *substr = NULL;

    int bufferPos = 0;
    int codePos = 0;
    int bufferLen = strlen(buffer);

    for (int a = 1; a < MAX_ANCHOR_POINTS; a++)
    {
        if (layout.anchors[a].enabled)
        {
            layout.anchors[a].x -= layout.refWindow.x;
            layout.anchors[a].y -= layout.refWindow.y;
        }
    }

    int i = 0;
    for (i = 0; i < bufferLen; i++)
    {
        if ((buffer[i] == '$') && (buffer[i + 1] == '('))
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
            codePos = strlen(toolstr);

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
                       if (layout.refWindow.width > 0) TextAppend(toolstr, FormatText("%i", (int)layout.refWindow.width), &codePos);
                       else TextAppend(toolstr, FormatText("%i", 800), &codePos);
                    }
                    else if (TextIsEqual(substr, "GUILAYOUT_WINDOW_HEIGHT"))
                    {
                       if (layout.refWindow.height > 0) TextAppend(toolstr, FormatText("%i", (int)layout.refWindow.height), &codePos);
                       else TextAppend(toolstr, FormatText("%i", 450), &codePos);
                    }

                    // C IMPLEMENTATION
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTION_DECLARATION_C")) WriteFunctionsDeclarationC(toolstr, &codePos, layout, config, tabs);
                    else if (TextIsEqual(substr, "GUILAYOUT_INITIALIZATION_C")) WriteInitializationC(toolstr, &codePos, layout, config, tabs);
                    else if (TextIsEqual(substr, "GUILAYOUT_DRAWING_C") && layout.controlsCount > 0) WriteDrawingC(toolstr, &codePos, layout, config, tabs);
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTION_DEFINITION_C")) WriteFunctionsDefinitionC(toolstr, &codePos, layout, config, tabs);

                    // H IMPLEMENTATION
                    else if (TextIsEqual(substr, "GUILAYOUT_STRUCT_TYPE")) WriteStruct(toolstr, &codePos, layout, config, tabs);
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTIONS_DECLARATION_H")) WriteFunctionsDeclarationH(toolstr, &codePos, config, tabs);
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTION_INITIALIZE_H")) WriteFunctionInitializeH(toolstr, &codePos, layout, config, tabs);
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTION_DRAWING_H") && layout.controlsCount > 0) WriteFunctionDrawingH(toolstr, &codePos, layout, config, tabs);

                    bufferPos += (j + 1);

                    break;
                }
            }
        }
    }

    substr = TextSubtext(buffer, bufferPos, i - bufferPos);
    strcpy(toolstr + codePos, substr);

    for (int a = 1; a < MAX_ANCHOR_POINTS; a++)
    {
        if (layout.anchors[a].enabled)
        {
            layout.anchors[a].x += layout.refWindow.x;
            layout.anchors[a].y += layout.refWindow.y;
        }
    }

    return toolstr;
}

//----------------------------------------------------------------------------------
// .C specific writting code functions (.h)
//----------------------------------------------------------------------------------

// Write functions declaration code (.c)
static void WriteFunctionsDeclarationC(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs)
{
    // Define required functions for calling
    int buttonsCount = 0;
    for (int i = 0; i < layout.controlsCount; i++)
    {
        int type = layout.controls[i].type;
        if (type == GUI_BUTTON || type == GUI_LABELBUTTON || type == GUI_IMAGEBUTTONEX)
        {
            buttonsCount++;
            TextAppend(toolstr, FormatText("static void %s();", layout.controls[i].name), pos);
            if (config.fullComments)
            {
                TABAPPEND(toolstr, pos, 4);
                TextAppend(toolstr, FormatText("// %s: %s logic", controlTypeName[layout.controls[i].type], layout.controls[i].name), pos);
            }
            ENDLINEAPPEND(toolstr, pos);
            TABAPPEND(toolstr, pos, tabs);
        }
    }
    if (buttonsCount > 0) *pos -= 2;
}

// Write variables initialization code (.c)
static void WriteInitializationC(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs)
{
    // Const text
    if (config.defineTexts) WriteConstText(toolstr, pos, layout, config, tabs);

    // Anchors
    if (config.exportAnchors && layout.anchorsCount > 1) WriteAnchors(toolstr, pos, layout, config, true, true, "", tabs);

    // Control variables
    if (layout.controlsCount > 0) WriteControlsVariables(toolstr, pos, layout, config, true, true, "", tabs);

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
        TextAppend(toolstr, FormatText("Rectangle layoutRecs[%i] = {", layout.controlsCount), pos);
        ENDLINEAPPEND(toolstr, pos);

        for (int k = 0; k < layout.controlsCount; k++)
        {
            TABAPPEND(toolstr, pos, tabs + 1);
            WriteRectangleVariables(toolstr, pos, layout.controls[k], config.exportAnchors, config.fullComments, "", tabs, false);
            ENDLINEAPPEND(toolstr, pos);
        }
        TABAPPEND(toolstr, pos, tabs);
        TextAppend(toolstr, "};", pos);
    }
}

// Write drawing code (.c)
static void WriteDrawingC(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs)
{
    if (layout.controlsCount > 0) WriteControlsDrawing(toolstr, pos, layout, config, "", tabs);
}

// Write functions definition code (.c)
static void WriteFunctionsDefinitionC(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs)
{
    for (int i = 0; i < layout.controlsCount; i++)
    {
        int type = layout.controls[i].type;
        if (type == GUI_BUTTON || type == GUI_LABELBUTTON || type == GUI_IMAGEBUTTONEX)
        {
            if (config.fullComments)
            {
                TextAppend(toolstr, FormatText("// %s: %s logic", controlTypeName[layout.controls[i].type], layout.controls[i].name), pos);
                ENDLINEAPPEND(toolstr, pos);
                TABAPPEND(toolstr, pos, tabs);
            }

            TextAppend(toolstr, FormatText("static void %s()", layout.controls[i].name), pos);
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

//----------------------------------------------------------------------------------
// .H specific writting code functions (.h)
//----------------------------------------------------------------------------------

// Write state structure code (.h)
static void WriteStruct(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs)
{
    TABAPPEND(toolstr, pos, tabs);
    TextAppend(toolstr, "typedef struct {", pos);
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);

    // Write anchors variables
    if (config.exportAnchors && layout.anchorsCount > 1) WriteAnchors(toolstr, pos, layout, config, true, false, "", tabs + 1);

    // Write controls variables
    if (layout.controlsCount > 0) WriteControlsVariables(toolstr, pos, layout, config, true, false, "", tabs + 1);

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

        TextAppend(toolstr, FormatText("Rectangle layoutRecs[%i];", layout.controlsCount), pos);
    }

    ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
    TextAppend(toolstr, "// Custom state variables (depend on development software)", pos);
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
    TextAppend(toolstr, "// NOTE: This variables should be added manually if required", pos);

    ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    TextAppend(toolstr, FormatText("} Gui%sState;", TextToPascal(config.name)), pos);
}

// Write variables declaration code (.h)
static void WriteFunctionsDeclarationH(unsigned char *toolstr, int *pos, GuiLayoutConfig config, int tabs)
{
    TextAppend(toolstr, FormatText("Gui%sState InitGui%s(void);", TextToPascal(config.name), TextToPascal(config.name)), pos);
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    TextAppend(toolstr, FormatText("void Gui%s(Gui%sState *state);", TextToPascal(config.name), TextToPascal(config.name)), pos);
}

// Write initialization function code (.h)
static void WriteFunctionInitializeH(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs)
{
     // Export InitGuiLayout function definition
    TextAppend(toolstr, FormatText("Gui%sState InitGui%s(void)", TextToPascal(config.name), TextToPascal(config.name)), pos);
    ENDLINEAPPEND(toolstr, pos);
    TextAppend(toolstr, "{", pos);
    ENDLINEAPPEND(toolstr, pos);
    TABAPPEND(toolstr, pos, tabs + 1);
    TextAppend(toolstr, FormatText("Gui%sState state = { 0 };", TextToPascal(config.name)), pos);
    ENDLINEAPPEND(toolstr, pos);
    ENDLINEAPPEND(toolstr, pos);
    TABAPPEND(toolstr, pos, tabs + 1);

    // Init anchors
    if (config.exportAnchors && layout.anchorsCount > 1)
    {
        WriteAnchors(toolstr, pos, layout, config, false, true, "state.", tabs + 1);
    }

    // Init controls variables
    if (layout.controlsCount > 0)
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

        for (int k = 0; k < layout.controlsCount; k++)
        {
            TextAppend(toolstr, FormatText("state.layoutRecs[%i] = ", k), pos);
            WriteRectangleVariables(toolstr, pos, layout.controls[k], config.exportAnchors, config.fullComments, "state.", tabs, true);
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

}

// Write functions drawing code (.h)
static void WriteFunctionDrawingH(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs)
{
    // Export GuiLayout draw function
    TextAppend(toolstr, FormatText("void Gui%s(Gui%sState *state)", TextToPascal(config.name), TextToPascal(config.name)), pos);
    ENDLINEAPPEND(toolstr, pos);
    TextAppend(toolstr, "{", pos);
    ENDLINEAPPEND(toolstr, pos);
    TABAPPEND(toolstr, pos, tabs + 1);

    // Const text
    if (config.defineTexts) WriteConstText(toolstr, pos, layout, config, tabs + 1);

    // Controls draw
    if (layout.controlsCount > 0) WriteControlsDrawing(toolstr, pos, layout, config, "state->", tabs + 1);

    ENDLINEAPPEND(toolstr, pos);
    TextAppend(toolstr, "}", pos);
}

//----------------------------------------------------------------------------------
// Generic writting code functions (.c/.h)
//----------------------------------------------------------------------------------

// Write rectangle variables code (.c/.h)
static void WriteRectangleVariables(unsigned char *toolstr, int *pos, GuiControl control, bool exportAnchors, bool fullComments, const char *preText, int tabs, bool exportH)
{
    if (exportAnchors && control.ap->id > 0)
    {
        TextAppend(toolstr, FormatText("(Rectangle){ %s%s.x + %i, %s%s.y + %i, %i, %i }", preText, control.ap->name, (int)control.rec.x, preText, control.ap->name, (int)control.rec.y, (int)control.rec.width, (int)control.rec.height), pos);
    }
    else
    {
        if (control.ap->id > 0) TextAppend(toolstr, FormatText("(Rectangle){ %i, %i, %i, %i }", (int)control.rec.x + control.ap->x, (int)control.rec.y + control.ap->y, (int)control.rec.width, (int)control.rec.height), pos);
        else TextAppend(toolstr, FormatText("(Rectangle){ %i, %i, %i, %i }", (int)control.rec.x - control.ap->x, (int)control.rec.y - control.ap->y, (int)control.rec.width, (int)control.rec.height), pos);
    }
    
    if (exportH) TextAppend(toolstr, ";", pos);
    else  TextAppend(toolstr, ",", pos);
    
    if (fullComments)
    {
        TABAPPEND(toolstr, pos, tabs);
        TextAppend(toolstr, FormatText("// %s: %s",controlTypeName[control.type], control.name), pos);
    }
}

// Write anchors code (.c/.h)
static void WriteAnchors(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, bool define, bool initialize, const char *preText, int tabs)
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
        GuiAnchorPoint anchor = layout.anchors[i];

        if (anchor.enabled)
        {
            if (define) TextAppend(toolstr, "Vector2 ", pos);
            else TextAppend(toolstr, FormatText("%s", preText), pos);
            TextAppend(toolstr, FormatText("%s", anchor.name), pos);
            if (initialize) 
            {
                TextAppend(toolstr, " = ", pos);
                if(!define) TextAppend(toolstr, "(Vector2)", pos);
                TextAppend(toolstr, FormatText("{ %i, %i }", (int)layout.anchors[i].x, (int)layout.anchors[i].y), pos);
            }
            TextAppend(toolstr, ";", pos);

            if (config.fullComments)
            {
                TABAPPEND(toolstr, pos, 3);
                TextAppend(toolstr, FormatText("// ANCHOR ID:%i", anchor.id), pos);
            }

            ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
        }
    }
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
}

// Write controls "text as const" code (.c/.h)
static void WriteConstText(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs)
{
    // Const variables and define text
    if (config.fullComments)
    {
        TextAppend(toolstr, "// Const text", pos);
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    }

    bool drawConstText = false;
    for (int i = 0; i < layout.controlsCount; i++)
    {
        int type = layout.controls[i].type;

        switch(type)
        {
            case GUI_WINDOWBOX:
            case GUI_GROUPBOX:
            case GUI_LABEL:
            case GUI_BUTTON:
            case GUI_LABELBUTTON:
            case GUI_IMAGEBUTTONEX:
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
                TextAppend(toolstr, FormatText("const char *%sText = \"%s\";", layout.controls[i].name, layout.controls[i].text), pos);
                if (config.fullComments)
                {
                    TABAPPEND(toolstr, pos, 1);
                    TextAppend(toolstr, FormatText("// %s: %s", TextToUpper(controlTypeName[layout.controls[i].type]), layout.controls[i].name), pos);
                }
                ENDLINEAPPEND(toolstr, pos);
                TABAPPEND(toolstr, pos, tabs);
                drawConstText = true;
            break;
            default: break;
        }
    }

    if (drawConstText) ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
}

// Write controls variables code (.c/.h)
static void WriteControlsVariables(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, bool define, bool initialize, const char *preText, int tabs)
{
    if (config.fullComments)
    {
        if (define) TextAppend(toolstr, "// Define controls variables", pos);
        else if (initialize) TextAppend(toolstr, "// Initilize controls variables", pos);
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    }

    for (int i = 0; i < layout.controlsCount; i++)
    {
        bool drawVariables = true;
        GuiControl control = layout.controls[i];
        switch (control.type)
        {
            case GUI_WINDOWBOX:
            {
                if (define) TextAppend(toolstr, "bool ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sActive", control.name), pos);
                if (initialize) TextAppend(toolstr, " = true", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_BUTTON:
            case GUI_LABELBUTTON:
            case GUI_IMAGEBUTTONEX:
            {
                if (define) TextAppend(toolstr, "bool ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sPressed", control.name), pos);
                if (initialize) TextAppend(toolstr, " = false", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_CHECKBOX:
            {
                if (define) TextAppend(toolstr, "bool ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sChecked", control.name), pos);
                if (initialize) TextAppend(toolstr, " = false", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_TOGGLE:
            {
                if (define) TextAppend(toolstr, "bool ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sActive", control.name), pos);
                if (initialize) TextAppend(toolstr, " = true", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_TOGGLEGROUP:
            case GUI_COMBOBOX:
            {
                if (define) TextAppend(toolstr, "int ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sActive", control.name), pos);
                if (initialize) TextAppend(toolstr, "= 0", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_LISTVIEW:
            {
                if (define) TextAppend(toolstr, "int ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sScrollIndex", control.name), pos);
                if (initialize) TextAppend(toolstr, " = 0", pos);
                TextAppend(toolstr, ";", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            }
            case GUI_DROPDOWNBOX:
            {
                if (define) TextAppend(toolstr, "bool ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sEditMode", control.name), pos);
                if (initialize) TextAppend(toolstr, " = false", pos);
                TextAppend(toolstr, ";", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);

                if (define) TextAppend(toolstr, "int ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sActive", control.name), pos);
                if (initialize) TextAppend(toolstr, " = 0", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_TEXTBOX:
            case GUI_TEXTBOXMULTI:
            {
                if (define) TextAppend(toolstr, "bool ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sEditMode", control.name), pos);
                if (initialize) TextAppend(toolstr, " = false", pos);
                TextAppend(toolstr, ";", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);

                if (define)
                {
                    TextAppend(toolstr, FormatText("unsigned char %sText[%i]", control.name, MAX_CONTROL_TEXT_LENGTH), pos);
                    if (initialize) TextAppend(toolstr, FormatText(" = \"%s\"", control.text), pos);
                }
                else if (initialize) TextAppend(toolstr, FormatText("strcpy(%s%sText, \"%s\")", preText, control.name, control.text), pos);
                TextAppend(toolstr, ";", pos);

            } break;
            case GUI_VALUEBOX:
            case GUI_SPINNER:
            {
                if (define) TextAppend(toolstr, "bool ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sEditMode", control.name), pos);
                if (initialize) TextAppend(toolstr, " = false", pos);
                TextAppend(toolstr, ";", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
                if (define) TextAppend(toolstr, "int ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sValue", control.name), pos);
                if (initialize) TextAppend(toolstr, " = 0", pos);
                TextAppend(toolstr, ";", pos);

            } break;
            case GUI_SLIDER:
            case GUI_SLIDERBAR:
            case GUI_PROGRESSBAR:
            {
                if (define) TextAppend(toolstr, "float ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sValue", control.name), pos);
                if (initialize) TextAppend(toolstr, " = 0.0f", pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_COLORPICKER:
            {
                if (define) TextAppend(toolstr, "Color ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sValue", control.name), pos);
                // TODO: if (initialize) TextAppend(toolstr, FormatText(" = { %i, %i }", (int)layout.anchors[i].x, (int)layout.anchors[i].y), pos);
                TextAppend(toolstr, ";", pos);
            } break;
            case GUI_SCROLLPANEL:
                if (define) TextAppend(toolstr, "Vector2 ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sScrollOffset", control.name), pos);
                if (initialize)
                {
                    TextAppend(toolstr, " = ", pos);
                    if(!define) TextAppend(toolstr, "(Vector2)", pos);
                    TextAppend(toolstr, "{ 0, 0 }", pos);
                }
                TextAppend(toolstr, ";", pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
                if (define) TextAppend(toolstr, "Vector2 ", pos);
                else TextAppend(toolstr, FormatText("%s", preText), pos);
                TextAppend(toolstr, FormatText("%sBoundsOffset", control.name), pos);
                if (initialize)
                {
                    TextAppend(toolstr, " = ", pos);
                    if(!define) TextAppend(toolstr, "(Vector2)", pos);
                    TextAppend(toolstr, "{ 0, 0 }", pos);
                }
                TextAppend(toolstr, ";", pos);
            // TODO SCROLLPANEL
            break;
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
                TextAppend(toolstr, FormatText("// %s: %s", controlTypeName[layout.controls[i].type], layout.controls[i].name), pos);
            }
            ENDLINEAPPEND(toolstr, pos);
            TABAPPEND(toolstr, pos, tabs);
        }
    }
    *pos -= tabs*4 + 1; // Remove last \n\t
}

// Write controls drawing code (full block) (.c/.h)
static void WriteControlsDrawing(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, const char *preText, int tabs)
{
    if (config.fullComments)
    {
        TextAppend(toolstr, "// Draw controls", pos);
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    }

    bool *draw = (bool *)malloc(layout.controlsCount*sizeof(bool));
    for (int i = 0; i < layout.controlsCount; i++) draw[i] = false;

    // If dropdown control exist, draw GuiLock condition
    bool dropDownExist = false;
    for (int i = 0; i < layout.controlsCount; i++)
    {
        if (layout.controls[i].type == GUI_DROPDOWNBOX)
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
            TextAppend(toolstr, FormatText("%sEditMode", GetControlNameParam(layout.controls[i].name, preText)), pos);
        }
    }

    if (dropDownExist)
    {
        TextAppend(toolstr, ") GuiLock();", pos);
        ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    }

    // Draw GUI_WINDOWBOX
    for (int i = 0; i < layout.controlsCount; i++)
    {
        if (!draw[i])
        {
            if (layout.controls[i].type == GUI_WINDOWBOX)
            {
                draw[i] = true;

                char *rec = GetControlRectangleText(i, layout.controls[i], config.defineRecs, config.exportAnchors, preText);

                TextAppend(toolstr, FormatText("if (%sActive)", GetControlNameParam(layout.controls[i].name, preText)), pos);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
                TextAppend(toolstr, "{", pos);

                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
                WriteControlDraw(toolstr, pos, i, layout.controls[i], config, preText); // Draw GUI_WINDOWBOX
                ENDLINEAPPEND(toolstr, pos);

                int windowAnchorID = layout.controls[i].ap->id;

                // Draw controls inside window
                for (int j = 0; j < layout.controlsCount; j++)
                {
                    if (!draw[j] && i != j && layout.controls[j].type != GUI_WINDOWBOX  && layout.controls[j].type != GUI_DROPDOWNBOX)
                    {
                        if (windowAnchorID == layout.controls[j].ap->id)
                        {
                            draw[j] = true;

                            TABAPPEND(toolstr, pos, tabs + 1);
                            WriteControlDraw(toolstr, pos, j, layout.controls[j], config, preText);
                            ENDLINEAPPEND(toolstr, pos);
                        }
                    }
                }

                // Draw GUI_DROPDOWNBOX inside GUI_WINDOWBOX
                for (int j = 0; j < layout.controlsCount; j++)
                {
                    if (!draw[j] && i != j && layout.controls[j].type == GUI_DROPDOWNBOX)
                    {
                        if (windowAnchorID == layout.controls[j].ap->id)
                        {
                            draw[j] = true;

                            TABAPPEND(toolstr, pos, tabs + 1);
                            WriteControlDraw(toolstr, pos, j, layout.controls[j], config, preText);
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
    for (int i = 0; i < layout.controlsCount; i++)
    {
        if (!draw[i])
        {
            if (layout.controls[i].type != GUI_DROPDOWNBOX)
            {
                draw[i] = true;
                WriteControlDraw(toolstr, pos, i, layout.controls[i], config, preText);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            }
        }
    }

    // Draw GUI_DROPDOWNBOX
    for (int i = 0; i < layout.controlsCount; i++)
    {
        if (!draw[i])
        {
            if (layout.controls[i].type == GUI_DROPDOWNBOX)
            {
                draw[i] = true;
                WriteControlDraw(toolstr, pos, i, layout.controls[i], config, preText);
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

	free(draw);
}

// Write control drawing code (individual controls) (.c/.h)
static void WriteControlDraw(unsigned char *toolstr, int *pos, int index, GuiControl control, GuiLayoutConfig config, const char *preText)
{
    char *rec = GetControlRectangleText(index, control, config.defineRecs, config.exportAnchors, preText);
    char *text = GetControlTextParam(control, config.defineTexts);
    char *name = GetControlNameParam(control.name, preText);

    // TODO: Define text for window, groupbox, buttons, toggles and dummyrecs
    switch (control.type)
    {
        case GUI_WINDOWBOX: TextAppend(toolstr, FormatText("%sActive = !GuiWindowBox(%s, %s);", name, rec, text), pos); break;
        case GUI_GROUPBOX: TextAppend(toolstr, FormatText("GuiGroupBox(%s, %s);", rec, text), pos); break;
        case GUI_LINE: TextAppend(toolstr, FormatText("GuiLine(%s, NULL);", rec), pos); break;
        case GUI_PANEL: TextAppend(toolstr, FormatText("GuiPanel(%s);", rec), pos); break;
        case GUI_LABEL: TextAppend(toolstr, FormatText("GuiLabel(%s, %s);", rec, text), pos); break;
        case GUI_BUTTON: TextAppend(toolstr, FormatText("%sPressed = GuiButton(%s, %s); ", name, rec, text), pos); break;
        case GUI_LABELBUTTON: TextAppend(toolstr, FormatText("%sPressed = GuiLabelButton(%s, %s);", name, rec, text), pos); break;
        case GUI_IMAGEBUTTONEX: TextAppend(toolstr, FormatText("%sPressed = GuiImageButtonEx(%s, GetTextureDefault(), (Rectangle){ 0, 0, 1, 1 }, %s);", name, rec, text), pos); break;
        case GUI_CHECKBOX: TextAppend(toolstr, FormatText("%sChecked = GuiCheckBox(%s, %s, %sChecked);", name, rec, text, name), pos); break;
        case GUI_TOGGLE: TextAppend(toolstr, FormatText("%sActive = GuiToggle(%s, %s, %sActive);", name, rec, text, name), pos); break;
        case GUI_TOGGLEGROUP:TextAppend(toolstr, FormatText("%sActive = GuiToggleGroup(%s, %s, %sActive);", name, rec, text, name), pos); break;
        case GUI_COMBOBOX: TextAppend(toolstr, FormatText("%sActive = GuiComboBox(%s, %s, %sActive);", name, rec, text, name), pos); break;
        case GUI_DROPDOWNBOX: TextAppend(toolstr, FormatText("if (GuiDropdownBox(%s, %s, &%sActive, %sEditMode)) %sEditMode = !%sEditMode;", rec, text, name, name, name, name), pos); break;
        case GUI_TEXTBOX: TextAppend(toolstr, FormatText("if (GuiTextBox(%s, %sText, %i, %sEditMode)) %sEditMode = !%sEditMode;", rec, name, MAX_CONTROL_TEXT_LENGTH, name, name, name), pos); break;
        case GUI_TEXTBOXMULTI: TextAppend(toolstr, FormatText("if (GuiTextBoxMulti(%s, %sText, %i, %sEditMode)) %sEditMode = !%sEditMode;", rec, name, MAX_CONTROL_TEXT_LENGTH, name, name, name), pos); break;
        case GUI_VALUEBOX: TextAppend(toolstr, FormatText("if (GuiValueBox(%s, &%sValue, 0, 100, %sEditMode)) %sEditMode = !%sEditMode;", rec, name, name, name, name), pos); break;
        case GUI_SPINNER: TextAppend(toolstr, FormatText("if (GuiSpinner(%s, &%sValue, 0, 100, 25, %sEditMode)) %sEditMode = !%sEditMode;", rec, name, name, name, name), pos); break;
        case GUI_SLIDER: TextAppend(toolstr, FormatText("%sValue = GuiSlider(%s, %s, %sValue, 0, 100, true);", name, rec, text, name), pos); break;
        case GUI_SLIDERBAR: TextAppend(toolstr, FormatText("%sValue = GuiSliderBar(%s, %s, %sValue, 0, 100, true);", name, rec, text, name), pos); break;
        case GUI_PROGRESSBAR: TextAppend(toolstr, FormatText("%sValue = GuiProgressBar(%s, %s, %sValue, 0, 1, true);", name, rec, text, name), pos); break;
        case GUI_STATUSBAR: TextAppend(toolstr, FormatText("GuiStatusBar(%s, %s);", rec, text), pos); break;
        case GUI_SCROLLPANEL: 
        {
            char *containerRec = GetScrollPanelContainerRecText(index, control, config.defineRecs, config.exportAnchors, preText);
            TextAppend(toolstr, FormatText("%sScrollOffset = GuiScrollPanel(%s, %s, %sScrollOffset);", name, containerRec, rec, name), pos); break;
        }
        case GUI_LISTVIEW: TextAppend(toolstr, FormatText("if (GuiListView(%s, %s, &%sActive, &%sScrollIndex, %sEditMode)) %sEditMode = !%sEditMode;", rec, text, name, name, name, name, name), pos); break;
        case GUI_COLORPICKER: TextAppend(toolstr, FormatText("%sValue = GuiColorPicker(%s, %sValue);", name, rec, name), pos); break;
        case GUI_DUMMYREC: TextAppend(toolstr, FormatText("GuiDummyRec(%s, %s);", rec, text), pos); break;
        default: break;
    }
}

// Get controls rectangle text
static char *GetControlRectangleText(int index, GuiControl control, bool defineRecs, bool exportAnchors, const char *preText)
{
    static char text[512];
    memset(text, 0, 512);

    if (defineRecs) 
    {
        strcpy(text, FormatText("%slayoutRecs[%i]", preText, index));
    }
    else
    {
        if (exportAnchors && control.ap->id > 0)
        {
            strcpy(text, FormatText("(Rectangle){ %s%s.x + %i, %s%s.y + %i, %i, %i }", preText, control.ap->name, (int)control.rec.x, preText, control.ap->name, (int)control.rec.y, (int)control.rec.width, (int)control.rec.height));
        }
        else
        {
            if (control.ap->id > 0) strcpy(text, FormatText("(Rectangle){ %i, %i, %i, %i }", (int)control.rec.x + control.ap->x, (int)control.rec.y + control.ap->y, (int)control.rec.width, (int)control.rec.height));
            else strcpy(text, FormatText("(Rectangle){ %i, %i, %i, %i }", (int)control.rec.x - control.ap->x, (int)control.rec.y - control.ap->y, (int)control.rec.width, (int)control.rec.height));
        }
    }

    return text;
}
static char *GetScrollPanelContainerRecText(int index, GuiControl control, bool defineRecs, bool exportAnchors, const char *preText)
{
    static char text[512];
    memset(text, 0, 512);
    
    if(defineRecs)
    {
        strcpy(text, FormatText("(Rectangle){%slayoutRecs[%i].x, %slayoutRecs[%i].y, %slayoutRecs[%i].width - %s%sBoundsOffset.x, %slayoutRecs[%i].height - %s%sBoundsOffset.y }", preText, index, preText, index, preText, index, preText, control.name, preText, index, preText, control.name));
    }
    else
    {
        if (exportAnchors && control.ap->id > 0)
        {
            strcpy(text, FormatText("(Rectangle){ %s%s.x + %i, %s%s.y + %i, %i - %s%sBoundsOffset.x, %i - %s%sBoundsOffset.y }", preText, control.ap->name, (int)control.rec.x, preText, control.ap->name, (int)control.rec.y, (int)control.rec.width, preText, control.name, (int)control.rec.height, preText, control.name));
        }
        else
        {
            // DOING
            if (control.ap->id > 0) strcpy(text, FormatText("(Rectangle){ %i, %i, %i - %s%sBoundsOffset.x, %i - %s%sBoundsOffset.y }", (int)control.rec.x + control.ap->x, (int)control.rec.y + control.ap->y, (int)control.rec.width, preText, control.name, (int)control.rec.height, preText, control.name));
            else strcpy(text, FormatText("(Rectangle){ %i, %i, %i - %s%sBoundsOffset.x, %i - %s%sBoundsOffset.y}", (int)control.rec.x - control.ap->x, (int)control.rec.y - control.ap->y, (int)control.rec.width, preText, control.name, (int)control.rec.height, preText, control.name));

        }
    }
}

// Get controls parameters text
static char *GetControlTextParam(GuiControl control, bool defineText)
{
    static char text[512];
    memset(text, 0, 512);
    
    if (defineText) strcpy(text, FormatText("%sText", control.name));
    else strcpy(text, FormatText("\"%s\"", control.text));    

    return text;
}

// Get controls name text
static char *GetControlNameParam(char *controlName, const char *preText)
{
    static char text[256];
    memset(text, 0, 256);

    strcpy(text, FormatText("%s%s", preText, controlName));

    return text;
}

#endif // CODEGEN_IMPLEMENTATION