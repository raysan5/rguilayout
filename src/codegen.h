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
unsigned char *GenerateLayoutCode(unsigned char *buffer, GuiLayout layout, GuiLayoutConfig config);

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
static void WriteRectangleVariables(unsigned char *toolstr, int *pos, GuiControl control, bool exportAnchors, bool fullComments, int tabs);
static void WriteAnchors(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, bool define, bool initialize, const char* preText, int tabs);
static void WriteConstText(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs);
static void WriteControlsVariables(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, bool define, bool initialize, const char *preText, int tabs);
static void WriteControlsDrawing(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, const char *preText, int tabs);
static void WriteControlDraw(unsigned char *toolstr, int *pos, int index, GuiControl control, GuiLayoutConfig config, const char *preText);

// Get controls specific texts functions
static char *GetControlRectangleText(int index, GuiControl control, bool defineRecs, bool exportAnchors, bool exportH);
static char *GetControlTextParam(GuiControl control, bool defineText);
static char *GetControlNameParam(char *controlName, const char *preText);

// Append one string at last position of a bigger string,
// i.e. use string as a file to add keep adding other strings...
static void sappend(char *str, int *pos, const char *buffer)
{
    strcpy(str + *pos, buffer);
    *pos += strlen(buffer);
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Generate layout code string
unsigned char *GenerateLayoutCode(unsigned char *buffer, GuiLayout layout, GuiLayoutConfig config)
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

                    if (TextIsEqual(substr, "GUILAYOUT_NAME")) sappend(toolstr, &codePos, config.name);
                    else if (TextIsEqual(substr, "GUILAYOUT_NAME_UPPERCASE")) sappend(toolstr, &codePos, TextToUpper(config.name));
                    else if (TextIsEqual(substr, "GUILAYOUT_NAME_LOWERCASE")) sappend(toolstr, &codePos, TextToLower(config.name));
                    else if (TextIsEqual(substr, "GUILAYOUT_VERSION")) sappend(toolstr, &codePos, config.version);
                    else if (TextIsEqual(substr, "GUILAYOUT_DESCRIPTION")) sappend(toolstr, &codePos, config.description);
                    else if (TextIsEqual(substr, "GUILAYOUT_COMPANY")) sappend(toolstr, &codePos, config.company);
                    else if (TextIsEqual(substr, "GUILAYOUT_WINDOW_WIDTH"))
                    {
                       if (layout.refWindow.width > 0) sappend(toolstr, &codePos, FormatText("%i", (int)layout.refWindow.width));
                       else sappend(toolstr, &codePos, FormatText("%i", 800));
                    }
                    else if (TextIsEqual(substr, "GUILAYOUT_WINDOW_HEIGHT"))
                    {
                       if (layout.refWindow.height > 0) sappend(toolstr, &codePos, FormatText("%i", (int)layout.refWindow.height));
                       else sappend(toolstr, &codePos, FormatText("%i", 450));
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
            sappend(toolstr, pos, FormatText("static void %s();", layout.controls[i].name));
            if (config.fullComments)
            {
                TABAPPEND(toolstr, pos, 4);
                sappend(toolstr, pos, FormatText("// %s: %s logic", controlTypeName[layout.controls[i].type], layout.controls[i].name));
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
            sappend(toolstr, pos, "// Define controls rectangles");
            ENDLINEAPPEND(toolstr, pos);
            TABAPPEND(toolstr, pos, tabs);
        }
        sappend(toolstr, pos, FormatText("Rectangle layoutRecs[%i] = {", layout.controlsCount));
        ENDLINEAPPEND(toolstr, pos);

        for (int k = 0; k < layout.controlsCount; k++)
        {
            TABAPPEND(toolstr, pos, tabs+1);
            WriteRectangleVariables(toolstr, pos, layout.controls[k], config.exportAnchors, config.fullComments, tabs);
            ENDLINEAPPEND(toolstr, pos);
        }
        TABAPPEND(toolstr, pos, tabs);
        sappend(toolstr, pos, "};");
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
                sappend(toolstr, pos, FormatText("// %s: %s logic", controlTypeName[layout.controls[i].type], layout.controls[i].name));
                ENDLINEAPPEND(toolstr, pos);
                TABAPPEND(toolstr, pos, tabs);
            }

            sappend(toolstr, pos, FormatText("static void %s()", layout.controls[i].name));
            ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            sappend(toolstr, pos, "{");
            ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
            sappend(toolstr, pos, "// TODO: Implement control logic");
            ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            sappend(toolstr, pos, "}");
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
    sappend(toolstr, pos, "typedef struct {");
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs+1);

    // Write anchors variables
    if (config.exportAnchors && layout.anchorsCount > 1) WriteAnchors(toolstr, pos, layout, config, true, false, "", tabs+1);

    // Write controls variables
    if (layout.controlsCount > 0) WriteControlsVariables(toolstr, pos, layout, config, true, false, "", tabs+1);

    // Export rectangles
    if (config.defineRecs)
    {
        ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs+1);

        // Write rectangles
        if (config.fullComments)
        {
            sappend(toolstr, pos, "// Define rectangles");
            ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs+1);
        }

        sappend(toolstr, pos, FormatText("Rectangle layoutRecs[%i];", layout.controlsCount));
    }

    ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs+1);
    sappend(toolstr, pos, "// Custom state variables (depend on development software)");
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs+1);
    sappend(toolstr, pos, "// NOTE: This variables should be added manually if required");

    ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    sappend(toolstr, pos, FormatText("} Gui%sState;", TextToPascal(config.name)));
}

