/*******************************************************************************************
*
*   rGuiLayout v2.0-dev - A simple and easy-to-use raygui layouts editor
*
*   CONFIGURATION:
*
*   #define VERSION_ONE
*       Enable PRO features for the tool. Usually command-line and export options related.
*
*   DEPENDENCIES:
*       raylib 2.3              - Windowing/input management and drawing.
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
*       Ramon Santamaria (@raysan5):    Supervision, design and maintenance.
*       Sergio Martinez (@anidealgift): Developer and designer (v2.0 - Jan.2019)
*       Adria Arranz (@Adri102):        Developer and designer (v1.0 - Jun.2018)
*       Jordi Jorba (@KoroBli):         Developer and designer (v1.0 - Jun.2018)
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
#include "rguilayout.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                         // Required for: IMGUI controls

#define CODEGEN_IMPLEMENTATION
#include "codegen.h"

#undef RAYGUI_IMPLEMENTATION
#define GUI_WINDOW_CODEGEN_IMPLEMENTATION
#include "gui_window_codegen.h"

#include "external/easings.h"               // Required for: Easing animations math
#include "external/tinyfiledialogs.h"       // Required for: Open/Save file dialogs

#include <stdlib.h>                         // Required for: calloc(), free()
#include <stdarg.h>                         // Required for: va_list, va_start(), vfprintf(), va_end()
#include <string.h>                         // Required for: strcpy(), strcat(), strlen()
#include <stdio.h>                          // Required for: FILE, fopen(), fclose()...
#include <ctype.h>                          // Required for: toupper(), tolower()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define VERSION_ONE                         // Enable PRO version features

#define TOOL_VERSION_TEXT     "2.0-dev"     // Tool version string


#define ANCHOR_RADIUS               20      // Default anchor radius
#define MIN_CONTROL_SIZE            10      // Minimum control size
#define GRID_LINE_SPACING            5      // Grid line spacing in pixels
#define MOUSE_SCALE_MARK_SIZE       12      // Mouse scale mark size (bottom-right corner)

#define MOVEMENT_FRAME_SPEED        10      // Controls movement speed in pixels per frame
#define PANELS_EASING_FRAMES        60      // Controls the easing time in frames

#define MAX_UNDO_LEVELS             10       // Undo levels supported for the ring buffer

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static GuiLayout layout = { 0 };            // Full gui layout
static char loadedFileName[256] = { 0 };    // Loaded layout file name
static bool saveChangesRequired = false;    // Flag to notice save changes are required

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(VERSION_ONE)
static void ShowCommandLineInfo(void);                      // Show command line usage info
static void ProcessCommandLine(int argc, char *argv[]);     // Process command line input
#endif

// Load/Save/Export data functions
static void LoadLayout(const char *fileName);                   // Load raygui layout (.rgl), text or binary
static void SaveLayout(const char *fileName, bool binary);      // Save raygui layout (.rgl), text or binary

static void DialogLoadLayout(void);                             // Show dialog: load layout file (.rgl)
static bool DialogSaveLayout(void);                             // Show dialog: save layout file (.rgl)
static void DialogExportLayout(unsigned char *toolstr, const char *name);         // Show dialog: export layout file (.c)

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
#if defined(VERSION_ONE)
        else
        {
            ProcessCommandLine(argc, argv);
            return 0;
        }
#endif      // VERSION_ONE
    }

    // GUI usage mode - Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1000;
    const int screenHeight = 800;

    SetTraceLog(0);                             // Disable trace log messsages
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);      // Window configuration flags
    InitWindow(screenWidth, screenHeight, FormatText("rGuiLayout v%s - A simple and easy-to-use raygui layouts editor", TOOL_VERSION_TEXT));
    SetWindowMinSize(800, 800);
    SetExitKey(0);

    // General app variables
    Vector2 mouse = { -1, -1 };             // Mouse position
    bool exitWindow = false;                // Exit window flag
    bool showGrid = true;                   // Show grid flag (KEY_G)

    // Modes
    bool dragMode = false;                  // Control drag mode
    bool useGlobalPos = false;              // Control global position mode
    bool snapMode = false;                  // Snap mode flag (KEY_S)
    bool textEditMode = false;              // Control text edit mode (KEY_T)
    bool nameEditMode = false;              // Control name edit mode (KEY_N)
    bool orderEditMode = false;             // Control order edit mode (focusedControl != -1 + KEY_LEFT_ALT)
    bool resizeMode = false;                // Control size mode (controlSelected != -1 + KEY_LEFT_ALT)
    bool precisionMode = false;             // Control precision mode (KEY_LEFT_SHIFT)
    bool showNamesMode = false;             // Show names of all controls
    bool refWindowEditMode = false;         // Refence window edit mode
    bool mouseScaleReady = false;           // Mouse is on position to start control scaling
    bool mouseScaleMode = false;            // Control is being scaled by mouse
    
    bool multiSelectMode = false;
    Rectangle multiSelectRec = { 0 };
    Vector2 multiSelectStartPos = { 0 };
    int multiSelectControls[20] = { -1 };
    int multiSelectCount = 0;

    int framesCounter = 0;
    int framesCounterMovement = 0;

    // Controls variables
    int selectedControl = -1;
    int storedControl = -1;
    int focusedControl = -1;
    int selectedType = GUI_WINDOWBOX;
    int selectedTypeDraw = GUI_LABEL;
    Vector2 panOffset = { 0 };
    Vector2 prevPosition = { 0 };
    Color selectedControlColor = RED;
    Color positionColor = MAROON;

    int movePixel = 1;
    int movePerFrame = 1;

    int textArrayPos = 0;

    const char *listViewData[4] = { "One", "Two", "Three", "Four" }; // ListView default data

    // Anchors control variables
    GuiAnchorPoint auxAnchor = { 9, 0, 0, 0 };
    bool anchorEditMode = false;
    bool anchorLinkMode = false;
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
        (Rectangle){ 0, 0, 125, 30 },           // GUI_LABELBUTTON
        (Rectangle){ 0, 0, 125, 125 },            // GUI_IMAGEBUTTONEX
        (Rectangle){ 0, 0, 15, 15},             // GUI_CHECKBOX
        (Rectangle){ 0, 0, 90, 25 },            // GUI_TOGGLE
        (Rectangle){ 0, 0, 125/3, 25 },           // GUI_TOGGLEGROUP
        (Rectangle){ 0, 0, 125, 25 },           // GUI_COMBOBOX
        (Rectangle){ 0, 0, 125, 25 },           // GUI_DROPDOWNBOX
        (Rectangle){ 0, 0, 125, 25 },           // GUI_TEXTBOX
        (Rectangle){ 0, 0, 125, 75 },           // GUI_TEXTBOXMULTI
        (Rectangle){ 0, 0, 125, 25 },           // GUI_VALUEBOX
        (Rectangle){ 0, 0, 125, 25 },           // GUI_SPINNER
        (Rectangle){ 0, 0, 125, 15 },           // GUI_SLIDER
        (Rectangle){ 0, 0, 125, 15 },           // GUI_SLIDERBAR
        (Rectangle){ 0, 0, 125, 15 },           // GUI_PROGRESSBAR
        (Rectangle){ 0, 0, 125, 25 },           // GUI_STATUSBAR
        (Rectangle){ 0, 0, 125, 75 },           // GUI_SCROLLPANEL
        (Rectangle){ 0, 0, 125, 75 },           // GUI_LISTVIEW
        (Rectangle){ 0, 0, 95, 95 },            // GUI_COLORPICKER
        (Rectangle){ 0, 0, 125, 30 },            // GUI_DUMMYREC
    };

    // Initialize anchor points to default values
    for (int i = 0; i < MAX_ANCHOR_POINTS; i++)
    {
        layout.anchors[i].id = i;
        layout.anchors[i].x = 0;
        layout.anchors[i].y = 0;
        layout.anchors[i].enabled = false;
        layout.anchors[i].hidding = false;
        memset(layout.anchors[i].name, 0, MAX_ANCHOR_NAME_LENGTH);
        if (i == 0) strcpy(layout.anchors[i].name, "anchorMain");
        else strcpy(layout.anchors[i].name, FormatText("anchor%02i", i));
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

    layout.refWindow = (Rectangle){ 0, 0, -1, -1};

    // Define palette variables
    Rectangle palettePanel = { 0, 15, 135, 835 };
    int paletteOffset = 0;
    int paletteSelect = -1;
    int paletteCounter = PANELS_EASING_FRAMES;
    int paletteStartPositionX = GetScreenWidth() + 130;
    int paletteDeltaPositionX = 0;
    bool paletteActive = false;

    // Define palette rectangles
    Rectangle paletteRecs[CONTROLS_TYPE_NUM] = {
        (Rectangle){ 5, 5, 125, 50 },     // WindowBox
        (Rectangle){ 5, 65, 125, 30 },    // GroupBox
        (Rectangle){ 5, 105, 125, 25 },   // Line
        (Rectangle){ 5, 140, 125, 35 },   // Panel
        (Rectangle){ 5, 185, 126, 25 },   // Label
        (Rectangle){ 5, 220, 125, 30 },   // Button
        (Rectangle){ 5, 220, 125, 30 },   // LabelButton
        (Rectangle){ 5, 220, 125, 30 },   // ImageButtonEx
        (Rectangle){ 105, 265, 15, 15 },  // CheckBox
        (Rectangle){ 5, 260, 90, 25 },    // Toggle
        (Rectangle){ 5, 295, 125, 25 },   // ToggleGroup
        (Rectangle){ 5, 330, 125, 25 },   // ComboBox
        (Rectangle){ 5, 365, 125, 25 },   // DropdownBox
        (Rectangle){ 5, 470, 125, 25 },   // TextBox
        (Rectangle){ 5, 470, 125, 25 },   // TextBoxMulti
        (Rectangle){ 5, 435, 125, 25 },   // ValueBox
        (Rectangle){ 5, 400, 125, 25 },   // Spinner
        (Rectangle){ 5, 505, 125, 15 },   // Slider
        (Rectangle){ 5, 530, 125, 15 },   // SliderBar
        (Rectangle){ 5, 555, 125, 15 },   // ProgressBar
        (Rectangle){ 5, 580, 125, 25 },   // StatusBar
        (Rectangle){ 5, 580, 125, 25 },   // ScrollPanel
        (Rectangle){ 5, 615, 125, 75 },   // ListView
        (Rectangle){ 5, 700, 95, 95 },    // ColorPicker
        (Rectangle){ 5, 800, 125, 30 },    // MessageBox
        (Rectangle){ 5, 800, 125, 30 },    // DummyRec
        (Rectangle){ 5, 800, 125, 30 }    // Grid

    };

    // Tracemap (background image for reference) variables
    Texture2D tracemap = { 0 };
    Rectangle tracemapRec = { 0 };
    bool tracemapBlocked = false;
    bool tracemapFocused = false;
    bool tracemapSelected = false;
    float tracemapFade = 0.5f;
    Color tracemapColor = RED;

    // Track previous text/name to cancel editing
    char prevText[MAX_CONTROL_TEXT_LENGTH];
    char prevName[MAX_CONTROL_NAME_LENGTH];

    // Close layout window variables
    bool closingWindowActive = false;

    // Export Window Layout: controls initialization
    //----------------------------------------------------------------------------------------
    GuiWindowCodegenState windowCodegenState = InitGuiWindowCodegen();
    //----------------------------------------------------------------------------------------

    // Generate code configuration
    GuiLayoutConfig config = { 0 };
    strcpy(config.name, "window_codegen");
    strcpy(config.version, TOOL_VERSION_TEXT);
    strcpy(config.company, "raylib technologies");
    strcpy(config.description, "tool description");
    config.exportAnchors = false;
    config.defineRecs = false;
    config.defineTexts = false;
    config.fullComments = false;

    // Delete current layout and reset variables
    bool resetWindowActive = false;
    bool resetLayout = false;

    // Controls temp variables
    int dropdownBoxActive = 0;
    int spinnerValue = 0;
    int valueBoxValue = 0;
    int listViewScrollIndex = 0;
    int listViewActive = 0;

    // Undo system variables
    GuiLayout *undoLayouts = NULL;
    int currentUndoIndex = 0;
    int firstUndoIndex = 0;
    int lastUndoIndex = 0;
    int undoFrameCounter = 0;

    undoLayouts = (GuiLayout *)calloc(MAX_UNDO_LEVELS, sizeof(GuiLayout));
    for (int i = 0; i < MAX_UNDO_LEVELS; i++) memcpy(&undoLayouts[i], &layout, sizeof(GuiLayout));

    SetTargetFPS(120);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!exitWindow)             // Detect window close button
    {
        // Basic program flow logic
        mouse = GetMousePosition();
        if (WindowShouldClose()) exitWindow = true;

        // Undo layout change logic
        //----------------------------------------------------------------------------------
        // Every second check if current layout has changed and record a new undo state
        if (!dragMode && !orderEditMode && !resizeMode && !refWindowEditMode &&
            !textEditMode && !nameEditMode && !anchorEditMode && !anchorLinkMode && !anchorMoveMode)
        {
            undoFrameCounter++;

            if (undoFrameCounter >= 120)
            {
                if (memcmp(&undoLayouts[currentUndoIndex], &layout, sizeof(GuiLayout)) != 0)
                {
                    // Move cursor to next available position to record undo
                    currentUndoIndex++;
                    if (currentUndoIndex >= MAX_UNDO_LEVELS) currentUndoIndex = 0;
                    if (currentUndoIndex == firstUndoIndex) firstUndoIndex++;
                    if (firstUndoIndex >= MAX_UNDO_LEVELS) firstUndoIndex = 0;

                    undoLayouts[currentUndoIndex] = layout;

                    lastUndoIndex = currentUndoIndex;
                    
                    // Set a '*' mark on loaded file name to notice save requirement
                    if ((loadedFileName[0] != '\0') && !saveChangesRequired)
                    {
                        saveChangesRequired = true;
                        SetWindowTitle(FormatText("rGuiLayout v%s - %s*", TOOL_VERSION_TEXT, GetFileName(loadedFileName)));
                    }
                }

                undoFrameCounter = 0;
            }
        }
        else undoFrameCounter = 120;

        // Recover previous layout state from buffer
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z))
        {
            if (currentUndoIndex != firstUndoIndex)
            {
                currentUndoIndex--;
                if (currentUndoIndex < 0) currentUndoIndex = MAX_UNDO_LEVELS - 1;

                if (memcmp(&undoLayouts[currentUndoIndex], &layout, sizeof(GuiLayout)) != 0) layout = undoLayouts[currentUndoIndex];
            }
        }

        // Recover next layout state from buffer
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Y))
        {
            if (currentUndoIndex != lastUndoIndex)
            {
                int nextUndoIndex = currentUndoIndex + 1;
                if (nextUndoIndex >= MAX_UNDO_LEVELS) nextUndoIndex = 0;

                if (nextUndoIndex != firstUndoIndex)
                {
                    currentUndoIndex = nextUndoIndex;

                    if (memcmp(&undoLayouts[currentUndoIndex], &layout, sizeof(GuiLayout)) != 0) layout = undoLayouts[currentUndoIndex];
                }
            }
        }
        //----------------------------------------------------------------------------------

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

                for (int i = 0; i < MAX_UNDO_LEVELS; i++) memcpy(&undoLayouts[i], &layout, sizeof(GuiLayout));
                currentUndoIndex = 0;
                firstUndoIndex = 0;
            }
            else if (IsFileExtension(droppedFileName, ".rgs")) GuiLoadStyle(droppedFileName);
            else if (IsFileExtension(droppedFileName, ".png")) // Tracemap image
            {
                if (tracemap.id > 0) UnloadTexture(tracemap);
                tracemap = LoadTexture(droppedFileName);
                tracemapRec = (Rectangle){30, 30, tracemap.width, tracemap.height};
            }

            ClearDroppedFiles();
        }
        //----------------------------------------------------------------------------------

        // Keyboard shortcuts
        //----------------------------------------------------------------------------------
        // Open layout file dialog logic
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O))
        {
            // Open file dialog
            const char *filters[] = { "*.rgl" };
            const char *fileName = tinyfd_openFileDialog("Load raygui layout file", "", 1, filters, "raygui Layout Files (*.rgl)", 0);

            if (fileName != NULL) LoadLayout(fileName);
        }

        // Save layout file dialog logic
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_S)) DialogSaveLayout();
        else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
        {
            if (loadedFileName[0] == '\0') DialogSaveLayout();
            else
            {
                SaveLayout(loadedFileName, false);
                saveChangesRequired = false;
                SetWindowTitle(FormatText("rGuiLayout v%s - %s", TOOL_VERSION_TEXT, GetFileName(loadedFileName)));
            }
        }

        // Show code generation window
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E)) { }

        if (!textEditMode && !nameEditMode)
        {
            // TODO: review -- Show save layout message window on ESC
            if (IsKeyPressed(KEY_ESCAPE))
            {
                // Close windows logic
                if (windowCodegenState.codeGenWindowActive) windowCodegenState.codeGenWindowActive = false;
                else if (resetWindowActive) resetWindowActive = false;
                else if ((layout.controlsCount <= 0) && (layout.anchorsCount <= 1)) exitWindow = true;  // Quit application
                else
                {
                    closingWindowActive = !closingWindowActive;
                    selectedControl = -1;
                    selectedAnchor = -1;
                }
            }

            if (!windowCodegenState.codeGenWindowActive && !closingWindowActive && !resetWindowActive)
            {
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
                if (IsKeyPressed(KEY_F)) useGlobalPos = !useGlobalPos;

                // Enable anchor mode editing
                if (IsKeyPressed(KEY_A)) anchorEditMode = true;
                if (IsKeyReleased(KEY_A)) anchorEditMode = false;

                // Enable show names mode
                if (IsKeyPressed(KEY_N)) showNamesMode = true;
                if (IsKeyReleased(KEY_N)) showNamesMode = false;

                // Enable/disable order edit mode
                if (IsKeyPressed(KEY_LEFT_ALT)) orderEditMode = true;
                if (IsKeyReleased(KEY_LEFT_ALT))  orderEditMode = false;

                // Enable/diable precision mode
                if (IsKeyDown(KEY_LEFT_SHIFT)) precisionMode = true;
                if (IsKeyReleased(KEY_LEFT_SHIFT)) precisionMode = false;

                // Enable/disable size modifier mode
                if (IsKeyDown(KEY_LEFT_CONTROL)) resizeMode = true;
                if (IsKeyReleased(KEY_LEFT_CONTROL)) resizeMode = false;

                // Enable/disable grid
                if (IsKeyPressed(KEY_G)) showGrid = !showGrid;

                // Enable/disable texture editing mode
                if (tracemap.id > 0 && IsKeyPressed(KEY_SPACE))
                {
                    if (tracemapSelected) tracemapBlocked = true;
                    else if (tracemapBlocked && tracemapFocused) tracemapBlocked = false;
                }

                if (IsKeyDown(KEY_LEFT_CONTROL))
                {
                    // Open reset window
                    if (IsKeyPressed(KEY_N)) resetWindowActive = true;

                    // Activate code generation export window
                    if (IsKeyPressed(KEY_ENTER))
                    {
                        strcpy(config.name, windowCodegenState.toolNameText);
                        strcpy(config.version, windowCodegenState.toolVersionText);
                        strcpy(config.company, windowCodegenState.companyText);
                        strcpy(config.description, windowCodegenState.toolDescriptionText);
                        config.exportAnchors = windowCodegenState.exportAnchorsChecked;
                        config.defineRecs = windowCodegenState.defineRecsChecked;
                        config.defineTexts = windowCodegenState.defineTextsChecked;
                        config.fullComments = windowCodegenState.fullCommentsChecked;

                        // unsigned char *template = LoadText("gui_code_template.c");
                        unsigned char *template = LoadText("gui_window_template.h");

                        windowCodegenState.generatedCode = GenerateLayoutCode(template, layout, config);
                        windowCodegenState.codeGenWindowActive = true;

                        free(template);
                    }
                }
            }
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
        palettePanel.x = GetScreenWidth() + paletteOffset;
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && (focusedAnchor == -1) && (focusedControl == -1))
        {
            paletteCounter = 0;
            paletteStartPositionX = paletteOffset;

            if (paletteActive) paletteDeltaPositionX = 0 - paletteStartPositionX;
            else paletteDeltaPositionX = -(palettePanel.width + 15) - paletteStartPositionX;

            paletteActive = !paletteActive;
        }
        if (paletteCounter <= PANELS_EASING_FRAMES)
        {
            paletteCounter++;
            paletteOffset = (int)EaseCubicInOut(paletteCounter, paletteStartPositionX, paletteDeltaPositionX, PANELS_EASING_FRAMES);
        }

        // Controls palette selector logic
        else if (paletteActive)
        {
            for (int i = 0; i < CONTROLS_TYPE_NUM; i++)
            {
                if (CheckCollisionPointRec(mouse, (Rectangle){palettePanel.x + paletteRecs[i].x, palettePanel.y + paletteRecs[i].y, paletteRecs[i].width, paletteRecs[i].height}))
                {
                    paletteSelect = i;
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedType = i;
                    break;
                }
                else paletteSelect = -1;
            }
        }
        //----------------------------------------------------------------------------------------------

        // Layout edition logic
        //----------------------------------------------------------------------------------------------
        if (!closingWindowActive && !windowCodegenState.codeGenWindowActive && !resetWindowActive)
        {
            if (!nameEditMode)
            {
                if (!textEditMode)
                {
                    // Mouse snap logic
                    //----------------------------------------------------------------------------------------------
                    if (snapMode && !anchorLinkMode)
                    {
                        int offsetX = (int)mouse.x%GRID_LINE_SPACING;
                        int offsetY = (int)mouse.y%GRID_LINE_SPACING;

                        if (offsetX >= GRID_LINE_SPACING/2) mouse.x += (GRID_LINE_SPACING - offsetX);
                        else mouse.x -= offsetX;

                        if (offsetY >= GRID_LINE_SPACING/2) mouse.y += (GRID_LINE_SPACING - offsetY);
                        else mouse.y -= offsetY;
                    }
                    //----------------------------------------------------------------------------------------------

                    // Palette selected control
                    //----------------------------------------------------------------------------------------------
                    if (focusedControl == -1) selectedType -= GetMouseWheelMove();  // Select type with mouse wheel

                    if (selectedType < GUI_WINDOWBOX) selectedType = GUI_WINDOWBOX;
                    else if (selectedType > GUI_DUMMYREC) selectedType = GUI_DUMMYREC;

                    if (selectedType != selectedTypeDraw)
                    {
                        selectedControl = -1;
                        selectedTypeDraw = selectedType;
                    }

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
                    //----------------------------------------------------------------------------------------------

                    // Controls selection and edition logic
                    //----------------------------------------------------------------------------------------------
                    if (!CheckCollisionPointRec(mouse, palettePanel))
                    {
                        if (!dragMode)
                        {
                            focusedControl = -1;
                            
                            // Focus control logic
                            if (!anchorEditMode && focusedAnchor == -1)
                            {
                                for (int i = layout.controlsCount; i >= 0; i--)
                                {
                                    if (!layout.controls[i].ap->hidding)
                                    {
                                        Rectangle layoutRec = layout.controls[i].rec;
                                        if (layout.controls[i].type == GUI_WINDOWBOX) layoutRec.height = WINDOW_SATUSBAR_HEIGHT;  // Defined inside raygui.h
                                        else if (layout.controls[i].type == GUI_GROUPBOX)
                                        {
                                            layoutRec.y -= 10;
                                            layoutRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE) * 2;
                                        }

                                        if (layout.controls[i].ap->id > 0)
                                        {
                                            layoutRec.x += layout.controls[i].ap->x;
                                            layoutRec.y += layout.controls[i].ap->y;
                                        }

                                        if (CheckCollisionPointRec(mouse, layoutRec))
                                        {
                                            focusedControl = i;
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        // On focused control
                        if (focusedControl == -1)
                        {
                            if (focusedAnchor == -1 && selectedAnchor == -1 && selectedControl == -1 && !tracemapFocused && !tracemapSelected)
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

                                        if ((layout.controls[layout.controlsCount].type == GUI_LABEL)
                                            || (layout.controls[layout.controlsCount].type == GUI_TEXTBOX)
                                            || (layout.controls[layout.controlsCount].type == GUI_TEXTBOXMULTI)
                                            || (layout.controls[layout.controlsCount].type == GUI_BUTTON)
                                            || (layout.controls[layout.controlsCount].type == GUI_LABELBUTTON)
                                            || (layout.controls[layout.controlsCount].type == GUI_IMAGEBUTTONEX)
                                            || (layout.controls[layout.controlsCount].type == GUI_CHECKBOX)
                                            || (layout.controls[layout.controlsCount].type == GUI_TOGGLE)
                                            || (layout.controls[layout.controlsCount].type == GUI_GROUPBOX)
                                            || (layout.controls[layout.controlsCount].type == GUI_WINDOWBOX)
                                            || (layout.controls[layout.controlsCount].type == GUI_STATUSBAR)
                                            || (layout.controls[layout.controlsCount].type == GUI_DUMMYREC)) 
                                        {
                                            strcpy(layout.controls[layout.controlsCount].text, "SAMPLE TEXT");
                                        }
                                            
                                        if ((layout.controls[layout.controlsCount].type == GUI_TOGGLEGROUP)
                                            || (layout.controls[layout.controlsCount].type == GUI_COMBOBOX)
                                            || (layout.controls[layout.controlsCount].type == GUI_DROPDOWNBOX))
                                        {
                                            strcpy(layout.controls[layout.controlsCount].text, "ONE;TWO;THREE");
                                        }

                                        strcpy(layout.controls[layout.controlsCount].name, FormatText("%s%03i", controlTypeName[layout.controls[layout.controlsCount].type], layout.controlsCount));

                                        layout.controls[layout.controlsCount].ap = &layout.anchors[0];        // Default anchor point (0, 0)

                                        // If we create new control inside a windowbox, then anchor the new control to the windowbox anchor
                                        for (int i = layout.controlsCount; i >= 0; i--)
                                        {
                                            if (layout.controls[i].type == GUI_WINDOWBOX || layout.controls[i].type == GUI_GROUPBOX)
                                            {
                                                if (CheckCollisionPointRec(mouse, (Rectangle){ layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }))
                                                {
                                                    layout.controls[layout.controlsCount].ap = layout.controls[i].ap;
                                                    break;
                                                }
                                            }
                                        }

                                        // Create anchor for windowbox control if we can
                                        if (layout.anchorsCount < MAX_ANCHOR_POINTS && (layout.controls[layout.controlsCount].type == GUI_WINDOWBOX || layout.controls[layout.controlsCount].type == GUI_GROUPBOX))
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

                                        if (layout.controls[layout.controlsCount].ap->id > 0)
                                        {
                                            layout.controls[layout.controlsCount].rec.x -= layout.controls[layout.controlsCount].ap->x;
                                            layout.controls[layout.controlsCount].rec.y -= layout.controls[layout.controlsCount].ap->y;
                                        }
                                        layout.controlsCount++;

                                        focusedControl = layout.controlsCount - 1;
                                        selectedControl = layout.controlsCount - 1;
                                    }
                                }
                            }
                        }
                        else //focusedControl != -1
                        {
                            //if (selectedControl == -1)
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

                        // Unselect control
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
                        {
                            selectedControl = focusedControl;
                            if (focusedAnchor != -1 || anchorLinkMode || anchorEditMode) selectedControl = -1;
                        }

                        // On selected control
                        if (selectedControl != -1)
                        {
                            // Mouse resize mode logic
                            //--------------------------------------------------------------------------
                            Rectangle rec = layout.controls[selectedControl].rec;
                            
                            if (CheckCollisionPointRec(mouse, rec) && 
                                CheckCollisionPointRec(mouse, (Rectangle){ rec.x + rec.width - MOUSE_SCALE_MARK_SIZE, rec.y + rec.height - MOUSE_SCALE_MARK_SIZE, MOUSE_SCALE_MARK_SIZE, MOUSE_SCALE_MARK_SIZE }))
                            {
                                mouseScaleReady = true;
                                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) mouseScaleMode = true;
                            }
                            else mouseScaleReady = false;
                            
                            if (mouseScaleMode)
                            {
                                mouseScaleReady = true;
                                
                                rec.width = (mouse.x - rec.x);
                                rec.height = (mouse.y - rec.y);
                                
                                if (rec.width < MOUSE_SCALE_MARK_SIZE) rec.width = MOUSE_SCALE_MARK_SIZE;
                                if (rec.height < MOUSE_SCALE_MARK_SIZE) rec.height = MOUSE_SCALE_MARK_SIZE;
                                
                                layout.controls[selectedControl].rec = rec;
                                
                                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) mouseScaleMode = false;
                            }
                            //--------------------------------------------------------------------------
                            
                            if (!anchorLinkMode)
                            {
                                if (dragMode && !mouseScaleMode)
                                {
                                    // Drag controls
                                    int controlPosX = prevPosition.x + (mouse.x - panOffset.x);
                                    int controlPosY = prevPosition.y + (mouse.y - panOffset.y);

                                    if (snapMode)
                                    {
                                        int offsetX = (int)controlPosX%movePixel;
                                        int offsetY = (int)controlPosY%movePixel;

                                        if (offsetX >= movePixel/2) controlPosX += (movePixel - offsetX);
                                        else controlPosX -= offsetX;

                                        if (offsetY >= movePixel/2) controlPosY += (movePixel - offsetY);
                                        else controlPosY -= offsetY;
                                    }

                                    if (useGlobalPos && (layout.controls[selectedControl].ap->id != 0))
                                    {
                                        controlPosX -= layout.controls[selectedControl].ap->x;
                                        controlPosY -= layout.controls[selectedControl].ap->y;
                                    }

                                    layout.controls[selectedControl].rec.x = controlPosX;
                                    layout.controls[selectedControl].rec.y = controlPosY;

                                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragMode = false;
                                }
                                else
                                {
                                    if (resizeMode)
                                    {
                                        if (IsKeyPressed(KEY_R) && layout.controls[selectedControl].type == GUI_WINDOWBOX)
                                        {
                                            Rectangle rec = layout.controls[selectedControl].rec;
                                            if (layout.controls[selectedControl].ap->id > 0)
                                            {
                                                rec.x += layout.controls[selectedControl].ap->x;
                                                rec.y += layout.controls[selectedControl].ap->y;
                                            }
                                            layout.anchors[0].x = rec.x;
                                            layout.anchors[0].y = rec.y;
                                            layout.refWindow = (Rectangle){layout.anchors[0].x, layout.anchors[0].y, rec.width, rec.height};

                                        }

                                        // Duplicate control
                                        if (IsKeyPressed(KEY_D))
                                        {
                                            layout.controls[layout.controlsCount].id = layout.controlsCount;
                                            layout.controls[layout.controlsCount].type = layout.controls[selectedControl].type;
                                            layout.controls[layout.controlsCount].rec = layout.controls[selectedControl].rec;
                                            layout.controls[layout.controlsCount].rec.x += 10;
                                            layout.controls[layout.controlsCount].rec.y += 10;
                                            strcpy(layout.controls[layout.controlsCount].text, layout.controls[selectedControl].text);
                                            strcpy(layout.controls[layout.controlsCount].name, FormatText("%s%03i", controlTypeName[layout.controls[layout.controlsCount].type], layout.controlsCount));
                                            layout.controls[layout.controlsCount].ap = layout.controls[selectedControl].ap;            // Default anchor point (0, 0)

                                            layout.controlsCount++;

                                            selectedControl = layout.controlsCount - 1;
                                        }

                                        // Resize control
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

                                        // Minimum size limit
                                        if (layout.controls[selectedControl].rec.width < MIN_CONTROL_SIZE) layout.controls[selectedControl].rec.width = MIN_CONTROL_SIZE;
                                        if (layout.controls[selectedControl].rec.height < MIN_CONTROL_SIZE) layout.controls[selectedControl].rec.height = MIN_CONTROL_SIZE;
                                    }
                                    else
                                    {
                                        // Move controls with arrows
                                        int controlPosX = (int)layout.controls[selectedControl].rec.x;
                                        int controlPosY = (int)layout.controls[selectedControl].rec.y;

                                        if (useGlobalPos && (layout.controls[selectedControl].ap->id != 0))
                                        {
                                            controlPosX += layout.controls[selectedControl].ap->x;
                                            controlPosY += layout.controls[selectedControl].ap->y;
                                        }

                                        int offsetX = (int)controlPosX%movePixel;
                                        int offsetY = (int)controlPosY%movePixel;

                                        if (precisionMode)
                                        {
                                            if (IsKeyPressed(KEY_RIGHT))  controlPosX += (movePixel - offsetX);
                                            else if (IsKeyPressed(KEY_LEFT))
                                            {
                                                if (offsetX == 0) offsetX = movePixel;
                                                controlPosX -= offsetX;
                                            }

                                            if (IsKeyPressed(KEY_DOWN)) controlPosY += (movePixel - offsetY);
                                            else if (IsKeyPressed(KEY_UP))
                                            {
                                                if (offsetY == 0) offsetY = movePixel;
                                                controlPosY -= offsetY;
                                            }

                                            framesCounterMovement = 0;
                                        }
                                        else
                                        {
                                            framesCounterMovement++;

                                            if ((framesCounterMovement%movePerFrame) == 0)
                                            {
                                                if (IsKeyDown(KEY_RIGHT)) controlPosX += (movePixel - offsetX);
                                                else if (IsKeyDown(KEY_LEFT))
                                                {
                                                    if (offsetX == 0) offsetX = movePixel;
                                                    controlPosX -= offsetX;
                                                }

                                                if (IsKeyDown(KEY_DOWN)) controlPosY += (movePixel - offsetY);
                                                else if (IsKeyDown(KEY_UP))
                                                {
                                                    if (offsetY == 0) offsetY = movePixel;
                                                    controlPosY -= offsetY;
                                                }

                                                framesCounterMovement = 0;
                                            }
                                        }

                                        if (useGlobalPos && (layout.controls[selectedControl].ap->id != 0))
                                        {
                                            controlPosX -= layout.controls[selectedControl].ap->x;
                                            controlPosY -= layout.controls[selectedControl].ap->y;
                                        }
                                        
                                        layout.controls[selectedControl].rec.x = controlPosX;
                                        layout.controls[selectedControl].rec.y = controlPosY;
                                        //---------------------------------------------------------------------

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

                                        // Enable drag mode (if not on mouse scale mode)
                                        else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !mouseScaleMode)
                                        {
                                            panOffset = mouse;
                                            
                                            if (useGlobalPos && (layout.controls[selectedControl].ap->id != 0))
                                            {
                                                prevPosition = (Vector2){ layout.controls[selectedControl].rec.x + layout.controls[selectedControl].ap->x, layout.controls[selectedControl].rec.y + layout.controls[selectedControl].ap->y };
                                            }
                                            else prevPosition = (Vector2){ layout.controls[selectedControl].rec.x, layout.controls[selectedControl].rec.y };
                                            
                                            dragMode = true;
                                        }

                                        // Enable anchor link mode
                                        else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) anchorLinkMode = true;

                                        // Enable text edit mode
                                        else if (IsKeyReleased(KEY_T))
                                        {
                                            if (layout.controls[selectedControl].type != GUI_LINE &&
                                                layout.controls[selectedControl].type != GUI_PANEL &&
                                                layout.controls[selectedControl].type != GUI_VALUEBOX &&
                                                layout.controls[selectedControl].type != GUI_SPINNER &&
                                                layout.controls[selectedControl].type != GUI_PROGRESSBAREX &&
                                                layout.controls[selectedControl].type != GUI_SCROLLPANEL &&
                                                layout.controls[selectedControl].type != GUI_LISTVIEW &&
                                                layout.controls[selectedControl].type != GUI_COLORPICKER)
                                            {
                                                strcpy(prevText, layout.controls[selectedControl].text);
                                                textEditMode = true;
                                            }
                                            else printf("Can't edit text to this control\n");
                                        }

                                        // Enable name edit mode
                                        else if (IsKeyReleased(KEY_N))
                                        {
                                            nameEditMode = true;
                                            strcpy(prevName, layout.controls[selectedControl].name);
                                        }
                                    }
                                }
                            }
                            else // anchorLinkMode == true
                            {
                                if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
                                {
                                    anchorLinkMode = false;

                                    if (layout.controls[selectedControl].ap->id > 0)
                                    {
                                        layout.controls[selectedControl].rec.x += layout.controls[selectedControl].ap->x;
                                        layout.controls[selectedControl].rec.y += layout.controls[selectedControl].ap->y;
                                    }
                                    layout.controls[selectedControl].ap = &layout.anchors[focusedAnchor];
                                    if (focusedAnchor > 0)
                                    {
                                        layout.controls[selectedControl].rec.x -= layout.anchors[focusedAnchor].x;
                                        layout.controls[selectedControl].rec.y -= layout.anchors[focusedAnchor].y;
                                    }
                                }
                            }
                        }
                    }
                    //----------------------------------------------------------------------------------------------

                    // Controls multi-selection and edition logic
                    //----------------------------------------------------------------------------------------------
                    if ((selectedControl == -1) && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
                    {
                        multiSelectMode = true;
                        multiSelectStartPos = mouse;
                        
                        multiSelectRec.x = multiSelectStartPos.x;
                        multiSelectRec.y = multiSelectStartPos.y;
                    }
                    
                    if (multiSelectMode)
                    {
                        multiSelectRec.width = mouse.x - multiSelectStartPos.x;
                        multiSelectRec.height = mouse.y - multiSelectStartPos.y;
                        
                        if (multiSelectRec.width < 0) 
                        {
                            multiSelectRec.x = mouse.x;
                            multiSelectRec.width *= -1;
                        }
                        
                        if (multiSelectRec.height < 0) 
                        {
                            multiSelectRec.y = mouse.y;
                            multiSelectRec.height *= -1;
                        }
                        
                        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
                        {
                            // Check all the controls inside the selection rectangle
                            for (int i = 0; i < layout.controlsCount; i++)
                            {
                                if (CheckCollisionRecs(multiSelectRec, layout.controls[i].rec))
                                {
                                    multiSelectControls[multiSelectCount] = i;
                                    multiSelectCount++;
                                }
                            }
                            
                            multiSelectMode = false;
                        }
                    }
                    
                    // Reset multi selection
                    if ((multiSelectCount > 0) && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
                    {
                        for (int i = 0; i < 20; i++) multiSelectControls[i] = -1;
                        multiSelectCount = 0;
                    }
                    
                    // TODO: Multi selection move logic
                    
                    // Multi selection delete logic
                    if ((multiSelectCount > 0) && IsKeyPressed(KEY_DELETE))
                    {
                        for (int j = 0; j < multiSelectCount; j++)
                        {
                            for (int i = multiSelectControls[j]; i < layout.controlsCount; i++)
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
                            
                            // As we moved all controls one position, we need to update selected
                            // controls position for next control deletion
                            for (int i = 0; i < multiSelectCount; i++) multiSelectControls[i] -= 1;
                        }

                        // Reset multi selection
                        for (int i = 0; i < 20; i++) multiSelectControls[i] = -1;
                        multiSelectCount = 0;

                        focusedControl = -1;
                        selectedControl = -1;
                    }
                    //----------------------------------------------------------------------------------------------
                    
                    // Anchors selection and edition logic
                    //----------------------------------------------------------------------------------------------
                    if (!dragMode)
                    {
                        focusedAnchor = -1;
                        
                        // Checks if mouse is over an anchor
                        for (int i = 0; i < MAX_ANCHOR_POINTS; i++)
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
                            if (!anchorLinkMode && anchorEditMode && layout.anchorsCount < MAX_ANCHOR_POINTS && !tracemapFocused && !tracemapSelected)
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
                                            focusedAnchor = i;
                                            selectedAnchor = i;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    // Unselect anchor
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
                            if (refWindowEditMode)
                            {
                                layout.refWindow.width = mouse.x - layout.refWindow.x;
                                layout.refWindow.height = mouse.y  - layout.refWindow.y;

                                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                                {
                                    if (layout.refWindow.width < 0) layout.refWindow.width = -1;
                                    if (layout.refWindow.height < 0) layout.refWindow.height = -1;
                                    refWindowEditMode = false;
                                }
                            }
                            else
                            {
                                if (dragMode)
                                {
                                    if (selectedAnchor == 0) anchorEditMode = false;
                                    // Move anchor without moving controls
                                    if (anchorMoveMode && !anchorEditMode)
                                    {
                                        for (int i = 0; i < layout.controlsCount; i++)
                                        {
                                            if (layout.controls[i].ap->id == 9) //auxAnchor ID
                                            {
                                                if (layout.controls[i].ap->id > 0)
                                                {
                                                    layout.controls[i].rec.x += layout.controls[i].ap->x;
                                                    layout.controls[i].rec.y += layout.controls[i].ap->y;
                                                }
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
                                    layout.anchors[selectedAnchor].x = mouse.x;
                                    layout.anchors[selectedAnchor].y = mouse.y;

                                    if (selectedAnchor == 0)
                                    {
                                        anchorEditMode = false;
                                        layout.refWindow = (Rectangle){layout.anchors[0].x, layout.anchors[0].y, layout.refWindow.width, layout.refWindow.height};
                                    }
                                    // Exit anchor position edit mode
                                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                                    {
                                        // If moving only the anchor, relink with controls
                                        if (anchorMoveMode)
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
                                    if (resizeMode) // Anchor cannot resize
                                    {
                                        if (IsKeyPressed(KEY_D)) // Duplicate anchor
                                        {
                                            if (layout.anchorsCount < MAX_ANCHOR_POINTS)
                                            {
                                                layout.anchorsCount++;
                                                for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
                                                {
                                                    if (!layout.anchors[i].enabled && i != selectedAnchor)
                                                    {
                                                        layout.anchors[i].x = layout.anchors[selectedAnchor].x + 10;
                                                        layout.anchors[i].y = layout.anchors[selectedAnchor].y + 10;
                                                        layout.anchors[i].enabled = true;
                                                        focusedAnchor = i;
                                                        selectedAnchor = i;
                                                        break;
                                                    }
                                                }
                                            }
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
                                        if (selectedAnchor == 0) layout.refWindow = (Rectangle){layout.anchors[0].x, layout.anchors[0].y, layout.refWindow.width, layout.refWindow.height};

                                        // Activate anchor position edit mode
                                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                                        {
                                            if (selectedAnchor == 0 && anchorEditMode) refWindowEditMode = true;
                                            else dragMode = true;
                                        }
                                        // Activate anchor link mode
                                        else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) anchorLinkMode = true;

                                        // Hide/Unhide anchors
                                        else if (IsKeyPressed(KEY_H)) layout.anchors[selectedAnchor].hidding = !layout.anchors[selectedAnchor].hidding;

                                         // Unlinks controls from selected anchor
                                        else if (IsKeyPressed(KEY_U) && selectedAnchor > 0)
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
                                        else if (IsKeyPressed(KEY_DELETE))
                                        {
                                            if (selectedAnchor == 0)
                                            {
                                                layout.anchors[selectedAnchor].x = 0;
                                                layout.anchors[selectedAnchor].y = 0;
                                                layout.refWindow = (Rectangle){ 0, 0, -1, -1 };
                                            }
                                            else
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

                                                layout.anchorsCount--;
                                            }
                                            selectedAnchor = -1;
                                            focusedAnchor = -1;
                                        }

                                        // Enable name edit mode
                                        else if (IsKeyReleased(KEY_N))
                                        {
                                            nameEditMode = true;
                                            strcpy(prevName, layout.anchors[selectedAnchor].name);
                                        }
                                    }
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
                                    if (layout.controls[focusedControl].ap->id > 0)
                                    {
                                        layout.controls[focusedControl].rec.x += layout.controls[focusedControl].ap->x;
                                        layout.controls[focusedControl].rec.y += layout.controls[focusedControl].ap->y;
                                    }

                                    layout.controls[focusedControl].ap = &layout.anchors[selectedAnchor];

                                    if (selectedAnchor> 0)
                                    {
                                        layout.controls[focusedControl].rec.x -= layout.anchors[selectedAnchor].x;
                                        layout.controls[focusedControl].rec.y -= layout.anchors[selectedAnchor].y;
                                    }
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
                    //----------------------------------------------------------------------------------------------

                    // Tracemap edition logic
                    //----------------------------------------------------------------------------------------------
                    if (!tracemapBlocked)
                    {
                        tracemapFocused = false;
                        if (CheckCollisionPointRec(mouse, tracemapRec) && focusedControl == -1 && focusedAnchor == -1) tracemapFocused = true;

                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) tracemapSelected = tracemapFocused;

                        if (tracemapSelected)
                        {
                            if (dragMode)
                            {
                                int offsetX = (int)mouse.x%GRID_LINE_SPACING;
                                int offsetY = (int)mouse.y%GRID_LINE_SPACING;

                                tracemapRec.x = prevPosition.x + (mouse.x - panOffset.x);
                                tracemapRec.y = prevPosition.y + (mouse.y - panOffset.y);

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

                                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragMode = false;
                            }
                            else
                            {
                                if (resizeMode)
                                {
                                    // NOTE: la escala no es proporcional ahora mismo, se tiene que ajustar
                                    if (precisionMode)
                                    {
                                        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_DOWN))
                                        {
                                            tracemapRec.height += movePixel;
                                            tracemapRec.width += movePixel;
                                        }
                                        else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_UP))
                                        {
                                            tracemapRec.height -= movePixel;
                                            tracemapRec.width -= movePixel;
                                        }

                                        framesCounterMovement = 0;
                                    }
                                    else
                                    {
                                        framesCounterMovement++;

                                        if ((framesCounterMovement%movePerFrame) == 0)
                                        {
                                            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_DOWN))
                                            {
                                                tracemapRec.height += movePixel;
                                                tracemapRec.width += movePixel;
                                            }
                                            else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_UP))
                                            {
                                                tracemapRec.height -= movePixel;
                                                tracemapRec.width -= movePixel;
                                            }

                                            framesCounterMovement = 0;
                                        }
                                    }

                                    //tracemap.height = tracemapRec.height;
                                    //tracemap.width = tracemapRec.width;
                                }
                                else
                                {
                                    // Move map with arrows
                                    int offsetX = (int)tracemapRec.x%movePixel;
                                    int offsetY = (int)tracemapRec.y%movePixel;

                                    if (precisionMode)
                                    {
                                        if (IsKeyPressed(KEY_RIGHT))  tracemapRec.x += (movePixel - offsetX);
                                        else if (IsKeyPressed(KEY_LEFT))
                                        {
                                            if (offsetX == 0) offsetX = movePixel;
                                            tracemapRec.x -= offsetX;
                                        }

                                        if (IsKeyPressed(KEY_DOWN)) tracemapRec.y += (movePixel - offsetY);
                                        else if (IsKeyPressed(KEY_UP))
                                        {
                                            if (offsetY == 0) offsetY = movePixel;
                                            tracemapRec.y -= offsetY;
                                        }

                                        framesCounterMovement = 0;
                                    }
                                    else
                                    {
                                        framesCounterMovement++;

                                        if ((framesCounterMovement%movePerFrame) == 0)
                                        {
                                            if (IsKeyDown(KEY_RIGHT)) tracemapRec.x += (movePixel - offsetX);
                                            else if (IsKeyDown(KEY_LEFT))
                                            {
                                                if (offsetX == 0) offsetX = movePixel;
                                                tracemapRec.x -= offsetX;
                                            }

                                            if (IsKeyDown(KEY_DOWN)) tracemapRec.y += (movePixel - offsetY);
                                            else if (IsKeyDown(KEY_UP))
                                            {
                                                if (offsetY == 0) offsetY = movePixel;
                                                tracemapRec.y -= offsetY;
                                            }

                                            framesCounterMovement = 0;
                                        }
                                    }
                                    //------------------------------------------------------------------

                                    // Change alpha NOTE: Mover fuera, que sea un control global.
                                    if (precisionMode)
                                    {
                                        if (IsKeyPressed(KEY_KP_ADD)) tracemapFade += 0.05f;
                                        else if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) tracemapFade -= 0.05f;
                                    }
                                    else
                                    {
                                        if (IsKeyDown(KEY_KP_ADD)) tracemapFade += 0.01f;
                                        else if (IsKeyDown(KEY_KP_SUBTRACT) || IsKeyDown(KEY_MINUS)) tracemapFade -= 0.01f;
                                    }

                                    if (tracemapFade < 0) tracemapFade = 0;
                                    else if (tracemapFade > 1) tracemapFade = 1;

                                    // Delete map
                                    if (IsKeyPressed(KEY_DELETE))
                                    {
                                        UnloadTexture(tracemap);
                                        tracemap.id = 0;
                                        tracemapRec.x = 0;
                                        tracemapRec.y = 0;

                                        //tracemapBlocked = false;
                                        tracemapFocused = false;
                                        tracemapSelected = false;
                                    }

                                    // Enable dragMode  mode
                                    else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                                    {
                                        panOffset = mouse;
                                        prevPosition = (Vector2){ tracemapRec.x, tracemapRec.y };

                                        dragMode = true;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        tracemapFocused = false;
                        tracemapSelected = false;

                        if (CheckCollisionPointRec(mouse, tracemapRec) && focusedControl == -1 && focusedAnchor == -1) tracemapFocused = true;
                    }
                    //----------------------------------------------------------------------------------------------
                }
                else    // textEditMode
                {
                    if (IsKeyPressed(KEY_ESCAPE))
                    {
                        // Cancel text edit mode
                        textEditMode = false;
                        if (selectedControl != -1)
                        {
                            memset(layout.controls[selectedControl].text, 0, MAX_CONTROL_TEXT_LENGTH);
                            strcpy(layout.controls[selectedControl].text, prevText);
                        }
                    }
                }
            }
            else    // nameEditMode
            {
                if (IsKeyPressed(KEY_ESCAPE))
                {
                    // Cancel name edit mode
                    nameEditMode = false;
                    if (selectedControl != -1)
                    {
                        memset(layout.controls[selectedControl].name, 0, MAX_CONTROL_NAME_LENGTH);
                        strcpy(layout.controls[selectedControl].name, prevName);
                    }
                    else if (selectedAnchor != -1)
                    {
                        memset(layout.anchors[selectedAnchor].name, 0, MAX_CONTROL_NAME_LENGTH);
                        strcpy(layout.anchors[selectedAnchor].name, prevName);
                    }
                }
            }
        }
        else    // (closingWindowActive || windowCodegenState.codeGenWindowActive || resetWindowActive)
        {
            nameEditMode = false;
            textEditMode = false;
            resizeMode = false;
            dragMode = false;
            precisionMode = false;
        }
        //----------------------------------------------------------------------------------------------

        // Reset layout logic
        //----------------------------------------------------------------------------------------------
        if (resetLayout)
        {
            focusedAnchor = -1;
            selectedAnchor = -1;
            focusedControl = -1;
            selectedControl = -1;

            resizeMode = false;
            dragMode = false;
            precisionMode = false;
            nameEditMode = false;
            textEditMode = false;

            // Resets all controls to default values
            for (int i = 0; i < layout.controlsCount; i++)
            {
                layout.controls[i].id = 0;
                layout.controls[i].type = 0;
                layout.controls[i].rec = (Rectangle){ 0, 0, 0, 0 };
                memset(layout.controls[i].text, 0, MAX_CONTROL_TEXT_LENGTH);
                memset(layout.controls[i].name, 0, MAX_CONTROL_NAME_LENGTH);
                layout.controls[i].ap = &layout.anchors[0];  // By default, set parent anchor
            }

            layout.controlsCount = 0;

            // Resets anchor points to default values
            for (int i = 0; i < MAX_ANCHOR_POINTS; i++)
            {
                layout.anchors[i].x = 0;
                layout.anchors[i].y = 0;
                layout.anchors[i].enabled = false;
                layout.anchors[i].hidding = false;
            }

            layout.anchors[0].enabled = true;
            layout.anchorsCount = 1;

            SetWindowTitle(FormatText("rGuiLayout v%s", TOOL_VERSION_TEXT));
            strcpy(loadedFileName, "\0");

            resetLayout = false;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            if (showGrid) GuiGrid((Rectangle){ 0, 0, GetScreenWidth(), GetScreenHeight() }, GRID_LINE_SPACING, 5);

            // Draw the tracemap texture if loaded
            if (tracemap.id > 0)
            {
                DrawTexturePro(tracemap, (Rectangle){ 0, 0, tracemap.width, tracemap.height }, tracemapRec, (Vector2){ 0, 0 }, 0.0f, Fade(WHITE, tracemapFade));

                if (tracemapBlocked)
                {
                    if (tracemapFocused) DrawRectangleLinesEx(tracemapRec, 1, MAROON);
                    else DrawRectangleLinesEx(tracemapRec, 1, GRAY);
                }
                else
                {
                    if (tracemapFocused)
                    {
                        DrawRectangleRec(tracemapRec, Fade(RED, 0.1f));
                        DrawRectangleLinesEx(tracemapRec, 1, MAROON);
                    }
                    if (tracemapSelected)
                    {
                        tracemapColor = RED;
                        if (!dragMode && resizeMode) tracemapColor = BLUE;
                        DrawRectangleRec(tracemapRec, Fade(tracemapColor, 0.5f));

                        positionColor = MAROON;
                        if (useGlobalPos) positionColor = RED;
                        if (snapMode) positionColor = LIME;
                        if (!dragMode && precisionMode) positionColor = BLUE;
                        DrawText(FormatText("[%i, %i, %i, %i]", (int)tracemapRec.x, (int)tracemapRec.y, (int)tracemapRec.width, (int)tracemapRec.height), tracemapRec.x, tracemapRec.y - 20, 20, positionColor);
                    }
                }
            }

            // Draw reference window edit mode
            if (layout.refWindow.width > 0 && layout.refWindow.height > 0)
            {
                if (refWindowEditMode)
                {
                    DrawRectangleRec(layout.refWindow, Fade(BLACK, 0.1f));

                    DrawText(FormatText("[%i]", (int)layout.refWindow.width), layout.refWindow.x + layout.refWindow.width - MeasureText(FormatText("[%i]", (int)layout.refWindow.width), 20) - 5, layout.refWindow.y + layout.refWindow.height + 5, 20,positionColor);
                    DrawText(FormatText("[%i]", (int)layout.refWindow.height), layout.refWindow.x + layout.refWindow.width + 5, layout.refWindow.y + layout.refWindow.height - 20, 20,positionColor);
                }
            }

            // Draws the controls placed on the grid
            GuiLock();
            for (int i = 0; i < layout.controlsCount; i++)
            {
                if (!layout.controls[i].ap->hidding)
                {
                    Vector2 anchorOffset = (Vector2){0,0};
                    if (layout.controls[i].ap->id > 0) anchorOffset = (Vector2){layout.controls[i].ap->x, layout.controls[i].ap->y};
                    switch (layout.controls[i].type)
                    {
                        case GUI_WINDOWBOX:
                        {
                            // TODO: review fade
                            GuiFade(0.8f);
                            GuiWindowBox((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text);
                            GuiFade(1.0f);
                        }break;
                        case GUI_GROUPBOX: GuiGroupBox((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text); break;
                        case GUI_LINE: GuiLine((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, 1); break;
                        case GUI_PANEL:
                        {
                            GuiFade(0.8f);
                            GuiPanel((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height });
                            GuiFade(1.0f);
                        }break;
                        case GUI_LABEL: GuiLabel((Rectangle){ anchorOffset.x + layout.controls[i].rec.x /*- refPoint.x*/, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text); break;
                        case GUI_BUTTON: GuiButton((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text); break;
                        case GUI_LABELBUTTON: GuiLabelButton((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text); break;
                        case GUI_IMAGEBUTTONEX: GuiImageButtonEx((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, GetTextureDefault(), (Rectangle){0,0,1,1}, layout.controls[i].text); break;
                        case GUI_CHECKBOX: GuiCheckBox((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text, false); break;
                        case GUI_TOGGLE: GuiToggle((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text, false); break;
                        case GUI_TOGGLEGROUP: GuiToggleGroup((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text, 1); break;
                        case GUI_COMBOBOX: GuiComboBox((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text, 1); break;
                        case GUI_DROPDOWNBOX: GuiDropdownBox((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text, &dropdownBoxActive, false); break;
                        case GUI_TEXTBOX: GuiTextBox((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text, MAX_CONTROL_TEXT_LENGTH, false); break;
                        case GUI_TEXTBOXMULTI: GuiTextBoxMulti((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text, MAX_CONTROL_TEXT_LENGTH, false); break;
                        case GUI_VALUEBOX: GuiValueBox((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, &valueBoxValue, 42, 100, false); break;
                        case GUI_SPINNER: GuiSpinner((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, &spinnerValue, 42, 3, 25, false); break;
                        case GUI_SLIDEREX: GuiSliderEx((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text, 42, 0, 100, true); break;
                        case GUI_SLIDERBAREX: GuiSliderBarEx((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text, 40, 0, 100, true); break;
                        case GUI_PROGRESSBAREX: GuiProgressBarEx((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, 40, 0, 100, true); break;
                        case GUI_STATUSBAR: GuiStatusBar((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text, 15); break;
                        case GUI_SCROLLPANEL: GuiDummyRec((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, "NOT AVAILABLE"); break;
                        case GUI_LISTVIEW: GuiListView((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, listViewData, 4, &listViewScrollIndex, &listViewActive, false); break;
                        case GUI_COLORPICKER: GuiColorPicker((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, RED); break;
                        case GUI_DUMMYREC: GuiDummyRec((Rectangle){ anchorOffset.x + layout.controls[i].rec.x, anchorOffset.y + layout.controls[i].rec.y, layout.controls[i].rec.width, layout.controls[i].rec.height }, layout.controls[i].text); break;
                        default: break;
                    }
                }
            }
            GuiUnlock();

            // Draw reference window
            anchorSelectedColor = DARKGRAY;
            anchorCircleColor = DARKGRAY;
            if (selectedAnchor == 0)
            {
                if (anchorEditMode) { anchorSelectedColor = ORANGE; anchorCircleColor = ORANGE;}
                DrawRectangle(layout.anchors[0].x - ANCHOR_RADIUS, layout.anchors[0].y - ANCHOR_RADIUS, ANCHOR_RADIUS*2, ANCHOR_RADIUS*2, Fade(anchorSelectedColor, 0.2f));
            }
            if (focusedAnchor == 0)
            {
                anchorCircleColor = BLACK;
                if (anchorEditMode) anchorCircleColor = ORANGE;
            }

            DrawRectangleLines(layout.anchors[0].x - ANCHOR_RADIUS, layout.anchors[0].y - ANCHOR_RADIUS, ANCHOR_RADIUS*2, ANCHOR_RADIUS*2, Fade(anchorCircleColor, 0.5f));
            DrawRectangle(layout.anchors[0].x - ANCHOR_RADIUS - 5, layout.anchors[0].y, ANCHOR_RADIUS*2 + 10, 1, Fade(anchorCircleColor, 0.8f));
            DrawRectangle(layout.anchors[0].x, layout.anchors[0].y - ANCHOR_RADIUS - 5, 1, ANCHOR_RADIUS*2 + 10, Fade(anchorCircleColor, 0.8f));

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

            if (!closingWindowActive && !windowCodegenState.codeGenWindowActive && !resetWindowActive)
            {
                if (!(CheckCollisionPointRec(mouse, palettePanel)))
                {
                    if (focusedAnchor == -1 && focusedControl == -1 && !tracemapFocused && !refWindowEditMode)
                    {
                        if (!anchorEditMode)
                        {
                            if (!anchorLinkMode && selectedAnchor == -1 && selectedControl == -1 && !tracemapSelected)
                            {
                                // Draw the default rectangle of the control selected
                                GuiLock();
                                GuiFade(0.5f);

                                switch (selectedTypeDraw)
                                {
                                    case GUI_WINDOWBOX: GuiWindowBox(defaultRec[selectedTypeDraw], "WINDOW BOX"); break;
                                    case GUI_GROUPBOX: GuiGroupBox(defaultRec[selectedTypeDraw], "GROUP BOX"); break;
                                    case GUI_LINE: GuiLine(defaultRec[selectedTypeDraw], 1); break;
                                    case GUI_PANEL: GuiPanel(defaultRec[selectedTypeDraw]); break;
                                    case GUI_LABEL: GuiLabel(defaultRec[selectedTypeDraw], "LABEL TEXT"); break;
                                    case GUI_BUTTON: GuiButton(defaultRec[selectedTypeDraw], "BUTTON"); break;
                                    case GUI_LABELBUTTON: GuiLabelButton(defaultRec[selectedTypeDraw], "LABEL_BUTTON"); break;
                                    case GUI_IMAGEBUTTONEX: GuiImageButtonEx(defaultRec[selectedTypeDraw], GetTextureDefault(), (Rectangle){0,0,1,1}, "IMAGE BUTTON"); break;
                                    case GUI_CHECKBOX: GuiCheckBox(defaultRec[selectedTypeDraw], "CHECK BOX", false); break;
                                    case GUI_TOGGLE: GuiToggle(defaultRec[selectedTypeDraw], "TOGGLE", false); break;
                                    case GUI_TOGGLEGROUP: GuiToggleGroup(defaultRec[selectedTypeDraw], "ONE;TWO;THREE", 1); break;
                                    case GUI_COMBOBOX: GuiComboBox(defaultRec[selectedTypeDraw], "ONE;TWO;THREE", 1); break;
                                    case GUI_DROPDOWNBOX: GuiDropdownBox(defaultRec[selectedTypeDraw], "ONE;TWO;THREE", &dropdownBoxActive, false); break;
                                    case GUI_TEXTBOX: GuiTextBox(defaultRec[selectedTypeDraw], "TEXT BOX", 7, false); break;
                                    case GUI_TEXTBOXMULTI: GuiTextBoxMulti(defaultRec[selectedTypeDraw], "MULTI TEX BOX", 7, false);break;
                                    case GUI_VALUEBOX: GuiValueBox(defaultRec[selectedTypeDraw], &valueBoxValue, 42, 100, false); break;
                                    case GUI_SPINNER: GuiSpinner(defaultRec[selectedTypeDraw], &spinnerValue, 42, 3, 25, false); break;
                                    case GUI_SLIDEREX: GuiSliderEx(defaultRec[selectedTypeDraw], "SLIDER", 42, 0, 100, true); break;
                                    case GUI_SLIDERBAREX: GuiSliderBarEx(defaultRec[selectedTypeDraw], "SLIDER BAR", 40, 0, 100, true); break;
                                    case GUI_PROGRESSBAREX: GuiProgressBarEx(defaultRec[selectedTypeDraw], 40, 0, 100, true); break;
                                    case GUI_STATUSBAR: GuiStatusBar(defaultRec[selectedTypeDraw], "STATUS BAR", 15); break;
                                    case GUI_SCROLLPANEL: GuiDummyRec(defaultRec[selectedTypeDraw], "NOT AVAILABLE"); break;
                                    case GUI_LISTVIEW: GuiListView(defaultRec[selectedTypeDraw], listViewData, 4, &listViewScrollIndex, &listViewActive, false); break;
                                    case GUI_COLORPICKER: GuiColorPicker(defaultRec[selectedTypeDraw], RED); break;
                                    case GUI_DUMMYREC: GuiDummyRec(defaultRec[selectedTypeDraw], "DUMMY REC"); break;
                                    default: break;
                                }
                                GuiFade(1.0f);
                                GuiUnlock();

                                // Draw default cursor
                                DrawRectangle(mouse.x - 8, mouse.y, 17, 1, RED);
                                DrawRectangle(mouse.x, mouse.y - 8, 1, 17, RED);

                                // Draw cursor position
                                positionColor = MAROON;
                                if (snapMode) positionColor = LIME;
                                DrawText(FormatText("[%i, %i, %i, %i]", (int)defaultRec[selectedType].x, (int)defaultRec[selectedType].y, (int)defaultRec[selectedType].width, (int)defaultRec[selectedType].height), (int)defaultRec[selectedType].x,(int)defaultRec[selectedType].y - 30, 20, Fade(positionColor, 0.5f));

                                if (showNamesMode)
                                {
                                    GuiLock();
                                    for (int i = 0; i < layout.controlsCount; i++)
                                    {
                                        Rectangle textboxRec = layout.controls[i].rec;
                                        int type = layout.controls[i].type;
                                        if (type == GUI_CHECKBOX || type == GUI_LABEL || type == GUI_SLIDEREX || type == GUI_SLIDERBAREX)
                                        {
                                            int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
                                            int textWidth = MeasureText(layout.controls[i].name, fontSize);
                                            if (textboxRec.width < textWidth + 20) textboxRec.width = textWidth + 20;
                                            if (textboxRec.height < fontSize) textboxRec.height += fontSize;
                                        }

                                        if (type == GUI_WINDOWBOX) textboxRec.height = WINDOW_SATUSBAR_HEIGHT;  // Defined inside raygui.h
                                        else if (type == GUI_GROUPBOX)
                                        {
                                            textboxRec.y -= 10;
                                            textboxRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE) * 2;
                                        }
                                        if (layout.controls[i].ap->id > 0)
                                        {
                                            textboxRec.x += layout.controls[i].ap->x;
                                            textboxRec.y += layout.controls[i].ap->y;
                                        }

                                        DrawRectangleRec(textboxRec, WHITE);
                                        //DrawRectangleRec(textboxRec, Fade(SKYBLUE, 0.5f));
                                        GuiTextBox(textboxRec, layout.controls[i].name, MAX_CONTROL_NAME_LENGTH, false);
                                        //DrawRectangleLinesEx(textboxRec,1, BLUE);
                                    }

                                    for (int i = 0; i < layout.anchorsCount; i++)
                                    {
                                        //layout.anchor[selectedAnchor].name
                                        Rectangle textboxRec = (Rectangle) {layout.anchors[i].x, layout.anchors[i].y, MeasureText(layout.anchors[i].name, GuiGetStyle(DEFAULT, TEXT_SIZE)) + 10, GuiGetStyle(DEFAULT, TEXT_SIZE) + 5};

                                        DrawRectangleRec(textboxRec, WHITE);
                                        DrawRectangleRec(textboxRec, Fade(ORANGE, 0.1f));
                                        GuiTextBox(textboxRec, layout.anchors[i].name, MAX_ANCHOR_NAME_LENGTH, false);
                                    }
                                    GuiUnlock();
                                }
                            }
                        }
                        else
                        {
                            // Draw anchor cursor
                            DrawCircleLines(mouse.x, mouse.y, ANCHOR_RADIUS, Fade(RED, 0.5f));
                            DrawRectangle(mouse.x - ANCHOR_RADIUS - 5, mouse.y, ANCHOR_RADIUS*2 + 10, 1, RED);
                            DrawRectangle(mouse.x , mouse.y - ANCHOR_RADIUS - 5, 1, ANCHOR_RADIUS*2 + 10, RED);
                        }
                    }
                }
                
                // Focused anchor draws
                if (focusedAnchor != 1)
                {
                    // Anchor links
                    for (int i = 0; i < layout.controlsCount; i++)
                    {
                        //if (!layout.controls[i].ap->hidding)
                        if (layout.controls[i].ap->id == focusedAnchor)
                        {
                            if (focusedAnchor == 0) DrawLine(layout.controls[i].ap->x, layout.controls[i].ap->y, layout.controls[i].rec.x, layout.controls[i].rec.y, LIGHTGRAY);
                            else if (!layout.controls[i].ap->hidding) DrawLine(layout.controls[i].ap->x, layout.controls[i].ap->y, layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, RED);
                            else DrawLine(layout.controls[i].ap->x, layout.controls[i].ap->y, layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, GRAY);
                        }
                    }
                }
                
                // Selected anchor draws
                if (selectedAnchor != -1)
                {
                    // Anchor coordinates
                    positionColor = anchorSelectedColor;
                    if (snapMode) positionColor = LIME;
                    if (!dragMode && precisionMode) positionColor = BLUE;

                    if (selectedAnchor > 0) DrawText(FormatText("[%i, %i]", (int)(layout.anchors[selectedAnchor].x - layout.refWindow.x), (int)(layout.anchors[selectedAnchor].y - layout.refWindow.y)), layout.anchors[selectedAnchor].x + ANCHOR_RADIUS, layout.anchors[selectedAnchor].y - 38, 20, positionColor);
                    else
                    {
                        if (layout.refWindow.width > 0 && layout.refWindow.height > 0) DrawText(FormatText("[%i, %i, %i, %i]", (int)(layout.refWindow.x), (int)(layout.refWindow.y), (int)(layout.refWindow.width), (int)(layout.refWindow.height)), layout.anchors[selectedAnchor].x + ANCHOR_RADIUS, layout.anchors[selectedAnchor].y - 38, 20, positionColor);
                        else DrawText(FormatText("[%i, %i]", (int)(layout.refWindow.x), (int)(layout.refWindow.y)), layout.anchors[selectedAnchor].x + ANCHOR_RADIUS, layout.anchors[selectedAnchor].y - 38, 20, positionColor);
                    }
                    // Anchor links
                    for (int i = 0; i < layout.controlsCount; i++)
                    {
                        //if (!layout.controls[i].ap->hidding)
                        if (layout.controls[i].ap->id == selectedAnchor)
                        {
                            if (selectedAnchor == 0) DrawLine(layout.controls[i].ap->x, layout.controls[i].ap->y, layout.controls[i].rec.x, layout.controls[i].rec.y, LIGHTGRAY);
                            else if (!layout.controls[i].ap->hidding) DrawLine(layout.controls[i].ap->x, layout.controls[i].ap->y, layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, RED);
                            else DrawLine(layout.controls[i].ap->x, layout.controls[i].ap->y, layout.controls[i].ap->x + layout.controls[i].rec.x, layout.controls[i].ap->y + layout.controls[i].rec.y, GRAY);
                        }
                    }

                    // Draw anchor link mode
                    if (anchorLinkMode)
                    {
                        if (selectedAnchor == 0) DrawLine(layout.anchors[selectedAnchor].x, layout.anchors[selectedAnchor].y, mouse.x, mouse.y, BLACK);
                        else DrawLine(layout.anchors[selectedAnchor].x, layout.anchors[selectedAnchor].y, mouse.x, mouse.y, RED);
                    }
                    
                    // Draw name edit mode
                    if (nameEditMode)
                    {
                        int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
                        int textWidth = MeasureText(layout.anchors[selectedAnchor].name, fontSize);
                        Rectangle textboxRec = (Rectangle) {layout.anchors[selectedAnchor].x, layout.anchors[selectedAnchor].y, textWidth + 15, fontSize + 5};

                        if (textboxRec.width < textWidth + 15) textboxRec.width = textWidth + 15;
                        if (textboxRec.height < fontSize) textboxRec.height += fontSize;

                        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(ORANGE, 0.2f));
                        DrawText("Anchor name edit mode", 20, 20, 20, DARKGRAY);
                        if (GuiTextBox(textboxRec, layout.anchors[selectedAnchor].name, MAX_ANCHOR_NAME_LENGTH, nameEditMode)) nameEditMode = !nameEditMode;
                    }
                }
                
                // Draw multi selected controls
                if (multiSelectMode) DrawRectangleLinesEx(multiSelectRec, 1, BLACK);
                
                if (multiSelectCount > 0)
                {
                    for (int i = 0; i < multiSelectCount; i++)
                    {
                        DrawRectangleRec(layout.controls[multiSelectControls[i]].rec, Fade(GOLD, 0.4f));
                    }
                }

                // Draw selected control
                if (selectedControl != -1)
                {
                    // Selection rectangle
                    selectedControlColor = RED;
                    if (!dragMode && resizeMode) selectedControlColor = BLUE;

                    Rectangle selectedRec = layout.controls[selectedControl].rec;
                    if (layout.controls[selectedControl].type == GUI_WINDOWBOX) selectedRec.height = 24;
                    else if (layout.controls[selectedControl].type == GUI_GROUPBOX)
                    {
                        selectedRec.y -= 10;
                        selectedRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE) * 2;
                    }

                    if (layout.controls[selectedControl].ap->id > 0)
                    {
                        selectedRec.x += layout.controls[selectedControl].ap->x;
                        selectedRec.y += layout.controls[selectedControl].ap->y;
                    }

                    DrawRectangleRec(selectedRec, Fade(selectedControlColor, 0.3f));
                    
                    // Control Link
                    if (layout.controls[selectedControl].ap->id > 0) DrawLine(layout.controls[selectedControl].ap->x, layout.controls[selectedControl].ap->y, selectedRec.x, selectedRec.y, RED);

                    // Linking
                    if (anchorLinkMode) DrawLine(selectedRec.x, selectedRec.y, mouse.x, mouse.y, RED);

                    // Control Coordinates
                    positionColor = MAROON;
                    if (useGlobalPos) positionColor = RED;
                    if (snapMode) positionColor = LIME;
                    if (!dragMode && precisionMode) positionColor = BLUE;

                    if (!useGlobalPos)
                    {
                        if (layout.controls[selectedControl].ap->id > 0) DrawText(FormatText("[%i, %i, %i, %i]", (int)(layout.controls[selectedControl].rec.x) , (int)(layout.controls[selectedControl].rec.y), (int)layout.controls[selectedControl].rec.width, (int)layout.controls[selectedControl].rec.height), selectedRec.x, selectedRec.y - 30, 20, positionColor);
                        else DrawText(FormatText("[%i, %i, %i, %i]", (int)(selectedRec.x - layout.refWindow.x) , (int)(selectedRec.y - layout.refWindow.y), (int)layout.controls[selectedControl].rec.width, (int)layout.controls[selectedControl].rec.height), selectedRec.x, selectedRec.y - 30, 20, positionColor);
                    }
                    else
                    {
                        DrawText(FormatText("[%i, %i, %i, %i]", (int)(selectedRec.x - layout.refWindow.x), (int)(selectedRec.y - layout.refWindow.y), (int)layout.controls[selectedControl].rec.width, (int)layout.controls[selectedControl].rec.height), selectedRec.x, selectedRec.y - 30, 20, positionColor);
                    }

                    // Text edit
                    if (textEditMode)
                    {
                        Rectangle textboxRec = layout.controls[selectedControl].rec;
                        int type = layout.controls[selectedControl].type;
                        if (type == GUI_CHECKBOX || 
                            type == GUI_LABEL || 
                            type == GUI_TOGGLEGROUP || 
                            type == GUI_COMBOBOX || 
                            type == GUI_DROPDOWNBOX || 
                            type == GUI_SLIDEREX ||
                            type == GUI_SLIDERBAREX)
                        {
                            int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
                            int textWidth = MeasureText(layout.controls[selectedControl].text, fontSize);
                            if (textboxRec.width < textWidth + 20) textboxRec.width = textWidth + 20;
                            if (textboxRec.height < fontSize) textboxRec.height += fontSize;
                        }

                        if (type == GUI_WINDOWBOX) textboxRec.height = WINDOW_SATUSBAR_HEIGHT;  // Defined inside raygui.h
                        else if (type == GUI_GROUPBOX)
                        {
                            textboxRec.y -= 10;
                            textboxRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE) * 2;
                        }

                        if (layout.controls[selectedControl].ap->id > 0)
                        {
                            textboxRec.x += layout.controls[selectedControl].ap->x;
                            textboxRec.y += layout.controls[selectedControl].ap->y;
                        }

                        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(SKYBLUE, 0.2f));
                        DrawText("Control text edit mode", 20, 25, 20, DARKGRAY);
                        
                        if (layout.controls[selectedControl].type == GUI_TEXTBOXMULTI)
                        {
                            if (GuiTextBoxMulti(textboxRec, layout.controls[selectedControl].text, MAX_CONTROL_TEXTMULTI_LENGTH, textEditMode)) textEditMode = !textEditMode;
                        }
                        else if (GuiTextBox(textboxRec, layout.controls[selectedControl].text, MAX_CONTROL_TEXT_LENGTH, textEditMode)) textEditMode = !textEditMode;
                    }

                    // Name edit
                    if (nameEditMode)
                    {
                        Rectangle textboxRec = layout.controls[selectedControl].rec;
                        int type = layout.controls[selectedControl].type;
                        if (type == GUI_CHECKBOX || type == GUI_LABEL || type == GUI_SLIDEREX || type == GUI_SLIDERBAREX)
                        {
                            int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
                            int textWidth = MeasureText(layout.controls[selectedControl].name, fontSize);
                            if (textboxRec.width < textWidth + 20) textboxRec.width = textWidth + 20;
                            if (textboxRec.height < fontSize) textboxRec.height += fontSize;
                        }

                        if (type == GUI_WINDOWBOX) textboxRec.height = WINDOW_SATUSBAR_HEIGHT;  // Defined inside raygui.h
                        else if (type == GUI_GROUPBOX)
                        {
                            textboxRec.y -= 10;
                            textboxRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE) * 2;
                        }

                        if (layout.controls[selectedControl].ap->id > 0)
                        {
                            textboxRec.x += layout.controls[selectedControl].ap->x;
                            textboxRec.y += layout.controls[selectedControl].ap->y;
                        }

                        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(GREEN, 0.2f));
                        DrawText("Control name edit mode", 20, 20, 20, DARKGRAY);
                        if (GuiTextBox(textboxRec, layout.controls[selectedControl].name, MAX_CONTROL_NAME_LENGTH, nameEditMode)) nameEditMode = !nameEditMode;
                    }
                }
                //else //selectedControl == 1
                {
                    // Draw controls IDs for layout order edition
                    if (orderEditMode)
                    {
                        for (int i = layout.controlsCount - 1; i >= 0; i--)
                        {
                            if (layout.controls[i].ap->id > 0) DrawText(FormatText("[%i]", layout.controls[i].id), layout.controls[i].rec.x + layout.controls[i].ap->x + layout.controls[i].rec.width, layout.controls[i].rec.y + layout.controls[i].ap->y - 10, 10, BLUE);
                            else DrawText(FormatText("[%i]", layout.controls[i].id), layout.controls[i].rec.x + layout.controls[i].rec.width, layout.controls[i].rec.y - 10, 10, BLUE);
                        }
                    }
                }

                // Draw focused control
                if (focusedControl != -1)
                {
                    // Focused rectangle
                    Rectangle focusRec = layout.controls[focusedControl].rec;
                    
                    if (layout.controls[focusedControl].type == GUI_WINDOWBOX) focusRec.height = WINDOW_SATUSBAR_HEIGHT;
                    else if (layout.controls[focusedControl].type == GUI_GROUPBOX)
                    {
                        focusRec.y -= 10;
                        focusRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE) * 2;
                    }

                    if (layout.controls[focusedControl].ap->id > 0)
                    {
                        focusRec.x += layout.controls[focusedControl].ap->x;
                        focusRec.y += layout.controls[focusedControl].ap->y;
                    }

                    if (focusedControl != selectedControl) DrawRectangleRec(focusRec, Fade(RED, 0.1f));
                    
                    DrawRectangleLinesEx(focusRec, 1, MAROON);

                    if (layout.controls[focusedControl].ap->id > 0) DrawLine(layout.controls[focusedControl].ap->x, layout.controls[focusedControl].ap->y, focusRec.x, focusRec.y, RED);
                }
                
                // Draw mouse scale mode selector
                if (mouseScaleReady) 
                {
                    Rectangle selectedRec = layout.controls[selectedControl].rec;
                    DrawRectangleLinesEx(selectedRec, 2, RED);
                    DrawTriangle((Vector2){ selectedRec.x + selectedRec.width - MOUSE_SCALE_MARK_SIZE, selectedRec.y + selectedRec.height }, 
                                 (Vector2){ selectedRec.x + selectedRec.width, selectedRec.y + selectedRec.height },
                                 (Vector2){ selectedRec.x + selectedRec.width, selectedRec.y + selectedRec.height - MOUSE_SCALE_MARK_SIZE }, RED);
                }

                // Draw reference window lines
                if ((layout.refWindow.width > 0) && (layout.refWindow.height > 0)) DrawRectangleLinesEx(layout.refWindow, 1, Fade(BLACK, 0.7f));

                // Draw the help panel (by default is out of screen)
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
                GuiLock();
                GuiPanel(palettePanel);
                GuiWindowBox((Rectangle){palettePanel.x + paletteRecs[0].x ,palettePanel.y + paletteRecs[0].y, paletteRecs[0].width, paletteRecs[0].height}, "WindowBox");
                GuiGroupBox((Rectangle){palettePanel.x + paletteRecs[1].x ,palettePanel.y + paletteRecs[1].y, paletteRecs[1].width, paletteRecs[1].height}, "GroupBox");
                GuiLine((Rectangle){palettePanel.x + paletteRecs[2].x ,palettePanel.y + paletteRecs[2].y, paletteRecs[2].width, paletteRecs[2].height}, 1);
                GuiPanel((Rectangle){palettePanel.x + paletteRecs[3].x ,palettePanel.y + paletteRecs[3].y, paletteRecs[3].width, paletteRecs[3].height});
                GuiLabel((Rectangle){palettePanel.x + paletteRecs[4].x ,palettePanel.y + paletteRecs[4].y, paletteRecs[4].width, paletteRecs[4].height}, "Label");
                GuiButton((Rectangle){palettePanel.x + paletteRecs[5].x ,palettePanel.y + paletteRecs[5].y, paletteRecs[5].width, paletteRecs[5].height}, "Button");
                GuiToggle((Rectangle){palettePanel.x + paletteRecs[6].x ,palettePanel.y + paletteRecs[6].y, paletteRecs[6].width, paletteRecs[6].height}, "Toggle", false);
                GuiCheckBox((Rectangle){palettePanel.x + paletteRecs[8].x ,palettePanel.y + paletteRecs[8].y, paletteRecs[8].width, paletteRecs[8].height}, "CheckBox",  false);
                GuiToggleGroup((Rectangle){palettePanel.x + paletteRecs[7].x ,palettePanel.y + paletteRecs[7].y, paletteRecs[7].width, paletteRecs[7].height}, "ONE;TWO;THREE", 8);
                GuiComboBox((Rectangle){palettePanel.x + paletteRecs[9].x ,palettePanel.y + paletteRecs[9].y, paletteRecs[9].width, paletteRecs[9].height},  "ONE;TWO;THREE", 9);
                GuiDropdownBox((Rectangle){palettePanel.x + paletteRecs[10].x ,palettePanel.y + paletteRecs[10].y, paletteRecs[10].width, paletteRecs[10].height}, "ONE;TWO;THREE", &dropdownBoxActive, false);
                GuiSpinner((Rectangle){palettePanel.x + paletteRecs[11].x ,palettePanel.y + paletteRecs[11].y, paletteRecs[11].width, paletteRecs[11].height}, &spinnerValue, 0, 100, 25, false);
                GuiValueBox((Rectangle){palettePanel.x + paletteRecs[12].x ,palettePanel.y + paletteRecs[12].y, paletteRecs[12].width, paletteRecs[12].height}, &valueBoxValue, 0, 100, false);
                GuiTextBox((Rectangle){palettePanel.x + paletteRecs[13].x ,palettePanel.y + paletteRecs[13].y, paletteRecs[13].width, paletteRecs[13].height}, "TextBox", 7, false);
                GuiSlider((Rectangle){palettePanel.x + paletteRecs[14].x ,palettePanel.y + paletteRecs[14].y, paletteRecs[14].width, paletteRecs[14].height}, 40, 0, 100);
                GuiSliderBar((Rectangle){palettePanel.x + paletteRecs[15].x ,palettePanel.y + paletteRecs[15].y, paletteRecs[15].width, paletteRecs[15].height}, 50, 0, 100);
                GuiProgressBar((Rectangle){palettePanel.x + paletteRecs[16].x ,palettePanel.y + paletteRecs[16].y, paletteRecs[16].width, paletteRecs[16].height}, 60, 0, 100);
                GuiStatusBar((Rectangle){palettePanel.x + paletteRecs[17].x ,palettePanel.y + paletteRecs[17].y, paletteRecs[17].width, paletteRecs[17].height}, "StatusBar", 10);
                GuiListView((Rectangle){palettePanel.x + paletteRecs[18].x ,palettePanel.y + paletteRecs[18].y, paletteRecs[18].width, paletteRecs[18].height}, listViewData, 3, &listViewScrollIndex, &listViewActive, false);
                GuiColorPicker((Rectangle){palettePanel.x + paletteRecs[19].x ,palettePanel.y + paletteRecs[19].y, paletteRecs[19].width, paletteRecs[19].height}, RED);
                GuiDummyRec((Rectangle){palettePanel.x + paletteRecs[20].x ,palettePanel.y + paletteRecs[20].y, paletteRecs[20].width, paletteRecs[20].height}, "DummyRec");
                GuiUnlock();

                DrawRectangleRec((Rectangle){palettePanel.x + paletteRecs[selectedType].x, palettePanel.y + paletteRecs[selectedType].y, paletteRecs[selectedType].width, paletteRecs[selectedType].height}, Fade(RED, 0.5f));

                if (paletteSelect > -1)
                {
                    if (selectedType != paletteSelect) DrawRectangleRec((Rectangle){palettePanel.x + paletteRecs[paletteSelect].x, palettePanel.y + paletteRecs[paletteSelect].y, paletteRecs[paletteSelect].width, paletteRecs[paletteSelect].height}, Fade(RED, 0.1f));
                    DrawRectangleLinesEx((Rectangle){palettePanel.x + paletteRecs[paletteSelect].x, palettePanel.y + paletteRecs[paletteSelect].y, paletteRecs[paletteSelect].width, paletteRecs[paletteSelect].height}, 1, MAROON);
                }
            }
            else    // (closingWindowActive || windowCodegenState.codeGenWindowActive || resetWindowActive)
            {
                // Draw reset message window (save)
                if (resetWindowActive)
                {
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(WHITE, 0.7f));
                    resetWindowActive = !GuiWindowBox((Rectangle){ GetScreenWidth()/2 - 125, GetScreenHeight()/2 - 50, 250, 100 }, "Creating new layout");

                    GuiLabel((Rectangle){ GetScreenWidth()/2 - 95, GetScreenHeight()/2 - 60, 200, 100 }, "Do you want to save the current layout?");

                    if (GuiButton((Rectangle){ GetScreenWidth()/2 - 94, GetScreenHeight()/2 + 10, 85, 25 }, "Yes"))
                    {
                        if (DialogSaveLayout())
                        {
                            resetLayout = true;
                            resetWindowActive = false;
                        }
                    }
                    else if (GuiButton((Rectangle){ GetScreenWidth()/2 + 10, GetScreenHeight()/2 + 10, 85, 25 }, "No"))
                    {
                        resetLayout = true;
                        resetWindowActive = false;
                    }
                }

                // Draw export options window
                if (windowCodegenState.codeGenWindowActive)
                {
                    GuiWindowCodegen(&windowCodegenState);

                    if (windowCodegenState.exportCodeButtonPressed)
                    {
                        DialogExportLayout(windowCodegenState.generatedCode, FormatText("%s.h", config.name));
                        windowCodegenState.codeGenWindowActive = false;
                    }
                }

                // Draw ending message window (save)
                if (closingWindowActive)
                {
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(WHITE, 0.7f));
                    closingWindowActive = !GuiWindowBox((Rectangle){ GetScreenWidth()/2 - 125, GetScreenHeight()/2 - 50, 250, 100 }, "Closing rGuiLayout");

                    GuiLabel((Rectangle){ GetScreenWidth()/2 - 95, GetScreenHeight()/2 - 60, 200, 100 }, "Do you want to save before quitting?");

                    if (GuiButton((Rectangle){ GetScreenWidth()/2 - 94, GetScreenHeight()/2 + 10, 85, 25 }, "Yes"))
                    {
                        if (DialogSaveLayout())
                        {
                            closingWindowActive = false;
                            exitWindow = true;
                        }
                    }
                    else if (GuiButton((Rectangle){ GetScreenWidth()/2 + 10, GetScreenHeight()/2 + 10, 85, 25 }, "No")) { exitWindow = true; }
                }
            }

            // Draw status bar bottom with debug information
            GuiStatusBar((Rectangle){ 0, GetScreenHeight() - 24, 126, 24}, FormatText("MOUSE: (%i, %i)", (int)mouse.x, (int)mouse.y), 15);
            GuiStatusBar((Rectangle){ 124, GetScreenHeight() - 24, 81, 24}, (snapMode ? "SNAP: ON" : "SNAP: OFF"), 10);
            GuiStatusBar((Rectangle){ 204, GetScreenHeight() - 24, 145, 24}, FormatText("CONTROLS COUNT: %i", layout.controlsCount), 20);
            if (selectedControl != -1) GuiStatusBar((Rectangle){ 348, GetScreenHeight() - 24, GetScreenWidth() - 348, 24}, FormatText("SELECTED CONTROL: #%03i  |  %s  |  REC (%i, %i, %i, %i)  |  %s", selectedControl, TextToUpper(controlTypeName[layout.controls[selectedControl].type]), (int)layout.controls[selectedControl].rec.x, (int)layout.controls[selectedControl].rec.y, (int)layout.controls[selectedControl].rec.width, (int)layout.controls[selectedControl].rec.height, layout.controls[selectedControl].name), 15);
            else GuiStatusBar((Rectangle){ 348, GetScreenHeight() - 24, GetScreenWidth() - 348, 24}, "", 15);

            /*
            // Draw UNDO system info
            //--------------------------------------------------------------------------------------------
            DrawText("UNDO SYSTEM", 215, 34, 10, DARKGRAY);

            // Draw index marks
            DrawRectangle(308 + 25*currentUndoIndex, 15, 8, 8, RED);
            DrawRectangleLines(302 + 25*firstUndoIndex, 50 + 2, 8, 8, BLACK);
            DrawRectangle(314 + 25*lastUndoIndex, 50 + 2, 8, 8, BLACK);

            // Draw background gray slots
            for (int i = 0; i < MAX_UNDO_LEVELS; i++)
            {
                DrawRectangle(300 + 25*i, 25, 25, 25, LIGHTGRAY);
                DrawRectangleLines(300 + 25*i, 25, 25, 25, GRAY);
            }

            // Draw occupied slots: firstUndoIndex --> lastUndoIndex
            if (firstUndoIndex <= lastUndoIndex)
            {
                for (int i = firstUndoIndex; i < lastUndoIndex + 1; i++)
                {
                    DrawRectangle(300 + 25*i, 25, 25, 25, SKYBLUE);
                    DrawRectangleLines(300 + 25*i, 25, 25, 25, BLUE);
                }
            }
            else if (lastUndoIndex < firstUndoIndex)
            {
                for (int i = firstUndoIndex; i < MAX_UNDO_LEVELS; i++)
                {
                    DrawRectangle(300 + 25*i, 25, 25, 25, SKYBLUE);
                    DrawRectangleLines(300 + 25*i, 25, 25, 25, BLUE);
                }

                for (int i = 0; i < lastUndoIndex + 1; i++)
                {
                    DrawRectangle(300 + 25*i, 25, 25, 25, SKYBLUE);
                    DrawRectangleLines(300 + 25*i, 25, 25, 25, BLUE);
                }
            }

            // Draw occupied slots: firstUndoIndex --> currentUndoIndex
            if (firstUndoIndex < currentUndoIndex)
            {
                for (int i = firstUndoIndex; i < currentUndoIndex; i++)
                {
                    DrawRectangle(300 + 25*i, 25, 25, 25, GREEN);
                    DrawRectangleLines(300 + 25*i, 25, 25, 25, LIME);
                }
            }
            else if (currentUndoIndex < firstUndoIndex)
            {
                for (int i = firstUndoIndex; i < MAX_UNDO_LEVELS; i++)
                {
                    DrawRectangle(300 + 25*i, 25, 25, 25, GREEN);
                    DrawRectangleLines(300 + 25*i, 25, 25, 25, LIME);
                }

                for (int i = 0; i < currentUndoIndex; i++)
                {
                    DrawRectangle(300 + 25*i, 25, 25, 25, GREEN);
                    DrawRectangleLines(300 + 25*i, 25, 25, 25, LIME);
                }
            }

            // Draw current selected UNDO slot
            DrawRectangle(300 + 25*currentUndoIndex, 25, 25, 25, GOLD);
            DrawRectangleLines(300 + 25*currentUndoIndex, 25, 25, 25, ORANGE);
            //--------------------------------------------------------------------------------------------
            */

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(tracemap);

    if (windowCodegenState.generatedCode != NULL) free(windowCodegenState.generatedCode);
    free(undoLayouts);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definitions (local)
//----------------------------------------------------------------------------------

#if defined(VERSION_ONE)
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
        strcpy(config.name, "window_codegen");
        strcpy(config.version, TOOL_VERSION_TEXT);
        strcpy(config.company, "raylib technologies");
        strcpy(config.description, "tool description");
        config.exportAnchors = true;
        config.defineRecs = false;
        config.fullComments = true;

        // Generate C code for gui layout.controls
        unsigned char *toolstr = GenerateLayoutCode(LoadText("gui_code_template.c"), layout, config);
        FILE *ftool = fopen(outFileName, "wt");
        fprintf(ftool, toolstr);    // Write code string to file
        fclose(ftool);

        free(toolstr);
    }

    if (showUsageInfo) ShowCommandLineInfo();
}
#endif      // VERSION_ONE

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

    // Reset all the controls
    layout.controlsCount = 0;
    layout.anchorsCount = 0;

    for (int i = 0; i < MAX_GUI_CONTROLS; i++)
    {
        layout.controls[i].id = 0;
        layout.controls[i].type = 0;
        layout.controls[i].rec = (Rectangle){ 0, 0, 0, 0 };
        memset(layout.controls[i].text, 0, MAX_CONTROL_TEXT_LENGTH);
        memset(layout.controls[i].name, 0, MAX_CONTROL_NAME_LENGTH);
        layout.controls[i].ap = &layout.anchors[0];
    }

    // Reset all the anchors
    for (int i = 0; i < MAX_ANCHOR_POINTS; i++)
    {
        layout.anchors[i].id = i;
        layout.anchors[i].x = 0;
        layout.anchors[i].y = 0;
        layout.anchors[i].enabled = false;
        layout.anchors[i].hidding = false;
        memset(layout.anchors[i].name, 0, MAX_ANCHOR_NAME_LENGTH);
    }

    layout.refWindow = (Rectangle){ 0, 0, -1, -1 };

    FILE *rglFile = fopen(fileName, "rt");

    if (rglFile != NULL)
    {
        fgets(buffer, 256, rglFile);

        if (buffer[0] != 'R')   // Text file!
        {
            while (!feof(rglFile))
            {
                switch (buffer[0])
                {
                    case 'r':
                    {
                        sscanf(buffer, "r %f %f %f %f", &layout.refWindow.x, &layout.refWindow.y, &layout.refWindow.width, &layout.refWindow.height);
                    } break;
                    case 'a':
                    {
                        sscanf(buffer, "a %03i %s %d %d %d", &layout.anchors[anchorCounter].id, layout.anchors[anchorCounter].name, &layout.anchors[anchorCounter].x, &layout.anchors[anchorCounter].y, &layout.anchors[anchorCounter].enabled);
                        if (TextIsEqual("", layout.anchors[anchorCounter].name)) strcpy(layout.anchors[anchorCounter].name, FormatText("anchor%02i", anchorCounter));
                        if (layout.anchors[anchorCounter].enabled) layout.anchorsCount++;
                        anchorCounter++;
                    } break;
                    case 'c':
                    {
                        sscanf(buffer, "c %d %d %s %f %f %f %f %d %[^\n]s", &layout.controls[layout.controlsCount].id, &layout.controls[layout.controlsCount].type, layout.controls[layout.controlsCount].name, &layout.controls[layout.controlsCount].rec.x, &layout.controls[layout.controlsCount].rec.y, &layout.controls[layout.controlsCount].rec.width, &layout.controls[layout.controlsCount].rec.height, &anchorId, layout.controls[layout.controlsCount].text);
                        layout.controls[layout.controlsCount].ap = &layout.anchors[anchorId];
                        layout.controlsCount++;
                    } break;
                    default: break;
                }

                fgets(buffer, 256, rglFile);
            }

            for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
            {
                layout.anchors[i].x += layout.anchors[0].x;
                layout.anchors[i].y += layout.anchors[0].y;
            }
        }
        else tryBinary = true;

        fclose(rglFile);
    }
/*
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

            fclose(rglFile);
        }
    }
*/
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
            fprintf(rglFile, "# Anchor info:   a <id> <name> <posx> <posy> <enabled>\n");
            fprintf(rglFile, "# Control info:  c <id> <type> <name> <rectangle> <anchor_id> <text>\n#\n");

            printf("r %i %i %i %i\n", (int)layout.refWindow.x, (int)layout.refWindow.y, (int)layout.refWindow.width, (int)layout.refWindow.height);
            fprintf(rglFile, "r %i %i %i %i\n", (int)layout.refWindow.x, (int)layout.refWindow.y, (int)layout.refWindow.width, (int)layout.refWindow.height);
            fprintf(rglFile, "a %03i %s %i %i %i\n", layout.anchors[0].id, layout.anchors[0].name, layout.anchors[0].x, layout.anchors[0].y, layout.anchors[0].enabled);
            
            for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
            {
                fprintf(rglFile, "a %03i %s %i %i %i\n", layout.anchors[i].id, layout.anchors[i].name, (int)(layout.anchors[i].x - layout.refWindow.x), (int)(layout.anchors[i].y - layout.refWindow.y), layout.anchors[i].enabled);
            }

            for (int i = 0; i < layout.controlsCount; i++)
            {
                fprintf(rglFile, "c %03i %i %s %i %i %i %i %i %s\n", layout.controls[i].id, layout.controls[i].type, layout.controls[i].name, (int)layout.controls[i].rec.x, (int)layout.controls[i].rec.y, (int)layout.controls[i].rec.width, (int)layout.controls[i].rec.height, layout.controls[i].ap->id, layout.controls[i].text);
            }

            fclose(rglFile);
        }
    }
}

// Show dialog: load layout file (.rgl)
static void DialogLoadLayout(void)
{

}

// Show save layout dialog
static bool DialogSaveLayout(void)
{
    bool success = false;
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
        success = true;
        
        saveChangesRequired = false;
    }

    return success;
}

// Show save layout dialog
static void DialogExportLayout(unsigned char *toolstr, const char *name)
{
    const char *filters[] = { "*.c", "*.h", "*.go", "*.lua" };
    const char *fileName = tinyfd_saveFileDialog("Export code file", name, 3, filters, "Code file");

    if (fileName != NULL)
    {
        char outFileName[256] = { 0 };
        strcpy(outFileName, fileName);
        if (GetExtension(fileName) == NULL) strcat(outFileName, ".c\0");     // No extension provided

        // Write code string to file
        FILE *ftool = fopen(outFileName, "wt");
        fprintf(ftool, toolstr);
        fclose(ftool);
    }
}
