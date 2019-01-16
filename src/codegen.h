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

static char *GetControlRectangleText(int index, GuiControl control, bool defineRecs, bool exportAnchors, bool exportH);    // Get control rectangle text
static char *GetControlParamText(int controlType, char *name, GuiLayoutConfig config);                          // Get control func parameters text

// .C export functions
static void WriteControlsVariables(unsigned char *toolstr, int *pos, GuiControl control, bool fullVariables,int tabs);         // Write controls variables code to file
static void WriteControlsDrawing(unsigned char *toolstr, int *pos, int index, GuiControl control, GuiLayoutConfig config); // Write controls drawing code to file

// .H export functions
static void WriteStruct(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs);
static void WriteFunctionsDeclarationH(unsigned char *toolstr, int *pos, GuiLayoutConfig config, int tabs);
static void WriteFunctionInitializeH(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs);
static void WriteFunctionDrawingH(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs);

// Generic code generation (.c/.h)
static void WriteRectangleVariables(unsigned char *toolstr, int *pos, GuiControl control, bool exportAnchors, bool fullComments, int tabs); // Write rectangle variables.
static void WriteAnchors(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, bool define, bool initialize, const char* preText, int tabs);
static void WriteConstText(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs);
static void WriteControlVariablesH(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, bool define, bool initialize, const char *preText, int tabs);
static void WriteControlDrawingH(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config const char *preText, int tabs);
static void WriteControlsDrawingHOLD(unsigned char *toolstr, int *pos, int index, GuiControl control, GuiLayoutConfig config);

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
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTION_DECLARATION_C"))
                    {
                        // Define required functions for calling
                        int buttonsCount = 0;
                        for (int i = 0; i < layout.controlsCount; i++)
                        {
                            if (layout.controls[i].type == GUI_BUTTON)
                            {
                                buttonsCount++;
                                sappend(toolstr, &codePos, FormatText("static void %s();", layout.controls[i].name));
                                if (config.fullComments)
                                {
                                    TABAPPEND(toolstr, &codePos, 4);
                                    sappend(toolstr, &codePos, FormatText("// %s: %s logic", controlTypeName[layout.controls[i].type], layout.controls[i].name));
                                }
                                ENDLINEAPPEND(toolstr, &codePos);
                            }
                        }
                        if (buttonsCount > 0) codePos--;
                    }
                    else if (TextIsEqual(substr, "GUILAYOUT_INITIALIZATION_C"))
                    {
                        // Anchors points export code
                        if (config.exportAnchors && layout.anchorsCount > 1)
                        {
                            for (int k = 1; k < MAX_ANCHOR_POINTS; k++)
                            {
                                if (layout.anchors[k].enabled)
                                {
                                    sappend(toolstr, &codePos, FormatText("Vector2 %s = { %i, %i };", layout.anchors[k].name, (int)layout.anchors[k].x, (int)layout.anchors[k].y));
                                    if (config.fullComments)
                                    {
                                        TABAPPEND(toolstr, &codePos, 1);
                                        sappend(toolstr, &codePos, FormatText("// ID:%i Name:%s", layout.anchors[k].id, layout.anchors[k].name));
                                    }
                                    ENDLINEAPPEND(toolstr, &codePos);
                                    TABAPPEND(toolstr, &codePos, tabs);
                                }
                            }
                            ENDLINEAPPEND(toolstr, &codePos);
                            TABAPPEND(toolstr, &codePos, tabs);
                        }

                        // Generate controls required variables code
                        if (layout.controlsCount > 0)
                        {
                            for (int k = 0; k < layout.controlsCount; k++)
                            {
                                int type = layout.controls[k].type;
                                if (type == GUI_LABEL)
                                {
                                    // TODO:  window, groupbox, buttons, toggles and dummyrecs
                                    if (config.defineTexts)
                                    {
                                        sappend(toolstr, &codePos, FormatText("const char *%sText = \"%s\";", layout.controls[k].name, layout.controls[k].text));
                                        ENDLINEAPPEND(toolstr, &codePos);
                                        TABAPPEND(toolstr, &codePos, tabs);
                                    }
                                }
                                else if (type != GUI_GROUPBOX && type != GUI_LINE && type != GUI_PANEL && type != GUI_BUTTON && type != GUI_DUMMYREC)
                                {
                                    WriteControlsVariables(toolstr, &codePos, layout.controls[k], false, tabs);
                                    if (config.fullComments)
                                    {
                                        TABAPPEND(toolstr, &codePos, 1);
                                        sappend(toolstr, &codePos, FormatText("// %s: %s", controlTypeName[layout.controls[k].type], layout.controls[k].name));
                                    }
                                    ENDLINEAPPEND(toolstr, &codePos);
                                    TABAPPEND(toolstr, &codePos, tabs);
                                }
                            }
                            codePos -= tabs*4 + 1; // remove last \n\t
                        }

                        if (config.defineRecs)
                        {
                            ENDLINEAPPEND(toolstr, &codePos);
                            TABAPPEND(toolstr, &codePos, tabs);
                            // Define controls rectangles
                            if (config.fullComments)
                            {
                                sappend(toolstr, &codePos, "// Define controls rectangles");
                                ENDLINEAPPEND(toolstr, &codePos);
                                TABAPPEND(toolstr, &codePos, tabs);
                            }
                            sappend(toolstr, &codePos, FormatText("Rectangle layoutRecs[%i] = {", layout.controlsCount));
                            ENDLINEAPPEND(toolstr, &codePos);

                            for (int k = 0; k < layout.controlsCount; k++)
                            {
                                TABAPPEND(toolstr, &codePos, tabs+1);
                                WriteRectangleVariables(toolstr, &codePos, layout.controls[k], config.exportAnchors, config.fullComments, tabs);
                                ENDLINEAPPEND(toolstr, &codePos);
                            }
                            TABAPPEND(toolstr, &codePos, tabs);
                            sappend(toolstr, &codePos, "};");
                        }
                    }
                    else if (TextIsEqual(substr, "GUILAYOUT_DRAWING_C") && layout.controlsCount > 0)
                    {
                        bool draw[layout.controlsCount];
                        for (int k = 0; k < layout.controlsCount; k++) draw[k] = false;

                        // If dropdown control exist, draw GuiLock condition
                        bool dropDownExist = false;
                        for (int k = 0; k < layout.controlsCount; k++)
                        {
                            if (layout.controls[k].type == GUI_DROPDOWNBOX)
                            {
                                if (!dropDownExist)
                                {
                                    dropDownExist = true;
                                    sappend(toolstr, &codePos, "if (");
                                }
                                else
                                {
                                    sappend(toolstr, &codePos, " || ");
                                }
                                sappend(toolstr, &codePos, FormatText("%sEditMode", layout.controls[k].name));
                            }
                        }
                        if (dropDownExist)
                        {
                            sappend(toolstr, &codePos, ") GuiLock();");
                            ENDLINEAPPEND(toolstr, &codePos); TABAPPEND(toolstr, &codePos, tabs);
                        }

                        // Draw GUI_WINDOWBOX
                        for (int k = 0; k < layout.controlsCount; k++)
                        {
                            if (!draw[k])
                            {
                                if (layout.controls[k].type == GUI_WINDOWBOX)
                                {
                                    draw[k] = true;

                                    char *rec = GetControlRectangleText(k, layout.controls[k], config.defineRecs, config.exportAnchors, false);
                                    sappend(toolstr, &codePos, FormatText("if (%sActive)", layout.controls[k].name));
                                    ENDLINEAPPEND(toolstr, &codePos); TABAPPEND(toolstr, &codePos, tabs);
                                    sappend(toolstr, &codePos, "{");
                                    ENDLINEAPPEND(toolstr, &codePos); TABAPPEND(toolstr, &codePos, tabs + 1);
                                    sappend(toolstr, &codePos, FormatText("%sActive = !GuiWindowBox(%s, \"%s\");", layout.controls[k].name, rec, layout.controls[k].text));
                                    ENDLINEAPPEND(toolstr, &codePos);

                                    int windowAnchorID = layout.controls[k].ap->id;

                                    // Draw controls inside window
                                    for (int l = 0; l < layout.controlsCount; l++)
                                    {
                                        if (!draw[l] && k != l && layout.controls[l].type != GUI_WINDOWBOX  && layout.controls[l].type != GUI_DROPDOWNBOX)
                                        {
                                            if (windowAnchorID == layout.controls[l].ap->id)
                                            {
                                                draw[l] = true;

                                                TABAPPEND(toolstr, &codePos, tabs + 1);
                                                WriteControlsDrawing(toolstr, &codePos, l, layout.controls[l], config);
                                                ENDLINEAPPEND(toolstr, &codePos);
                                            }
                                        }
                                    }

                                    // Draw GUI_DROPDOWNBOX inside GUI_WINDOWBOX
                                    for (int l = 0; l < layout.controlsCount; l++)
                                    {
                                        if (!draw[l] && k != l && layout.controls[l].type == GUI_DROPDOWNBOX)
                                        {
                                            if (windowAnchorID == layout.controls[l].ap->id)
                                            {
                                                draw[l] = true;

                                                TABAPPEND(toolstr, &codePos, tabs + 1);
                                                WriteControlsDrawing(toolstr, &codePos, l, layout.controls[l], config);
                                                ENDLINEAPPEND(toolstr, &codePos);
                                            }
                                        }
                                    }
                                    TABAPPEND(toolstr, &codePos, tabs);
                                    sappend(toolstr, &codePos, "}");
                                    ENDLINEAPPEND(toolstr, &codePos); TABAPPEND(toolstr, &codePos, tabs);
                                }
                            }
                        }

                        // Draw the rest of controls
                        for (int k = 0; k < layout.controlsCount; k++)
                        {
                            if (!draw[k])
                            {
                                if (layout.controls[k].type != GUI_DROPDOWNBOX)
                                {
                                    draw[k] = true;
                                    WriteControlsDrawing(toolstr, &codePos, k, layout.controls[k], config);
                                    ENDLINEAPPEND(toolstr, &codePos); TABAPPEND(toolstr, &codePos, tabs);
                                }
                            }
                        }

                        // Draw GUI_DROPDOWNBOX
                        for (int k = 0; k < layout.controlsCount; k++)
                        {
                            if (!draw[k])
                            {
                                if (layout.controls[k].type == GUI_DROPDOWNBOX)
                                {
                                    draw[k] = true;
                                    WriteControlsDrawing(toolstr, &codePos, k, layout.controls[k], config);
                                    ENDLINEAPPEND(toolstr, &codePos); TABAPPEND(toolstr, &codePos, tabs);
                                }
                            }
                        }

                        if (dropDownExist)
                        {
                            sappend(toolstr, &codePos, "GuiUnlock();");
                            ENDLINEAPPEND(toolstr, &codePos); TABAPPEND(toolstr, &codePos, tabs);
                        }

                        codePos -= tabs*4 + 1; // Delete last tabs and \n
                    }
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTION_DEFINITION_C"))
                    {
                        for (int k = 0; k < layout.controlsCount; k++)
                        {
                            if (layout.controls[k].type == GUI_BUTTON)
                            {
                                if (config.fullComments)
                                {
                                    sappend(toolstr, &codePos, FormatText("// %s: %s logic", controlTypeName[layout.controls[k].type], layout.controls[k].name));
                                    ENDLINEAPPEND(toolstr, &codePos);
                                }
                                sappend(toolstr, &codePos, FormatText("static void %s()", layout.controls[k].name)); ENDLINEAPPEND(toolstr, &codePos);
                                sappend(toolstr, &codePos, "{"); ENDLINEAPPEND(toolstr, &codePos);
                                TABAPPEND(toolstr, &codePos, 1); sappend(toolstr, &codePos, "// TODO: Implement control logic"); ENDLINEAPPEND(toolstr, &codePos);
                                sappend(toolstr, &codePos, "}"); ENDLINEAPPEND(toolstr, &codePos);
                            }
                        }
                    }

                    // H IMPLEMENTATION
                    else if (TextIsEqual(substr, "GUILAYOUT_STRUCT_TYPE"))
                    {
                        WriteStruct(toolstr, &codePos, layout, config, tabs);
                    }
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTIONS_DECLARATION_H"))
                    {
                        WriteFunctionsDeclarationH(toolstr, &codePos, config, tabs);
                    }
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTION_INITIALIZE_H"))
                    {
                        WriteFunctionInitializeH(toolstr, &codePos, layout, config, tabs);
                    }
                    else if (TextIsEqual(substr, "GUILAYOUT_FUNCTION_DRAWING_H") && layout.controlsCount > 0)
                    {
                        WriteFunctionDrawingH(toolstr, &codePos, layout, config, tabs);
                    }

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
// Module Internal Functions Definition
//----------------------------------------------------------------------------------
static void WriteRectangleVariables(unsigned char *toolstr, int *pos, GuiControl control, bool exportAnchors, bool fullComments, int tabs)
{
    //sappend(toolstr, pos, FormatText("bool %sActive = true;", control.name));
    if (exportAnchors && control.ap->id > 0)
    {
        sappend(toolstr, pos, FormatText("(Rectangle){ %s.x + %i, %s.y + %i, %i, %i }", control.ap->name, (int)control.rec.x, control.ap->name, (int)control.rec.y, (int)control.rec.width, (int)control.rec.height));
    }
    else
    {
        if (control.ap->id > 0) sappend(toolstr, pos, FormatText("(Rectangle){ %i, %i, %i, %i }", (int)control.rec.x + control.ap->x, (int)control.rec.y + control.ap->y, (int)control.rec.width, (int)control.rec.height));
        else sappend(toolstr, pos, FormatText("(Rectangle){ %i, %i, %i, %i }", (int)control.rec.x - control.ap->x, (int)control.rec.y - control.ap->y, (int)control.rec.width, (int)control.rec.height));
    }
    if (fullComments)
    {
        TABAPPEND(toolstr, pos, tabs);
        sappend(toolstr, pos, FormatText("// %s: %s",controlTypeName[control.type], control.name));
    }
}
// Get control rectangle text (considering anchor or not)
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
// Get control func parameters text
static char *GetControlParamText(int controlType, char *name, GuiLayoutConfig config)
{
    static char text[512];
    memset(text, 0, 512);

    switch (controlType)
    {
        case GUI_SLIDEREX:
        case GUI_SLIDERBAREX: strcpy(text, "0, 100"); break;
        case GUI_DROPDOWNBOX:
        case GUI_COMBOBOX:
        case GUI_LISTVIEW:
        case GUI_TOGGLEGROUP: strcpy(text, "3"); break;
        case GUI_TEXTBOX: strcpy(text, FormatText("%i", MAX_CONTROL_TEXT_LENGTH)); break;
    }

    return text;
}