// Write variables declaration code (.h)
static void WriteFunctionsDeclarationH(unsigned char *toolstr, int *pos, GuiLayoutConfig config, int tabs)
{
    sappend(toolstr, pos, FormatText("Gui%sState InitGui%s(void);", TextToPascal(config.name), TextToPascal(config.name)));
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    sappend(toolstr, pos, FormatText("void Gui%s(Gui%sState *state);", TextToPascal(config.name), TextToPascal(config.name)));
}

// Write initialization function code (.h)
static void WriteFunctionInitializeH(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs)
{
     // Export InitGuiLayout function definition
    sappend(toolstr, pos, FormatText("Gui%sState InitGui%s(void)", TextToPascal(config.name), TextToPascal(config.name)));
    ENDLINEAPPEND(toolstr, pos);
    sappend(toolstr, pos, "{"); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
    sappend(toolstr, pos, FormatText("Gui%sState state = { 0 };", TextToPascal(config.name)));
    ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);

    // Init anchors
    if (config.exportAnchors && layout.anchorsCount > 1)
    {
        WriteAnchors(toolstr, pos, layout, config, false, true, "state.", tabs+1);
    }

    // Init controls variables
    if (layout.controlsCount > 0)
    {
        WriteControlsVariables(toolstr, pos, layout, config, false, true, "state.", tabs+1);
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
            sappend(toolstr, pos, "// Init controls rectangles");
            ENDLINEAPPEND(toolstr, pos);
            TABAPPEND(toolstr, pos, tabs + 1);
        }

        for (int k = 0; k < layout.controlsCount; k++)
        {
            sappend(toolstr, pos, FormatText("state.layoutRecs[%i] = ", k));
            WriteRectangleVariables(toolstr, pos, layout.controls[k], config.exportAnchors, config.fullComments, tabs);
            ENDLINEAPPEND(toolstr, pos);
            TABAPPEND(toolstr, pos, tabs + 1);
        }

        *pos -= (tabs + 1)*4 + 1;
    }

    ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs+1);
    sappend(toolstr, pos, "// Custom variables initialization");
    ENDLINEAPPEND(toolstr, pos);

    // Return gui state after defining all its variables
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs+1);
    sappend(toolstr, pos, "return state;");

    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    sappend(toolstr, pos, "}");

}

// Write functions drawing code (.h)
static void WriteFunctionDrawingH(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs)
{
    // Export GuiLayout draw function
    sappend(toolstr, pos, FormatText("void Gui%s(Gui%sState *state)", TextToPascal(config.name), TextToPascal(config.name)));
    ENDLINEAPPEND(toolstr, pos);
    sappend(toolstr, pos, "{"); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);

    // Const text
    if (config.defineTexts) WriteConstText(toolstr, pos, layout, config, tabs + 1);

    // Controls draw
    if (layout.controlsCount > 0) WriteControlsDrawing(toolstr, pos, layout, config, "state->", tabs + 1);

    ENDLINEAPPEND(toolstr, pos);
    sappend(toolstr, pos, "}");
}

