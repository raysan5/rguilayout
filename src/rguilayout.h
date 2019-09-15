/**********************************************************************************************
*
*   rGuiLayout v2.1 - common types and global variables
*
*   LICENSE: Propietary License
*
*   Copyright (c) 2018-2019 raylib technologies (@raylibtech). All Rights Reserved.
*
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   This project is proprietary and confidential unless the owner allows
*   usage in any other form by expresely written permission.
*
**********************************************************************************************/

#include "raylib.h"

#ifndef RGUILAYOUT_H
#define RGUILAYOUT_H

#define MAX_GUI_CONTROLS                256      // Maximum number of gui controls
#define MAX_ANCHOR_POINTS                 8      // Maximum number of anchor points

#define MAX_ANCHOR_NAME_LENGTH           64      // Maximum length of anchor name
#define MAX_CONTROL_NAME_LENGTH          64      // Maximum length of control name
#define MAX_CONTROL_TEXT_LENGTH         128      // Maximum length of control text

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Controls type id
// WARNING: Do not change those values to avoid breaking all layouts created
// In case a control type needs to be removed, just remove it without changing values
// In case a control type needs to be added, just add at the end or use an empty space value
// WARNING: Controls names array (defined below) MUST be aligned with GuiControlType enum values
typedef enum {
    GUI_WINDOWBOX       = 0,
    GUI_GROUPBOX        = 1,
    GUI_LINE            = 2,
    GUI_PANEL           = 3,
    GUI_LABEL           = 4,
    GUI_BUTTON          = 5,
    GUI_LABELBUTTON     = 6,
    GUI_IMAGEBUTTONEX   = 7,
    GUI_CHECKBOX        = 8,
    GUI_TOGGLE          = 9,
    GUI_TOGGLEGROUP     = 10,
    GUI_COMBOBOX        = 11,
    GUI_DROPDOWNBOX     = 12,
    GUI_TEXTBOX         = 13,
    GUI_TEXTBOXMULTI    = 14,
    GUI_VALUEBOX        = 15,
    GUI_SPINNER         = 16,
    GUI_SLIDER          = 17,
    GUI_SLIDERBAR       = 18,
    GUI_PROGRESSBAR     = 19,
    GUI_STATUSBAR       = 20,
    GUI_SCROLLPANEL     = 21,
    GUI_LISTVIEW        = 22,
    GUI_COLORPICKER     = 23,
    GUI_DUMMYREC        = 24
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

// Gui layout control type
typedef struct {
    int id;
    int type;
    Rectangle rec;
    unsigned char name[MAX_CONTROL_NAME_LENGTH];
    unsigned char text[MAX_CONTROL_TEXT_LENGTH];
    GuiAnchorPoint *ap;
} GuiLayoutControl;

// Gui layout type
typedef struct {
    int controlsCount;
    int anchorsCount;
    GuiLayoutControl controls[MAX_GUI_CONTROLS];
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
    bool exportButtonFunctions;
} GuiLayoutConfig;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------

// Control type names
// WARNING: Controls names MUST be aligned with GuiControlType enum values
const char *controlTypeName[] = {
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
    "Slider",
    "SliderBar",
    "ProgressBar",
    "StatusBar",
    "ScrollPanel",
    "ListView",
    "ColorPicker",
    "MessageBox",
    "DummyRec",
    "Grid"
};

#endif // RGUILAYOUT_H
