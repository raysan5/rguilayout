/**********************************************************************************************
*
*   rGuiLayout - Common types and globals
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

#ifndef RGUILAYOUT_H
#define RGUILAYOUT_H

#define MAX_GUI_CONTROLS                256      // Maximum number of gui controls
#define MAX_ANCHOR_POINTS                16      // Maximum number of anchor points

#define MAX_ANCHOR_NAME_LENGTH           64      // Maximum length of anchor name
#define MAX_CONTROL_NAME_LENGTH          64      // Maximum length of control name
#define MAX_CONTROL_VALUES_LENGTH        64      // Maximum length of control values
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
    GUI_CHECKBOX        = 7,
    GUI_TOGGLE          = 8,
    GUI_TOGGLEGROUP     = 9,
    GUI_COMBOBOX        = 10,
    GUI_DROPDOWNBOX     = 11,
    GUI_TEXTBOX         = 12,
    GUI_TEXTBOXMULTI    = 13,
    GUI_VALUEBOX        = 14,
    GUI_SPINNER         = 15,
    GUI_SLIDER          = 16,
    GUI_SLIDERBAR       = 17,
    GUI_PROGRESSBAR     = 18,
    GUI_STATUSBAR       = 19,
    GUI_SCROLLPANEL     = 20,
    GUI_LISTVIEW        = 21,
    GUI_COLORPICKER     = 22,
    GUI_DUMMYREC        = 23
} GuiControlType;

// Anchor point type
typedef struct GuiAnchorPoint GuiAnchorPoint;
struct GuiAnchorPoint {
    int id;
    int x;
    int y;
    bool enabled;
    bool hidding;
    unsigned char name[MAX_ANCHOR_NAME_LENGTH];     // 64 bytes
    GuiAnchorPoint *ap;
};

// Gui layout control type
typedef struct {
    int id;
    int type;
    Rectangle rec;
    unsigned char name[MAX_CONTROL_NAME_LENGTH];     // 64 bytes
    unsigned char text[MAX_CONTROL_TEXT_LENGTH];     // 128 bytes
	unsigned char values[MAX_CONTROL_VALUES_LENGTH]; // 63 bytes
    GuiAnchorPoint *ap;
} GuiLayoutControl;

// Gui layout type
typedef struct {
    int controlCount;
    int anchorCount;
    GuiLayoutControl controls[MAX_GUI_CONTROLS];    // 256 controls
    GuiAnchorPoint anchors[MAX_ANCHOR_POINTS];      // 8 anchors
    Rectangle refWindow;
} GuiLayout;

// Gui layout configuration for code exportation
typedef struct {
    unsigned char name[64];
    unsigned char version[32];
    unsigned char company[128];
    unsigned char description[256];
    const char *template;                     // Code template
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
    "DummyRec"
};

#endif // RGUILAYOUT_H
