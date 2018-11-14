/*******************************************************************************************
*
*   rGuiLayout v1.2 - A simple and easy-to-use raygui layouts editor
*
*   CONFIGURATION:
*
*   #define ENABLE_PRO_FEATURES
*       Enable PRO features for the tool. Usually command-line and export options related.
*
*   DEPENDENCIES:
*       raylib 2.0              - Windowing/input management and drawing.
*       raygui 2.0              - IMGUI controls (based on raylib).
*       tinyfiledialogs 3.3.7   - Open/save file dialogs, it requires linkage with comdlg32 and ole32 libs.
*
*   COMPILATION (Windows - MinGW):
*       gcc -o rguilayout.exe rguilayout.c external/tinyfiledialogs.c -s -Iexternal /
*           -lraylib -lopengl32 -lgdi32 -lcomdlg32 -lole32 -std=c99
*
*   COMPILATION (Linux - GCC):
*       gcc -o rguilayout rguilayout.c external/tinyfiledialogs.c -s -Iexternal -no-pie -D_DEFAULT_SOURCE /
*           -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
*
*   DEVELOPERS:
*       Ramon Santamaria (@raysan5):  Supervision, design and maintenance.
*       Adria Arranz (@Adri102):      Developer and designer (2018)
*       Jordi Jorba (@KoroBli):       Developer and designer (2018)
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018 raylib technologies (@raysan5).
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

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_STYLE_SAVE_LOAD
#include "raygui.h"                         // Required for: IMGUI controls

#include "external/easings.h"               // Required for: Easing animations math
#include "external/tinyfiledialogs.h"       // Required for: Open/Save file dialogs

#include <stdlib.h>                         // Required for: malloc(), free()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define ENABLE_PRO_FEATURES                 // Enable PRO version features

#define TOOL_VERSION_TEXT         "1.2"     // Tool version string

#define MAX_GUI_CONTROLS           256      // Maximum number of gui controls
#define CONTROLS_TYPE_NUM           32
#define MAX_ANCHOR_POINTS            8      // Maximum number of anchor points
#define ANCHOR_RADIUS               20      // Default anchor radius

#define MAX_CONTROL_TEXT_LENGTH     64      // Maximum length of control text
#define MAX_CONTROL_NAME_LENGTH     32      // Maximum length of control name (used on code generation)

#define GRID_LINE_SPACING           20      // Grid line spacing in pixels

#define MOVEMENT_FRAME_SPEED        10      // Controls movement speed in pixels per frame

#define PANELS_EASING_FRAMES        60      // Controls the easing time in frames

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
    GUI_TOGGLE,
    GUI_TOGGLEGROUP,
    GUI_CHECKBOX,
    GUI_COMBOBOX,
    GUI_DROPDOWNBOX,
    GUI_SPINNER,
    GUI_VALUEBOX,
    GUI_TEXTBOX,
    GUI_SLIDER,
    GUI_SLIDERBAR,
    GUI_PROGRESSBAR,
    GUI_STATUSBAR,
    GUI_LISTVIEW,
    GUI_COLORPICKER,
    GUI_DUMMYREC
} GuiControlType;

// Anchor point type
typedef struct {
    int id;
    int x;
    int y;
    bool enabled;
    bool hidding;
} GuiAnchorPoint;

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
} GuiLayout;

// Gui layout configuration for code exportation
typedef struct {
    int width;
    int height;
    unsigned char name[64];
    unsigned char version[32];
    unsigned char company[128];
    unsigned char description[256];
    bool defineRecs;
    bool exportAnchors;
    bool exportAnchor0;
    bool fullComments;
    bool defineTexts;
    bool cropWindow;
    bool fullVariables;
} GuiLayoutConfig;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static GuiLayout layout = { 0 };

const char *controlTypeName[] = { "WINDOWBOX", "GROUPBOX", "LINE", "PANEL", "LABEL", "BUTTON", "TOGGLE", "TOGGLEGROUP", "CHECKBOX", "COMBOBOX", "DROPDOWNBOX", "SPINNER", "VALUEBOX", "TEXTBOX", "SLIDER", "SLIDERBAR", "PROGRESSBAR", "STATUSBAR", "LISTVIEW", "COLORPICKER", "DUMMYREC" };
const char *controlTypeNameLow[] = { "WindowBox", "GroupBox", "Line", "Panel", "Label", "Button", "Toggle", "ToggleGroup", "CheckBox", "ComboBox", "DropdownBox", "Spinner", "ValueBox", "TextBox", "Slider", "SliderBar", "ProgressBar", "StatusBar", "ListView", "ColorPicker", "DummyRec" };
const char *controlTypeNameShort[] = { "wdwbox", "grpbox", "lne", "pnl", "lbl", "btn", "tgl", "tglgrp", "chkbox", "combox", "ddwnbox", "spnr", "vlbox", "txtbox", "sldr", "sldrb", "prgssb", "stsb", "lstvw", "clrpckr", "dmyrc" };

static bool cancelSave = false;
static char loadedFileName[256] = { 0 };    // Loaded layout file name

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(ENABLE_PRO_FEATURES)
static void ShowCommandLineInfo(void);                      // Show command line usage info
static void ProcessCommandLine(int argc, char *argv[]);     // Process command line input
#endif

// Load/Save/Export data functions
static void LoadLayout(const char *fileName);                   // Load raygui layout (.rgl), text or binary
static void SaveLayout(const char *fileName, bool binary);      // Save raygui layout (.rgl), text or binary

static void DialogSaveLayout(void);                             // Show dialog: save layout file (.rgl)
static void DialogExportLayout(GuiLayoutConfig config);         // Show dialog: export layout file (.c)

// Code generation functions
static char *GetControlRectangleText(int index, GuiControl control, GuiLayoutConfig config);    // Get control rectangle text
static char *GetControlParamText(int controlType, char *name, GuiLayoutConfig config);          // Get control func parameters text
static void WriteControlsVariables(FILE *ftool, GuiControl control, GuiLayoutConfig config);    // Write controls variables code to file
static void WriteControlsDrawing(FILE *ftool,int index, GuiControl control, GuiLayoutConfig config);      // Write controls drawing code to file
static void GenerateCode(const char *fileName, GuiLayoutConfig config);                         // Generate C code for gui layout

//----------------------------------------------------------------------------------
// Program main entry point
//----------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    char inFileName[256] = { 0 };       // Input file name (required in case of drag & drop over executable)

    // Command-line usage mode
    //--------------------------------------------------------------------------------------
    if (argc > 1)
    {
        if ((argc == 2) &&
            (strcmp(argv[1], "-h") != 0) &&
            (strcmp(argv[1], "--help") != 0))       // One argument (file dropped over executable?)
        {
            if (IsFileExtension(argv[1], ".rgl"))
            {
                // Open file with graphic interface
                strcpy(inFileName, argv[1]);        // Read input filename
            }
        }
#if defined(ENABLE_PRO_FEATURES)
        else
        {
            ProcessCommandLine(argc, argv);
            return 0;
        }
#endif      // ENABLE_PRO_FEATURES
    }

    // GUI usage mode - Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 800;

    SetTraceLog(0);                             // Disable trace log messsages
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);      // Window configuration flags
    InitWindow(screenWidth, screenHeight, FormatText("rGuiLayout v%s - A simple and easy-to-use raygui layouts editor", TOOL_VERSION_TEXT));
    SetWindowMinSize(800, 800);
    SetExitKey(0);

    // General app variables
    Vector2 mouse = { -1, -1 };         // Mouse position
    bool exitWindow = false;            // Exit window flag    
    bool showGrid = true;               // Show grid flag (KEY_G) 
    
    // Modes
    bool dragMode = false;       // Control drag mode
    bool showGlobalPos = false;      // Control global position mode
    bool snapMode = false;              // Snap mode flag (KEY_S)
    bool textEditMode = false;          // Control text edit mode (KEY_T)
    bool nameEditMode = false;          // Control name edit mode (KEY_N)
    bool orderEditMode = false;         // Control order edit mode (focusedControl != -1 + KEY_LEFT_ALT)
    bool resizeMode = false;              // Control size mode (controlSelected != -1 + KEY_LEFT_ALT)
    bool precisionMode = false;         // Control precision mode (KEY_LEFT_SHIFT)
    
    int framesCounter = 0;
    int framesCounterSnap = 0;
    
    // Controls variables
    int selectedControl = -1;
    int storedControl = -1;
    int focusedControl = -1;
    int selectedType = GUI_WINDOWBOX;
    int selectedTypeDraw = GUI_LABEL;
    Vector2 panControlOffset = { 0 };
    Vector2 prevControlPosition = { 0 };
    Color selectedControlColor = RED;
    Color positionColor = MAROON;    
    
    int framesCounterMovement = 0;
    int movePixel = 1;
    int movePerFrame = 1;

    int textArrayPos = 0;

    const char *listData[3] = { "ONE", "TWO", "THREE" };    // ToggleGroup, ComboBox, DropdownBox default data
    const char *listViewData[4] = { "One", "Two", "Three", "Four" }; // ListView default data
    
    // Anchors control variables
    GuiAnchorPoint auxAnchor = { 9, 0, 0, 0 };
    bool anchorEditMode = false;
    bool anchorLinkMode = false;
    bool anchorLockMode = false;
    bool anchorMoveMode = false;
    int selectedAnchor = -1;
    int focusedAnchor = -1;
    Color anchorCircleColor = BLACK;
    Color anchorSelectedColor = RED;

    // Help panel variables
    int helpPositionX = -300;
    int helpCounter = 0;
    int helpStartPositionX = -300;
    int helpDeltaPositionX = 0;
    bool helpActive = false;

    // Rectangles used on controls preview drawing
    Rectangle defaultRec[CONTROLS_TYPE_NUM] = {
        (Rectangle){ 0, 0, 125, 50},            // GUI_WINDOWBOX
        (Rectangle){ 0, 0, 125, 30},            // GUI_GROUPBOX
        (Rectangle){ 0, 0, 125, 25 },           // GUI_LINE
        (Rectangle){ 0, 0, 125, 35 },           // GUI_PANEL
        (Rectangle){ 0, 0, 126, 25 },           // GUI_LABEL
        (Rectangle){ 0, 0, 125, 30 },           // GUI_BUTTON
        (Rectangle){ 0, 0, 90, 25 },            // GUI_TOGGLE
        (Rectangle){ 0, 0, 125, 25 },           // GUI_TOGGLEGROUP
        (Rectangle){ 0, 0, 15, 15},             // GUI_CHECKBOX
        (Rectangle){ 0, 0, 125, 25 },           // GUI_COMBOBOX
        (Rectangle){ 0, 0, 125, 25 },           // GUI_DROPDOWNBOX
        (Rectangle){ 0, 0, 125, 25 },           // GUI_SPINNER
        (Rectangle){ 0, 0, 125, 25 },           // GUI_VALUEBOX
        (Rectangle){ 0, 0, 125, 25 },           // GUI_TEXTBOX
        (Rectangle){ 0, 0, 125, 15 },           // GUI_SLIDER
        (Rectangle){ 0, 0, 125, 15 },           // GUI_SLIDERBAR
        (Rectangle){ 0, 0, 125, 15 },           // GUI_PROGRESSBAR
        (Rectangle){ 0, 0, 125, 25 },           // GUI_STATUSBAR
        (Rectangle){ 0, 0, 125, 75 },           // GUI_LISTVIEW
        (Rectangle){ 0, 0, 95, 95 },            // GUI_COLORPICKER
        (Rectangle){ 0, 0, 125, 30 }            // GUI_DUMMYREC
    };

    // Initialize anchor points to default values
    for (int i = 0; i < MAX_ANCHOR_POINTS; i++)
    {
        layout.anchors[i].id = i;
        layout.anchors[i].x = 0;
        layout.anchors[i].y = 0;
        layout.anchors[i].enabled = false;
        layout.anchors[i].hidding = false;
    }

    layout.anchors[0].enabled = true;      // Enable layout parent anchor (0, 0)
    layout.anchorsCount = 1;
    layout.controlsCount = 0;
    // Initialize layout controls data
    for (int i = 0; i < MAX_GUI_CONTROLS; i++)
    {
        layout.controls[i].id = 0;
        layout.controls[i].type = 0;
        layout.controls[i].rec = (Rectangle){ 0, 0, 0, 0 };
        memset(layout.controls[i].text, 0, MAX_CONTROL_TEXT_LENGTH);
        memset(layout.controls[i].name, 0, MAX_CONTROL_NAME_LENGTH);
        layout.controls[i].ap = &layout.anchors[0];  // By default, set parent anchor
    }

    // Define palette variables
    Rectangle palettePanel = { GetScreenWidth() + 130, 15, 135, 835 };
    int paletteSelect = -1;
    int paletteCounter = 0;
    int paletteStartPositionX = GetScreenWidth() + 130;
    int paletteDeltaPositionX = 0;
    bool paletteActive = false;

    // Define palette rectangles
    Rectangle paletteRecs[CONTROLS_TYPE_NUM] = {
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 5, 125, 50 },     // WindowBox
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 65, 125, 30 },    // GroupBox
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 105, 125, 25 },   // Line
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 140, 125, 35 },   // Panel
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 185, 126, 25 },   // Label
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 220, 125, 30 },   // Button
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 260, 90, 25 },    // Toggle
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 295, 125, 25 },   // ToggleGroup
        (Rectangle){ palettePanel.x + 105, palettePanel.y + 265, 15, 15 },  // CheckBox
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 330, 125, 25 },   // ComboBox
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 365, 125, 25 },   // DropdownBox
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 400, 125, 25 },   // Spinner
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 435, 125, 25 },   // ValueBox
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 470, 125, 25 },   // TextBox
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 505, 125, 15 },   // Slider
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 530, 125, 15 },   // SliderBar
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 555, 125, 15 },   // ProgressBar
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 580, 125, 25 },   // StatusBar
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 615, 125, 75 },   // ListView
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 700, 95, 95 },    // ColorPicker
        (Rectangle){ palettePanel.x + 5, palettePanel.y + 800, 125, 30 }    // DummyRec
    };

    // Tracemap (background image for reference) variables
    Texture2D tracemap = { 0 };
    Rectangle tracemapRec = { 0 };
    bool tracemapEditMode = false;
    float tracemapFade = 0.5f;

    // loadedTexture for checking if texture is a tracemap or a style
    Texture2D loadedTexture = { 0 };

    // Very basic undo system
    // Undo last-selected rectangle changes
    // Undo text/name editing on cancel (KEY_ESC)
    int undoSelectedControl = -1;
    Rectangle undoLastRec;
    char prevControlText[MAX_CONTROL_TEXT_LENGTH];
    char prevControlName[MAX_CONTROL_NAME_LENGTH];

    // Close layout window variables
    bool closingWindowActive = false;

    // Generate code options window variables
    Vector2 exportWindowPos = { 50, 50 };
    bool generateWindowActive = false;
    int toolNameSize = 32;
    int toolVersionSize = 32;
    int companySize = 32;
    int toolDescriptionSize = 32;

    // Generate code configuration
    GuiLayoutConfig config;
    memset(&config, 0, sizeof(GuiLayoutConfig));
    config.width = 800;
    config.height = 600;
    strcpy(config.name, "layout_file_name");
    strcpy(config.version, TOOL_VERSION_TEXT);
    strcpy(config.company, "raylib technologies");
    strcpy(config.description, "tool description");
    config.defineRecs = false;
    config.exportAnchors = false;
    config.exportAnchor0 = false;
    config.fullComments = false;
    config.defineTexts = false;
    config.fullVariables = false;

    // Delete current layout and reset variables
    bool resetWindowActive = false;
    bool resetLayout = false;

    // Controls temp variables
    int dropdownBoxActive = 0;
    int spinnerValue = 0;
    int valueBoxValue = 0;
    int listViewScrollIndex = 0;
    int listViewActive = 0;

    bool widthBoxEditMode = false;
    bool heightBoxEditMode = false;

    SetTargetFPS(120);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!exitWindow)             // Detect window close button
    {
        // Dropped files logic
        //----------------------------------------------------------------------------------
        if (IsFileDropped())
        {
            int fileCount = 0;
            char **droppedFiles = { 0 };
            char droppedFileName[256];
            droppedFiles = GetDroppedFiles(&fileCount);
            strcpy(droppedFileName, droppedFiles[0]);

            if (IsFileExtension(droppedFileName, ".rgl"))
            {
                selectedControl = -1;
                LoadLayout(droppedFileName);
                strcpy(loadedFileName, droppedFileName);
                SetWindowTitle(FormatText("rGuiLayout v%s - %s", TOOL_VERSION_TEXT, GetFileName(loadedFileName)));
            }
            else if (IsFileExtension(droppedFileName, ".rgs")) GuiLoadStyle(droppedFileName);
            else if (IsFileExtension(droppedFileName, ".png"))
            {
                if (loadedTexture.id > 0) UnloadTexture(loadedTexture);
                loadedTexture = LoadTexture(droppedFileName);

                if (loadedTexture.width == 64 && loadedTexture.height == 16) GuiLoadStylePaletteImage(droppedFileName);
                else
                {
                    if (tracemap.id > 0) UnloadTexture(tracemap);
                    tracemap = LoadTexture(droppedFileName);
                }

                UnloadTexture(loadedTexture);
                SetTextureFilter(tracemap, FILTER_BILINEAR);

                tracemapRec.width = tracemap.width;
                tracemapRec.height = tracemap.height;
            }

            ClearDroppedFiles();
        }
        //----------------------------------------------------------------------------------

        // Keyboard shortcuts
        //----------------------------------------------------------------------------------
        /*
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) BtnSaveStyle(false);    // Show save style dialog (.rgs text)
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O)) BtnLoadStyle();         // Show load style dialog (.rgs)
        //if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E)) BtnExportStyle(wave);   // Show export style dialog (.rgs, .png, .h)
        // TODO: Support style name definition!
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E)) ExportStyle("style_table.png", CONTROLS_TABLE_IMAGE);
        */
        //----------------------------------------------------------------------------------

        // Basic program flow logic
        //----------------------------------------------------------------------------------
        framesCounterSnap++;
        mouse = GetMousePosition();
        if (WindowShouldClose()) exitWindow = true;

        // SHORTCUTS
        // ---------------------------------------------------------------------------------------------

        if (!textEditMode && !nameEditMode)
        {
            // TODO: review -- Show save layout message window on ESC
            if (IsKeyPressed(KEY_ESCAPE))
            {
                if (layout.controlsCount <= 0) exitWindow = false;
                else if (generateWindowActive) generateWindowActive = false;
                else if (resetWindowActive) resetWindowActive = false;
                else
                {
                    closingWindowActive = !closingWindowActive;
                    selectedControl = -1;
                }
            }

            // Enables or disables snapMode if not in textEditMode
            if (IsKeyPressed(KEY_S)) 
            {
                snapMode = !snapMode;
                if (snapMode) 
                {
                    movePixel = GRID_LINE_SPACING;
                    movePerFrame = MOVEMENT_FRAME_SPEED;
                }
                else 
                {
                    movePixel = 1;
                    movePerFrame = 1;
                }
            }

            // Enables or disables position reference information(anchor reference or global reference)
            if (IsKeyPressed(KEY_F)) showGlobalPos = !showGlobalPos;

            // Enable anchor mode editing
            if (IsKeyPressed(KEY_A)) anchorEditMode = true;
            if (IsKeyReleased(KEY_A)) anchorEditMode = false;
            
            // Enable/disable order edit mode
            if (IsKeyPressed(KEY_LEFT_ALT)) orderEditMode = true;
            if (IsKeyReleased(KEY_LEFT_ALT))  orderEditMode = false;
            
            // Enable/diable precision mode
            if (IsKeyDown(KEY_LEFT_SHIFT)) precisionMode = true;
            if (IsKeyReleased(KEY_LEFT_SHIFT)) precisionMode = false;
            
            // Enable/disable size modifier mode
            if (IsKeyDown(KEY_LEFT_CONTROL)) resizeMode = true;
            if (IsKeyReleased(KEY_LEFT_CONTROL)) resizeMode = false;
                
        }

        // Toggle help info
        if (IsKeyPressed(KEY_TAB))
        {
            helpCounter = 0;
            helpStartPositionX = helpPositionX;

            if (helpActive) helpDeltaPositionX = -300 - helpStartPositionX;
            else helpDeltaPositionX = 0 - helpStartPositionX;

            helpActive = !helpActive;
        }

        if (helpCounter <= PANELS_EASING_FRAMES)
        {
            helpCounter++;
            helpPositionX = (int)EaseCubicInOut(helpCounter, helpStartPositionX, helpDeltaPositionX, PANELS_EASING_FRAMES);
        }

        // Toggle palette selector
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && (focusedAnchor == -1) && (focusedControl == -1))
        {
            paletteCounter = 0;
            paletteStartPositionX = palettePanel.x;

            if (paletteActive) paletteDeltaPositionX = GetScreenWidth() + 130 - paletteStartPositionX;
            else paletteDeltaPositionX = (GetScreenWidth() - 145) - paletteStartPositionX;

            paletteActive = !paletteActive;
        }        
        if (paletteCounter <= PANELS_EASING_FRAMES)
        {
            paletteCounter++;
            palettePanel.x = (int)EaseCubicInOut(paletteCounter, paletteStartPositionX, paletteDeltaPositionX, PANELS_EASING_FRAMES);

            for (int i = 0; i < CONTROLS_TYPE_NUM; i++)
            {
                if (i == 8) paletteRecs[i].x = palettePanel.x + 105;
                else paletteRecs[i].x = palettePanel.x + 5;
            }
        }
        // Controls palette selector logic
        else if (paletteActive)
        {
            for (int i = 0; i < CONTROLS_TYPE_NUM; i++)
            {
                if (CheckCollisionPointRec(mouse, paletteRecs[i]))
                {
                    paletteSelect = i;
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedType = i;
                    break;
                }
                else paletteSelect = -1;
            }
        }

        // Exit textEditMode TODO: Move this
        if (textEditMode)
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                textEditMode = false;
                framesCounter = 0;
            }
            else if (IsKeyPressed(KEY_ESCAPE))
            {
                textEditMode = false;
                framesCounter = 0;
                memset(layout.controls[selectedControl].text, 0, MAX_CONTROL_TEXT_LENGTH);
                strcpy(layout.controls[selectedControl].text, prevControlText);
            }
        }

        // Exit nameEditMode TODO: Move this
        if (nameEditMode)
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                nameEditMode = false;
                framesCounter = 0;
            }
            else if (IsKeyPressed(KEY_ESCAPE))
            {
                nameEditMode = false;
                framesCounter = 0;
                memset(layout.controls[selectedControl].name, 0, MAX_CONTROL_NAME_LENGTH);
                strcpy(layout.controls[selectedControl].name, prevControlName);
            }
        }
        // ---------------------------------------------------------------------------------------------

        // MOUSE SNAP
        if (snapMode)
        {
            int offsetX = (int)mouse.x%GRID_LINE_SPACING;
            int offsetY = (int)mouse.y%GRID_LINE_SPACING;

            if (offsetX >= GRID_LINE_SPACING/2) mouse.x += (GRID_LINE_SPACING - offsetX);
            else mouse.x -= offsetX;

            if (offsetY >= GRID_LINE_SPACING/2) mouse.y += (GRID_LINE_SPACING - offsetY);
            else mouse.y -= offsetY;
        }
        
        // DEFAULT CONTROL
        // ---------------------------------------------------------------------------------------------
        
        // Updates the selected type with the mouse wheel
        if (focusedControl == -1) selectedType -= GetMouseWheelMove();

        if (selectedType < GUI_WINDOWBOX) selectedType = GUI_WINDOWBOX;
        else if (selectedType > GUI_DUMMYREC) selectedType = GUI_DUMMYREC;

        selectedTypeDraw = selectedType;  
        
        // Updates the default rectangle position
        defaultRec[selectedType].x = mouse.x - defaultRec[selectedType].width/2;
        defaultRec[selectedType].y = mouse.y - defaultRec[selectedType].height/2;
        
        if (snapMode)
        {
            int offsetX = (int)defaultRec[selectedType].x%movePixel;
            int offsetY = (int)defaultRec[selectedType].y%movePixel;
            
            if (offsetX >= GRID_LINE_SPACING/2) defaultRec[selectedType].x += (GRID_LINE_SPACING - offsetX);
            else defaultRec[selectedType].x -= offsetX;

            if (offsetY >= GRID_LINE_SPACING/2) defaultRec[selectedType].y += (GRID_LINE_SPACING - offsetY);
            else defaultRec[selectedType].y -= offsetY;
        }
        // ---------------------------------------------------------------------------------------------
        
        // CONTROLS
        // ---------------------------------------------------------------------------------------------
        
        // TODO: Review this condition...
        if (!CheckCollisionPointRec(mouse, palettePanel) && !textEditMode && !nameEditMode && !closingWindowActive && !generateWindowActive && !resetWindowActive && !tracemapEditMode)
        {
            // Checks if mouse is over a control
            if (!dragMode)
            {
                focusedControl = -1;
                for (int i = layout.controlsCount; i >= 0; i--)
                {
                    if (!layout.controls[i].ap->hidding)
                    {
                        int layoutHeight = layout.controls[i].rec.height;
                        if (layout.controls[i].type == GUI_WINDOWBOX) layoutHeight = 24;
                        
                        if (CheckCollisionPointRec(mouse, (Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layoutHeight }))
                        {
                            focusedControl = i;
                            break;
                        }
                    }
                } 
            } 
            
            if (focusedControl == -1)
            {
                if (focusedAnchor == -1 && selectedAnchor == -1 && selectedControl == -1)
                {
                    // Create new control
                    if (!anchorEditMode && !anchorLinkMode)
                    {
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                        {
                            // Config new control
                            layout.controls[layout.controlsCount].id = layout.controlsCount;
                            layout.controls[layout.controlsCount].type = selectedType;
                            layout.controls[layout.controlsCount].rec = (Rectangle){ defaultRec[selectedType].x, defaultRec[selectedType].y, defaultRec[selectedType].width, defaultRec[selectedType].height };
                            
                            if ((layout.controls[layout.controlsCount].type == GUI_LABEL) || (layout.controls[layout.controlsCount].type == GUI_TEXTBOX) || (layout.controls[layout.controlsCount].type == GUI_BUTTON) || (layout.controls[layout.controlsCount].type == GUI_TOGGLE)
                                || (layout.controls[layout.controlsCount].type == GUI_GROUPBOX) || (layout.controls[layout.controlsCount].type == GUI_WINDOWBOX) || (layout.controls[layout.controlsCount].type == GUI_STATUSBAR) || (layout.controls[layout.controlsCount].type == GUI_DUMMYREC)) strcpy(layout.controls[layout.controlsCount].text, "SAMPLE TEXT");
                            
                            strcpy(layout.controls[layout.controlsCount].name, FormatText("%s%03i", controlTypeNameLow[layout.controls[layout.controlsCount].type], layout.controlsCount));
                            
                            layout.controls[layout.controlsCount].ap = &layout.anchors[0];        // Default anchor point (0, 0)

                            // If we create new control inside a windowbox, then anchor the new control to the windowbox anchor
                            for (int i = layout.controlsCount; i >= 0; i--)
                            {
                                if (layout.controls[i].type == GUI_WINDOWBOX)
                                {
                                    if (CheckCollisionPointRec(mouse, (Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }))
                                    {
                                        layout.controls[layout.controlsCount].ap = layout.controls[i].ap;
                                        break;
                                    }                                    
                                }
                            }

                            // Create anchor for windowbox control if we can
                            if (layout.anchorsCount < MAX_ANCHOR_POINTS && layout.controls[layout.controlsCount].type == GUI_WINDOWBOX)
                            {
                                for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
                                {
                                    if (!layout.anchors[i].enabled)
                                    {
                                        layout.anchors[i].x = layout.controls[layout.controlsCount].rec.x;
                                        layout.anchors[i].y = layout.controls[layout.controlsCount].rec.y;

                                        if (snapMode)
                                        {
                                            int offsetX = layout.anchors[i].x%GRID_LINE_SPACING;
                                            int offsetY = layout.anchors[i].y%GRID_LINE_SPACING;

                                            if (offsetX >= GRID_LINE_SPACING/2) layout.anchors[i].x += (GRID_LINE_SPACING - offsetX);
                                            else layout.anchors[i].x -= offsetX;

                                            if (offsetY >= GRID_LINE_SPACING/2) layout.anchors[i].y += (GRID_LINE_SPACING - offsetY);
                                            else layout.anchors[i].y -= offsetY;
                                        }

                                        layout.controls[layout.controlsCount].rec.x = layout.anchors[i].x;
                                        layout.controls[layout.controlsCount].rec.y = layout.anchors[i].y;

                                        layout.anchors[i].enabled = true;
                                        layout.controls[layout.controlsCount].ap = &layout.anchors[i];
                                        
                                        layout.anchorsCount++;
                                        break;
                                    }
                                }
                            }

                            layout.controls[layout.controlsCount].rec.x -= layout.controls[layout.controlsCount].ap->x;
                            layout.controls[layout.controlsCount].rec.y -= layout.controls[layout.controlsCount].ap->y;
                            layout.controlsCount++;
                        }
                    }
                }
            }
            else //focusedControl != -1
            {
                if (selectedControl == -1)
                {
                    // Change controls layer order (position inside array)
                    if (orderEditMode)
                    {
                        int newOrder = 0;
                        if (IsKeyPressed(KEY_UP)) newOrder = 1;
                        else if (IsKeyPressed(KEY_DOWN)) newOrder = -1;
                        else newOrder -= GetMouseWheelMove();
                        
                        if ((newOrder > 0) && (focusedControl < layout.controlsCount - 1))
                        {
                            // Move control towards beginning of array
                            GuiControl auxControl = layout.controls[focusedControl];
                            layout.controls[focusedControl] = layout.controls[focusedControl + 1];
                            layout.controls[focusedControl].id -= 1;
                            layout.controls[focusedControl + 1] = auxControl;
                            layout.controls[focusedControl + 1].id += 1;
                            selectedControl = -1;
                        }
                        else if ((newOrder < 0) && (focusedControl > 0))
                        {
                            // Move control towards end of array
                            GuiControl auxControl = layout.controls[focusedControl];
                            layout.controls[focusedControl] = layout.controls[focusedControl - 1];
                            layout.controls[focusedControl].id += 1;
                            layout.controls[focusedControl - 1] = auxControl;
                            layout.controls[focusedControl - 1].id -= 1;
                            selectedControl = -1;
                        }                   
                    }
                }
            }
            
            // (Des)select control
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) 
            {
                selectedControl = focusedControl;
                if (focusedAnchor != -1 || anchorLinkMode || anchorEditMode) selectedControl = -1;
            } 
            
            if (selectedControl != -1)
            {
                if (!anchorLinkMode)
                {
                    if (dragMode)
                    {
                        // Drag controls
                        
                        layout.controls[selectedControl].rec.x = prevControlPosition.x + (mouse.x - panControlOffset.x);
                        layout.controls[selectedControl].rec.y = prevControlPosition.y + (mouse.y - panControlOffset.y);
                        
                        int offsetX = (int)layout.controls[selectedControl].rec.x%movePixel;
                        int offsetY = (int)layout.controls[selectedControl].rec.y%movePixel;
                        
                        if (offsetX >= movePixel/2) layout.controls[selectedControl].rec.x += (movePixel - offsetX);
                        else layout.controls[selectedControl].rec.x -= offsetX;

                        if (offsetY >= movePixel/2) layout.controls[selectedControl].rec.y += (movePixel - offsetY);
                        else layout.controls[selectedControl].rec.y -= offsetY;
                        
                        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragMode = false;
                    }
                    else
                    {
                        // Resize controls
                        if (resizeMode)
                        {
                            int offsetX = (int)layout.controls[selectedControl].rec.width%movePixel;
                            int offsetY = (int)layout.controls[selectedControl].rec.height%movePixel;
                                    
                            if (precisionMode)
                            {                            
                                if (IsKeyPressed(KEY_RIGHT)) layout.controls[selectedControl].rec.width += (movePixel - offsetX);
                                else if (IsKeyPressed(KEY_LEFT))
                                {
                                    if (offsetX == 0) offsetX = movePixel;
                                    layout.controls[selectedControl].rec.width -= offsetX;
                                }
                                
                                if (IsKeyPressed(KEY_DOWN)) layout.controls[selectedControl].rec.height += (movePixel - offsetY);
                                else if (IsKeyPressed(KEY_UP)) 
                                {
                                    if (offsetY == 0) offsetY = movePixel;
                                    layout.controls[selectedControl].rec.height -= offsetY;
                                }
                                
                                framesCounterMovement = 0;
                            }       
                            else
                            {
                                framesCounterMovement++;
                                
                                if ((framesCounterMovement%movePerFrame) == 0)
                                {
                                    if (IsKeyDown(KEY_RIGHT)) layout.controls[selectedControl].rec.width += (movePixel - offsetX);
                                    else if (IsKeyDown(KEY_LEFT))
                                    {
                                        if (offsetX == 0) offsetX = movePixel;
                                        layout.controls[selectedControl].rec.width -= offsetX;
                                    }
                                    
                                    if (IsKeyDown(KEY_DOWN)) layout.controls[selectedControl].rec.height += (movePixel - offsetY);
                                    else if (IsKeyDown(KEY_UP))
                                    {
                                        if (offsetY == 0) offsetY = movePixel;
                                        layout.controls[selectedControl].rec.height -= offsetY;
                                    }
                                    
                                    framesCounterMovement = 0;
                                }
                            }                         
                        }
                        else
                        {
                            int offsetX = (int)layout.controls[selectedControl].rec.x%movePixel;                            
                            int offsetY = (int)layout.controls[selectedControl].rec.y%movePixel;
                            
                            // Move control with arrows
                            if (precisionMode)
                            {                                
                                if (IsKeyPressed(KEY_RIGHT))  layout.controls[selectedControl].rec.x += (movePixel - offsetX);
                                else if (IsKeyPressed(KEY_LEFT)) 
                                {
                                    if (offsetX == 0) offsetX = movePixel;
                                    layout.controls[selectedControl].rec.x -= offsetX;
                                }                                
                                
                                if (IsKeyPressed(KEY_DOWN)) layout.controls[selectedControl].rec.y += (movePixel - offsetY);
                                else if (IsKeyPressed(KEY_UP)) 
                                {
                                    if (offsetY == 0) offsetY = movePixel;
                                    layout.controls[selectedControl].rec.y -= offsetY;
                                }
                                
                                framesCounterMovement = 0;
                            }
                            else 
                            {
                                framesCounterMovement++;
                                
                                if ((framesCounterMovement%movePerFrame) == 0)
                                {
                                    if (IsKeyDown(KEY_RIGHT)) layout.controls[selectedControl].rec.x += (movePixel - offsetX);
                                    else if (IsKeyDown(KEY_LEFT))
                                    {
                                        if (offsetX == 0) offsetX = movePixel;
                                        layout.controls[selectedControl].rec.x -= offsetX;
                                    } 
                                    
                                    if (IsKeyDown(KEY_DOWN)) layout.controls[selectedControl].rec.y += (movePixel - offsetY);
                                    else if (IsKeyDown(KEY_UP))
                                    {
                                        if (offsetY == 0) offsetY = movePixel;
                                        layout.controls[selectedControl].rec.y -= offsetY;
                                    }
                                    
                                    framesCounterMovement = 0;
                                }
                            } 
                        
                            // Unlinks the control selected from its current anchor
                            if (layout.controls[selectedControl].ap->id != 0 && IsKeyPressed(KEY_U))
                            {
                                layout.controls[selectedControl].rec.x += layout.controls[selectedControl].ap->x;
                                layout.controls[selectedControl].rec.y += layout.controls[selectedControl].ap->y;
                                layout.controls[selectedControl].ap = &layout.anchors[0];
                            }
                            
                            // Delete selected control
                            if (IsKeyPressed(KEY_DELETE))
                            {
                                for (int i = selectedControl; i < layout.controlsCount; i++)
                                {
                                    layout.controls[i].type = layout.controls[i + 1].type;
                                    layout.controls[i].rec = layout.controls[i + 1].rec;
                                    memset(layout.controls[i].text, 0, MAX_CONTROL_TEXT_LENGTH);
                                    memset(layout.controls[i].name, 0, MAX_CONTROL_NAME_LENGTH);
                                    strcpy(layout.controls[i].text, layout.controls[i + 1].text);
                                    strcpy(layout.controls[i].name, layout.controls[i + 1].name);
                                    layout.controls[i].ap = layout.controls[i + 1].ap;
                                }

                                layout.controlsCount--;
                                focusedControl = -1;
                                selectedControl = -1;
                            }
                            
                            // Activate dragMode  mode
                            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                            {
                                panControlOffset = mouse;
                                prevControlPosition = (Vector2){ layout.controls[selectedControl].rec.x, layout.controls[selectedControl].rec.y };
                                dragMode = true;
                            }
                            
                            // Activate anchor link mode
                            if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) anchorLinkMode = true;
                        }  
                    }
                }
                else // anchorLinkMode == true
                {
                    if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
                    {
                        anchorLinkMode = false;

                        if (focusedAnchor != -1)
                        {
                            layout.controls[selectedControl].rec.x += layout.controls[selectedControl].ap->x;
                            layout.controls[selectedControl].rec.y += layout.controls[selectedControl].ap->y;
                            layout.controls[selectedControl].ap = &layout.anchors[focusedAnchor];
                            layout.controls[selectedControl].rec.x -= layout.anchors[focusedAnchor].x;
                            layout.controls[selectedControl].rec.y -= layout.anchors[focusedAnchor].y;
                        }
                    }
                }
            }
            else //selectedControl == -1
            {
                
            }
        }  

        // Check if control has text to edit
        if (textEditMode)
        {
            // Locks the selectedControl for text editing
            selectedControl = storedControl;
            int key = GetKeyPressed();
            int keyCount = strlen(layout.controls[selectedControl].text); // Keeps track of text length

            // TODO: Move cursor properly on text edition
            if (IsKeyPressed(KEY_LEFT)) textArrayPos--;
            else if (IsKeyPressed(KEY_RIGHT)) textArrayPos++;

            if (textArrayPos >= 0) textArrayPos = 0;
            else if (textArrayPos <= keyCount*(-1)) textArrayPos = keyCount*(-1);

            // Replaces characters with pressed keys or '\0' in case of backspace
            // NOTE: Only allow keys in range [32..125]
            if (((key >= 32) && (key <= 125)) && (keyCount < MAX_CONTROL_TEXT_LENGTH - 1))
            {
                for (int i = keyCount; i > keyCount + textArrayPos; i--)
                {
                    layout.controls[selectedControl].text[i] = layout.controls[selectedControl].text[i - 1];
                }
                layout.controls[selectedControl].text[keyCount + textArrayPos] = (unsigned char)key;
            }

            if ((textArrayPos == 0))
            {
                if ((keyCount > 0) && IsKeyPressed(KEY_BACKSPACE))
                {
                    layout.controls[selectedControl].text[keyCount - 1] = '\0';
                    framesCounterSnap = 0;
                    if (keyCount < 0) keyCount = 0;
                }
                else if ((keyCount > 0) && IsKeyDown(KEY_BACKSPACE))
                {
                    if ((framesCounterSnap > 60) && ((framesCounterSnap%4) == 0)) layout.controls[selectedControl].text[keyCount - 1] = '\0';
                    if (keyCount < 0) keyCount = 0;
                }
            }
            else if(textArrayPos*(-1) != keyCount)
            {
                if ((keyCount > 0) && IsKeyPressed(KEY_BACKSPACE))
                {
                    framesCounterSnap = 0;
                    for (int i = -1; i <= -textArrayPos; i++)
                    {
                        layout.controls[selectedControl].text[keyCount + textArrayPos + i] = layout.controls[selectedControl].text[keyCount + textArrayPos + i + 1];
                        if (i == -textArrayPos) layout.controls[selectedControl].text[keyCount - 1] = '\0';
                    }
                }
                else if ((keyCount > 0) && IsKeyDown(KEY_BACKSPACE))
                {
                    if ((framesCounterSnap > 60) && ((framesCounterSnap%4) == 0))
                    {
                        for (int i = -1; i <= -textArrayPos; i++)
                        {
                            layout.controls[selectedControl].text[keyCount + textArrayPos + i] = layout.controls[selectedControl].text[keyCount + textArrayPos + i + 1];
                            if (i == -textArrayPos) layout.controls[selectedControl].text[keyCount - 1] = '\0';
                        }
                    }
                }
            }

            // Used to show the cursor('|') in textEditMode
            if (keyCount < MAX_CONTROL_TEXT_LENGTH) framesCounter++;
            else if (keyCount == MAX_CONTROL_TEXT_LENGTH) framesCounter = 21;
        }

        if ((nameEditMode))
        {
            // Locks the selectedControl for text editing
            selectedControl = storedControl;
            int key = GetKeyPressed();
            int keyCount = strlen(layout.controls[selectedControl].name); // Keeps track of name length

            // Replaces characters with pressed keys or '\0' in case of backspace
            // NOTE: Only allow keys in range [48..57], [65..90] and [97..122]
            if ((((key >= 48) && (key <= 57)) || ((key >= 65) && (key <= 90)) || ((key >= 97) && (key <= 122))) && (keyCount < MAX_CONTROL_NAME_LENGTH - 1))
            {
                layout.controls[selectedControl].name[keyCount] = (unsigned char)key;
            }

            if ((keyCount > 0) && IsKeyPressed(KEY_BACKSPACE))
            {
                layout.controls[selectedControl].name[keyCount - 1] = '\0';
                if (keyCount < 0) keyCount = 0;
            }

            // Used to show the cursor('|') in textEditMode
            if (keyCount < MAX_CONTROL_NAME_LENGTH) framesCounter++;
            else if (keyCount == MAX_CONTROL_NAME_LENGTH) framesCounter = 21;
        }

        // Turn on text edit mode
        if (IsKeyPressed(KEY_T) && !nameEditMode && (selectedControl != -1) && (!generateWindowActive) && (!anchorEditMode) &&
           ((layout.controls[selectedControl].type == GUI_LABEL) || (layout.controls[selectedControl].type == GUI_CHECKBOX) || (layout.controls[selectedControl].type == GUI_SLIDERBAR) || (layout.controls[selectedControl].type == GUI_SLIDER) || (layout.controls[selectedControl].type == GUI_TEXTBOX) || (layout.controls[selectedControl].type == GUI_BUTTON) || (layout.controls[selectedControl].type == GUI_TOGGLE) || (layout.controls[selectedControl].type == GUI_GROUPBOX) || (layout.controls[selectedControl].type == GUI_WINDOWBOX) || (layout.controls[selectedControl].type == GUI_STATUSBAR) || (layout.controls[selectedControl].type == GUI_DUMMYREC)))
        {
            textEditMode = true;
            textArrayPos = 0;
            storedControl = selectedControl;
            strcpy(prevControlText, layout.controls[selectedControl].text);
        }

        // Turn on name edit mode
        if (IsKeyPressed(KEY_N) && (!IsKeyDown(KEY_LEFT_CONTROL)) && (!resetWindowActive) && !textEditMode && (selectedControl != -1) && (!generateWindowActive))
        {
            nameEditMode = true;
            strcpy(prevControlName, layout.controls[selectedControl].name);
            storedControl = selectedControl;
        }        
        // -------------------------------------------------------------------------------------------------------------------
       
        // ANCHOR POINTS 
        // -------------------------------------------------------------------------------------------------------------------
        if (!generateWindowActive && !resetWindowActive && !closingWindowActive && !textEditMode && !nameEditMode)
        {
            // Checks if mouse is over an anchor
            if (!dragMode)
            {
                focusedAnchor = -1;
                for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
                {
                    if (layout.anchors[i].enabled)
                    {
                        if (CheckCollisionPointCircle(mouse, (Vector2){ layout.anchors[i].x, layout.anchors[i].y }, ANCHOR_RADIUS))
                        {
                            focusedAnchor = i;
                        }
                    }
                }
            }
            
            // Editing anchors
            if (focusedAnchor == -1)
            {
                if (focusedControl == -1)
                {
                    // Create new anchor
                    if (!anchorLinkMode && anchorEditMode && layout.anchorsCount < MAX_ANCHOR_POINTS)
                    {
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                        {
                            layout.anchorsCount++;
                            for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
                            {
                                if (!layout.anchors[i].enabled)
                                {
                                    layout.anchors[i].x = mouse.x;
                                    layout.anchors[i].y = mouse.y;
                                    layout.anchors[i].enabled = true;
                                    //focusedAnchor = i;
                                    break;
                                }
                            }
                        }
                    }
                }
            }            
            // (Des)select anchor
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) 
            {
                selectedAnchor = focusedAnchor;
                if (anchorLinkMode) selectedAnchor = -1;
            }

            // Actions with one anchor selected
            if (selectedAnchor != -1)
            {
                // Link anchor
                if (!anchorLinkMode)
                {          
                    if (dragMode)
                    {
                        layout.anchors[selectedAnchor].x = mouse.x;
                        layout.anchors[selectedAnchor].y = mouse.y;
                        
                        // Move anchor without moving controls
                        if (anchorMoveMode && !anchorEditMode)
                        {
                            for (int i = 0; i < layout.controlsCount; i++)
                            {
                                if (layout.controls[i].ap->id == 9) //auxAnchor ID
                                {
                                    layout.controls[i].rec.x += layout.controls[i].ap->x;
                                    layout.controls[i].rec.y += layout.controls[i].ap->y;
                                    layout.controls[i].ap = &layout.anchors[selectedAnchor];
                                    layout.controls[i].rec.x -= layout.anchors[selectedAnchor].x;
                                    layout.controls[i].rec.y -= layout.anchors[selectedAnchor].y;
                                }
                            }
                            anchorMoveMode = false;
                        }
                        // Move anchor without moving controls
                        if (!anchorMoveMode && anchorEditMode) 
                        {
                            anchorMoveMode = true;
                            
                            for (int i = 0; i < layout.controlsCount; i++)
                            {
                                if (layout.controls[i].ap->id == selectedAnchor)
                                {
                                    layout.controls[i].rec.x += layout.controls[i].ap->x;
                                    layout.controls[i].rec.y += layout.controls[i].ap->y;
                                    layout.controls[i].ap = &auxAnchor;
                                }
                            }
                        }
                        
                        // Exit anchor position edit mode
                        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) 
                        { 
                            // If moving only the anchor, relink with controls
                            if(anchorMoveMode)
                            {
                                for (int i = 0; i < layout.controlsCount; i++)
                                {
                                    if (layout.controls[i].ap->id == 9) //auxAnchor ID
                                    {
                                        layout.controls[i].rec.x += layout.controls[i].ap->x;
                                        layout.controls[i].rec.y += layout.controls[i].ap->y;
                                        layout.controls[i].ap = &layout.anchors[selectedAnchor];
                                        layout.controls[i].rec.x -= layout.anchors[selectedAnchor].x;
                                        layout.controls[i].rec.y -= layout.anchors[selectedAnchor].y;
                                    }
                                }                                    
                                anchorMoveMode = false;
                            }                                
                            dragMode = false;
                        }
                    }
                    else
                    {
                        int offsetX = (int)layout.anchors[selectedAnchor].x%movePixel;                            
                        int offsetY = (int)layout.anchors[selectedAnchor].y%movePixel;
                        // Move anchor with arrows once
                        if (precisionMode)
                        {
                            if (IsKeyPressed(KEY_RIGHT)) layout.anchors[selectedAnchor].x+= (movePixel - offsetX);
                            else if (IsKeyPressed(KEY_LEFT))
                            {
                                if (offsetX == 0) offsetX = movePixel;
                                layout.anchors[selectedAnchor].x-= offsetX;
                            }                            
                            
                            if (IsKeyPressed(KEY_DOWN)) layout.anchors[selectedAnchor].y+= (movePixel - offsetY);
                            else if (IsKeyPressed(KEY_UP)) 
                            {
                                if (offsetY == 0) offsetY = movePixel;
                                layout.anchors[selectedAnchor].y-= offsetY;
                            }
                            
                            framesCounterMovement = 0;
                        }
                        // Move anchor with arrows 
                        else 
                        {
                            framesCounterMovement++;
                            
                            if ((framesCounterMovement%movePerFrame) == 0)
                            {
                                if (IsKeyDown(KEY_RIGHT)) layout.anchors[selectedAnchor].x += (movePixel - offsetX);
                                else if (IsKeyDown(KEY_LEFT))
                                {
                                    if (offsetX == 0) offsetX = movePixel;
                                    layout.anchors[selectedAnchor].x -= offsetX;
                                }   
                                
                                
                                if (IsKeyDown(KEY_DOWN)) layout.anchors[selectedAnchor].y +=(movePixel - offsetY);
                                else if (IsKeyDown(KEY_UP))
                                {
                                    if (offsetY == 0) offsetY = movePixel;
                                    layout.anchors[selectedAnchor].y -= offsetY;
                                }
                                
                                framesCounterMovement = 0;
                            }
                        }
                        // Activate anchor position edit mode
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) dragMode = true;
                        // Activate anchor link mode
                        else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) anchorLinkMode = true;
                        
                        // Hide/Unhide anchors
                        if (IsKeyPressed(KEY_H)) layout.anchors[selectedAnchor].hidding = !layout.anchors[selectedAnchor].hidding;
                        
                         // Unlinks controls from selected anchor
                        if (IsKeyPressed(KEY_U))
                        {
                            for (int i = 0; i < layout.controlsCount; i++)
                            {
                                if (layout.controls[i].ap->id == selectedAnchor)
                                {
                                    layout.controls[i].rec.x += layout.controls[i].ap->x;
                                    layout.controls[i].rec.y += layout.controls[i].ap->y;
                                    layout.controls[i].ap = &layout.anchors[0];
                                }
                            }
                        }
                        // Delete anchor
                        if (IsKeyPressed(KEY_DELETE))
                        {
                            for (int i = 0; i < layout.controlsCount; i++)
                            {
                                if (layout.controls[i].ap->id == selectedAnchor)
                                {
                                    layout.controls[i].rec.x += layout.controls[i].ap->x;
                                    layout.controls[i].rec.y += layout.controls[i].ap->y;
                                    layout.controls[i].ap = &layout.anchors[0];
                                }
                            }
                            
                            layout.anchors[selectedAnchor].x = 0;
                            layout.anchors[selectedAnchor].y = 0;
                            layout.anchors[selectedAnchor].enabled = false;
                            layout.anchors[selectedAnchor].hidding = false;
                            
                            selectedAnchor = -1;
                            focusedAnchor = -1;
                        }
                    }
                }
                else // anchorLinkMode == true
                {
                    if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
                    {
                        anchorLinkMode = false;

                        if (focusedControl != -1)
                        {
                            layout.controls[focusedControl].rec.x += layout.controls[focusedControl].ap->x;
                            layout.controls[focusedControl].rec.y += layout.controls[focusedControl].ap->y;
                            layout.controls[focusedControl].ap = &layout.anchors[selectedAnchor];
                            layout.controls[focusedControl].rec.x -= layout.anchors[selectedAnchor].x;
                            layout.controls[focusedControl].rec.y -= layout.anchors[selectedAnchor].y;
                        }
                        //selectedAnchor = -1;
                    }
                }
            }
            else
            {
                // TODO: this is for anchors and controls...move this condition
                if (anchorLinkMode && IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) anchorLinkMode = false;
            }
        }        
        // -------------------------------------------------------------------------------------------------------------------

        // Checks the minimum size of the rec
        // TODO: Redesign this... looks very bad
        if (selectedControl != -1)
        {
            // Sets the minimum limit of the width
            if (layout.controls[selectedControl].type == GUI_LABEL || layout.controls[selectedControl].type == GUI_BUTTON || layout.controls[selectedControl].type == GUI_TOGGLE || layout.controls[selectedControl].type == GUI_TEXTBOX)
            {
                if (layout.controls[selectedControl].rec.width <  MeasureText(layout.controls[selectedControl].text, GuiGetStyle(DEFAULT, TEXT_SIZE))) layout.controls[selectedControl].rec.width = MeasureText(layout.controls[selectedControl].text , GuiGetStyle(DEFAULT, TEXT_SIZE));
            }
            else if (layout.controls[selectedControl].type == GUI_WINDOWBOX || layout.controls[selectedControl].type == GUI_GROUPBOX || layout.controls[selectedControl].type == GUI_STATUSBAR)
            {
                if (layout.controls[selectedControl].rec.width <  MeasureText(layout.controls[selectedControl].text, GuiGetStyle(DEFAULT, TEXT_SIZE)) + 31) layout.controls[selectedControl].rec.width = MeasureText(layout.controls[selectedControl].text , GuiGetStyle(DEFAULT, TEXT_SIZE)) + 31;
            }
            else if (layout.controls[selectedControl].type == GUI_CHECKBOX)
            {
                if (layout.controls[selectedControl].rec.width <= 10) layout.controls[selectedControl].rec.width = 10;
            }
            else if (layout.controls[selectedControl].rec.width <= 20) layout.controls[selectedControl].rec.width = 20;

            // Sets the minimum limit of the height
            if (layout.controls[selectedControl].type == GUI_WINDOWBOX)
            {
                if (layout.controls[selectedControl].rec.height < 50) layout.controls[selectedControl].rec.height = 50;
            }
            else if (layout.controls[selectedControl].type == GUI_PROGRESSBAR || layout.controls[selectedControl].type == GUI_SLIDER || layout.controls[selectedControl].type == GUI_SLIDERBAR || layout.controls[selectedControl].type == GUI_CHECKBOX || layout.controls[selectedControl].type == GUI_LINE)
            {
                if (layout.controls[selectedControl].rec.height <= 10 ) layout.controls[selectedControl].rec.height = 10;
            }
            else if (layout.controls[selectedControl].rec.height <= 20) layout.controls[selectedControl].rec.height = 20;
        }

        // Shows or hides the grid
        if (IsKeyPressed(KEY_G) && (!nameEditMode) && (!textEditMode) && (!generateWindowActive)) showGrid = !showGrid;

        // Duplicate selected control
        if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D)) && (selectedControl != -1) && !anchorEditMode)
        {
            // Add a copy of selected control
            layout.controls[layout.controlsCount].id = layout.controlsCount;
            layout.controls[layout.controlsCount].type = layout.controls[selectedControl].type;
            layout.controls[layout.controlsCount].rec = layout.controls[selectedControl].rec;
            layout.controls[layout.controlsCount].rec.x += 10;
            layout.controls[layout.controlsCount].rec.y += 10;
            strcpy(layout.controls[layout.controlsCount].text, layout.controls[selectedControl].text);
            strcpy(layout.controls[layout.controlsCount].name, FormatText("%s%03i", controlTypeNameLow[layout.controls[layout.controlsCount].type], layout.controlsCount));
            layout.controls[layout.controlsCount].ap = layout.controls[selectedControl].ap;            // Default anchor point (0, 0)

            layout.controlsCount++;
        }

        // Save layout file dialog logic
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_S)) DialogSaveLayout();
        else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
        {
            // TODO: Save only .rgl

            if (loadedFileName[0] == '\0') DialogSaveLayout();
            else SaveLayout(loadedFileName, false);
        }

        // Open laout file dialog logic
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O))
        {
            // Open file dialog
            const char *filters[] = { "*.rgl" };
            const char *fileName = tinyfd_openFileDialog("Load raygui layout file", "", 1, filters, "raygui Layout Files (*.rgl)", 0);

            if (fileName != NULL) LoadLayout(fileName);
        }

        // Activate code generation export window
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_ENTER) && !closingWindowActive) generateWindowActive = true;

        if (generateWindowActive)   // Keep window in the middle of screen
        {
            exportWindowPos.x = GetScreenWidth()/2 - 200;
            exportWindowPos.y = GetScreenHeight()/2 - 112;
        }

        // Tracemap texture control logic
        if (tracemap.id > 0)
        {
            // Toggles Texture editting mode between true or false
            if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_T)) tracemapEditMode = !tracemapEditMode;

            if (tracemapEditMode)
            {
                int offsetX = (int)mouse.x%GRID_LINE_SPACING;
                int offsetY = (int)mouse.y%GRID_LINE_SPACING;

                // Moves the texture with the mouse
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    panControlOffset = mouse;
                    prevControlPosition = (Vector2){ tracemapRec.x, tracemapRec.y };
                }

                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                {
                    tracemapRec.x = prevControlPosition.x + (mouse.x - panControlOffset.x);
                    tracemapRec.y = prevControlPosition.y + (mouse.y - panControlOffset.y);

                    if (snapMode)
                    {
                        if (offsetX >= GRID_LINE_SPACING/2) mouse.x += (GRID_LINE_SPACING - offsetX);
                        else mouse.x -= offsetX;

                        if (offsetY >= GRID_LINE_SPACING/2) mouse.y += (GRID_LINE_SPACING - offsetY);
                        else mouse.y -= offsetY;

                        offsetX = (int)tracemapRec.x%GRID_LINE_SPACING;
                        offsetY = (int)tracemapRec.y%GRID_LINE_SPACING;

                        if (offsetX >= GRID_LINE_SPACING/2) tracemapRec.x += (GRID_LINE_SPACING - offsetX);
                        else tracemapRec.x -= offsetX;

                        if (offsetY >= GRID_LINE_SPACING/2) tracemapRec.y += (GRID_LINE_SPACING - offsetY);
                        else tracemapRec.y -= offsetY;
                    }
                }

                // Moves and scales the texture with snap.
                if (IsKeyDown(KEY_LEFT_CONTROL))
                {
                    tracemapRec.height -= GetMouseWheelMove();
                    tracemapRec.width -= GetMouseWheelMove();
                }
                else
                {
                    tracemapRec.height -= 10*GetMouseWheelMove();
                    tracemapRec.width -= 10*GetMouseWheelMove();
                }

                tracemap.height = tracemapRec.height;
                tracemap.width = tracemapRec.width;

                // Change texture fade
                if (IsKeyDown(KEY_LEFT_CONTROL))
                {
                    if (precisionMode)
                    {
                        if (IsKeyPressed(KEY_LEFT)) tracemapRec.x--;
                        else if (IsKeyPressed(KEY_RIGHT)) tracemapRec.x++;

                        if (IsKeyPressed(KEY_UP)) tracemapRec.y--;
                        else if (IsKeyPressed(KEY_DOWN)) tracemapRec.y++;
                    }
                    else
                    {
                        if (IsKeyDown(KEY_LEFT)) tracemapRec.x--;
                        else if (IsKeyDown(KEY_RIGHT)) tracemapRec.x++;

                        if (IsKeyDown(KEY_UP)) tracemapRec.y--;
                        else if (IsKeyDown(KEY_DOWN)) tracemapRec.y++;
                    }
                }
                else
                {
                    if (IsKeyDown(KEY_LEFT)) tracemapFade-= 0.01f;
                    else if (IsKeyDown(KEY_RIGHT)) tracemapFade+=0.01f;
                }

                if (tracemapFade < 0) tracemapFade = 0;
                else if (tracemapFade > 1) tracemapFade = 1;

                // Deletes the texture and resets it
                if (IsKeyPressed(KEY_DELETE))
                {
                    UnloadTexture(tracemap);
                    tracemap.id = 0;
                    tracemapEditMode = false;
                    tracemapRec.x = 0;
                    tracemapRec.y = 0;
                }
            }
        }

        if ((IsKeyDown(KEY_LEFT_CONTROL)) && (IsKeyPressed(KEY_Z))) layout.controls[undoSelectedControl].rec = undoLastRec;

        if ((IsKeyDown(KEY_LEFT_CONTROL)) && (IsKeyPressed(KEY_N)) && (!generateWindowActive) && (!closingWindowActive))
        {
            resetWindowActive = true;
            resetLayout = false;
        }

        if (resetWindowActive && resetLayout)
        {
            // Resets all controls to default values
            for (int i = selectedControl; i < layout.controlsCount; i++)
            {
                layout.controls[i].id = 0;
                layout.controls[i].type = 0;
                layout.controls[i].rec = (Rectangle){ 0, 0, 0, 0 };
                memset(layout.controls[i].text, 0, MAX_CONTROL_TEXT_LENGTH);
                memset(layout.controls[i].name, 0, MAX_CONTROL_NAME_LENGTH);
                layout.controls[i].ap = &layout.anchors[0];  // By default, set parent anchor
            }

            // Resets anchor points to default values
            for (int i = 0; i < MAX_ANCHOR_POINTS; i++)
            {
                layout.anchors[i].x = 0;
                layout.anchors[i].y = 0;
                layout.anchors[i].enabled = false;
                layout.anchors[i].hidding = false;
            }

            SetWindowTitle(FormatText("rGuiLayout v%s", TOOL_VERSION_TEXT));
            strcpy(loadedFileName, "\0");
            layout.controlsCount = 0;
            resetWindowActive = false;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // TODO: Draw global app screen limits (black rectangle with black default anchor)

            if (showGrid) GuiGrid((Rectangle){ 0, 0, GetScreenWidth(), GetScreenHeight() }, GRID_LINE_SPACING, 5);

            // Draw the tracemap texture if loaded
            if (tracemap.id > 0)
            {
                DrawTexture(tracemap, tracemapRec.x, tracemapRec.y, Fade(WHITE, tracemapFade));

                // Draw the tracemap rectangle
                if (tracemapEditMode) DrawRectangleLines(tracemapRec.x, tracemapRec.y, tracemapRec.width, tracemapRec.height, RED);
                else DrawRectangleLines(tracemapRec.x, tracemapRec.y, tracemapRec.width, tracemapRec.height, GRAY);
            }

            // Draws the controls placed on the grid
            for (int i = 0; i < layout.controlsCount; i++)
            {
                if (!layout.controls[i].ap->hidding)
                {
                    switch (layout.controls[i].type)
                    {
                        case GUI_WINDOWBOX:
                        {
                            // TODO: review fade
                            GuiFade(0.8f);
                            GuiWindowBox((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text);
                            GuiFade(1.0f);
                        }break;
                        case GUI_GROUPBOX: GuiGroupBox((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text); break;
                        case GUI_LINE: GuiLine((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, 1); break;
                        case GUI_PANEL:
                        {
                            GuiFade(0.8f);
                            GuiPanel((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height });
                            GuiFade(1.0f);
                        }break;
                        case GUI_LABEL: GuiLabel((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text); break;
                        case GUI_BUTTON: GuiButton((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text); break;
                        case GUI_TOGGLE: GuiToggle((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text, false); break;
                        case GUI_TOGGLEGROUP: GuiToggleGroup((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, listData, 3, 1); break;
                        case GUI_CHECKBOX: GuiCheckBoxEx((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, false, layout.controls[i].text); break;
                        case GUI_COMBOBOX: GuiComboBox((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, listData, 3, 1); break;
                        case GUI_DROPDOWNBOX: GuiDropdownBox((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, listData, 3, &dropdownBoxActive, false); break;
                        case GUI_SPINNER: GuiSpinner((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, &spinnerValue, 42, 3, 25, false); break;
                        case GUI_VALUEBOX: GuiValueBox((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, &valueBoxValue, 42, 100, false); break;
                        case GUI_TEXTBOX: GuiTextBox((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text, MAX_CONTROL_TEXT_LENGTH, false); break;
                        case GUI_SLIDER: GuiSliderEx((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, 42, 0, 100, layout.controls[i].text, true); break;
                        case GUI_SLIDERBAR: GuiSliderBarEx((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, 40, 0, 100, layout.controls[i].text, true); break;
                        case GUI_PROGRESSBAR: GuiProgressBarEx((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, 40, 0, 100, true); break;
                        case GUI_STATUSBAR: GuiStatusBar((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text, 15); break;
                        case GUI_LISTVIEW: GuiListView((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, listViewData, 4, &listViewScrollIndex, &listViewActive, false); break;
                        case GUI_COLORPICKER: GuiColorPicker((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, RED); break;
                        case GUI_DUMMYREC: GuiDummyRec((Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text); break;
                        default: break;
                    }
                }
            }

            // Draw the default rectangle of the control selected
            if (!anchorLinkMode && (focusedControl == -1) && (focusedAnchor == -1) && (selectedAnchor == -1) && (selectedControl == -1) && !anchorEditMode && !tracemapEditMode && !closingWindowActive && !generateWindowActive && !(CheckCollisionPointRec(mouse, palettePanel)))
            {
                GuiFade(0.5f);
                switch (selectedTypeDraw)
                {
                    case GUI_WINDOWBOX: GuiWindowBox(defaultRec[selectedTypeDraw], "WINDOW BOX"); break;
                    case GUI_GROUPBOX: GuiGroupBox(defaultRec[selectedTypeDraw], "GROUP BOX"); break;
                    case GUI_LINE: GuiLine(defaultRec[selectedTypeDraw], 1); break;
                    case GUI_PANEL: GuiPanel(defaultRec[selectedTypeDraw]); break;
                    case GUI_LABEL: GuiLabel(defaultRec[selectedTypeDraw], "TEXT SAMPLE"); break;
                    case GUI_BUTTON: GuiButton(defaultRec[selectedTypeDraw], "GUI_BUTTON"); break;
                    case GUI_TOGGLE: GuiToggle(defaultRec[selectedTypeDraw], "GUI_TOGGLE", false); break;
                    case GUI_TOGGLEGROUP: GuiToggleGroup(defaultRec[selectedTypeDraw], listData, 3, 1); break;
                    case GUI_CHECKBOX: GuiCheckBoxEx(defaultRec[selectedTypeDraw], false, "TEXT SAMPLE"); break;
                    case GUI_COMBOBOX: GuiComboBox(defaultRec[selectedTypeDraw], listData, 3, 1); break;
                    case GUI_DROPDOWNBOX: GuiDropdownBox(defaultRec[selectedTypeDraw], listData, 3, &dropdownBoxActive, false); break;
                    case GUI_SPINNER: GuiSpinner(defaultRec[selectedTypeDraw], &spinnerValue, 42, 3, 25, false); break;
                    case GUI_VALUEBOX: GuiValueBox(defaultRec[selectedTypeDraw], &valueBoxValue, 42, 100, false); break;
                    case GUI_TEXTBOX: GuiTextBox(defaultRec[selectedTypeDraw], "GUI_TEXTBOX", 7, false); break;
                    case GUI_SLIDER: GuiSliderEx(defaultRec[selectedTypeDraw], 42, 0, 100, "TEXT SAMPLE", true); break;
                    case GUI_SLIDERBAR: GuiSliderBarEx(defaultRec[selectedTypeDraw], 40, 0, 100, "TEXT SAMPLE", true); break;
                    case GUI_PROGRESSBAR: GuiProgressBarEx(defaultRec[selectedTypeDraw], 40, 0, 100, true); break;
                    case GUI_STATUSBAR: GuiStatusBar(defaultRec[selectedTypeDraw], "STATUS BAR", 15); break;
                    case GUI_LISTVIEW: GuiListView(defaultRec[selectedTypeDraw], listViewData, 4, &listViewScrollIndex, &listViewActive, false); break;
                    case GUI_COLORPICKER: GuiColorPicker(defaultRec[selectedTypeDraw], RED); break;
                    case GUI_DUMMYREC: GuiDummyRec(defaultRec[selectedTypeDraw], "DUMMY REC"); break;
                    default: break;
                }
                GuiFade(1.0f);
            }

            // Draw the anchor points
            for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
            {
                if (layout.anchors[i].enabled)
                {
                    if (selectedAnchor > 0 && layout.anchors[i].id == selectedAnchor)
                    {
                        if (layout.anchors[i].hidding)
                        {
                            anchorCircleColor = GRAY;
                            anchorSelectedColor = GRAY;
                        }
                        else
                        {
                            anchorCircleColor = (Color){ 253, 86, 95, 255 }; //LIGHTRED
                            anchorSelectedColor = RED;
                        }
                        if (anchorMoveMode || anchorEditMode && (focusedAnchor > 0 && layout.anchors[i].id == focusedAnchor)) anchorSelectedColor = ORANGE;
                        DrawCircle(layout.anchors[i].x, layout.anchors[i].y, ANCHOR_RADIUS, Fade(anchorSelectedColor, 0.2f));
                    }
                    else if (layout.anchors[i].hidding) anchorCircleColor = GRAY;
                    else anchorCircleColor = BLUE;
                    if (focusedAnchor > 0 && layout.anchors[i].id == focusedAnchor) 
                    {
                        if (anchorEditMode) anchorCircleColor = ORANGE;
                        else anchorCircleColor = RED;
                    }
                    DrawCircleLines(layout.anchors[i].x, layout.anchors[i].y, ANCHOR_RADIUS, Fade(anchorCircleColor, 0.5f));
                    DrawRectangle(layout.anchors[i].x - ANCHOR_RADIUS - 5, layout.anchors[i].y, ANCHOR_RADIUS*2 + 10, 1, anchorCircleColor);
                    DrawRectangle(layout.anchors[i].x, layout.anchors[i].y - ANCHOR_RADIUS - 5, 1, ANCHOR_RADIUS*2 + 10, anchorCircleColor);
                }
            }
            if (selectedAnchor != -1) 
            {
                positionColor = anchorSelectedColor;
                if (snapMode) positionColor = LIME;
                if (precisionMode) positionColor = BLUE;
                
                DrawText(FormatText("[%i, %i]", layout.anchors[selectedAnchor].x, layout.anchors[selectedAnchor].y), layout.anchors[selectedAnchor].x + ANCHOR_RADIUS, layout.anchors[selectedAnchor].y - 38, 20, positionColor);
            }
            // Draw selected control selection rectangle (transparent RED/WHITE)
            if (((selectedControl != -1) || (focusedControl != -1)) && (selectedControl < layout.controlsCount))
            {
                if (selectedControl != -1) 
                {
                    selectedControlColor = RED;
                    if (resizeMode) selectedControlColor = BLUE;
                    DrawRectangleRec((Rectangle){ layout.controls[selectedControl].ap->x + layout.controls[selectedControl].rec.x, layout.controls[selectedControl].ap->y + layout.controls[selectedControl].rec.y, layout.controls[selectedControl].rec.width, layout.controls[selectedControl].rec.height }, (nameEditMode) ? Fade(WHITE, 0.7f) : Fade(selectedControlColor, 0.5f));
                }
                if (focusedControl != -1) 
                    DrawRectangleLinesEx((Rectangle){ layout.controls[focusedControl].ap->x + layout.controls[focusedControl].rec.x, layout.controls[focusedControl].ap->y + layout.controls[focusedControl].rec.y, layout.controls[focusedControl].rec.width, layout.controls[focusedControl].rec.height }, 1, RED);
            }

            // Draw anchor lines from one control
            if (focusedControl != -1 && layout.controls[focusedControl].ap->id > 0) DrawLine(layout.controls[focusedControl].ap->x, layout.controls[focusedControl].ap->y, layout.controls[focusedControl].ap->x + layout.controls[focusedControl].rec.x, layout.controls[focusedControl].ap->y + layout.controls[focusedControl].rec.y, RED);
            if (selectedControl != -1 && layout.controls[selectedControl].ap->id > 0) DrawLine(layout.controls[selectedControl].ap->x, layout.controls[selectedControl].ap->y, layout.controls[selectedControl].ap->x + layout.controls[selectedControl].rec.x, layout.controls[selectedControl].ap->y + layout.controls[selectedControl].rec.y, RED);
            
            // Draw all links from one anchor
            if (focusedAnchor != -1)
            {
                 for (int i = 0; i < layout.controlsCount; i++)
                {
                    //if (!layout.controls[i].ap->hidding)
                    if (layout.controls[i].ap->id == focusedAnchor)
                    {
                        if(!layout.controls[i].ap->hidding) DrawLine(layout.controls[i].ap->x, layout.controls[i].ap->y, layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, RED);
                        else DrawLine(layout.controls[i].ap->x, layout.controls[i].ap->y, layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, GRAY);
                    }
                }
            }
            if (selectedAnchor != -1)
            {
                 for (int i = 0; i < layout.controlsCount; i++)
                {
                    //if (!layout.controls[i].ap->hidding)
                    if (layout.controls[i].ap->id == selectedAnchor)
                    {
                        if(!layout.controls[i].ap->hidding) DrawLine(layout.controls[i].ap->x, layout.controls[i].ap->y, layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, RED);
                        else DrawLine(layout.controls[i].ap->x, layout.controls[i].ap->y, layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, GRAY);
                    }
                }
            }  
            
            // Draw cursor (control mode or anchor mode)
            if ((focusedControl == -1)  && (focusedAnchor == -1))
            {
                if (anchorEditMode)
                {
                    DrawCircleLines(mouse.x, mouse.y, ANCHOR_RADIUS, Fade(RED, 0.5f));
                    DrawRectangle(mouse.x - ANCHOR_RADIUS - 5, mouse.y, ANCHOR_RADIUS*2 + 10, 1, RED);
                    DrawRectangle(mouse.x , mouse.y - ANCHOR_RADIUS - 5, 1, ANCHOR_RADIUS*2 + 10, RED);
                }
                else
                {
                    DrawRectangle(mouse.x - 8, mouse.y, 17, 1, RED);
                    DrawRectangle(mouse.x, mouse.y - 8, 1, 17, RED);
                }
            }

            // Draw cursor on textEditMode
            if (textEditMode)
            {
               if (((framesCounter/20)%2) == 0)
               {
                    if (layout.controls[selectedControl].type == GUI_LABEL) DrawText("|", layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x + MeasureText(layout.controls[selectedControl].text, GuiGetStyle(DEFAULT, TEXT_SIZE)), layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y - GuiGetStyle(DEFAULT, TEXT_SIZE)/2 + layout.controls[selectedControl].rec.height/2, GuiGetStyle(DEFAULT, TEXT_SIZE) + 2, BLACK);
                    else if (layout.controls[selectedControl].type == GUI_TEXTBOX) DrawText("|", layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x + MeasureText(layout.controls[selectedControl].text, GuiGetStyle(DEFAULT, TEXT_SIZE)) + 4, layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y - GuiGetStyle(DEFAULT, TEXT_SIZE)/2 + layout.controls[selectedControl].rec.height/2, GuiGetStyle(DEFAULT, TEXT_SIZE) + 2, BLACK);
                    else if (layout.controls[selectedControl].type == GUI_GROUPBOX) DrawText("|", layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x + 15 + MeasureText(layout.controls[selectedControl].text, GuiGetStyle(DEFAULT, TEXT_SIZE)), layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y - GuiGetStyle(DEFAULT, TEXT_SIZE)/2, GuiGetStyle(DEFAULT, TEXT_SIZE) + 2, BLACK);
                    else if (layout.controls[selectedControl].type == GUI_WINDOWBOX) DrawText("|", layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x + 10 + MeasureText(layout.controls[selectedControl].text, GuiGetStyle(DEFAULT, TEXT_SIZE)), layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y + GuiGetStyle(DEFAULT, TEXT_SIZE)/2, GuiGetStyle(DEFAULT, TEXT_SIZE) + 2, BLACK);
                    else if (layout.controls[selectedControl].type == GUI_STATUSBAR) DrawText("|", layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x + 15 + MeasureText(layout.controls[selectedControl].text, GuiGetStyle(DEFAULT, TEXT_SIZE)), layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y - GuiGetStyle(DEFAULT, TEXT_SIZE)/2 + layout.controls[selectedControl].rec.height/2, GuiGetStyle(DEFAULT, TEXT_SIZE) + 2, BLACK);
                    else if (layout.controls[selectedControl].type == GUI_CHECKBOX) DrawText("|", layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x + layout.controls[selectedControl].rec.width + 5 + MeasureText(layout.controls[selectedControl].text, GuiGetStyle(DEFAULT, TEXT_SIZE)), layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y - GuiGetStyle(DEFAULT, TEXT_SIZE)/2 + layout.controls[selectedControl].rec.height/2, GuiGetStyle(DEFAULT, TEXT_SIZE) + 2, BLACK);
                    else if (layout.controls[selectedControl].type == GUI_SLIDERBAR || layout.controls[selectedControl].type == GUI_SLIDER) DrawText("|", layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x - 5, layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y - GuiGetStyle(DEFAULT, TEXT_SIZE)/2 + layout.controls[selectedControl].rec.height/2, GuiGetStyle(DEFAULT, TEXT_SIZE) + 2, BLACK);
                    else DrawText("|", layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x + layout.controls[selectedControl].rec.width/2 + MeasureText(layout.controls[selectedControl].text , GuiGetStyle(DEFAULT, TEXT_SIZE))/2 + 2, layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y + layout.controls[selectedControl].rec.height/2 - 6, GuiGetStyle(DEFAULT, TEXT_SIZE) + 2, BLACK);
               }
            }

            // Draw nameEditMode
            if (nameEditMode)
            {
                DrawText(FormatText("%s", layout.controls[selectedControl].name), layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x + layout.controls[selectedControl].rec.width/2 - MeasureText(layout.controls[selectedControl].name, GuiGetStyle(DEFAULT, TEXT_SIZE)*2)/2, layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y + layout.controls[selectedControl].rec.height/2 - 10, GuiGetStyle(DEFAULT, TEXT_SIZE)*2, BLACK);

                if (((framesCounter/20)%2) == 0) DrawText("|", layout.controls[selectedControl].rec.x + layout.controls[selectedControl].rec.width/2 + layout.controls[selectedControl].ap->x + MeasureText(layout.controls[selectedControl].name, GuiGetStyle(DEFAULT, TEXT_SIZE)*2)/2 + 2, layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y + layout.controls[selectedControl].rec.height/2 - 10, GuiGetStyle(DEFAULT, TEXT_SIZE)*2 + 2, BLACK);
            }
            else if ((IsKeyDown(KEY_N)) && (!textEditMode) && (!generateWindowActive) && (!resetWindowActive))
            {
                if (layout.controlsCount > 0) DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(WHITE, 0.7f));

                for (int i = 0; i < layout.controlsCount; i++)
                {
                    // Draws the Controls when placed on the grid.
                    if (!layout.controls[i].ap->hidding)
                    DrawText(FormatText("%s", layout.controls[i].name), layout.controls[i].rec.x + layout.controls[i].ap->x + layout.controls[i].rec.width/2 - MeasureText(layout.controls[i].name, GuiGetStyle(DEFAULT, TEXT_SIZE)*2)/2, layout.controls[i].rec.y + layout.controls[i].ap->y + layout.controls[i].rec.height/2 - 10, GuiGetStyle(DEFAULT, TEXT_SIZE)*2, BLACK);
                }
            }

            // Draw anchor linking line
            if (anchorLinkMode) 
            {
                if (selectedAnchor != -1) DrawLine(layout.anchors[selectedAnchor].x, layout.anchors[selectedAnchor].y, mouse.x, mouse.y, RED);
                else if (selectedControl != -1) DrawLine(layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x, layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y, mouse.x, mouse.y, RED);
            }

            // Draw rectangle position
            if (selectedControl != -1)
            {
                positionColor = MAROON;
                if (showGlobalPos) positionColor = RED;
                if (snapMode) positionColor = LIME;
                if (precisionMode) positionColor = BLUE;
                
                if (!showGlobalPos) DrawText(FormatText("[%i, %i, %i, %i]", (int)layout.controls[selectedControl].rec.x, (int)layout.controls[selectedControl].rec.y, (int)layout.controls[selectedControl].rec.width, (int)layout.controls[selectedControl].rec.height), (int)layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x, (int)layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y - 30, 20, positionColor);
                else DrawText(FormatText("[%i, %i, %i, %i]", (int)layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x, (int)layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y, (int)layout.controls[selectedControl].rec.width, (int)layout.controls[selectedControl].rec.height), (int)layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x, (int)layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y - 30, 20, positionColor);
            }
            // Draw cursor position
            if (selectedControl == -1 && focusedControl == -1 && selectedAnchor == -1 && focusedAnchor == -1 && !anchorEditMode)
            {
                positionColor = MAROON;
                if (snapMode) positionColor = LIME;
                DrawText(FormatText("[%i, %i, %i, %i]", (int)defaultRec[selectedType].x, (int)defaultRec[selectedType].y, (int)defaultRec[selectedType].width, (int)defaultRec[selectedType].height), (int)defaultRec[selectedType].x, (int)defaultRec[selectedType].y - 30, 20, Fade(positionColor, 0.5f));
            }

            // Draw image info
            if (tracemapEditMode) DrawText(FormatText("[%i, %i, %i, %i]", tracemapRec.x, tracemapRec.y, tracemapRec.width, tracemapRec.height), tracemapRec.x + 25, tracemapRec.y + 25, 20, MAROON);

            // Draw the id of all controls
            if (selectedControl == -1 && orderEditMode)
            {
                for (int i = layout.controlsCount - 1; i >= 0; i--) DrawText(FormatText("[%i]", layout.controls[i].id), layout.controls[i].rec.x + layout.controls[i].ap->x + layout.controls[i].rec.width, layout.controls[i].rec.y + layout.controls[i].ap->y - 10, 10, BLUE);
            }

            // Draw the help listData (by default is out of screen)
            if (helpPositionX > -280)
            {
                DrawRectangleRec((Rectangle){ helpPositionX + 20, 15, 280, 550 }, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
                GuiGroupBox((Rectangle){ helpPositionX + 20, 15, 280, 550 }, "TAB - Shortcuts");
                GuiLabel((Rectangle){ helpPositionX + 30, 30, 0, 0 }, "G - Toggle grid mode");
                GuiLabel((Rectangle){ helpPositionX + 30, 50, 0, 0 }, "S - Toggle snap to grid mode");
                GuiLabel((Rectangle){ helpPositionX + 30, 70, 0, 0 }, "F - Toggle control position (global/anchor)");
                GuiLine((Rectangle){ helpPositionX + 30, 85, 260, 10 }, 1);
                GuiLabel((Rectangle){ helpPositionX + 30, 100, 0, 0 }, "SPACE - Lock/unlock control for editing");
                GuiLabel((Rectangle){ helpPositionX + 30, 120, 0, 0 }, "ARROWS - Edit control position");
                GuiLabel((Rectangle){ helpPositionX + 30, 140, 0, 0 }, "LSHIFT + ARROWS - Smooth edit position");
                GuiLabel((Rectangle){ helpPositionX + 30, 160, 0, 0 }, "LCTRL + ARROWS - Edit control scale");
                GuiLabel((Rectangle){ helpPositionX + 30, 180, 0, 0 }, "LCTRL + LSHIFT + ARROWS - Smooth edit scale");
                GuiLabel((Rectangle){ helpPositionX + 30, 200, 0, 0 }, "LCTRL + R - Resize control to closest snap");
                GuiLabel((Rectangle){ helpPositionX + 30, 220, 0, 0 }, "LCTRL + D - Duplicate selected control");
                GuiLabel((Rectangle){ helpPositionX + 30, 240, 0, 0 }, "LCTRL + N - Resets layout");
                GuiLabel((Rectangle){ helpPositionX + 30, 260, 0, 0 }, "DEL - Delete selected control");
                GuiLine((Rectangle){ helpPositionX + 30, 275, 260, 10 }, 1);
                GuiLabel((Rectangle){ helpPositionX + 30, 290, 0, 0 }, "T - Control text editing (if possible)");
                GuiLabel((Rectangle){ helpPositionX + 30, 310, 0, 0 }, "N - Control name editing ");
                GuiLabel((Rectangle){ helpPositionX + 30, 330, 0, 0 }, "ESC - Exit text/name editing mode");
                GuiLabel((Rectangle){ helpPositionX + 30, 350, 0, 0 }, "ENTER - Validate text/name edition");
                GuiLine((Rectangle){ helpPositionX + 30, 365, 260, 10 }, 1);
                GuiLabel((Rectangle){ helpPositionX + 30, 380, 0, 0 }, "LALT + UP/DOWN - Control layer order");
                GuiLine((Rectangle){ helpPositionX + 30, 395, 260, 10 }, 1);
                GuiLabel((Rectangle){ helpPositionX + 30, 410, 0, 0 }, "A - Anchor editing mode");
                GuiLabel((Rectangle){ helpPositionX + 30, 430, 0, 0 }, "RMB - Link anchor to control");
                GuiLabel((Rectangle){ helpPositionX + 30, 450, 0, 0 }, "U - Unlink control from anchor");
                GuiLabel((Rectangle){ helpPositionX + 30, 470, 0, 0 }, "H - Hide/Unhide controls for selected anchor");
                GuiLine((Rectangle){ helpPositionX + 30, 485, 260, 10 }, 1);
                GuiLabel((Rectangle){ helpPositionX + 30, 500, 0, 0 }, "LCTRL + S - Save layout file (.rgl)");
                GuiLabel((Rectangle){ helpPositionX + 30, 520, 0, 0 }, "LCTRL + O - Open layout file (.rgl)");
                GuiLabel((Rectangle){ helpPositionX + 30, 540, 0, 0 }, "LCTRL + ENTER - Export layout to code");
            }

            // Draw right panel controls palette
            GuiPanel(palettePanel);
            GuiWindowBox(paletteRecs[0], "WindowBox");
            GuiGroupBox(paletteRecs[1], "GroupBox");
            GuiLine(paletteRecs[2], 1);
            GuiPanel(paletteRecs[3]);
            GuiLabel(paletteRecs[4], "Label (SAMPLE TEXT)");
            GuiButton(paletteRecs[5], "Button");
            GuiToggle(paletteRecs[6], "Toggle", false);
            GuiCheckBox(paletteRecs[8], false);
            GuiToggleGroup(paletteRecs[7], listData, 3, 0);
            GuiComboBox(paletteRecs[9],  listData, 3, 0);
            GuiDropdownBox(paletteRecs[10], listData, 3, &dropdownBoxActive, false);
            GuiSpinner(paletteRecs[11], &spinnerValue, 42, 100, 25, false);
            GuiValueBox(paletteRecs[12], &valueBoxValue, 42, 100, false);
            GuiTextBox(paletteRecs[13], "GUI_TEXTBOX", 7, false);
            GuiSlider(paletteRecs[14], 42, 0, 100);
            GuiSliderBar(paletteRecs[15], 42, 0, 100);
            GuiProgressBar(paletteRecs[16], 42, 0, 100);
            GuiStatusBar(paletteRecs[17], "StatusBar", 10);
            GuiListView(paletteRecs[18], listData, 3, &listViewScrollIndex, &listViewActive, false);
            GuiColorPicker(paletteRecs[19], RED);
            GuiDummyRec(paletteRecs[20], "DummyRec");

            DrawRectangleRec(paletteRecs[selectedType], Fade(RED, 0.5f));

            if (paletteSelect > -1) DrawRectangleLinesEx(paletteRecs[paletteSelect], 1, RED);

            // Draw export options window
            if (generateWindowActive)
            {
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(WHITE, 0.7f));
                generateWindowActive = !GuiWindowBox((Rectangle){ exportWindowPos.x, exportWindowPos.y, 400, 225 }, "Generate Code Options - layout");

                GuiLabel((Rectangle){ exportWindowPos.x + 10, exportWindowPos.y + 35, 65, 25 }, "Name:");
                GuiTextBox((Rectangle){ exportWindowPos.x + 75, exportWindowPos.y + 35, 135, 25 }, config.name, toolNameSize, true);
                GuiLabel((Rectangle){ exportWindowPos.x + 225, exportWindowPos.y + 35, 50, 25 }, "Version:");
                GuiTextBox((Rectangle){ exportWindowPos.x + 275, exportWindowPos.y + 35, 115, 25 }, config.version, toolVersionSize, true);
                GuiLabel((Rectangle){ exportWindowPos.x + 10, exportWindowPos.y + 65, 65, 25 }, "Window size:");
                if (GuiValueBox((Rectangle){ exportWindowPos.x + 75, exportWindowPos.y + 65, 60, 25 }, &config.width, 400, 1000, widthBoxEditMode)) widthBoxEditMode = !widthBoxEditMode;
                GuiLabel((Rectangle){ exportWindowPos.x + 140, exportWindowPos.y + 65, 10, 25 }, "x");
                if (GuiValueBox((Rectangle){ exportWindowPos.x + 150, exportWindowPos.y + 65, 60, 25 }, &config.height, 400, 1000, heightBoxEditMode)) heightBoxEditMode = !heightBoxEditMode;
                GuiLabel((Rectangle){ exportWindowPos.x + 225, exportWindowPos.y + 65, 50, 25 }, "Company:");
                GuiTextBox((Rectangle){ exportWindowPos.x + 275, exportWindowPos.y + 65, 115, 25 }, config.company, companySize, true);
                GuiLabel((Rectangle){ exportWindowPos.x + 10, exportWindowPos.y + 95, 65, 25 }, "Description:");
                GuiTextBox((Rectangle){ exportWindowPos.x + 75, exportWindowPos.y + 95, 315, 55 }, config.description, toolDescriptionSize, true);
                config.defineRecs = GuiCheckBoxEx((Rectangle){ exportWindowPos.x + 10, exportWindowPos.y + 160, 15, 15 }, config.defineRecs, "Define Rectangles");
                config.defineTexts = GuiCheckBoxEx((Rectangle){ exportWindowPos.x + 10, exportWindowPos.y + 180, 15, 15 }, config.defineTexts, "Define text const");
                config.exportAnchors = GuiCheckBoxEx((Rectangle){ exportWindowPos.x + 140, exportWindowPos.y + 160, 15, 15 }, config.exportAnchors, "Export anchors");
                config.exportAnchor0 = GuiCheckBoxEx((Rectangle){ exportWindowPos.x + 140, exportWindowPos.y + 180, 15, 15 }, config.exportAnchor0, "Export anchor 0");
                config.fullComments = GuiCheckBoxEx((Rectangle){ exportWindowPos.x + 140, exportWindowPos.y + 200, 15, 15 }, config.fullComments, "Full comments");
                config.cropWindow = GuiCheckBoxEx((Rectangle){ exportWindowPos.x + 275, exportWindowPos.y + 160, 15, 15 }, config.cropWindow, "Crop to Window");
                config.fullVariables = GuiCheckBoxEx((Rectangle){ exportWindowPos.x + 10, exportWindowPos.y + 200, 15, 15 }, config.fullVariables, "Full variables");

                if (GuiButton((Rectangle){ exportWindowPos.x + 275, exportWindowPos.y + 185, 115, 30 }, "Generate Code"))
                {
                    DialogExportLayout(config);
                    generateWindowActive = false;
                }
            }

            // Draw status bar bottom with debug information
            GuiStatusBar((Rectangle){ 0, GetScreenHeight() - 24, 126, 24}, FormatText("MOUSE: (%i, %i)", (int)mouse.x, (int)mouse.y), 15);
            GuiStatusBar((Rectangle){ 124, GetScreenHeight() - 24, 81, 24}, (snapMode ? "SNAP: ON" : "SNAP: OFF"), 10);
            GuiStatusBar((Rectangle){ 204, GetScreenHeight() - 24, 145, 24}, FormatText("CONTROLS COUNT: %i", layout.controlsCount), 20);
            if (selectedControl != -1) GuiStatusBar((Rectangle){ 348, GetScreenHeight() - 24, GetScreenWidth() - 348, 24}, FormatText("SELECTED CONTROL: #%03i  |  %s  |  REC (%i, %i, %i, %i)  |  %s", selectedControl, controlTypeName[layout.controls[selectedControl].type], (int)layout.controls[selectedControl].rec.x, (int)layout.controls[selectedControl].rec.y, (int)layout.controls[selectedControl].rec.width, (int)layout.controls[selectedControl].rec.height, layout.controls[selectedControl].name), 15);
            else GuiStatusBar((Rectangle){ 348, GetScreenHeight() - 24, GetScreenWidth() - 348, 24}, "", 15);

            // Draw ending message window (save)
            if (closingWindowActive)
            {
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(WHITE, 0.7f));
                closingWindowActive = !GuiWindowBox((Rectangle){ GetScreenWidth()/2 - 125, GetScreenHeight()/2 - 50, 250, 100 }, "Closing rGuiLayout");

                GuiLabel((Rectangle){ GetScreenWidth()/2 - 95, GetScreenHeight()/2 - 60, 200, 100 }, "Do you want to save before quitting?");

                if (GuiButton((Rectangle){ GetScreenWidth()/2 - 94, GetScreenHeight()/2 + 10, 85, 25 }, "Yes"))
                {
                    cancelSave = false;
                    DialogSaveLayout();
                    if (cancelSave) exitWindow = true;
                }
                else if (GuiButton((Rectangle){ GetScreenWidth()/2 + 10, GetScreenHeight()/2 + 10, 85, 25 }, "No")) { exitWindow = true; }
            }

            // Draw reset message window (save)
            if (resetWindowActive)
            {
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(WHITE, 0.7f));
                resetWindowActive = !GuiWindowBox((Rectangle){ GetScreenWidth()/2 - 125, GetScreenHeight()/2 - 50, 250, 100 }, "Creating new layout");

                GuiLabel((Rectangle){ GetScreenWidth()/2 - 95, GetScreenHeight()/2 - 60, 200, 100 }, "Do you want to save the current layout?");

                if (GuiButton((Rectangle){ GetScreenWidth()/2 - 94, GetScreenHeight()/2 + 10, 85, 25 }, "Yes"))
                {
                    cancelSave = false;
                    DialogSaveLayout();
                    if (cancelSave) resetLayout = true;
                }
                else if (GuiButton((Rectangle){ GetScreenWidth()/2 + 10, GetScreenHeight()/2 + 10, 85, 25 }, "No")) { resetLayout = true; }
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(tracemap);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definitions (local)
//----------------------------------------------------------------------------------

#if defined(ENABLE_PRO_FEATURES)
// Show command line usage info
static void ShowCommandLineInfo(void)
{
    printf("\n//////////////////////////////////////////////////////////////////////////////////\n");
    printf("//                                                                              //\n");
    printf("// rGuiLayout v%s - A simple and easy-to-use raygui layout editor              //\n", TOOL_VERSION_TEXT);
    printf("// powered by raylib v2.0 (www.raylib.com) and raygui v2.0                      //\n");
    printf("// more info and bugs-report: github.com/raysan5/rguilayout                     //\n");
    printf("//                                                                              //\n");
    printf("// Copyright (c) 2018 raylib technologies (@raylibtech)                         //\n");
    printf("//                                                                              //\n");
    printf("//////////////////////////////////////////////////////////////////////////////////\n\n");

    printf("USAGE:\n\n");
    printf("    > rguilayout [--help] --input <filename.ext> [--output <filename.ext>]\n");
    printf("                 [--format <styleformat>] [--edit-prop <property> <value>]\n");

    printf("\nOPTIONS:\n\n");
    printf("    -h, --help                      : Show tool version and command line usage help\n");
    printf("    -i, --input <filename.ext>      : Define input file.\n");
    printf("                                      Supported extensions: .rgs, .png\n");
    printf("    -o, --output <filename.ext>     : Define output file.\n");
    printf("                                      Supported extensions: .rgs, .png, .h\n");
    printf("                                      NOTE: Extension could be modified depending on format\n\n");
    printf("    -f, --format <type_value>       : Define output file format to export style data.\n");
    printf("                                      Supported values:\n");
    printf("                                          0 - Style text format (.rgs)  \n");
    printf("                                          1 - Style binary format (.rgs)\n");
    printf("                                          2 - Palette image (.png)\n");
    printf("                                          3 - Palette as int array (.h)\n");
    printf("                                          4 - Controls table image (.png)\n\n");
    printf("    -e, --edit-prop <property> <value>\n");
    printf("                                    : Edit specific property from input to output.\n");

    printf("\nEXAMPLES:\n\n");
    printf("    > rguilayout --input tools.rgl --output tools.png\n");
}

// Process command line input
static void ProcessCommandLine(int argc, char *argv[])
{
    // CLI required variables
    bool showUsageInfo = false;     // Toggle command line usage info

    char inFileName[256] = { 0 };   // Input file name
    char outFileName[256] = { 0 };  // Output file name
    int outputFormat = 0;           // Supported output formats

    // Process command line arguments
    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
        {
            showUsageInfo = true;
        }
        else if ((strcmp(argv[i], "-i") == 0) || (strcmp(argv[i], "--input") == 0))
        {
            // Verify an image is provided with a supported extension
            // Check that no "--" is comming after --input
            if (((i + 1) < argc) && (argv[i + 1][0] != '-') && (IsFileExtension(argv[i + 1], ".rgl")))
            {
                strcpy(inFileName, argv[i + 1]);    // Read input filename
                i++;
            }
            else printf("WARNING: Input file extension not recognized\n");
        }
        else if ((strcmp(argv[i], "-o") == 0) || (strcmp(argv[i], "--output") == 0))
        {
            if (((i + 1) < argc) && (argv[i + 1][0] != '-') && (IsFileExtension(argv[i + 1], ".c")))
            {
                strcpy(outFileName, argv[i + 1]);   // Read output filename
                i++;
            }
            else printf("WARNING: Output file extension not recognized\n");
        }
        else if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--format") == 0))
        {

        }
    }

    // Process input file
    if (inFileName[0] != '\0')
    {
        if (outFileName[0] == '\0') strcpy(outFileName, "output.c");  // Set a default name for output in case not provided

        printf("\nInput file:       %s", inFileName);
        printf("\nOutput file:      %s", outFileName);
        //printf("\nOutput params:    %i Hz, %i bits, %s\n\n", sampleRate, sampleSize, (channels == 1) ? "Mono" : "Stereo");

        // Support .rlg layout processing to generate .c
        LoadLayout(argv[1]);    // Updates global: layout.controls

        int len = strlen(argv[1]);
        char outName[256] = { 0 };
        strcpy(outName, argv[1]);
        outName[len - 3] = 'c';
        outName[len - 2] = '\0';

        GuiLayoutConfig config;
        memset(&config, 0, sizeof(GuiLayoutConfig));

        config.width = 800;
        config.height = 600;
        strcpy(config.name, "layout_file_name");
        strcpy(config.version, TOOL_VERSION_TEXT);
        strcpy(config.company, "raylib technologies");
        strcpy(config.description, "tool description");
        config.defineRecs = false;
        config.exportAnchors = true;
        config.exportAnchor0 = false;
        config.fullComments = true;

        // Generate C code for gui layout.controls
        GenerateCode(outName, config);
    }

    if (showUsageInfo) ShowCommandLineInfo();
}
#endif      // ENABLE_PRO_FEATURES

//--------------------------------------------------------------------------------------------
// Load/Save/Export data functions
//--------------------------------------------------------------------------------------------

// Import gui layout information
// NOTE: Updates global variable: layout
static void LoadLayout(const char *fileName)
{
    char buffer[256];
    bool tryBinary = false;

    int anchorId = 0;       // TODO: Review!!!
    int anchorX = 0;
    int anchorY = 0;
    int anchorCounter = 0;

    FILE *rglFile = fopen(fileName, "rt");

    if (rglFile != NULL)
    {
        // Reset all the controls
        for (int i = 0; i < MAX_GUI_CONTROLS; i++)
        {
            layout.controls[i].id = 0;
            layout.controls[i].type = 0;
            layout.controls[i].rec = (Rectangle){ 0, 0, 0, 0 };
            memset(layout.controls[i].text, 0, MAX_CONTROL_TEXT_LENGTH);
            memset(layout.controls[i].name, 0, MAX_CONTROL_NAME_LENGTH);
            layout.controls[i].ap = &layout.anchors[0];
        }
        for (int i = 0; i < MAX_ANCHOR_POINTS; i++) layout.anchors[i].hidding = false;

        fgets(buffer, 256, rglFile);

        if (buffer[0] != 'R')   // Text file!
        {
            layout.controlsCount = 0;

            while (!feof(rglFile))
            {
                if ((buffer[0] != '\n') && (buffer[0] != '#') && (buffer[0] == 'a'))
                {
                    sscanf(buffer, "a %03i %i %i %i", &layout.anchors[anchorCounter].id, &layout.anchors[anchorCounter].x, &layout.anchors[anchorCounter].y, &layout.anchors[anchorCounter].enabled);
                    //printf("a %03i %i %i %i\n", layout.anchors[anchorCounter].id, layout.anchors[anchorCounter].x, layout.anchors[anchorCounter].y, layout.anchors[anchorCounter].enabled);
                    anchorCounter++;
                }
                else if ((buffer[0] != '\n') && (buffer[0] != '#') && (buffer[0] == 'c'))
                {
                    sscanf(buffer, "c %d %i %s %f %f %f %f %d %[^\n]s", &layout.controls[layout.controlsCount].id, &layout.controls[layout.controlsCount].type, layout.controls[layout.controlsCount].name, &layout.controls[layout.controlsCount].rec.x, &layout.controls[layout.controlsCount].rec.y, &layout.controls[layout.controlsCount].rec.width, &layout.controls[layout.controlsCount].rec.height, &anchorId, layout.controls[layout.controlsCount].text);
                    //printf("c %d %i %i %i %i %i %i %s\n", layout.controls[layout.controlsCount].id, layout.controls[layout.controlsCount].type, layout.controls[layout.controlsCount].rec.x, layout.controls[layout.controlsCount].rec.y, layout.controls[layout.controlsCount].rec.width, layout.controls[layout.controlsCount].rec.height, anchorId, layout.controls[layout.controlsCount].text);

                    layout.controls[layout.controlsCount].ap = &layout.anchors[anchorId];
                    layout.controlsCount++;
                }
                fgets(buffer, 256, rglFile);
            }
        }
        else tryBinary = true;

        fclose(rglFile);
    }

    if (tryBinary)
    {
        FILE *rglFile = fopen(fileName, "rb");

        if (rglFile != NULL)
        {
            char signature[5] = "";
            short version = 0;
            short reserved = 0;

            fread(signature, 1, 4, rglFile);
            fread(&version, 1, sizeof(short), rglFile);
            fread(&reserved, 1, sizeof(short), rglFile);

            if ((signature[0] == 'R') &&
                (signature[1] == 'G') &&
                (signature[2] == 'L') &&
                (signature[3] == ' ')) fread(&layout, 1, sizeof(GuiLayout), rglFile);
            else TraceLog(LOG_WARNING, "[raygui] Invalid layout file");

            printf("[GuiLayout] Controls counter: %i\n", layout.controlsCount);

            fclose(rglFile);
        }
    }

    printf("[GuiLayout] Layout data loaded successfully\n");
}

// Save gui layout information
static void SaveLayout(const char *fileName, bool binary)
{
    if (binary)
    {
        #define RGL_FILE_VERSION_BINARY 100

        FILE *rglFile = fopen(fileName, "wb");

        if (rglFile != NULL)
        {
            // Write some header info (12 bytes)
            // id: "RGL "       - 4 bytes
            // version: 100     - 2 bytes
            // reserved         - 2 bytes

            char signature[5] = "RGL ";
            short version = RGL_FILE_VERSION_BINARY;
            short reserved = 0;

            fwrite(signature, 1, 4, rglFile);
            fwrite(&version, 1, sizeof(short), rglFile);
            fwrite(&reserved, 1, sizeof(short), rglFile);

            fwrite(&layout, 1, sizeof(GuiLayout), rglFile);

            fclose(rglFile);
        }
    }
    else
    {
        #define RGL_FILE_VERSION_TEXT "1.0"

        FILE *rglFile = fopen(fileName, "wt");

        if (rglFile != NULL)
        {
             // Write some description comments
            fprintf(rglFile, "#\n# rgl text file (v%s) - raygui layout text file generated using rGuiLayout\n#\n", RGL_FILE_VERSION_TEXT);
            fprintf(rglFile, "# Total number of controls:     %i\n", layout.controlsCount);
            fprintf(rglFile, "# Anchor info:   a <id> <posx> <posy> <enabled>\n");
            fprintf(rglFile, "# Control info:  c <id> <type> <name> <rectangle> <anchor_id> <text>\n#\n");

            for (int i = 0; i < MAX_ANCHOR_POINTS; i++)
            {
                fprintf(rglFile, "a %03i %i %i %i\n", layout.anchors[i].id, layout.anchors[i].x, layout.anchors[i].y, layout.anchors[i].enabled);
            }

            for (int i = 0; i < layout.controlsCount; i++)
            {
                fprintf(rglFile, "c %03i %i %s %i %i %i %i %i %s\n", layout.controls[i].id, layout.controls[i].type, layout.controls[i].name, (int)layout.controls[i].rec.x, (int)layout.controls[i].rec.y, (int)layout.controls[i].rec.width, (int)layout.controls[i].rec.height, layout.controls[i].ap->id, layout.controls[i].text);
            }

            fclose(rglFile);
        }
    }
}

// Show save layout dialog
static void DialogSaveLayout(void)
{
    const char *filters[] = { "*.rgl" };
    const char *fileName = tinyfd_saveFileDialog("Save raygui layout text file", "", 1, filters, "raygui Layout Files (*.rgl)");

    // Save layout.controls file (text or binary)
    if (fileName != NULL)
    {
        char outFileName[256] = { 0 };
        strcpy(outFileName, fileName);
        if (GetExtension(fileName) == NULL) strcat(outFileName, ".rgl\0");     // No extension provided
        SaveLayout(outFileName, false);
        strcpy(loadedFileName, outFileName);
        SetWindowTitle(FormatText("rGuiLayout v%s - %s", TOOL_VERSION_TEXT, GetFileName(loadedFileName)));
        cancelSave = true;
    }
}

// Show save layout dialog
static void DialogExportLayout(GuiLayoutConfig config)
{
    const char *filters[] = { "*.c", "*.go", "*.lua" };
    const char *fileName = tinyfd_saveFileDialog("Generate code file", config.name, 3, filters, "Code file");

    if (fileName != NULL)
    {
        char outFileName[256] = { 0 };
        strcpy(outFileName, fileName);
        if (GetExtension(fileName) == NULL) strcat(outFileName, ".c\0");     // No extension provided
        GenerateCode(outFileName, config);
    }
}


//--------------------------------------------------------------------------------------------
// Code generation functions
//--------------------------------------------------------------------------------------------

// Get control rectangle text (considering anchor or not)
static char *GetControlRectangleText(int index, GuiControl control, GuiLayoutConfig config)
{
    static char text[512];
    memset(text, 0, 512);

    if (!config.defineRecs)
    {
        if (config.exportAnchors && config.exportAnchor0) strcpy(text, FormatText("(Rectangle){ anchor%02i.x + %i, anchor%02i.y + %i, %i, %i }", control.ap->id, (int)control.rec.x, control.ap->id, (int)control.rec.y, (int)control.rec.width, (int)control.rec.height));
        else if (config.exportAnchors && !config.exportAnchor0)
        {
            if (control.ap->id > 0) strcpy(text, FormatText("(Rectangle){ anchor%02i.x + %i, anchor%02i.y + %i, %i, %i }", control.ap->id, (int)control.rec.x, control.ap->id, (int)control.rec.y, (int)control.rec.width, (int)control.rec.height));
            else strcpy(text, FormatText("(Rectangle){ %i, %i, %i, %i }", (int)control.rec.x, (int)control.rec.y, (int)control.rec.width, (int)control.rec.height));

        }
        else if (!config.exportAnchors && config.exportAnchor0)
        {
            if (control.ap->id > 0) strcpy(text, FormatText("(Rectangle){ %i, %i, %i, %i }", layout.anchors[control.ap->id].x + (int)control.rec.x, layout.anchors[control.ap->id].y + (int)control.rec.y, (int)control.rec.width, (int)control.rec.height));
            else strcpy(text, FormatText("(Rectangle){ anchor%02i.x + %i, anchor%02i.y + %i, %i, %i }", control.ap->id, (int)control.rec.x, control.ap->id, (int)control.rec.y, (int)control.rec.width, (int)control.rec.height));
        }
        else strcpy(text, FormatText("(Rectangle){ %i, %i, %i, %i }", layout.anchors[control.ap->id].x + (int)control.rec.x, layout.anchors[control.ap->id].y + (int)control.rec.y, (int)control.rec.width, (int)control.rec.height));
    }
    else strcpy(text, FormatText("layoutRecs[%i]", index));

    return text;
}

// Get control func parameters text
static char *GetControlParamText(int controlType, char *name, GuiLayoutConfig config)
{
    static char text[512];
    memset(text, 0, 512);

    switch(controlType)
    {
        case GUI_SLIDER:
        case GUI_SLIDERBAR:
        {
            if (config.fullVariables) strcpy(text, FormatText("%sMinValue, %sMaxValue", name, name));
            else strcpy(text, "0, 100");
        } break;
        case GUI_DROPDOWNBOX:
        case GUI_COMBOBOX:
        case GUI_LISTVIEW:
        case GUI_TOGGLEGROUP:
        {
            if (config.fullVariables) strcpy(text, FormatText("%sCount", name));
            else strcpy(text, "3");
        } break;
        case GUI_TEXTBOX:
        {
            if (config.fullVariables) strcpy(text, FormatText("%sSize", name));
            else strcpy(text, FormatText("%i", MAX_CONTROL_TEXT_LENGTH));
        } break;
    }

    return text;
}

// Write controls variables code to file
static void WriteControlsVariables(FILE *ftool, GuiControl control, GuiLayoutConfig config)
{
    switch (control.type)
    {
        case GUI_WINDOWBOX:
        case GUI_TOGGLE:
        {
            if (config.fullComments) fprintf(ftool, "    \n    // %s: %s\n", controlTypeNameLow[control.type], control.name);
            fprintf(ftool, "    bool %sActive = true;\n", control.name);
        } break;
        case GUI_CHECKBOX:
        {
            if (config.fullComments) fprintf(ftool, "    \n    // %s: %s\n", controlTypeNameLow[control.type], control.name);
            fprintf(ftool, "    bool %sChecked = false;\n", control.name);
        } break;
        case GUI_LABEL:
        {
            if (config.defineTexts)
            {
                if (config.fullComments) fprintf(ftool, "    \n    // %s: %s\n", controlTypeNameLow[control.type], control.name);
                fprintf(ftool, "    const char *%sText = \"%s\";\n", control.name, control.text);
            }
        } break;
        case GUI_STATUSBAR:
        {
            if (config.fullComments) fprintf(ftool, "    \n    // %s: %s\n", controlTypeNameLow[control.type], control.name);
            fprintf(ftool, "    char *%sText = \"%s\";\n", control.name, control.text);
        } break;
        case GUI_LISTVIEW:
        case GUI_DROPDOWNBOX:
        {
            if (config.fullComments) fprintf(ftool, "    \n    // %s: %s\n", controlTypeNameLow[control.type], control.name);
            if (config.fullVariables) fprintf(ftool, "    int %sCount = 3;\n", control.name);
            fprintf(ftool, "    const char *%sTextList[3] = { \"ONE\", \"TWO\", \"THREE\" };\n", control.name);
            fprintf(ftool, "    int %sActive = 0;\n", control.name);
            fprintf(ftool, "    bool %sEditMode = false;\n", control.name);
        } break;
        case GUI_COMBOBOX:
        case GUI_TOGGLEGROUP:
        {
            if (config.fullComments) fprintf(ftool, "    \n    // %s: %s\n", controlTypeNameLow[control.type], control.name);
            if (config.fullVariables) fprintf(ftool, "    int %sCount = 3;\n", control.name);
            fprintf(ftool, "    const char *%sTextList[3] = { \"ONE\", \"TWO\", \"THREE\" };\n", control.name);
           fprintf(ftool, "    int %sActive = 0;\n", control.name);
        } break;
        case GUI_SLIDER:
        case GUI_SLIDERBAR:
        {
            if (config.fullComments) fprintf(ftool, "    \n    // %s: %s\n", controlTypeNameLow[control.type], control.name);
            fprintf(ftool, "    float %sValue = 50.0f;\n", control.name);
            if (config.fullVariables)
            {
                fprintf(ftool, "    const float %sMinValue = 0.0f;\n", control.name);
                fprintf(ftool, "    const float %sMaxValue = 100.0f;\n", control.name);
            }
        } break;
        case GUI_PROGRESSBAR:
        {
            if (config.fullComments) fprintf(ftool, "    \n    // %s: %s\n", controlTypeNameLow[control.type], control.name);
            fprintf(ftool, "    float %sValue = 50.0f;\n", control.name);
        } break;
        case GUI_VALUEBOX:
        case GUI_SPINNER:
        {
            if (config.fullComments) fprintf(ftool, "    \n    // %s: %s\n", controlTypeNameLow[control.type], control.name);
            fprintf(ftool, "    int %sValue = 0;\n", control.name);
        } break;
        case GUI_COLORPICKER:
        {
            if (config.fullComments) fprintf(ftool, "    \n    // %s: %s\n", controlTypeNameLow[control.type], control.name);
            fprintf(ftool, "    Color %sValue;\n", control.name);
        } break;
        case GUI_TEXTBOX:
        {
            if (config.fullComments) fprintf(ftool, "    \n    // %s: %s\n", controlTypeNameLow[control.type], control.name);
            if (config.fullVariables) fprintf(ftool, "    int %sSize = %i;\n", control.name, MAX_CONTROL_TEXT_LENGTH);
            fprintf(ftool, "    char %sText[%i] = \"%s\";\n", control.name, MAX_CONTROL_TEXT_LENGTH, control.text);
        } break;
        default: break;
    }
}

// Write controls drawing code to file
static void WriteControlsDrawing(FILE *ftool, int index, GuiControl control, GuiLayoutConfig config)
{
    int i = index;

    switch (control.type)
    {
        case GUI_LABEL:
        {
            if (config.defineTexts) fprintf(ftool, "            GuiLabel(%s, %sText);\n", GetControlRectangleText(i, control, config), control.name);
            else fprintf(ftool, "            GuiLabel(%s, \"%s\");\n", GetControlRectangleText(i, control, config), control.text);
        }
        break;
        case GUI_BUTTON: fprintf(ftool, "            if (GuiButton(%s, \"%s\")) %s(); \n\n", GetControlRectangleText(i, control, config), control.text, control.name); break;
        case GUI_VALUEBOX: fprintf(ftool, "            if (GuiValueBox(%s, %sValue, 0, 100, %sEditMode)) %sEditMode = !%sEditMode;\n", control.name, GetControlRectangleText(i, control, config), control.name, control.name, control.name, control.name); break;
        case GUI_TOGGLE: fprintf(ftool, "            %sActive = GuiToggle(%s, \"%s\", %sActive);\n", control.name, GetControlRectangleText(i, control, config), control.text, control.name); break;
        case GUI_TOGGLEGROUP: fprintf(ftool, "            %sActive = GuiToggleGroup(%s, %sTextList, %s, %sActive);\n", control.name, GetControlRectangleText(i, control, config), control.name, GetControlParamText(control.type, control.name, config), control.name); break;
        case GUI_SLIDER:
        {
            if (control.text[0] != '\0') fprintf(ftool, "            %sValue = GuiSliderEx(%s, %sValue, %s, \"%s\", true);\n", control.name, GetControlRectangleText(i, control, config), control.name, GetControlParamText(control.type, control.name, config), control.text);
            else fprintf(ftool, "            %sValue = GuiSlider(%s, %sValue, %s);\n", control.name, GetControlRectangleText(i, control, config), control.name, GetControlParamText(control.type, control.name, config));
        } break;
        case GUI_SLIDERBAR:
        {
            if (control.text[0] != '\0') fprintf(ftool, "            %sValue = GuiSliderBarEx(%s, %sValue, %s, \"%s\", true);\n", control.name, GetControlRectangleText(i, control, config), control.name, GetControlParamText(control.type, control.name, config), control.text);
            else fprintf(ftool, "            %sValue = GuiSliderBar(%s, %sValue, %s);\n", control.name, GetControlRectangleText(i, control, config), control.name, GetControlParamText(control.type, control.name, config));
        } break;
        case GUI_PROGRESSBAR: fprintf(ftool, "            %sValue = GuiProgressBarEx(%s, %sValue, 0, 100, true);\n", control.name, GetControlRectangleText(i, control, config), control.name); break;
        case GUI_SPINNER: fprintf(ftool, "            if (GuiSpinner(%s, %sValue, 0, 100, 25, %sEditMode)) %sEditMode = !%sEditMode;\n", GetControlRectangleText(i, control, config), control.name, control.name, control.name, control.name); break;
        case GUI_COMBOBOX: fprintf(ftool, "            %sActive = GuiComboBox(%s, %sTextList, %s, %sActive);\n", control.name, GetControlRectangleText(i, control, config), control.name, GetControlParamText(control.type, control.name, config), control.name); break;
        case GUI_CHECKBOX:
        {
            if (control.text[0] != '\0') fprintf(ftool, "            %sChecked = GuiCheckBoxEx(%s, %sChecked, \"%s\");\n", control.name, GetControlRectangleText(i, control, config), control.name, control.text);
            else fprintf(ftool, "            %sChecked = GuiCheckBox(%s, %sChecked); \n", control.name, GetControlRectangleText(i, control, config), control.name);
        } break;
        case GUI_LISTVIEW: fprintf(ftool, "            if (GuiListView(%s, %sTextList, %s, &%sScrollIndex, &%sActive, %sEditMode)) %sEditMode = !%sEditMode;\n", GetControlRectangleText(i, control, config), control.name, GetControlParamText(control.type, control.name, config), control.name, control.name, control.name, control.name, control.name); break;
        case GUI_TEXTBOX: fprintf(ftool, "            GuiTextBox(%s, %sText, %s, true);\n", GetControlRectangleText(i, control, config), control.name, GetControlParamText(control.type, control.name, config)); break;
        case GUI_GROUPBOX: fprintf(ftool, "            GuiGroupBox(%s, \"%s\");\n", GetControlRectangleText(i, control, config), control.text); break;
        case GUI_WINDOWBOX:
        {
            fprintf(ftool, "            if (%sActive)\n            {\n", control.name);
            fprintf(ftool, "                %sActive = !GuiWindowBox(%s, \"%s\");\n", control.name, GetControlRectangleText(i, control, config), control.text);
            fprintf(ftool, "            }\n");
        }break;
        case GUI_DUMMYREC: fprintf(ftool, "            GuiDummyRec(%s, \"%s\");\n", GetControlRectangleText(i, control, config), control.text); break;
        case GUI_DROPDOWNBOX: fprintf(ftool, "            if (GuiDropdownBox(%s, %sTextList, %s, &%sActive, %sEditMode)) %sEditMode = !%sEditMode; \n", GetControlRectangleText(i, control, config), control.name, GetControlParamText(control.type, control.name, config), control.name, control.name, control.name, control.name); break;
        case GUI_STATUSBAR: fprintf(ftool, "            GuiStatusBar(%s, %sText, 10);\n", GetControlRectangleText(i, control, config), control.name); break;
        case GUI_COLORPICKER: fprintf(ftool, "            %sValue = GuiColorPicker(%s, %sValue);\n", control.name, GetControlRectangleText(i, control, config), control.name); break;
        case GUI_LINE: fprintf(ftool, "            GuiLine(%s, 1);\n", GetControlRectangleText(i, control, config)); break;
        case GUI_PANEL: fprintf(ftool, "            GuiPanel(%s);\n", GetControlRectangleText(i, control, config)); break;

        default: break;
    }
}

// Generate C code for gui layout
static void GenerateCode(const char *fileName, GuiLayoutConfig config)
{
    FILE *ftool = fopen(fileName, "wt");

    // File description info
    fprintf(ftool, "/*******************************************************************************************\n");
    fprintf(ftool, "*\n");
    fprintf(ftool, "*   %s - %s\n", config.name, config.description);
    fprintf(ftool, "*\n");
    fprintf(ftool, "*   LICENSE: zlib/libpng\n");
    fprintf(ftool, "*\n");
    fprintf(ftool, "*   Copyright (c) %i %s\n", 2018, config.company);
    fprintf(ftool, "*\n");
    fprintf(ftool, "**********************************************************************************************/\n\n");
    fprintf(ftool, "#include \"raylib.h\"\n\n");
    fprintf(ftool, "#define RAYGUI_IMPLEMENTATION\n");
    fprintf(ftool, "#include \"raygui.h\"\n\n");
    fprintf(ftool, "//----------------------------------------------------------------------------------\n");
    fprintf(ftool, "// Controls Functions Declaration\n");
    fprintf(ftool, "//----------------------------------------------------------------------------------\n");

    // Define required functions for calling
    for (int i = 0; i < layout.controlsCount; i++)
    {
        if (layout.controls[i].type == GUI_BUTTON) fprintf(ftool, "static void %s();        // %s: %s logic\n", layout.controls[i].name, controlTypeNameLow[layout.controls[i].type], layout.controls[i].name);
    }

    fprintf(ftool, "\n");
    fprintf(ftool, "//------------------------------------------------------------------------------------\n");
    fprintf(ftool, "// Program main entry point\n");
    fprintf(ftool, "//------------------------------------------------------------------------------------\n");
    fprintf(ftool, "int main()\n");
    fprintf(ftool, "{\n");
    fprintf(ftool, "    // Initialization\n");
    fprintf(ftool, "    //---------------------------------------------------------------------------------------\n");
    fprintf(ftool, "    int screenWidth = %i;\n", config.width);
    fprintf(ftool, "    int screenHeight = %i;\n\n", config.height);
    fprintf(ftool, "    InitWindow(screenWidth, screenHeight, \"%s\");\n\n", config.name);

    fprintf(ftool, "    // %s: controls initialization\n", config.name);
    fprintf(ftool, "    //----------------------------------------------------------------------------------\n");

    // Anchors points export code
    if (config.exportAnchors)
    {
        fprintf(ftool, "    // Anchor points\n");

        for(int i = 0; i < MAX_ANCHOR_POINTS; i++)
        {
            for (int j = 0; j < layout.controlsCount; j++)
            {
                if (layout.controls[j].ap->id == layout.anchors[i].id)
                {
                    if ((!config.exportAnchor0) && layout.controls[j].ap->id == 0) break;
                    fprintf(ftool, "    Vector2 %s%02i = { %i, %i };\n", "anchor", i, layout.anchors[i].x, layout.anchors[i].y);
                    break;
                }
            }
        }
    }

    if (!config.fullComments) fprintf(ftool, "\n");

    // Generate controls required variables code
    for (int i = 0; i < layout.controlsCount; i++) WriteControlsVariables(ftool, layout.controls[i], config);

    if (config.defineRecs)
    {
        // Define controls rectangles
        fprintf(ftool, "\n    // Define controls rectangles\n");
        fprintf(ftool, "    Rectangle layoutRecs[%i] = {\n", layout.controlsCount);

        for (int i = 0; i < layout.controlsCount; i++)
        {
            fprintf(ftool, "        %s", GetControlRectangleText(i, layout.controls[i], config));
            fprintf(ftool, (i == layout.controlsCount - 1) ? "        // %s: %s\n    };\n\n" : ",        // %s: %s\n", controlTypeNameLow[layout.controls[i].type], layout.controls[i].name);
        }
    }

    fprintf(ftool, "    //----------------------------------------------------------------------------------\n\n");

    fprintf(ftool, "    SetTargetFPS(60);\n");
    fprintf(ftool, "    //--------------------------------------------------------------------------------------\n\n");
    fprintf(ftool, "    // Main game loop\n");
    fprintf(ftool, "    while (!WindowShouldClose())    // Detect window close button or ESC key\n");
    fprintf(ftool, "    {\n");
    fprintf(ftool, "        // Update\n");
    fprintf(ftool, "        //----------------------------------------------------------------------------------\n");
    fprintf(ftool, "        // TODO: Implement required update logic\n");
    fprintf(ftool, "        //----------------------------------------------------------------------------------\n\n");
    fprintf(ftool, "        // Draw\n");
    fprintf(ftool, "        //----------------------------------------------------------------------------------\n");
    fprintf(ftool, "        BeginDrawing();\n\n");
    fprintf(ftool, "            ClearBackground(GetColor(style[DEFAULT_BACKGROUND_COLOR]));\n\n");

    fprintf(ftool, "            // raygui: controls drawing\n");
    fprintf(ftool, "            //----------------------------------------------------------------------------------\n");

    // Generate controls drawing code
    for (int i = 0; i < layout.controlsCount; i++) WriteControlsDrawing(ftool, i, layout.controls[i], config);

    fprintf(ftool, "            //----------------------------------------------------------------------------------\n\n");
    fprintf(ftool, "        EndDrawing();\n");
    fprintf(ftool, "        //----------------------------------------------------------------------------------\n");
    fprintf(ftool, "    }\n\n");
    fprintf(ftool, "    // De-Initialization\n");
    fprintf(ftool, "    //--------------------------------------------------------------------------------------\n");
    fprintf(ftool, "    CloseWindow();        // Close window and OpenGL context\n");
    fprintf(ftool, "    //--------------------------------------------------------------------------------------\n\n");
    fprintf(ftool, "    return 0;\n");
    fprintf(ftool, "}\n\n");

    fprintf(ftool, "//------------------------------------------------------------------------------------\n");
    fprintf(ftool, "// Controls Functions Definitions (local)\n");
    fprintf(ftool, "//------------------------------------------------------------------------------------\n");

    for (int i = 0; i < layout.controlsCount; i++)
    {
        if (layout.controls[i].type == GUI_BUTTON)
        {
            fprintf(ftool, "// %s: %s logic\n", controlTypeNameLow[layout.controls[i].type], layout.controls[i].name);
            fprintf(ftool, "static void %s()\n{\n    // TODO: Implement control logic\n}\n\n", layout.controls[i].name);
        }
    }

    fclose(ftool);
}

// New exportation option (self-contained modules):
/*
// Gui window structure declaration
typedef struct {
    Vector2 position;
    bool active;
    int width;
    int height;
} GuiWindowAboutState;

// Initialization
GuiWindowAboutState windowAboutState = {
    .position = (Vector2){ 0, 0 };
    .active = false;
    .width = 330;
    .height = 380;
};

// Update & Draw
GuiWindowAbout(&windowAboutState);

// Function definition
static void GuiWindowAbout(GuiWindowAboutState *state)
*/