// Export C functions
static void WriteControlsVariables(unsigned char *toolstr, int *pos, GuiControl control, bool fullVariables, int tabs)
{
    switch (control.type)
    {
        case GUI_WINDOWBOX:
        case GUI_TOGGLE:
        {
            sappend(toolstr, pos, FormatText("bool %sActive = true;", control.name));
        } break;
        case GUI_CHECKBOX:
        {
            sappend(toolstr, pos, FormatText("bool %sChecked = false;", control.name));
        } break;
        case GUI_STATUSBAR:
        {
            sappend(toolstr, pos, FormatText("char *%sText = \"%s\";", control.name, control.text));
        } break;
        case GUI_LISTVIEW:
        case GUI_DROPDOWNBOX:
        {
            if (fullVariables)
            {
                sappend(toolstr, pos, FormatText("int %sCount = 3;", control.name));
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            }
            sappend(toolstr, pos, FormatText("const char *%sTextList[3] = { \"ONE\", \"TWO\", \"THREE\" };", control.name));
            ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            sappend(toolstr, pos, FormatText("int %sActive = 0;", control.name));
            ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            sappend(toolstr, pos, FormatText("bool %sEditMode = false;", control.name));
        } break;
        case GUI_COMBOBOX:
        case GUI_TOGGLEGROUP:
        {
            if (fullVariables)
            {
                sappend(toolstr, pos, FormatText("int %sCount = 3;", control.name));
                ENDLINEAPPEND(toolstr, pos);
                TABAPPEND(toolstr, pos, tabs);
            }
            sappend(toolstr, pos, FormatText("const char *%sTextList[3] = { \"ONE\", \"TWO\", \"THREE\" };", control.name));
            ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            sappend(toolstr, pos, FormatText("int %sActive = 0;", control.name));
        } break;
        case GUI_SLIDEREX:
        case GUI_SLIDERBAREX:
        {
            if (fullVariables)
            {
                sappend(toolstr, pos, FormatText("const float %sMinValue = 0.0f;", control.name));
                ENDLINEAPPEND(toolstr, pos);
                TABAPPEND(toolstr, pos, tabs);

                sappend(toolstr, pos, FormatText("const float %sMaxValue = 100.0f;", control.name));
                ENDLINEAPPEND(toolstr, pos);
                TABAPPEND(toolstr, pos, tabs);
            }
            sappend(toolstr, pos, FormatText("float %sValue = 50.0f;", control.name));

        } break;
        case GUI_PROGRESSBAREX:
        {
            sappend(toolstr, pos, FormatText("float %sValue = 50.0f;", control.name));
        } break;
        case GUI_VALUEBOX:
        case GUI_SPINNER:
        {
            sappend(toolstr, pos, FormatText("int %sValue = 0;", control.name));
            ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            sappend(toolstr, pos, FormatText("bool %sEditMode = false;", control.name));
        } break;
        case GUI_COLORPICKER:
        {
            sappend(toolstr, pos, FormatText("Color %sValue;", control.name));
        } break;
        case GUI_TEXTBOX:
        {
            if (fullVariables)
            {
                sappend(toolstr, pos, FormatText("int %sSize = %i;", control.name, MAX_CONTROL_TEXT_LENGTH));
                ENDLINEAPPEND(toolstr, pos);
                TABAPPEND(toolstr, pos, tabs);
            }
            sappend(toolstr, pos, FormatText("char %sText[%i] = \"%s\";", control.name, MAX_CONTROL_TEXT_LENGTH, control.text));
            ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
            sappend(toolstr, pos, FormatText("bool %sEditMode = false;", control.name));
        } break;
        default: break;
    }
}
static void WriteControlsDrawing(unsigned char *toolstr, int *pos, int index, GuiControl control, GuiLayoutConfig config)
{
    char *rec = GetControlRectangleText(index, control, config.defineRecs, config.exportAnchors, false);
    // TODO: definetext for window, groupbox, buttons, toggles and dummyrecs
    switch (control.type)
    {
        case GUI_LABEL:
        {
            if (config.defineTexts) sappend(toolstr, pos, FormatText("GuiLabel(%s, %sText);", rec, control.name));
            else sappend(toolstr, pos, FormatText("GuiLabel(%s, \"%s\");", rec, control.text));
        }
        break;
        case GUI_BUTTON: sappend(toolstr, pos, FormatText("if (GuiButton(%s, \"%s\")) %s();", rec, control.text, control.name)); break;
        case GUI_VALUEBOX: sappend(toolstr, pos, FormatText("if (GuiValueBox(%s, %sValue, 0, 100, %sEditMode)) %sEditMode = !%sEditMode;", rec, control.name, control.name, control.name, control.name)); break;
        case GUI_TOGGLE: sappend(toolstr, pos, FormatText("%sActive = GuiToggle(%s, \"%s\", %sActive);", control.name, rec, control.text, control.name)); break;
        case GUI_TOGGLEGROUP: sappend(toolstr, pos, FormatText("%sActive = GuiToggleGroup(%s, %sTextList, %s, %sActive);", control.name, rec, control.name, GetControlParamText(control.type, control.name, config), control.name)); break;
        case GUI_SLIDEREX:
        {
            if (control.text[0] != '\0') sappend(toolstr, pos, FormatText("%sValue = GuiSliderEx(%s, %sValue, %s, \"%s\", true);", control.name, rec, control.name, GetControlParamText(control.type, control.name, config), control.text));
            else sappend(toolstr, pos, FormatText("%sValue = GuiSlider(%s, %sValue, %s);", control.name, rec, control.name, GetControlParamText(control.type, control.name, config)));
        } break;
        case GUI_SLIDERBAREX:
        {
            if (control.text[0] != '\0') sappend(toolstr, pos, FormatText("%sValue = GuiSliderBarEx(%s, %sValue, %s, \"%s\", true);", control.name, rec, control.name, GetControlParamText(control.type, control.name, config), control.text));
            else sappend(toolstr, pos, FormatText("%sValue = GuiSliderBar(%s, %sValue, %s);", control.name, rec, control.name, GetControlParamText(control.type, control.name, config)));
        } break;
        case GUI_PROGRESSBAREX: sappend(toolstr, pos, FormatText("%sValue = GuiProgressBarEx(%s, %sValue, 0, 100, true);", control.name, rec, control.name)); break;
        case GUI_SPINNER: sappend(toolstr, pos, FormatText("if (GuiSpinner(%s, %sValue, 0, 100, 25, %sEditMode)) %sEditMode = !%sEditMode;", rec, control.name, control.name, control.name, control.name)); break;
        case GUI_COMBOBOX: sappend(toolstr, pos, FormatText("%sActive = GuiComboBox(%s, %sTextList, %s, %sActive);", control.name, rec, control.name, GetControlParamText(control.type, control.name, config), control.name)); break;
        case GUI_CHECKBOX:
        {
            if (control.text[0] != '\0') sappend(toolstr, pos, FormatText("%sChecked = GuiCheckBoxEx(%s, %sChecked, \"%s\");", control.name, rec, control.name, control.text));
            else sappend(toolstr, pos, FormatText("%sChecked = GuiCheckBox(%s, %sChecked);", control.name, rec, control.name));
        } break;
        case GUI_LISTVIEW: sappend(toolstr, pos, FormatText("if (GuiListView(%s, %sTextList, %s, &%sScrollIndex, &%sActive, %sEditMode)) %sEditMode = !%sEditMode;", rec, control.name, GetControlParamText(control.type, control.name, config), control.name, control.name, control.name, control.name, control.name)); break;
        case GUI_TEXTBOX: sappend(toolstr, pos, FormatText("if (GuiTextBox(%s, %sText, %s, %sEditMode)) %sEditMode = !%sEditMode;", rec, control.name, GetControlParamText(control.type, control.name, config), control.name, control.name, control.name)); break;
        case GUI_GROUPBOX: sappend(toolstr, pos, FormatText("GuiGroupBox(%s, \"%s\");", rec, control.text)); break;
        case GUI_DUMMYREC: sappend(toolstr, pos, FormatText("GuiDummyRec(%s, \"%s\");", rec, control.text)); break;
        case GUI_DROPDOWNBOX: sappend(toolstr, pos, FormatText("if (GuiDropdownBox(%s, %sTextList, %s, &%sActive, %sEditMode)) %sEditMode = !%sEditMode;", rec, control.name, GetControlParamText(control.type, control.name, config), control.name, control.name, control.name, control.name)); break;
        case GUI_STATUSBAR: sappend(toolstr, pos, FormatText("GuiStatusBar(%s, %sText, 10);", rec, control.name)); break;
        case GUI_COLORPICKER: sappend(toolstr, pos, FormatText("%sValue = GuiColorPicker(%s, %sValue);", control.name, rec, control.name)); break;
        case GUI_LINE: sappend(toolstr, pos, FormatText("GuiLine(%s, 1);", rec)); break;
        case GUI_PANEL: sappend(toolstr, pos, FormatText("GuiPanel(%s);", rec)); break;

        default: break;
    }
}

