/**********************************************************************************************
*
*   rGuiLayout v2.0 - common types and global variables
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

#ifndef RGUILAYOUT_H
#define RGUILAYOUT_H

#define MAX_GUI_CONTROLS           256      // Maximum number of gui controls
#define CONTROLS_TYPE_NUM           32
#define MAX_ANCHOR_POINTS            8      // Maximum number of anchor points

#define MAX_CONTROL_TEXT_LENGTH     64      // Maximum length of control text
#define MAX_CONTROL_TEXTMULTI_LENGTH    512      // Maximum length of control text
#define MAX_CONTROL_NAME_LENGTH     32      // Maximum length of control name (used on code generation)
#define MAX_ANCHOR_NAME_LENGTH      32

#define TABAPPEND(x, y, z)          { for (int t = 0; t < z; t++) sappend(x, y, "    "); }
#define ENDLINEAPPEND(x, y)         sappend(x, y, "\n");

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum {
    GUI_WINDOWBOX = 0,
    GUI_GROUPBOX,
    GUI_LINE,
    GUI_PANEL,
    GUI_LABEL,
    GUI_BUTTON,
    GUI_LABELBUTTON,
    GUI_IMAGEBUTTONEX,
    GUI_CHECKBOX,
    GUI_TOGGLE,
    GUI_TOGGLEGROUP,
    GUI_COMBOBOX,
    GUI_DROPDOWNBOX,
    GUI_TEXTBOX,
    GUI_TEXTMULTIBOX,
    GUI_VALUEBOX,
    GUI_SPINNER,
    GUI_SLIDEREX,
    GUI_SLIDERBAREX,
    GUI_PROGRESSBAREX,
    GUI_STATUSBAR,
    GUI_SCROLLPANEL,
    GUI_LISTVIEW,
    GUI_COLORPICKER,
    GUI_DUMMYREC
} GuiControlType;

// Anchor point type
typedef struct GuiAnchorPoint GuiAnchorPoint;
struct GuiAnchorPoint {
    int id;
    int x;
    int y;
    bool enabled;
    bool hidding;
    unsigned char name[MAX_ANCHOR_NAME_LENGTH];
    GuiAnchorPoint *ap;
};

// Gui control type
typedef struct {
    int id;
    int type;
    Rectangle rec;
    unsigned char name[MAX_CONTROL_NAME_LENGTH];
    unsigned char text[MAX_CONTROL_TEXT_LENGTH];
    GuiAnchorPoint *ap;
} GuiControl;

// Gui layout type
typedef struct {
    int controlsCount;
    int anchorsCount;
    GuiControl controls[MAX_GUI_CONTROLS];
    GuiAnchorPoint anchors[MAX_ANCHOR_POINTS];
    Rectangle refWindow;
} GuiLayout;

// Gui layout configuration for code exportation
typedef struct {
    unsigned char name[64];
    unsigned char version[32];
    unsigned char company[128];
    unsigned char description[256];
    bool exportAnchors;
    bool defineRecs;
    bool defineTexts;
    bool fullComments;
} GuiLayoutConfig;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
const char *controlTypeName[] =
{
    "WindowBox",
    "GroupBox",
    "Line",
    "Panel",
    "Label",
    "Button",
    "LabelButton",
    "ImageButtonEx",
    "CheckBoxEx",
    "Toggle",
    "ToggleGroup",
    "ComboBox",
    "DropdownBox",
    "TextBox",
    "TextmultiBox",
    "ValueBOx",
    "Spinner",
    "SliderEx",
    "SliderBarEx",
    "PrograssBarEx",
    "StatusBar",
    "ScrollPanel",
    "ListView",
    "ColorPicker",
    "MessageBox",
    "DummyRec",
    "Grid"
};

#endif // RGUILAYOUT_H