//----------------------------------------------------------------------------------
// Generic writting code functions (.c/.h)
//----------------------------------------------------------------------------------

// Write rectangle variables code (.c/.h)
static void WriteRectangleVariables(unsigned char *toolstr, int *pos, GuiControl control, bool exportAnchors, bool fullComments, int tabs)
{
    //sappend(toolstr, pos, FormatText("bool %sActive = true;", control.name));
    if (exportAnchors && control.ap->id > 0)
    {
        sappend(toolstr, pos, FormatText("(Rectangle){ %s.x + %i, %s.y + %i, %i, %i };", control.ap->name, (int)control.rec.x, control.ap->name, (int)control.rec.y, (int)control.rec.width, (int)control.rec.height));
    }
    else
    {
        if (control.ap->id > 0) sappend(toolstr, pos, FormatText("(Rectangle){ %i, %i, %i, %i };", (int)control.rec.x + control.ap->x, (int)control.rec.y + control.ap->y, (int)control.rec.width, (int)control.rec.height));
        else sappend(toolstr, pos, FormatText("(Rectangle){ %i, %i, %i, %i };", (int)control.rec.x - control.ap->x, (int)control.rec.y - control.ap->y, (int)control.rec.width, (int)control.rec.height));
    }
    if (fullComments)
    {
        TABAPPEND(toolstr, pos, tabs);
        sappend(toolstr, pos, FormatText("// %s: %s",controlTypeName[control.type], control.name));
    }
}