// Export H functions
static void WriteStruct(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs)
{
    TABAPPEND(toolstr, pos, tabs);
    sappend(toolstr, pos, "typedef struct {");
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs+1);

    // Write anchors variables
    if (config.exportAnchors && layout.anchorsCount > 1) WriteAnchors(toolstr, pos, layout, config, true, false, "", tabs+1);

    // Write controls variables
    if (layout.controlsCount > 0) WriteControlVariablesH(toolstr, pos, layout, config, true, true, "", tabs+1);

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
static void WriteFunctionsDeclarationH(unsigned char *toolstr, int *pos, GuiLayoutConfig config, int tabs)
{
    sappend(toolstr, pos, FormatText("Gui%sState InitGui%s(void);", TextToPascal(config.name), TextToPascal(config.name)));
    ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
    sappend(toolstr, pos, FormatText("void Gui%s(Gui%sState *state);", TextToPascal(config.name), TextToPascal(config.name)));
}
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
        WriteControlVariablesH(toolstr, pos, layout, config, false, true, "state.", tabs+1);
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
static void WriteFunctionDrawingH(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, int tabs)
{    
    // Export GuiLayout draw function
    sappend(toolstr, pos, FormatText("void Gui%s(Gui%sState *state)", TextToPascal(config.name), TextToPascal(config.name)));
    ENDLINEAPPEND(toolstr, pos);
    sappend(toolstr, pos, "{"); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);

    // Const text
    if (config.defineTexts)
    {
        WriteConstText(toolstr, pos, layout, config, tabs + 1);
    }
    
    // Controls draw
    if (layout.controlsCount > 0)
    {
        WriteControlDrawingH(toolstr, pos, layout, config, "state->", tabs + 1);
    }

    ENDLINEAPPEND(toolstr, pos);
    sappend(toolstr, pos, "}");
}