// Write anchors code (.c/.h)
static void WriteAnchors(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, bool define, bool initialize, const char *preText, int tabs)
{
    if (config.fullComments)
    {
        if (define) sappend(toolstr, pos, "// Define anchors");
        else if (initialize) sappend(toolstr, pos, "// Init anchors");
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    }

    for (int i = 0; i < MAX_ANCHOR_POINTS; i++)
    {
        GuiAnchorPoint anchor = layout.anchors[i];

        if (anchor.enabled)
        {
            if (define) sappend(toolstr, pos, "Vector2 ");
            else sappend(toolstr, pos, FormatText("%s", preText));
            sappend(toolstr, pos, FormatText("%s", anchor.name));
            if (initialize) sappend(toolstr, pos, FormatText(" = { %i, %i }", (int)layout.anchors[i].x, (int)layout.anchors[i].y));
            sappend(toolstr, pos, ";");

            if (config.fullComments)
            {
                TABAPPEND(toolstr, pos, 3);
                sappend(toolstr, pos, FormatText("// ANCHOR ID:%i", anchor.id));
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
        sappend(toolstr, pos, "// Const text");
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
                sappend(toolstr, pos, FormatText("const char *%sText = \"%s\";", layout.controls[i].name, layout.controls[i].text));
                if (config.fullComments)
                {
                    TABAPPEND(toolstr, pos, 1);
                    sappend(toolstr, pos, FormatText("// %s: %s", TextToUpper(controlTypeName[layout.controls[i].type]), layout.controls[i].name));
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
        if (define) sappend(toolstr, pos, "// Define controls variables");
        else if (initialize) sappend(toolstr, pos, "// Initilize controls variables");
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
                if (define) sappend(toolstr, pos, "bool ");
                else sappend(toolstr, pos, FormatText("%s", preText));
                sappend(toolstr, pos, FormatText("%sActive", control.name));
                if (initialize) sappend(toolstr, pos, " = true");
                sappend(toolstr, pos, ";");
            } break;
            case GUI_BUTTON:
            case GUI_LABELBUTTON:
            case GUI_IMAGEBUTTONEX:
            {
                if (define) sappend(toolstr, pos, "bool ");
                else sappend(toolstr, pos, FormatText("%s", preText));
                sappend(toolstr, pos, FormatText("%sPressed", control.name));
                if (initialize) sappend(toolstr, pos, " = false");
                sappend(toolstr, pos, ";");
            } break;
            case GUI_CHECKBOX:
            {
                if (define) sappend(toolstr, pos, "bool ");
                else sappend(toolstr, pos, FormatText("%s", preText));
                sappend(toolstr, pos, FormatText("%sChecked", control.name));
                if (initialize) sappend(toolstr, pos, " = false");
                sappend(toolstr, pos, ";");
            } break;
            case GUI_TOGGLE:
            {
                if (define) sappend(toolstr, pos, "bool ");
                else sappend(toolstr, pos, FormatText("%s", preText));
                sappend(toolstr, pos, FormatText("%sActive", control.name));
                if (initialize) sappend(toolstr, pos, " = true");
                sappend(toolstr, pos, ";");
            } break;
            case GUI_TOGGLEGROUP:
            case GUI_COMBOBOX:
            {
                if (define) sappend(toolstr, pos, "int ");
                else sappend(toolstr, pos, FormatText("%s", preText));
                sappend(toolstr, pos, FormatText("%sActive", control.name));
                if (initialize) sappend(toolstr, pos, "= 0");
                sappend(toolstr, pos, ";");
            } break;
            case GUI_LISTVIEW:
            {
                if (define) sappend(toolstr, pos, "int ");
                else sappend(toolstr, pos, FormatText("%s", preText));
                sappend(toolstr, pos, FormatText("%sScrollIndex", control.name));
                if (initialize) sappend(toolstr, pos, " = 0");
                sappend(toolstr, pos, ";");
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            }
            case GUI_DROPDOWNBOX:
            {
                if (define) sappend(toolstr, pos, "bool ");
                else sappend(toolstr, pos, FormatText("%s", preText));
                sappend(toolstr, pos, FormatText("%sEditMode", control.name));
                if (initialize) sappend(toolstr, pos, " = false");
                sappend(toolstr, pos, ";");
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);

                if (define) sappend(toolstr, pos, "int ");
                else sappend(toolstr, pos, FormatText("%s", preText));
                sappend(toolstr, pos, FormatText("%sActive", control.name));
                if (initialize) sappend(toolstr, pos, " = 0");
                sappend(toolstr, pos, ";");
            } break;
            case GUI_TEXTBOX:
            case GUI_TEXTBOXMULTI:
            {
                if (define) sappend(toolstr, pos, "bool ");
                else sappend(toolstr, pos, FormatText("%s", preText));
                sappend(toolstr, pos, FormatText("%sEditMode", control.name));
                if (initialize) sappend(toolstr, pos, " = false");
                sappend(toolstr, pos, ";");
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);

                if (define)
                {
                    sappend(toolstr, pos, FormatText("unsigned char %sText[%i]", control.name, MAX_CONTROL_TEXT_LENGTH));
                    if (initialize) sappend(toolstr, pos, FormatText(" = \"%s\"", control.text));
                }
                else if (initialize) sappend(toolstr, pos, FormatText("strcpy(%s%sText, \"%s\")", preText, control.name, control.text));
                sappend(toolstr, pos, ";");

            } break;
            case GUI_VALUEBOX:
            case GUI_SPINNER:
            {
                if (define) sappend(toolstr, pos, "bool ");
                else sappend(toolstr, pos, FormatText("%s", preText));
                sappend(toolstr, pos, FormatText("%sEditMode", control.name));
                if (initialize) sappend(toolstr, pos, " = false");
                sappend(toolstr, pos, ";");
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
                if (define) sappend(toolstr, pos, "int ");
                else sappend(toolstr, pos, FormatText("%s", preText));
                sappend(toolstr, pos, FormatText("%sValue", control.name));
                if (initialize) sappend(toolstr, pos, " = 0");
                sappend(toolstr, pos, ";");

            } break;
            case GUI_SLIDER:
            case GUI_SLIDERBAR:
            case GUI_PROGRESSBAR:
            {
                if (define) sappend(toolstr, pos, "float ");
                else sappend(toolstr, pos, FormatText("%s", preText));
                sappend(toolstr, pos, FormatText("%sValue", control.name));
                if (initialize) sappend(toolstr, pos, " = 0.0f");
                sappend(toolstr, pos, ";");
            } break;
            case GUI_COLORPICKER:
            {
                if (define) sappend(toolstr, pos, "Color ");
                else sappend(toolstr, pos, FormatText("%s", preText));
                sappend(toolstr, pos, FormatText("%sValue", control.name));
                // TODO: if (initialize) sappend(toolstr, pos, FormatText(" = { %i, %i }", (int)layout.anchors[i].x, (int)layout.anchors[i].y));
                sappend(toolstr, pos, ";");
            } break;
            case GUI_GROUPBOX:
            case GUI_LINE:
            case GUI_PANEL:
            case GUI_LABEL:
            case GUI_DUMMYREC:
            case GUI_STATUSBAR:
            case GUI_SCROLLPANEL:
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
                sappend(toolstr, pos, FormatText("// %s: %s", controlTypeName[layout.controls[i].type], layout.controls[i].name));
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
        sappend(toolstr, pos, "// Draw controls");
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    }

    bool draw[layout.controlsCount];
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
                sappend(toolstr, pos, "if (");
            }
            else
            {
                sappend(toolstr, pos, " || ");
            }
            sappend(toolstr, pos, FormatText("%sEditMode", GetControlNameParam(layout.controls[i].name, preText)));
        }
    }

    if (dropDownExist)
    {
        sappend(toolstr, pos, ") GuiLock();");
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

                char *rec = GetControlRectangleText(i, layout.controls[i], config.defineRecs, config.exportAnchors, true);

                sappend(toolstr, pos, FormatText("if (%sActive)", GetControlNameParam(layout.controls[i].name, preText)));
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
                sappend(toolstr, pos, "{");

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
                sappend(toolstr, pos, "}");
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
        sappend(toolstr, pos, "GuiUnlock();");
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    }

    *pos -= (tabs)*4 + 1; // Delete last tabs and \n
}

// Write control drawing code (individual controls) (.c/.h)
static void WriteControlDraw(unsigned char *toolstr, int *pos, int index, GuiControl control, GuiLayoutConfig config, const char *preText)
{
    char *rec = GetControlRectangleText(index, control, config.defineRecs, config.exportAnchors, true);
    char *text = GetControlTextParam(control, config.defineTexts);
    char *name = GetControlNameParam(control.name, preText);

    // TODO: Define text for window, groupbox, buttons, toggles and dummyrecs
    switch (control.type)
    {
        case GUI_WINDOWBOX: sappend(toolstr, pos, FormatText("%sActive = !GuiWindowBox(%s, %s);", name, rec, text)); break;
        case GUI_GROUPBOX: sappend(toolstr, pos, FormatText("GuiGroupBox(%s, %s);", rec, control.text, text)); break;
        case GUI_LINE: sappend(toolstr, pos, FormatText("GuiLine(%s, 1);", rec)); break;
        case GUI_PANEL: sappend(toolstr, pos, FormatText("GuiPanel(%s);", rec)); break;
        case GUI_LABEL: sappend(toolstr, pos, FormatText("GuiLabel(%s, %s);", rec, text)); break;
        case GUI_BUTTON: sappend(toolstr, pos, FormatText("%sPressed = GuiButton(%s, %s); ", name, rec, text)); break;
        case GUI_LABELBUTTON: sappend(toolstr, pos, FormatText("%sPressed = GuiLabelButton(%s, %s);", name, rec, text)); break;
        case GUI_IMAGEBUTTONEX: sappend(toolstr, pos, FormatText("%sPressed = GuiImageButtonEx(%s, GetTextureDefault(), (Rectangle){ 0, 0, 1, 1 }, %s);", name, rec, text)); break;
        case GUI_CHECKBOX: sappend(toolstr, pos, FormatText("%sChecked = GuiCheckBox(%s, %s, %sChecked);", name, rec, text, name)); break;
        case GUI_TOGGLE: sappend(toolstr, pos, FormatText("%sActive = GuiToggle(%s, %s, %sActive);", name, rec, text, name)); break;
        case GUI_TOGGLEGROUP:sappend(toolstr, pos, FormatText("%sActive = GuiToggleGroup(%s, %s, %sActive);", name, rec, text, name)); break;
        case GUI_COMBOBOX: sappend(toolstr, pos, FormatText("%sActive = GuiComboBox(%s, %s, %sActive);", name, rec, text, name)); break;
        case GUI_DROPDOWNBOX: sappend(toolstr, pos, FormatText("if (GuiDropdownBox(%s, %s, &%sActive, %sEditMode)) %sEditMode = !%sEditMode;", rec, text, name, name, name, name)); break;
        case GUI_TEXTBOX: sappend(toolstr, pos, FormatText("if (GuiTextBox(%s, %sText, %i, %sEditMode)) %sEditMode = !%sEditMode;", rec, name, MAX_CONTROL_TEXT_LENGTH, name, name, name)); break;
        case GUI_TEXTBOXMULTI: sappend(toolstr, pos, FormatText("if (GuiTextBoxMulti(%s, %sText, %i, %sEditMode)) %sEditMode = !%sEditMode;", rec, name, MAX_CONTROL_TEXT_LENGTH, name, name, name)); break;
        case GUI_VALUEBOX: sappend(toolstr, pos, FormatText("if (GuiValueBox(%s, &%sValue, 0, 100, %sEditMode)) %sEditMode = !%sEditMode;", rec, name, name, name, name)); break;
        case GUI_SPINNER: sappend(toolstr, pos, FormatText("if (GuiSpinner(%s, &%sValue, 0, 100, 25, %sEditMode)) %sEditMode = !%sEditMode;", rec, name, name, name, name)); break;
        case GUI_SLIDER: sappend(toolstr, pos, FormatText("%sValue = GuiSlider(%s, %s, %sValue, 0, 100, true);", name, rec, text, name)); break;
        case GUI_SLIDERBAR: sappend(toolstr, pos, FormatText("%sValue = GuiSliderBar(%s, %s, %sValue, 0, 100, true);", name, rec, text, name)); break;
        case GUI_PROGRESSBAR: sappend(toolstr, pos, FormatText("%sValue = GuiProgressBar(%s, %s, %sValue, 0, 1, true);", name, rec, text, name)); break;
        case GUI_STATUSBAR: sappend(toolstr, pos, FormatText("GuiStatusBar(%s, %s, 15);", rec, text)); break;
        case GUI_SCROLLPANEL: sappend(toolstr, pos, FormatText("%ScrollOffset = GuiScrollPanel(%s, %s, %ScrollOffset)", name, rec, rec, name)); break;
        case GUI_LISTVIEW: sappend(toolstr, pos, FormatText("if (GuiListView(%s, %s, &%sActive, &%sScrollIndex, %sEditMode)) %sEditMode = !%sEditMode;", rec, text, name, name, name, name, name)); break;
        case GUI_COLORPICKER: sappend(toolstr, pos, FormatText("%sValue = GuiColorPicker(%s, %sValue);", name, rec, name)); break;
        case GUI_DUMMYREC: sappend(toolstr, pos, FormatText("GuiDummyRec(%s, %s);", rec, text)); break;
        default: break;
    }
}