// Code generation
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
            case GUI_SLIDEREX:
            case GUI_SLIDERBAREX:
            case GUI_PROGRESSBAREX:
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
            break;
        }
    }    
    if (drawConstText) ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs);
}
static void WriteControlVariablesH(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config, bool define, bool initialize, const char *preText, int tabs)
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
            case GUI_DROPDOWNBOX:
            case GUI_LISTVIEW:
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
            case GUI_SLIDEREX:
            case GUI_SLIDERBAREX:
            case GUI_PROGRESSBAREX:
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
                //TODO if (initialize) sappend(toolstr, pos, FormatText(" = { %i, %i }", (int)layout.anchors[i].x, (int)layout.anchors[i].y));
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
static void WriteControlDrawingH(unsigned char *toolstr, int *pos, GuiLayout layout, GuiLayoutConfig config const char *preText, int tabs)
{   
    if (config.fullComments)
    {
        sappend(toolstr, pos, "// Draw controls");
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
    }

    bool draw[layout.controlsCount];
    for (int k = 0; k < layout.controlsCount; k++) draw[k] = false;

    // If dropdown control exist, draw GuiLock condition
    bool dropDownExist = false;
    for (int k = 0; k < layout.controlsCount; k++)
    {
        if (layout.controls[k].type == GUI_DROPDOWNBOX)
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
            sappend(toolstr, pos, FormatText("state->%sEditMode", layout.controls[k].name));
        }
    }
    if (dropDownExist)
    {
        sappend(toolstr, pos, ") GuiLock();");
        ENDLINEAPPEND(toolstr, pos); ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
    }

    // Draw GUI_WINDOWBOX
    for (int k = 0; k < layout.controlsCount; k++)
    {
        if (!draw[k])
        {
            if (layout.controls[k].type == GUI_WINDOWBOX)
            {
                draw[k] = true;

               // state-> GetControlRectangleText ...
                char *rec = GetControlRectangleText(k, layout.controls[k], config.defineRecs, config.exportAnchors, true);
                sappend(toolstr, pos, FormatText("if (state->%sActive)", layout.controls[k].name));
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
                sappend(toolstr, pos, "{");
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 2);
                sappend(toolstr, pos, FormatText("state->%sActive = !GuiWindowBox(%s, \"%s\");", layout.controls[k].name, rec, layout.controls[k].text));
                ENDLINEAPPEND(toolstr, pos);

                int windowAnchorID = layout.controls[k].ap->id;

                // Draw controls inside window
                for (int m = 0; m < layout.controlsCount; m++)
                {
                    if (!draw[m] && k != m && layout.controls[m].type != GUI_WINDOWBOX  && layout.controls[m].type != GUI_DROPDOWNBOX)
                    {
                        if (windowAnchorID == layout.controls[m].ap->id)
                        {
                            draw[m] = true;

                            TABAPPEND(toolstr, pos, tabs + 2);
                            WriteControlsDrawingH(toolstr, pos, m, layout.controls[m], config);
                            ENDLINEAPPEND(toolstr, pos);
                        }
                    }
                }

                // Draw GUI_DROPDOWNBOX inside GUI_WINDOWBOX
                for (int m = 0; m < layout.controlsCount; m++)
                {
                    if (!draw[m] && k != m && layout.controls[m].type == GUI_DROPDOWNBOX)
                    {
                        if (windowAnchorID == layout.controls[m].ap->id)
                        {
                            draw[m] = true;

                            TABAPPEND(toolstr, pos, tabs + 2);
                            WriteControlsDrawingH(toolstr, pos, m, layout.controls[m], config);
                            ENDLINEAPPEND(toolstr, pos);
                        }
                    }
                }
                TABAPPEND(toolstr, pos, tabs+1);
                sappend(toolstr, pos, "}");
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
            }
        }
    }

    // Draw the rest of controls
    for (int k = 0; k < layout.controlsCount; k++)
    {
        if (!draw[k])
        {
            if (layout.controls[k].type != GUI_DROPDOWNBOX)
            {
                draw[k] = true;
                WriteControlsDrawingH(toolstr, pos, k, layout.controls[k], config);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
            }
        }
    }

    // Draw GUI_DROPDOWNBOX
    for (int k = 0; k < layout.controlsCount; k++)
    {
        if (!draw[k])
        {
            if (layout.controls[k].type == GUI_DROPDOWNBOX)
            {
                draw[k] = true;
                WriteControlsDrawingH(toolstr, pos, k, layout.controls[k], config);
                ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
            }
        }
    }

    if (dropDownExist)
    {
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
        sappend(toolstr, pos, "GuiUnlock();");
        ENDLINEAPPEND(toolstr, pos); TABAPPEND(toolstr, pos, tabs + 1);
    }
    *pos -= (tabs+1)*4 + 1; // Delete last tabs and \n    
}
static void WriteControlsDrawingHOLD(unsigned char *toolstr, int *pos, int index, GuiControl control, GuiLayoutConfig config)
{
    char *rec = GetControlRectangleText(index, control, config.defineRecs, config.exportAnchors, true);
    // TODO:  define text for window, groupbox, buttons, toggles and dummyrecs
    switch (control.type)
    {
        case GUI_LABEL:
        {
            if (config.defineTexts) sappend(toolstr, pos, FormatText("GuiLabel(%s, %sText);", rec, control.name));
            else sappend(toolstr, pos, FormatText("GuiLabel(%s, \"%s\");", rec, control.text));
        }
        break;
        case GUI_BUTTON: sappend(toolstr, pos, FormatText("if (GuiButton(%s, \"%s\")) %s();", rec, control.text, control.name)); break;
        case GUI_VALUEBOX: sappend(toolstr, pos, FormatText("if (GuiValueBox(%s, state->%sValue, 0, 100, state->%sEditMode)) state->%sEditMode = !state->%sEditMode;", rec, control.name, control.name, control.name, control.name)); break;
        case GUI_TOGGLE: sappend(toolstr, pos, FormatText("state->%sActive = GuiToggle(%s, \"%s\", state->%sActive);", control.name, rec, control.text, control.name)); break;
        case GUI_TOGGLEGROUP: sappend(toolstr, pos, FormatText("state->%sActive = GuiToggleGroup(%s, %sTextList, state->%s, state->%sActive);", control.name, rec, control.name, GetControlParamText(control.type, control.name, config), control.name)); break;
        case GUI_SLIDEREX:
        {
            if (control.text[0] != '\0') sappend(toolstr, pos, FormatText("state->%sValue = GuiSliderEx(%s, state->%sValue, %s, \"%s\", true);", control.name, rec, control.name, GetControlParamText(control.type, control.name, config), control.text));
            else sappend(toolstr, pos, FormatText("state->%sValue = GuiSlider(%s, state->%sValue, %s);", control.name, rec, control.name, GetControlParamText(control.type, control.name, config)));
        } break;
        case GUI_SLIDERBAREX:
        {
            if (control.text[0] != '\0') sappend(toolstr, pos, FormatText("%sValue = GuiSliderBarEx(%s, %sValue, %s, \"%s\", true);", control.name, rec, control.name, GetControlParamText(control.type, control.name, config), control.text));
            else sappend(toolstr, pos, FormatText("state->%sValue = GuiSliderBar(%s, state->%sValue, %s);", control.name, rec, control.name, GetControlParamText(control.type, control.name, config)));
        } break;
        case GUI_PROGRESSBAREX: sappend(toolstr, pos, FormatText("state->%sValue = GuiProgressBarEx(%s, state->%sValue, 0, 100, true);", control.name, rec, control.name)); break;
        case GUI_SPINNER: sappend(toolstr, pos, FormatText("if (GuiSpinner(%s, state->%sValue, 0, 100, 25, state->%sEditMode)) state->%sEditMode = !state->%sEditMode;", rec, control.name, control.name, control.name, control.name)); break;
        case GUI_COMBOBOX: sappend(toolstr, pos, FormatText("state->%sActive = GuiComboBox(%s, %sTextList, state->%s, state->%sActive);", control.name, rec, control.name, GetControlParamText(control.type, control.name, config), control.name)); break;
        case GUI_CHECKBOX:
        {
            if (control.text[0] != '\0') sappend(toolstr, pos, FormatText("%sChecked = GuiCheckBoxEx(%s, %sChecked, \"%s\");", control.name, rec, control.name, control.text));
            else sappend(toolstr, pos, FormatText("state->%sChecked = GuiCheckBox(%s, state->%sChecked);", control.name, rec, control.name));
        } break;
        case GUI_LISTVIEW: sappend(toolstr, pos, FormatText("if (GuiListView(%s, %sTextList, state->%s, &state->%sScrollIndex, &state->%sActive, state->%sEditMode)) state->%sEditMode = !state->%sEditMode;", rec, control.name, GetControlParamText(control.type, control.name, config), control.name, control.name, control.name, control.name, control.name)); break;
        case GUI_TEXTBOX: sappend(toolstr, pos, FormatText("if (GuiTextBox(%s, state->%sText, state->%s, state->%sEditMode)) state->%sEditMode = !state->%sEditMode;", rec, control.name, GetControlParamText(control.type, control.name, config), control.name, control.name, control.name)); break;
        case GUI_GROUPBOX: sappend(toolstr, pos, FormatText("GuiGroupBox(%s, \"%s\");", rec, control.text)); break;
        case GUI_DUMMYREC: sappend(toolstr, pos, FormatText("GuiDummyRec(%s, \"%s\");", rec, control.text)); break;
        case GUI_DROPDOWNBOX: sappend(toolstr, pos, FormatText("if (GuiDropdownBox(%s, %sTextList, state->%s, &state->%sActive, state->%sEditMode)) state->%sEditMode = !state->%sEditMode;", rec, control.name, GetControlParamText(control.type, control.name, config), control.name, control.name, control.name, control.name)); break;
        case GUI_STATUSBAR: sappend(toolstr, pos, FormatText("GuiStatusBar(%s, state->%sText, 10);", rec, control.name)); break;
        case GUI_COLORPICKER: sappend(toolstr, pos, FormatText("state->%sValue = GuiColorPicker(%s, state->%sValue);", control.name, rec, control.name)); break;
        case GUI_LINE: sappend(toolstr, pos, FormatText("GuiLine(%s, 1);", rec)); break;
        case GUI_PANEL: sappend(toolstr, pos, FormatText("GuiPanel(%s);", rec)); break;

        default: break;
    }
}

#endif // CODEGEN_IMPLEMENTATION