// Get controls rectangle text
static char *GetControlRectangleText(int index, GuiControl control, bool defineRecs, bool exportAnchors, bool exportH)
{
    static char text[512];
    memset(text, 0, 512);

    if (defineRecs)
    {
        if (exportH) strcpy(text, FormatText("state->layoutRecs[%i]", index));
        else strcpy(text, FormatText("layoutRecs[%i]", index));
    }
    else
    {
        if (exportAnchors && control.ap->id > 0)
        {
            strcpy(text, FormatText("(Rectangle){ %s.x + %i, %s.y + %i, %i, %i }", control.ap->name, (int)control.rec.x, control.ap->name, (int)control.rec.y, (int)control.rec.width, (int)control.rec.height));
        }
        else
        {
            if (control.ap->id > 0) strcpy(text, FormatText("(Rectangle){ %i, %i, %i, %i }", (int)control.rec.x + control.ap->x, (int)control.rec.y + control.ap->y, (int)control.rec.width, (int)control.rec.height));
            else strcpy(text, FormatText("(Rectangle){ %i, %i, %i, %i }", (int)control.rec.x - control.ap->x, (int)control.rec.y - control.ap->y, (int)control.rec.width, (int)control.rec.height));
        }
    }

    return text;
}

// Get controls parameters text
static char *GetControlTextParam(GuiControl control, bool defineText)
{
    static char text[512];
    memset(text, 0, 512);

    switch (control.type)
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
            if (defineText) strcpy(text, FormatText("%sText", control.name));
            else strcpy(text, FormatText("\"%s\"", control.text));
        break;
        default: break;
    }

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