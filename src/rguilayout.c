/*******************************************************************************************
*
*   rGuiLayout v2.6-dev - A simple and easy-to-use raygui layouts editor
*
*   CONFIGURATION:
*
*   #define VERSION_ONE
*       Enable command-line usage and PRO features for the tool
*
*   #define CUSTOM_MODAL_DIALOGS
*       Use custom raygui generated modal dialogs instead of native OS ones
*       NOTE: Avoids including tinyfiledialogs depencency library
*
*   VERSIONS HISTORY:
*       2.6  (Mar-2022) Updated to raylib 4.1 and raygui 3.2
*       2.5  (05-Jan-2022) Updated to raylib 4.0 and raygui 3.1
*       2.0  (15-Sep-2019) Rewriten from scratch
*       1.0  (14-May-2018) First release
*
*   DEPENDENCIES:
*       raylib 4.1              - Windowing/input management and drawing
*       raygui 3.2              - Immediate-mode GUI controls with custom styling and icons
*       tinyfiledialogs 3.8.8   - Open/save file dialogs, it requires linkage with comdlg32 and ole32 libs
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
*
*   LICENSE: Propietary License
*
*   Copyright (c) 2017-2022 raylib technologies (@raylibtech). All Rights Reserved.
*
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   This project is proprietary and confidential unless the owner allows
*   usage in any other form by expresely written permission.
*
**********************************************************************************************/

#define TOOL_NAME               "rGuiLayout"
#define TOOL_SHORT_NAME         "rGL"
#define TOOL_VERSION            "2.5"
#define TOOL_DESCRIPTION        "A simple and easy-to-use raygui layouts editor"
#define TOOL_RELEASE_DATE       "Dec.2021"
#define TOOL_LOGO_COLOR         0x7da9b9ff

#include "raylib.h"
#include "rguilayout.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

#define RAYGUI_GRID_ALPHA                 0.1f
#define RAYGUI_TEXTSPLIT_MAX_ELEMENTS     256
#define RAYGUI_TEXTSPLIT_MAX_TEXT_SIZE   4096
#define RAYGUI_TOGGLEGROUP_MAX_ELEMENTS   256
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                         // Required for: IMGUI controls

#undef RAYGUI_IMPLEMENTATION                // Avoid including raygui implementation again

//#define GUI_MAIN_TOOLBAR_IMPLEMENTATION
// TODO: #include "gui_main_toolbar.h"               // GUI: Main toolbar panel (file and visualization)

#define GUI_WINDOW_ABOUT_IMPLEMENTATION
#include "gui_window_about.h"               // GUI: About Window

#define GUI_CONTROLS_PALETTE_IMPLEMENTATION
#include "gui_controls_palette.h"           // GUI: Controls Palette

#define GUI_FILE_DIALOGS_IMPLEMENTATION
#include "gui_file_dialogs.h"               // GUI: File Dialog

#define CODEGEN_IMPLEMENTATION
#include "codegen.h"                        // Code generation functions
#include "templates.h"                      // Code template files (char buffers)

#define GUI_WINDOW_CODEGEN_IMPLEMENTATION
#include "gui_window_codegen.h"             // GUI: Code Generation Window

#include <stdlib.h>                         // Required for: calloc(), free()
#include <stdarg.h>                         // Required for: va_list, va_start(), vfprintf(), va_end()
#include <string.h>                         // Required for: strcpy(), strcat(), strlen()
#include <stdio.h>                          // Required for: FILE, fopen(), fclose()...
#include <ctype.h>                          // Required for: toupper(), tolower()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#if (!defined(_DEBUG) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)))
bool __stdcall FreeConsole(void);       // Close console from code (kernel32.lib)
#endif

// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
  #define LOG(...) printf(__VA_ARGS__)
#else
  #define LOG(...)
#endif

#define ANCHOR_RADIUS               20      // Default anchor radius
#define MIN_CONTROL_SIZE            10      // Minimum control size
#define SCALE_BOX_CORNER_SIZE       12      // Scale box bottom-right corner square size

#define MOVEMENT_FRAME_SPEED        2       // Controls movement speed in pixels per frame: TODO: Review

#define MAX_UNDO_LEVELS             10      // Undo levels supported for the ring buffer

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum {
    NONE = 0,

    CONTROL_TESTING,
    CONTROL_EDIT_MOVE,
    CONTROL_EDIT_RESIZE,
    CONTROL_EDIT_RESIZE_MOUSE,
    CONTROL_EDIT_TEXT,
    CONTROL_EDIT_NAME,
    //CONTROL_MULTISELECTION,

    ANCHOR_MODE,
    ANCHOR_EDIT_MOVE,
    ANCHOR_EDIT_LINK,

    TRACEMAP_EDIT,
} LayoutEditMode;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const char *toolName = TOOL_NAME;
static const char *toolVersion = TOOL_VERSION;
static const char *toolDescription = TOOL_DESCRIPTION;

static bool saveChangesRequired = false;    // Flag to notice save changes are required

#define HELP_LINES_COUNT    33

static const char *helpLines[HELP_LINES_COUNT] = {
    "-File Options",
    "LCTRL + N - New layout",
    "LCTRL + O - Open layout file (.rgl)",
    "LCTRL + S - Save layout file (.rgl)",
    "LCTRL + ENTER - Export layout to code",
    "-Control Edition",
    "LSHIFT + ARROWS - Smooth edit position",
    "LCTRL + ARROWS - Edit control scale",
    "LCTRL + LSHIFT + ARROWS - Smooth edit scale",
    "LCTRL + R - Resize control to closest snap",
    "ARROWS - Edit control position (No Snap)",
    "LCTRL + D - Duplicate selected control",
    "DEL - Delete selected control",
    "LCTRL + Z - Undo Action",
    "LCTRL + Y - Redo Action",
    "-Control Edition Ex",
    "T - Control text editing",
    "N - Control name editing",
    "ESC - Exit text/name editing mode",
    "ENTER - Validate text/name edition",
    "LALT + UP/DOWN - Control layer order",
    "-Anchor Edition",
    "A - Anchor editing mode",
    "RMB - Link anchor to control",
    "U - Unlink control from anchor",
    "H - Hide/Unhide controls for selected anchor",
    "-Visual Options",
    "F1 - Toggle Help panel (this one)",
    "F2 - Toggle About window",
    "G - Toggle grid mode",
    "S - Toggle snap to grid mode",
    //"RALT + UP/DOWN - Grid spacing + snap",
    "F - Toggle control position info (global/anchor)",
    "SPACE - Toggle tracemap Lock/Unlock",
};

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(VERSION_ONE)
static void ShowCommandLineInfo(void);                      // Show command line usage info
static void ProcessCommandLine(int argc, char *argv[]);     // Process command line input
#endif

// Init/Load/Save/Export data functions
static GuiLayout *LoadLayout(const char *fileName);         // Load raygui layout: empty (NULL) or from file (.rgl)
static void UnloadLayout(GuiLayout *layout);                // Unload raygui layout
static void ResetLayout(GuiLayout *layout);                 // Reset layout to default values
static void SaveLayout(GuiLayout *layout, const char *fileName);     // Save raygui layout as text file (.rgl)

// Draw help panel with the provided lines
static void GuiHelpPanel(int posX, int posY, const char *title, const char **helpLines, int helpLinesCount);

//----------------------------------------------------------------------------------
// Program main entry point
//----------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    char inFileName[512] = { 0 };       // Input file name (required in case of drag & drop over executable)
    char outFileName[512] = { 0 };      // Output file name (required for file save/export)

#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messsages
#endif
#if defined(VERSION_ONE)
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
        else
        {
            ProcessCommandLine(argc, argv);
            return 0;
        }
    }
#endif      // VERSION_ONE
#if (!defined(_DEBUG) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)))
    // WARNING (Windows): If program is compiled as Window application (instead of console),
    // no console is available to show output info... solution is compiling a console application
    // and closing console (FreeConsole()) when changing to GUI interface
    FreeConsole();
#endif

    // GUI usage mode - Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 800;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);  // Window configuration flags
    InitWindow(screenWidth, screenHeight, TextFormat("%s v%s | %s", toolName, toolVersion, toolDescription));
    SetWindowMinSize(540, 540);
    SetExitKey(0);

    // General app variables
    Vector2 mouse = { -1, -1 };             // Mouse position

    // Grid control variables
    bool showGrid = true;                   // Show grid flag (KEY_G)
    int gridSpacing = 8;                    // Grid minimum spacing in pixels (between every subdivision)
    int gridSubdivisions = 3;               // Grid subdivisions (total size for every big line is gridSpacing*gridSubdivisions)
    int gridSnapDelta = 1;                  // Grid snap minimum value in pixels
    int moveFrameCounter = 0;               // Movement frames counter
    int moveFrameSpeed = 1;                 // Movement speed per frame

    // Work modes
    bool snapMode = false;                  // Snap mode flag (KEY_S)
    bool useGlobalPos = false;              // Control global position mode
    bool precisionMode = false;             // Control precision mode (KEY_LEFT_SHIFT)

    // NOTE: [E] - Exclusive mode operation, all other modes blocked
    bool dragMoveMode = false;              // [E] Control drag mode
    bool textEditMode = false;              // [E] Control text edit mode (KEY_T)
    bool showIconPanel = false;             // Show icon panel for selection
    bool nameEditMode = false;              // [E] Control name edit mode (KEY_N)
    bool orderEditMode = false;             // Control order edit mode (focusedControl != -1 + KEY_LEFT_ALT)
    bool resizeMode = false;                // [E] Control size mode (controlSelected != -1 + KEY_LEFT_ALT)
    bool mouseScaleMode = false;            // [E] Control is being scaled by mouse
    bool mouseScaleReady = false;           // Mouse is on position to start control scaling
    bool refWindowEditMode = false;         // [E] Refence window edit mode

    bool anchorEditMode = false;            // [E] Anchor edition mode
    bool anchorLinkMode = false;            // [E] Anchor linkage mode
    bool anchorMoveMode = false;            // [E] Anchor move mode

    // TODO: Check exclusive modes (work on its own) and combinable modes (can work in combination with other)
    // Replace all bool values by enumerator value, it should simplify code...
    int layoutEditMode = NONE;              // Layout edition mode
    bool windowOverActive = false;           // Check for any blocking window active

    // Multiselection variables
    /*
    bool multiSelectMode = false;           // [E] Multiselection mode
    Rectangle multiSelectRec = { 0 };
    Vector2 multiSelectStartPos = { 0 };
    int multiSelectControls[20] = { -1 };
    int multiSelectCount = 0;
    */

    // Controls variables
    int selectedControl = -1;               // Control selected on layout
    int focusedControl = -1;                // Control focused on layout
    int selectedType = GUI_WINDOWBOX;       // Control type selected on panel
    Color selectedControlColor = RED;       // Control selected color
    Color positionColor = MAROON;           // Control position text color

    Vector2 panOffset = { 0 };
    Vector2 prevPosition = { 0 };

    // Anchors control variables
    GuiAnchorPoint auxAnchor = { 9, 0, 0, 0 };
    int selectedAnchor = -1;
    int focusedAnchor = -1;
    Color anchorCircleColor = BLACK;
    Color anchorSelectedColor = RED;

    // Init default layout
    GuiLayout *layout = NULL;
    if (inFileName[0] != '\0')          // Load dropped file if provided
    {
        layout = LoadLayout(inFileName);
        SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, GetFileName(inFileName)));
    }
    else layout = LoadLayout(NULL);     // Load empty layout

    // Define undo system variables
    GuiLayout *undoLayouts = (GuiLayout *)calloc(MAX_UNDO_LEVELS, sizeof(GuiLayout));   // Layouts array
    int currentUndoIndex = 0;
    int firstUndoIndex = 0;
    int lastUndoIndex = 0;
    int undoFrameCounter = 0;

    // Init undo system with current layout
    for (int i = 0; i < MAX_UNDO_LEVELS; i++) memcpy(&undoLayouts[i], layout, sizeof(GuiLayout));

    // Tracemap (background image for reference) variables
    Texture2D tracemap = { 0 };
    Rectangle tracemapRec = { 0 };
    bool tracemapBlocked = false;
    bool tracemapFocused = false;
    bool tracemapSelected = false;
    float tracemapFade = 0.5f;
    Color tracemapColor = RED;

    // Track previous text/name to cancel editing
    char prevText[MAX_CONTROL_TEXT_LENGTH] = { 0 };
    char prevName[MAX_CONTROL_NAME_LENGTH] = { 0 };

    // GUI: Help panel
    //-----------------------------------------------------------------------------------
    bool helpActive = true;
    //-----------------------------------------------------------------------------------
    
    // GUI: Main toolbar panel (file and visualization)
    //----------------------------------------------------------------------------------
    //GuiMainToolbarState mainToolbarState = InitGuiMainToolbar();
    //-----------------------------------------------------------------------------------

    // GUI: Controls Selection Palette
    //-----------------------------------------------------------------------------------
    GuiControlsPaletteState paletteState = InitGuiControlsPalette();
    //-----------------------------------------------------------------------------------

    // GUI: Layout Code Generation Window
    //-----------------------------------------------------------------------------------
    GuiWindowCodegenState windowCodegenState = InitGuiWindowCodegen();
    //-----------------------------------------------------------------------------------

    // GUI: About Window
    //-----------------------------------------------------------------------------------
    GuiWindowAboutState windowAboutState = InitGuiWindowAbout();
    //-----------------------------------------------------------------------------------

    // GUI: Exit Window
    //-----------------------------------------------------------------------------------
    bool exitWindow = false;
    bool windowExitActive = false;
    //-----------------------------------------------------------------------------------

    // GUI: Reset Layout Window
    //-----------------------------------------------------------------------------------
    bool windowResetActive = false;
    bool resetProgram = false;
    //-----------------------------------------------------------------------------------

    // GUI: Custom file dialogs
    //-----------------------------------------------------------------------------------
    bool showLoadFileDialog = false;
    bool showSaveFileDialog = false;
    bool showExportFileDialog = false;
    //-----------------------------------------------------------------------------------

    // Rectangles used on controls preview drawing, copied from palette
    // NOTE: [x, y] position is set on mouse movement and cosidering snap mode
    Rectangle defaultRec[CONTROLS_PALETTE_COUNT] = { 0 };
    for (int i = 0; i < CONTROLS_PALETTE_COUNT; i++)
    {
        defaultRec[i].width = paletteState.controlRecs[i].width;
        defaultRec[i].height = paletteState.controlRecs[i].height;
    }

    // Generate code configuration
    GuiLayoutConfig config = { 0 };
    strcpy(config.name, "window_codegen");
    strcpy(config.version, toolVersion);
    strcpy(config.company, "raylib technologies");
    strcpy(config.description, "tool description");
    config.exportAnchors = false;
    config.defineRecs = false;
    config.defineTexts = false;
    config.fullComments = false;
    config.exportButtonFunctions = false;
    int currentCodeTemplate = 0;

    GuiLayoutConfig prevConfig = { 0 };
    memcpy(&prevConfig, &config, sizeof(GuiLayoutConfig));

    // Controls temp variables
    int dropdownBoxActive = 0;
    int spinnerValue = 0;
    int valueBoxValue = 0;
    int listViewScrollIndex = 0;
    int listViewActive = 0;

    int workAreaOffsetY = 0;        // TODO: Main toolbar height

    // Select icon ToggleGroup()
    int selectedIcon = 0; 
    char toggleIconsText[16*13*6] = { 0 };  // 13 lines with 16 icons per line -> TODO: Review if more icons are added!
    for (int i = 0; i < 16*13; i++)
    {
        // NOTE: Every icon requires 6 text characters: "#001#;"
        if ((i + 1)%16 == 0) strncpy(toggleIconsText + 6*i, TextFormat("#%03i#\n", i), 6);
        else strncpy(toggleIconsText + 6*i, TextFormat("#%03i#;", i), 6);
    }
    toggleIconsText[16*13*6 - 1] = '\0';

    SetTargetFPS(120);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!exitWindow)             // Detect window close button
    {
        if (WindowShouldClose()) windowExitActive = true;

        // Undo layout change logic
        //----------------------------------------------------------------------------------
        // Every second check if current layout has changed and record a new undo state
        if (!dragMoveMode && !orderEditMode && !resizeMode && !refWindowEditMode &&
            !textEditMode && !showIconPanel && !nameEditMode && !anchorEditMode && !anchorLinkMode && !anchorMoveMode)
        {
            undoFrameCounter++;

            if (undoFrameCounter >= 120)
            {
                if (memcmp(&undoLayouts[currentUndoIndex], layout, sizeof(GuiLayout)) != 0)
                {
                    // Move cursor to next available position to record undo
                    currentUndoIndex++;
                    if (currentUndoIndex >= MAX_UNDO_LEVELS) currentUndoIndex = 0;
                    if (currentUndoIndex == firstUndoIndex) firstUndoIndex++;
                    if (firstUndoIndex >= MAX_UNDO_LEVELS) firstUndoIndex = 0;

                    memcpy(&undoLayouts[currentUndoIndex], layout, sizeof(GuiLayout));

                    lastUndoIndex = currentUndoIndex;

                    // Set a '*' mark on loaded file name to notice save requirement
                    if ((inFileName[0] != '\0') && !saveChangesRequired)
                    {
                        SetWindowTitle(TextFormat("%s v%s - %s*", toolName, toolVersion, GetFileName(inFileName)));
                        saveChangesRequired = true;
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

                if (memcmp(&undoLayouts[currentUndoIndex], layout, sizeof(GuiLayout)) != 0)
                {
                    memcpy(layout, &undoLayouts[currentUndoIndex], sizeof(GuiLayout));
                }
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

                    if (memcmp(&undoLayouts[currentUndoIndex], layout, sizeof(GuiLayout)) != 0)
                    {
                        memcpy(layout, &undoLayouts[currentUndoIndex], sizeof(GuiLayout));
                    }
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
                GuiLayout *tempLayout = LoadLayout(droppedFileName);

                if (tempLayout != NULL)
                {
                    memcpy(layout, tempLayout, sizeof(GuiLayout));

                    // HACK: When leaving scope, tempLayout internal pointer references are lost,
                    // so we manually reset those references to layout internals
                    // TODO: Probably this system should be designed in a diferent way...
                    for (int i = 0; i < layout->controlCount; i++) layout->controls[i].ap = &layout->anchors[tempLayout->controls[i].ap->id];

                    strcpy(inFileName, droppedFileName);
                    SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, GetFileName(inFileName)));

                    for (int i = 0; i < MAX_UNDO_LEVELS; i++) memcpy(&undoLayouts[i], layout, sizeof(GuiLayout));
                    currentUndoIndex = 0;
                    firstUndoIndex = 0;

                    UnloadLayout(tempLayout);
                }
            }
            else if (IsFileExtension(droppedFileName, ".png")) // Tracemap image
            {
                if (tracemap.id > 0) UnloadTexture(tracemap);
                tracemap = LoadTexture(droppedFileName);
                tracemapRec = (Rectangle){30, 30, tracemap.width, tracemap.height};
            }
            else if (IsFileExtension(droppedFileName, ".rgs")) GuiLoadStyle(droppedFileName);

            ClearDroppedFiles();
        }
        //----------------------------------------------------------------------------------

        // Keyboard shortcuts
        //----------------------------------------------------------------------------------
        // Show window: load layout
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O)) showLoadFileDialog = true;

        // Show dialog: save layout
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_S)) showSaveFileDialog = true;
        else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
        {
            if (inFileName[0] == '\0') showSaveFileDialog = true;
            else
            {
                SaveLayout(layout, inFileName);
                SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, GetFileName(inFileName)));
                saveChangesRequired = false;
            }
        }

        // Show window: help
        if (IsKeyPressed(KEY_F1)) helpActive = !helpActive;

        // Show window: about
        if (IsKeyPressed(KEY_F2)) windowAboutState.windowActive = !windowAboutState.windowActive;

        // Show save layout message window on ESC
        if (IsKeyPressed(KEY_ESCAPE))
        {
            if (textEditMode)       // Cancel text edit mode
            {
                textEditMode = false;
                showIconPanel = false;
                if (selectedControl != -1)
                {
                    memset(layout->controls[selectedControl].text, 0, MAX_CONTROL_TEXT_LENGTH);
                    strcpy(layout->controls[selectedControl].text, prevText);
                }
            }
            else if (nameEditMode)  // Cancel name edit mode
            {
                nameEditMode = false;
                if (selectedControl != -1)
                {
                    memset(layout->controls[selectedControl].name, 0, MAX_CONTROL_NAME_LENGTH);
                    strcpy(layout->controls[selectedControl].name, prevName);
                }
                else if (selectedAnchor != -1)
                {
                    memset(layout->anchors[selectedAnchor].name, 0, MAX_CONTROL_NAME_LENGTH);
                    strcpy(layout->anchors[selectedAnchor].name, prevName);
                }
            }
            else
            {
                // Close windows logic
                if (windowAboutState.windowActive) windowAboutState.windowActive = false;
                else if (windowCodegenState.windowCodegenActive) windowCodegenState.windowCodegenActive = false;
                else if (windowResetActive) windowResetActive = false;
#if !defined(PLATFORM_WEB)
                else if ((layout->controlCount <= 0) && (layout->anchorCount <= 1)) exitWindow = true;  // Quit application
                else
                {
                    windowExitActive = !windowExitActive;
                    selectedControl = -1;
                    selectedAnchor = -1;
                }
#endif
            }
        }

        // Check for any blocking mode (window or text/name edition)
        if (!windowOverActive && !textEditMode && !nameEditMode)
        {
            // Enables or disables snapMode if not in textEditMode
            if (IsKeyPressed(KEY_S))
            {
                snapMode = !snapMode;
                if (snapMode)
                {
                    gridSnapDelta = gridSpacing;
                    moveFrameSpeed = MOVEMENT_FRAME_SPEED;
                }
                else
                {
                    gridSnapDelta = 1;      // 1 pixel variation
                    moveFrameSpeed = 1;
                }
            }

            // Work modes
            if (IsKeyPressed(KEY_F)) useGlobalPos = !useGlobalPos;      // Toggle global position info (anchor reference or global reference)
            if (IsKeyPressed(KEY_G)) showGrid = !showGrid;              // Toggle Grid mode

            anchorEditMode = IsKeyDown(KEY_A);              // Toggle anchor mode editing (on key down)
            orderEditMode = IsKeyDown(KEY_LEFT_ALT);        // Toggle controls drawing order

            precisionMode = IsKeyDown(KEY_LEFT_SHIFT);      // Toggle precision move/scale mode
            resizeMode = IsKeyDown(KEY_LEFT_CONTROL);       // Toggle control resize mode

            // Enable/disable texture editing mode
            if ((tracemap.id > 0) && IsKeyPressed(KEY_SPACE))
            {
                if (tracemapSelected) tracemapBlocked = true;
                else if (tracemapBlocked) tracemapBlocked = false;
            }

            // Check modes requiring LEFT_CONTROL modifier
            if (IsKeyDown(KEY_LEFT_CONTROL))
            {
                // Open reset window
                if (IsKeyPressed(KEY_N)) windowResetActive = true;

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
                    config.exportButtonFunctions = windowCodegenState.generateButtonFunctionsChecked;

                    memcpy(&prevConfig, &config, sizeof(GuiLayoutConfig));

                    free(windowCodegenState.codeText);
                    windowCodegenState.codeText = GenLayoutCode(guiTemplateStandardCode, *layout, config);
                    windowCodegenState.windowCodegenActive = true;
                }
            }

            // Change grid spacing
            // TODO: Look for a better mechanism
            /*
            if (IsKeyDown(KEY_RIGHT_ALT))
            {
                if (IsKeyPressed(KEY_UP)) gridSpacing++;
                else if (IsKeyPressed(KEY_DOWN)) gridSpacing--;

                gridSnapDelta = gridSpacing;
            }
            */
        }

        // Basic program flow logic
        mouse = GetMousePosition();

        // Code generation window logic
        //----------------------------------------------------------------------------------
        if (windowCodegenState.windowCodegenActive)
        {
            strcpy(config.name, windowCodegenState.toolNameText);
            strcpy(config.version, windowCodegenState.toolVersionText);
            strcpy(config.company, windowCodegenState.companyText);
            strcpy(config.description, windowCodegenState.toolDescriptionText);
            config.exportAnchors = windowCodegenState.exportAnchorsChecked;
            config.defineRecs = windowCodegenState.defineRecsChecked;
            config.defineTexts = windowCodegenState.defineTextsChecked;
            config.fullComments = windowCodegenState.fullCommentsChecked;
            config.exportButtonFunctions = windowCodegenState.generateButtonFunctionsChecked;

            if ((currentCodeTemplate != windowCodegenState.codeTemplateActive) ||
                (memcmp(&config, &prevConfig, sizeof(GuiLayoutConfig)) != 0))
            {
                const unsigned char *template = NULL;
                if (windowCodegenState.codeTemplateActive == 0) template = guiTemplateStandardCode;
                else if (windowCodegenState.codeTemplateActive >= 1) template = guiTemplateHeaderOnly;
                //else if (windowCodegenState.codeTemplateActive == 2) template = LoadFileText(/*custom_template*/);
                currentCodeTemplate = windowCodegenState.codeTemplateActive;

                free(windowCodegenState.codeText);
                windowCodegenState.codeText = GenLayoutCode(template, *layout, config);
                memcpy(&prevConfig, &config, sizeof(GuiLayoutConfig));

                windowCodegenState.codePanelScrollOffset = (Vector2){ 0, 0 };
            }
        }
        //----------------------------------------------------------------------------------

        // Layout edition logic
        //----------------------------------------------------------------------------------------------
        // Check for any blocking mode (window or text/name edition)
        if (!windowOverActive && !nameEditMode && !textEditMode)
        {
            // Mouse snap logic
            //----------------------------------------------------------------------------------------------
            if (snapMode && !anchorLinkMode)
            {
                int offsetX = (int)mouse.x%gridSpacing;
                int offsetY = (int)mouse.y%gridSpacing;

                if (offsetX >= gridSpacing/2) mouse.x += (gridSpacing - offsetX);
                else mouse.x -= offsetX;

                if (offsetY >= gridSpacing/2) mouse.y += (gridSpacing - offsetY);
                else mouse.y -= offsetY;
            }
            //----------------------------------------------------------------------------------------------

            // Palette selected control logic
            //----------------------------------------------------------------------------------------------
            if (!CheckCollisionPointRec(mouse, paletteState.scrollPanelBounds))
            {
                if (focusedControl == -1) paletteState.selectedControl -= GetMouseWheelMove();

                if (paletteState.selectedControl < GUI_WINDOWBOX) paletteState.selectedControl = GUI_WINDOWBOX;
                else if (paletteState.selectedControl > GUI_DUMMYREC) paletteState.selectedControl = GUI_DUMMYREC;

                selectedType = paletteState.selectedControl;
            }
            //----------------------------------------------------------------------------------------------

            // Controls selection and edition logic
            //----------------------------------------------------------------------------------------------

            // Updates the default rectangle position
            defaultRec[selectedType].x = mouse.x - defaultRec[selectedType].width/2;
            defaultRec[selectedType].y = mouse.y - defaultRec[selectedType].height/2;

            if (snapMode)
            {
                // TODO: Review depending on the Grid size and position

                int offsetX = (int)defaultRec[selectedType].x%gridSnapDelta;
                int offsetY = (int)defaultRec[selectedType].y%gridSnapDelta;

                if (offsetX >= gridSpacing/2) defaultRec[selectedType].x += (gridSpacing - offsetX);
                else defaultRec[selectedType].x -= offsetX;

                if (offsetY >= gridSpacing/2) defaultRec[selectedType].y += (gridSpacing - offsetY);
                else defaultRec[selectedType].y -= offsetY;
            }

            if (!CheckCollisionPointRec(mouse, paletteState.scrollPanelBounds))
            {
                if (!dragMoveMode)
                {
                    focusedControl = -1;

                    // Focus control logic
                    if (!anchorEditMode && focusedAnchor == -1)
                    {
                        for (int i = layout->controlCount - 1; i >= 0; i--)
                        {
                            if (!layout->controls[i].ap->hidding)
                            {
                                Rectangle layoutRec = layout->controls[i].rec;
                                if (layout->controls[i].type == GUI_WINDOWBOX) layoutRec.height = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT;  // Defined inside raygui.h
                                else if (layout->controls[i].type == GUI_GROUPBOX)
                                {
                                    layoutRec.y -= 10;
                                    layoutRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE)*2.0f;
                                }

                                if (layout->controls[i].ap->id > 0)
                                {
                                    layoutRec.x += layout->controls[i].ap->x;
                                    layoutRec.y += layout->controls[i].ap->y;
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
                    if ((focusedAnchor == -1) && (selectedAnchor == -1) && (selectedControl == -1) && !tracemapFocused && !tracemapSelected)
                    {
                        // Create new control
                        if (!anchorEditMode && !anchorLinkMode)
                        {
                            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                            {
                                // Config new control
                                layout->controls[layout->controlCount].id = layout->controlCount;
                                layout->controls[layout->controlCount].type = selectedType;
                                layout->controls[layout->controlCount].rec = (Rectangle){ defaultRec[selectedType].x, defaultRec[selectedType].y,
                                                                            defaultRec[selectedType].width, defaultRec[selectedType].height };

                                if ((layout->controls[layout->controlCount].type == GUI_LABEL)
                                    || (layout->controls[layout->controlCount].type == GUI_TEXTBOX)
                                    || (layout->controls[layout->controlCount].type == GUI_TEXTBOXMULTI)
                                    || (layout->controls[layout->controlCount].type == GUI_BUTTON)
                                    || (layout->controls[layout->controlCount].type == GUI_LABELBUTTON)
                                    || (layout->controls[layout->controlCount].type == GUI_CHECKBOX)
                                    || (layout->controls[layout->controlCount].type == GUI_TOGGLE)
                                    || (layout->controls[layout->controlCount].type == GUI_GROUPBOX)
                                    || (layout->controls[layout->controlCount].type == GUI_VALUEBOX)
                                    || (layout->controls[layout->controlCount].type == GUI_SPINNER)
                                    || (layout->controls[layout->controlCount].type == GUI_WINDOWBOX)
                                    || (layout->controls[layout->controlCount].type == GUI_STATUSBAR)
                                    || (layout->controls[layout->controlCount].type == GUI_DUMMYREC))
                                {
                                    strcpy(layout->controls[layout->controlCount].text, "SAMPLE TEXT");
                                }

                                if ((layout->controls[layout->controlCount].type == GUI_TOGGLEGROUP)
                                    || (layout->controls[layout->controlCount].type == GUI_COMBOBOX)
                                    || (layout->controls[layout->controlCount].type == GUI_DROPDOWNBOX)
                                    || (layout->controls[layout->controlCount].type == GUI_LISTVIEW))
                                {
                                    strcpy(layout->controls[layout->controlCount].text, "ONE;TWO;THREE");
                                }

                                strcpy(layout->controls[layout->controlCount].name,
                                       TextFormat("%s%03i", controlTypeName[layout->controls[layout->controlCount].type], layout->controlCount));

                                layout->controls[layout->controlCount].ap = &layout->anchors[0];        // Default anchor point (0, 0)

                                // If we create new control inside a windowbox, then anchor the new control to the windowbox anchor
                                for (int i = layout->controlCount; i >= 0; i--)
                                {
                                    if ((layout->controls[i].type == GUI_WINDOWBOX) || (layout->controls[i].type == GUI_GROUPBOX))
                                    {
                                        if (CheckCollisionPointRec(mouse, (Rectangle){ layout->controls[i].ap->x + layout->controls[i].rec.x,
                                                                                       layout->controls[i].ap->y + layout->controls[i].rec.y,
                                                                                       layout->controls[i].rec.width, layout->controls[i].rec.height }))
                                        {
                                            layout->controls[layout->controlCount].ap = layout->controls[i].ap;
                                            break;
                                        }
                                    }
                                }

                                // Create anchor for windowbox control if we can
                                if ((layout->anchorCount < MAX_ANCHOR_POINTS) &&
                                    ((layout->controls[layout->controlCount].type == GUI_WINDOWBOX) || (layout->controls[layout->controlCount].type == GUI_GROUPBOX)))
                                {
                                    for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
                                    {
                                        if (!layout->anchors[i].enabled)
                                        {
                                            layout->anchors[i].x = layout->controls[layout->controlCount].rec.x;
                                            layout->anchors[i].y = layout->controls[layout->controlCount].rec.y;

                                            if (snapMode)
                                            {
                                                int offsetX = layout->anchors[i].x%gridSpacing;
                                                int offsetY = layout->anchors[i].y%gridSpacing;

                                                if (offsetX >= gridSpacing/2) layout->anchors[i].x += (gridSpacing - offsetX);
                                                else layout->anchors[i].x -= offsetX;

                                                if (offsetY >= gridSpacing/2) layout->anchors[i].y += (gridSpacing - offsetY);
                                                else layout->anchors[i].y -= offsetY;
                                            }

                                            layout->controls[layout->controlCount].rec.x = layout->anchors[i].x;
                                            layout->controls[layout->controlCount].rec.y = layout->anchors[i].y;

                                            layout->anchors[i].enabled = true;
                                            layout->controls[layout->controlCount].ap = &layout->anchors[i];

                                            layout->anchorCount++;
                                            break;
                                        }
                                    }
                                }

                                if (layout->controls[layout->controlCount].ap->id > 0)
                                {
                                    layout->controls[layout->controlCount].rec.x -= layout->controls[layout->controlCount].ap->x;
                                    layout->controls[layout->controlCount].rec.y -= layout->controls[layout->controlCount].ap->y;
                                }
                                layout->controlCount++;

                                focusedControl = layout->controlCount - 1;
                                selectedControl = layout->controlCount - 1;
                            }
                        }
                    }
                }
                else //focusedControl != -1
                {
                    // Change controls layer order (position inside array)
                    if (orderEditMode)
                    {
                        int newOrder = 0;
                        if (IsKeyPressed(KEY_UP)) newOrder = 1;
                        else if (IsKeyPressed(KEY_DOWN)) newOrder = -1;
                        else newOrder -= GetMouseWheelMove();

                        if ((newOrder > 0) && (focusedControl < layout->controlCount - 1))
                        {
                            // Move control towards beginning of array
                            GuiLayoutControl auxControl = layout->controls[focusedControl];
                            layout->controls[focusedControl] = layout->controls[focusedControl + 1];
                            layout->controls[focusedControl].id -= 1;
                            layout->controls[focusedControl + 1] = auxControl;
                            layout->controls[focusedControl + 1].id += 1;
                            selectedControl = -1;
                        }
                        else if ((newOrder < 0) && (focusedControl > 0))
                        {
                            // Move control towards end of array
                            GuiLayoutControl auxControl = layout->controls[focusedControl];
                            layout->controls[focusedControl] = layout->controls[focusedControl - 1];
                            layout->controls[focusedControl].id += 1;
                            layout->controls[focusedControl - 1] = auxControl;
                            layout->controls[focusedControl - 1].id -= 1;
                            selectedControl = -1;
                        }
                    }
                }

                // Unselect control
                if (!mouseScaleReady && (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)))
                {
                    selectedControl = focusedControl;
                    if ((focusedAnchor != -1) || anchorLinkMode || anchorEditMode) selectedControl = -1;
                }

                // On selected control
                if (selectedControl != -1)
                {
                    // Mouse resize mode logic
                    //--------------------------------------------------------------------------
                    Rectangle rec = layout->controls[selectedControl].rec;

                    // NOTE: We must consider anchor offset!
                    if (layout->controls[selectedControl].ap->id > 0)
                    {
                        rec.x += layout->controls[selectedControl].ap->x;
                        rec.y += layout->controls[selectedControl].ap->y;
                    }

                    if (CheckCollisionPointRec(mouse, rec) &&
                        CheckCollisionPointRec(mouse, (Rectangle){ rec.x + rec.width - SCALE_BOX_CORNER_SIZE,
                                                                   rec.y + rec.height - SCALE_BOX_CORNER_SIZE,
                                                                   SCALE_BOX_CORNER_SIZE, SCALE_BOX_CORNER_SIZE }))
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

                        if (rec.width < SCALE_BOX_CORNER_SIZE) rec.width = SCALE_BOX_CORNER_SIZE;
                        if (rec.height < SCALE_BOX_CORNER_SIZE) rec.height = SCALE_BOX_CORNER_SIZE;

                        // NOTE: We must consider anchor offset!
                        if (layout->controls[selectedControl].ap->id > 0)
                        {
                            rec.x -= layout->controls[selectedControl].ap->x;
                            rec.y -= layout->controls[selectedControl].ap->y;
                        }

                        layout->controls[selectedControl].rec = rec;

                        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) mouseScaleMode = false;
                    }
                    //--------------------------------------------------------------------------

                    if (!anchorLinkMode)
                    {
                        if (dragMoveMode && !mouseScaleMode)
                        {
                            // Drag controls
                            int controlPosX = prevPosition.x + (mouse.x - panOffset.x);
                            int controlPosY = prevPosition.y + (mouse.y - panOffset.y);

                            if (snapMode)
                            {
                                int offsetX = (int)controlPosX%gridSnapDelta;
                                int offsetY = (int)controlPosY%gridSnapDelta;

                                if (offsetX >= gridSnapDelta/2) controlPosX += (gridSnapDelta - offsetX);
                                else controlPosX -= offsetX;

                                if (offsetY >= gridSnapDelta/2) controlPosY += (gridSnapDelta - offsetY);
                                else controlPosY -= offsetY;
                            }

                            if (useGlobalPos && (layout->controls[selectedControl].ap->id != 0))
                            {
                                controlPosX -= layout->controls[selectedControl].ap->x;
                                controlPosY -= layout->controls[selectedControl].ap->y;
                            }

                            layout->controls[selectedControl].rec.x = controlPosX;
                            layout->controls[selectedControl].rec.y = controlPosY;

                            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragMoveMode = false;
                        }
                        else
                        {
                            if (resizeMode)
                            {
                                if (IsKeyPressed(KEY_R) && layout->controls[selectedControl].type == GUI_WINDOWBOX)
                                {
                                    Rectangle rec = layout->controls[selectedControl].rec;

                                    if (layout->controls[selectedControl].ap->id > 0)
                                    {
                                        rec.x += layout->controls[selectedControl].ap->x;
                                        rec.y += layout->controls[selectedControl].ap->y;
                                    }

                                    layout->anchors[0].x = rec.x;
                                    layout->anchors[0].y = rec.y;
                                    layout->refWindow = (Rectangle){layout->anchors[0].x, layout->anchors[0].y, rec.width, rec.height};
                                }

                                // Duplicate control
                                if (IsKeyPressed(KEY_D))
                                {
                                    layout->controls[layout->controlCount].id = layout->controlCount;
                                    layout->controls[layout->controlCount].type = layout->controls[selectedControl].type;
                                    layout->controls[layout->controlCount].rec = layout->controls[selectedControl].rec;
                                    layout->controls[layout->controlCount].rec.x += 10;
                                    layout->controls[layout->controlCount].rec.y += 10;
                                    strcpy(layout->controls[layout->controlCount].text, layout->controls[selectedControl].text);
                                    strcpy(layout->controls[layout->controlCount].name, TextFormat("%s%03i", controlTypeName[layout->controls[layout->controlCount].type], layout->controlCount));
                                    layout->controls[layout->controlCount].ap = layout->controls[selectedControl].ap;            // Default anchor point (0, 0)

                                    layout->controlCount++;

                                    selectedControl = layout->controlCount - 1;
                                }

                                // Resize control
                                int offsetX = (int)layout->controls[selectedControl].rec.width%gridSnapDelta;
                                int offsetY = (int)layout->controls[selectedControl].rec.height%gridSnapDelta;

                                if (precisionMode)
                                {
                                    if (IsKeyPressed(KEY_RIGHT)) layout->controls[selectedControl].rec.width += (gridSnapDelta - offsetX);
                                    else if (IsKeyPressed(KEY_LEFT))
                                    {
                                        if (offsetX == 0) offsetX = gridSnapDelta;
                                        layout->controls[selectedControl].rec.width -= offsetX;
                                    }

                                    if (IsKeyPressed(KEY_DOWN)) layout->controls[selectedControl].rec.height += (gridSnapDelta - offsetY);
                                    else if (IsKeyPressed(KEY_UP))
                                    {
                                        if (offsetY == 0) offsetY = gridSnapDelta;
                                        layout->controls[selectedControl].rec.height -= offsetY;
                                    }

                                    moveFrameCounter = 0;
                                }
                                else
                                {
                                    moveFrameCounter++;

                                    if ((moveFrameCounter%moveFrameSpeed) == 0)
                                    {
                                        if (IsKeyDown(KEY_RIGHT)) layout->controls[selectedControl].rec.width += (gridSnapDelta - offsetX);
                                        else if (IsKeyDown(KEY_LEFT))
                                        {
                                            if (offsetX == 0) offsetX = gridSnapDelta;
                                            layout->controls[selectedControl].rec.width -= offsetX;
                                        }

                                        if (IsKeyDown(KEY_DOWN)) layout->controls[selectedControl].rec.height += (gridSnapDelta - offsetY);
                                        else if (IsKeyDown(KEY_UP))
                                        {
                                            if (offsetY == 0) offsetY = gridSnapDelta;
                                            layout->controls[selectedControl].rec.height -= offsetY;
                                        }

                                        moveFrameCounter = 0;
                                    }
                                }

                                // Minimum size limit
                                if (layout->controls[selectedControl].rec.width < MIN_CONTROL_SIZE) layout->controls[selectedControl].rec.width = MIN_CONTROL_SIZE;
                                if (layout->controls[selectedControl].rec.height < MIN_CONTROL_SIZE) layout->controls[selectedControl].rec.height = MIN_CONTROL_SIZE;
                            }
                            else
                            {
                                // Move controls with arrows
                                int controlPosX = (int)layout->controls[selectedControl].rec.x;
                                int controlPosY = (int)layout->controls[selectedControl].rec.y;

                                if (useGlobalPos && (layout->controls[selectedControl].ap->id != 0))
                                {
                                    controlPosX += layout->controls[selectedControl].ap->x;
                                    controlPosY += layout->controls[selectedControl].ap->y;
                                }

                                int offsetX = (int)controlPosX%gridSnapDelta;
                                int offsetY = (int)controlPosY%gridSnapDelta;

                                if (precisionMode)
                                {
                                    if (IsKeyPressed(KEY_RIGHT))  controlPosX += (gridSnapDelta - offsetX);
                                    else if (IsKeyPressed(KEY_LEFT))
                                    {
                                        if (offsetX == 0) offsetX = gridSnapDelta;
                                        controlPosX -= offsetX;
                                    }

                                    if (IsKeyPressed(KEY_DOWN)) controlPosY += (gridSnapDelta - offsetY);
                                    else if (IsKeyPressed(KEY_UP))
                                    {
                                        if (offsetY == 0) offsetY = gridSnapDelta;
                                        controlPosY -= offsetY;
                                    }

                                    moveFrameCounter = 0;
                                }
                                else
                                {
                                    moveFrameCounter++;

                                    if ((moveFrameCounter%moveFrameSpeed) == 0)
                                    {
                                        if (IsKeyDown(KEY_RIGHT)) controlPosX += (gridSnapDelta - offsetX);
                                        else if (IsKeyDown(KEY_LEFT))
                                        {
                                            if (offsetX == 0) offsetX = gridSnapDelta;
                                            controlPosX -= offsetX;
                                        }

                                        if (IsKeyDown(KEY_DOWN)) controlPosY += (gridSnapDelta - offsetY);
                                        else if (IsKeyDown(KEY_UP))
                                        {
                                            if (offsetY == 0) offsetY = gridSnapDelta;
                                            controlPosY -= offsetY;
                                        }

                                        moveFrameCounter = 0;
                                    }
                                }

                                if (useGlobalPos && (layout->controls[selectedControl].ap->id != 0))
                                {
                                    controlPosX -= layout->controls[selectedControl].ap->x;
                                    controlPosY -= layout->controls[selectedControl].ap->y;
                                }

                                layout->controls[selectedControl].rec.x = controlPosX;
                                layout->controls[selectedControl].rec.y = controlPosY;
                                //---------------------------------------------------------------------

                                // Unlinks the control selected from its current anchor
                                if (layout->controls[selectedControl].ap->id != 0 && IsKeyPressed(KEY_U))
                                {

                                    layout->controls[selectedControl].rec.x += layout->controls[selectedControl].ap->x;
                                    layout->controls[selectedControl].rec.y += layout->controls[selectedControl].ap->y;
                                    layout->controls[selectedControl].ap = &layout->anchors[0];
                                }

                                // Delete selected control
                                if (IsKeyPressed(KEY_DELETE))
                                {
                                    mouseScaleReady = false;

                                    for (int i = selectedControl; i < layout->controlCount; i++)
                                    {
                                        layout->controls[i].type = layout->controls[i + 1].type;
                                        layout->controls[i].rec = layout->controls[i + 1].rec;
                                        memset(layout->controls[i].text, 0, MAX_CONTROL_TEXT_LENGTH);
                                        memset(layout->controls[i].name, 0, MAX_CONTROL_NAME_LENGTH);
                                        strcpy(layout->controls[i].text, layout->controls[i + 1].text);
                                        strcpy(layout->controls[i].name, layout->controls[i + 1].name);
                                        layout->controls[i].ap = layout->controls[i + 1].ap;
                                    }

                                    layout->controlCount--;
                                    focusedControl = -1;
                                    selectedControl = -1;
                                }

                                // Enable drag mode (if not on mouse scale mode)
                                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !mouseScaleMode)
                                {
                                    panOffset = mouse;

                                    if (useGlobalPos && (layout->controls[selectedControl].ap->id != 0))
                                    {
                                        prevPosition = (Vector2){ layout->controls[selectedControl].rec.x + layout->controls[selectedControl].ap->x,
                                                                  layout->controls[selectedControl].rec.y + layout->controls[selectedControl].ap->y };
                                    }
                                    else prevPosition = (Vector2){ layout->controls[selectedControl].rec.x, layout->controls[selectedControl].rec.y };

                                    dragMoveMode = true;
                                }
                                else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
                                {
                                    anchorLinkMode = true;      // Enable anchor link mode
                                }
                                else if (IsKeyReleased(KEY_T))  // Enable text edit mode
                                {
                                    if (layout->controls[selectedControl].type != GUI_PANEL &&
                                        layout->controls[selectedControl].type != GUI_SCROLLPANEL &&
                                        //layout->controls[selectedControl].type != GUI_LISTVIEW &&
                                        layout->controls[selectedControl].type != GUI_COLORPICKER)
                                    {
                                        strcpy(prevText, layout->controls[selectedControl].text);
                                        textEditMode = true;
                                    }
                                    else LOG("WARNING: Can't edit text on this control\n");
                                }

                                // Enable name edit mode
                                else if (IsKeyReleased(KEY_N))
                                {
                                    nameEditMode = true;
                                    strcpy(prevName, layout->controls[selectedControl].name);
                                }
                            }
                        }
                    }
                    else // anchorLinkMode == true
                    {
                        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
                        {
                            anchorLinkMode = false;

                            if (layout->controls[selectedControl].ap->id > 0)
                            {
                                layout->controls[selectedControl].rec.x += layout->controls[selectedControl].ap->x;
                                layout->controls[selectedControl].rec.y += layout->controls[selectedControl].ap->y;
                            }
                            layout->controls[selectedControl].ap = &layout->anchors[focusedAnchor];
                            if (focusedAnchor > 0)
                            {
                                layout->controls[selectedControl].rec.x -= layout->anchors[focusedAnchor].x;
                                layout->controls[selectedControl].rec.y -= layout->anchors[focusedAnchor].y;
                            }
                        }
                    }
                }
            }
            //----------------------------------------------------------------------------------------------

            // Controls multi-selection and edition logic
            //----------------------------------------------------------------------------------------------
            /*
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
                    for (int i = 0; i < layout->controlCount; i++)
                    {
                        if (CheckCollisionRecs(multiSelectRec, layout->controls[i].rec))
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
                    for (int i = multiSelectControls[j]; i < layout->controlCount; i++)
                    {
                        layout->controls[i].type = layout->controls[i + 1].type;
                        layout->controls[i].rec = layout->controls[i + 1].rec;
                        memset(layout->controls[i].text, 0, MAX_CONTROL_TEXT_LENGTH);
                        memset(layout->controls[i].name, 0, MAX_CONTROL_NAME_LENGTH);
                        strcpy(layout->controls[i].text, layout->controls[i + 1].text);
                        strcpy(layout->controls[i].name, layout->controls[i + 1].name);
                        layout->controls[i].ap = layout->controls[i + 1].ap;
                    }

                    layout->controlCount--;

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
            */
            //----------------------------------------------------------------------------------------------

            // Anchors selection and edition logic
            //----------------------------------------------------------------------------------------------
            if (!dragMoveMode)
            {
                focusedAnchor = -1;

                // Checks if mouse is over an anchor
                for (int i = 0; i < MAX_ANCHOR_POINTS; i++)
                {
                    if (layout->anchors[i].enabled)
                    {
                        if (CheckCollisionPointCircle(mouse, (Vector2){ layout->anchors[i].x, layout->anchors[i].y }, ANCHOR_RADIUS))
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
                    if (!anchorLinkMode && anchorEditMode && layout->anchorCount < MAX_ANCHOR_POINTS && !tracemapFocused && !tracemapSelected)
                    {
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                        {
                            layout->anchorCount++;
                            for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
                            {
                                if (!layout->anchors[i].enabled)
                                {
                                    layout->anchors[i].x = mouse.x;
                                    layout->anchors[i].y = mouse.y;
                                    layout->anchors[i].enabled = true;
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
                        layout->refWindow.width = mouse.x - layout->refWindow.x;
                        layout->refWindow.height = mouse.y  - layout->refWindow.y;

                        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                        {
                            if (layout->refWindow.width < 0) layout->refWindow.width = -1;
                            if (layout->refWindow.height < 0) layout->refWindow.height = -1;
                            refWindowEditMode = false;
                        }
                    }
                    else
                    {
                        if (dragMoveMode)
                        {
                            if (selectedAnchor == 0) anchorEditMode = false;
                            // Move anchor without moving controls
                            if (anchorMoveMode && !anchorEditMode)
                            {
                                for (int i = 0; i < layout->controlCount; i++)
                                {
                                    if (layout->controls[i].ap->id == 9) //auxAnchor ID
                                    {
                                        if (layout->controls[i].ap->id > 0)
                                        {
                                            layout->controls[i].rec.x += layout->controls[i].ap->x;
                                            layout->controls[i].rec.y += layout->controls[i].ap->y;
                                        }
                                        layout->controls[i].ap = &layout->anchors[selectedAnchor];
                                        layout->controls[i].rec.x -= layout->anchors[selectedAnchor].x;
                                        layout->controls[i].rec.y -= layout->anchors[selectedAnchor].y;
                                    }
                                }
                                anchorMoveMode = false;
                            }

                            // Move anchor without moving controls
                            if (!anchorMoveMode && anchorEditMode)
                            {
                                anchorMoveMode = true;

                                for (int i = 0; i < layout->controlCount; i++)
                                {
                                    if (layout->controls[i].ap->id == selectedAnchor)
                                    {
                                        layout->controls[i].rec.x += layout->controls[i].ap->x;
                                        layout->controls[i].rec.y += layout->controls[i].ap->y;
                                        layout->controls[i].ap = &auxAnchor;
                                    }
                                }
                            }
                            layout->anchors[selectedAnchor].x = mouse.x;
                            layout->anchors[selectedAnchor].y = mouse.y;

                            if (selectedAnchor == 0)
                            {
                                anchorEditMode = false;
                                layout->refWindow = (Rectangle){layout->anchors[0].x, layout->anchors[0].y, layout->refWindow.width, layout->refWindow.height};
                            }

                            // Exit anchor position edit mode
                            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                            {
                                // If moving only the anchor, relink with controls
                                if (anchorMoveMode)
                                {
                                    for (int i = 0; i < layout->controlCount; i++)
                                    {
                                        if (layout->controls[i].ap->id == 9) //auxAnchor ID
                                        {
                                            layout->controls[i].rec.x += layout->controls[i].ap->x;
                                            layout->controls[i].rec.y += layout->controls[i].ap->y;
                                            layout->controls[i].ap = &layout->anchors[selectedAnchor];
                                            layout->controls[i].rec.x -= layout->anchors[selectedAnchor].x;
                                            layout->controls[i].rec.y -= layout->anchors[selectedAnchor].y;
                                        }

                                    }
                                    anchorMoveMode = false;
                                }
                                dragMoveMode = false;
                            }
                        }
                        else
                        {
                            if (resizeMode)     // Anchor cannot resize
                            {
                                if (IsKeyPressed(KEY_D)) // Duplicate anchor
                                {
                                    if (layout->anchorCount < MAX_ANCHOR_POINTS)
                                    {
                                        layout->anchorCount++;
                                        for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
                                        {
                                            if (!layout->anchors[i].enabled && i != selectedAnchor)
                                            {
                                                layout->anchors[i].x = layout->anchors[selectedAnchor].x + 10;
                                                layout->anchors[i].y = layout->anchors[selectedAnchor].y + 10;
                                                layout->anchors[i].enabled = true;
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
                                int offsetX = (int)layout->anchors[selectedAnchor].x%gridSnapDelta;
                                int offsetY = (int)layout->anchors[selectedAnchor].y%gridSnapDelta;

                                // Move anchor with arrows once
                                if (precisionMode)
                                {
                                    if (IsKeyPressed(KEY_RIGHT)) layout->anchors[selectedAnchor].x+= (gridSnapDelta - offsetX);
                                    else if (IsKeyPressed(KEY_LEFT))
                                    {
                                        if (offsetX == 0) offsetX = gridSnapDelta;
                                        layout->anchors[selectedAnchor].x-= offsetX;
                                    }

                                    if (IsKeyPressed(KEY_DOWN)) layout->anchors[selectedAnchor].y+= (gridSnapDelta - offsetY);
                                    else if (IsKeyPressed(KEY_UP))
                                    {
                                        if (offsetY == 0) offsetY = gridSnapDelta;
                                        layout->anchors[selectedAnchor].y-= offsetY;
                                    }

                                    moveFrameCounter = 0;
                                }
                                else        // Move anchor with arrows
                                {
                                    moveFrameCounter++;

                                    if ((moveFrameCounter%moveFrameSpeed) == 0)
                                    {
                                        if (IsKeyDown(KEY_RIGHT)) layout->anchors[selectedAnchor].x += (gridSnapDelta - offsetX);
                                        else if (IsKeyDown(KEY_LEFT))
                                        {
                                            if (offsetX == 0) offsetX = gridSnapDelta;
                                            layout->anchors[selectedAnchor].x -= offsetX;
                                        }


                                        if (IsKeyDown(KEY_DOWN)) layout->anchors[selectedAnchor].y +=(gridSnapDelta - offsetY);
                                        else if (IsKeyDown(KEY_UP))
                                        {
                                            if (offsetY == 0) offsetY = gridSnapDelta;
                                            layout->anchors[selectedAnchor].y -= offsetY;
                                        }

                                        moveFrameCounter = 0;
                                    }
                                }

                                if (selectedAnchor == 0) layout->refWindow = (Rectangle){layout->anchors[0].x, layout->anchors[0].y, layout->refWindow.width, layout->refWindow.height};

                                // Activate anchor position edit mode
                                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                                {
                                    if (selectedAnchor == 0 && anchorEditMode) refWindowEditMode = true;
                                    else dragMoveMode = true;
                                }
                                else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) anchorLinkMode = true;  // Activate anchor link mode
                                else if (IsKeyPressed(KEY_H)) layout->anchors[selectedAnchor].hidding = !layout->anchors[selectedAnchor].hidding;   // Hide/Unhide anchors
                                else if (IsKeyPressed(KEY_U) && selectedAnchor > 0)                     // Unlinks controls from selected anchor
                                {
                                    for (int i = 0; i < layout->controlCount; i++)
                                    {
                                        if (layout->controls[i].ap->id == selectedAnchor)
                                        {
                                            layout->controls[i].rec.x += layout->controls[i].ap->x;
                                            layout->controls[i].rec.y += layout->controls[i].ap->y;
                                            layout->controls[i].ap = &layout->anchors[0];
                                        }
                                    }
                                }
                                else if (IsKeyPressed(KEY_DELETE))      // Delete anchor
                                {
                                    if (selectedAnchor == 0)
                                    {
                                        layout->anchors[selectedAnchor].x = 0;
                                        layout->anchors[selectedAnchor].y = 0;
                                        layout->refWindow = (Rectangle){ 0, 0, -1, -1 };
                                    }
                                    else
                                    {
                                        for (int i = 0; i < layout->controlCount; i++)
                                        {
                                            if (layout->controls[i].ap->id == selectedAnchor)
                                            {
                                                layout->controls[i].rec.x += layout->controls[i].ap->x;
                                                layout->controls[i].rec.y += layout->controls[i].ap->y;
                                                layout->controls[i].ap = &layout->anchors[0];
                                            }
                                        }

                                        layout->anchors[selectedAnchor].x = 0;
                                        layout->anchors[selectedAnchor].y = 0;
                                        layout->anchors[selectedAnchor].enabled = false;
                                        layout->anchors[selectedAnchor].hidding = false;

                                        layout->anchorCount--;
                                    }
                                    selectedAnchor = -1;
                                    focusedAnchor = -1;
                                }
                                else if (IsKeyReleased(KEY_N))          // Enable name edit mode
                                {
                                    nameEditMode = true;
                                    strcpy(prevName, layout->anchors[selectedAnchor].name);
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
                            if (layout->controls[focusedControl].ap->id > 0)
                            {
                                layout->controls[focusedControl].rec.x += layout->controls[focusedControl].ap->x;
                                layout->controls[focusedControl].rec.y += layout->controls[focusedControl].ap->y;
                            }

                            layout->controls[focusedControl].ap = &layout->anchors[selectedAnchor];

                            if (selectedAnchor> 0)
                            {
                                layout->controls[focusedControl].rec.x -= layout->anchors[selectedAnchor].x;
                                layout->controls[focusedControl].rec.y -= layout->anchors[selectedAnchor].y;
                            }
                        }
                        //selectedAnchor = -1;
                    }
                }
            }

            if (anchorLinkMode && (selectedAnchor == -1) && IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) anchorLinkMode = false;
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
                    if (dragMoveMode)
                    {
                        int offsetX = (int)mouse.x%gridSpacing;
                        int offsetY = (int)mouse.y%gridSpacing;

                        tracemapRec.x = prevPosition.x + (mouse.x - panOffset.x);
                        tracemapRec.y = prevPosition.y + (mouse.y - panOffset.y);

                        if (snapMode)
                        {
                            if (offsetX >= gridSpacing/2) mouse.x += (gridSpacing - offsetX);
                            else mouse.x -= offsetX;

                            if (offsetY >= gridSpacing/2) mouse.y += (gridSpacing - offsetY);
                            else mouse.y -= offsetY;

                            offsetX = (int)tracemapRec.x%gridSpacing;
                            offsetY = (int)tracemapRec.y%gridSpacing;

                            if (offsetX >= gridSpacing/2) tracemapRec.x += (gridSpacing - offsetX);
                            else tracemapRec.x -= offsetX;

                            if (offsetY >= gridSpacing/2) tracemapRec.y += (gridSpacing - offsetY);
                            else tracemapRec.y -= offsetY;
                        }

                        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragMoveMode = false;
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
                                    tracemapRec.height += gridSnapDelta;
                                    tracemapRec.width += gridSnapDelta;
                                }
                                else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_UP))
                                {
                                    tracemapRec.height -= gridSnapDelta;
                                    tracemapRec.width -= gridSnapDelta;
                                }

                                moveFrameCounter = 0;
                            }
                            else
                            {
                                moveFrameCounter++;

                                if ((moveFrameCounter%moveFrameSpeed) == 0)
                                {
                                    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_DOWN))
                                    {
                                        tracemapRec.height += gridSnapDelta;
                                        tracemapRec.width += gridSnapDelta;
                                    }
                                    else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_UP))
                                    {
                                        tracemapRec.height -= gridSnapDelta;
                                        tracemapRec.width -= gridSnapDelta;
                                    }

                                    moveFrameCounter = 0;
                                }
                            }

                            //tracemap.height = tracemapRec.height;
                            //tracemap.width = tracemapRec.width;
                        }
                        else
                        {
                            // Move map with arrows
                            int offsetX = (int)tracemapRec.x%gridSnapDelta;
                            int offsetY = (int)tracemapRec.y%gridSnapDelta;

                            if (precisionMode)
                            {
                                if (IsKeyPressed(KEY_RIGHT))  tracemapRec.x += (gridSnapDelta - offsetX);
                                else if (IsKeyPressed(KEY_LEFT))
                                {
                                    if (offsetX == 0) offsetX = gridSnapDelta;
                                    tracemapRec.x -= offsetX;
                                }

                                if (IsKeyPressed(KEY_DOWN)) tracemapRec.y += (gridSnapDelta - offsetY);
                                else if (IsKeyPressed(KEY_UP))
                                {
                                    if (offsetY == 0) offsetY = gridSnapDelta;
                                    tracemapRec.y -= offsetY;
                                }

                                moveFrameCounter = 0;
                            }
                            else
                            {
                                moveFrameCounter++;

                                if ((moveFrameCounter%moveFrameSpeed) == 0)
                                {
                                    if (IsKeyDown(KEY_RIGHT)) tracemapRec.x += (gridSnapDelta - offsetX);
                                    else if (IsKeyDown(KEY_LEFT))
                                    {
                                        if (offsetX == 0) offsetX = gridSnapDelta;
                                        tracemapRec.x -= offsetX;
                                    }

                                    if (IsKeyDown(KEY_DOWN)) tracemapRec.y += (gridSnapDelta - offsetY);
                                    else if (IsKeyDown(KEY_UP))
                                    {
                                        if (offsetY == 0) offsetY = gridSnapDelta;
                                        tracemapRec.y -= offsetY;
                                    }

                                    moveFrameCounter = 0;
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

                            // Enable dragMoveMode mode
                            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                            {
                                panOffset = mouse;
                                prevPosition = (Vector2){ tracemapRec.x, tracemapRec.y };

                                dragMoveMode = true;
                            }
                        }
                    }
                }
            }
            else
            {
                tracemapFocused = false;
                tracemapSelected = false;

                //if (CheckCollisionPointRec(mouse, tracemapRec) && focusedControl == -1 && focusedAnchor == -1) tracemapFocused = true;
            }
            //----------------------------------------------------------------------------------------------
        }

        // If any window is shown, cancel any edition mode
        if (windowAboutState.windowActive ||
            windowCodegenState.windowCodegenActive ||
            windowResetActive ||
            windowExitActive ||
            showLoadFileDialog || 
            showSaveFileDialog || 
            showExportFileDialog)
        {
            nameEditMode = false;
            textEditMode = false;
            resizeMode = false;
            dragMoveMode = false;
            precisionMode = false;

            windowOverActive = true;        // There is some window overlap!
        }
        else windowOverActive = false;
        //----------------------------------------------------------------------------------------------

        // Reset program logic
        //----------------------------------------------------------------------------------------------
        if (resetProgram)
        {
            focusedAnchor = -1;
            selectedAnchor = -1;
            focusedControl = -1;
            selectedControl = -1;

            resizeMode = false;
            dragMoveMode = false;
            precisionMode = false;
            nameEditMode = false;
            textEditMode = false;
            showIconPanel = false;

            ResetLayout(layout);

            strcpy(inFileName, "\0");
            SetWindowTitle(TextFormat("%s v%s", toolName, toolVersion));

            for (int i = 0; i < MAX_UNDO_LEVELS; i++) memcpy(&undoLayouts[i], layout, sizeof(GuiLayout));
            currentUndoIndex = 0;
            firstUndoIndex = 0;

            resetProgram = false;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

            // WARNING: Some windows should lock the main screen controls when shown
            if (windowOverActive) GuiLock();
            else GuiUnlock();

            // Draw background grid
            if (showGrid) GuiGrid((Rectangle){ 0, workAreaOffsetY, GetScreenWidth(), GetScreenHeight() }, NULL, gridSpacing*gridSubdivisions, gridSubdivisions);

            // Draw the tracemap texture if loaded
            //---------------------------------------------------------------------------------
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
                        if (!dragMoveMode && resizeMode) tracemapColor = BLUE;
                        DrawRectangleRec(tracemapRec, Fade(tracemapColor, 0.5f));

                        positionColor = MAROON;
                        if (useGlobalPos) positionColor = RED;
                        if (snapMode) positionColor = LIME;
                        if (!dragMoveMode && precisionMode) positionColor = BLUE;
                        DrawText(TextFormat("[%i, %i, %i, %i]",
                                            (int)tracemapRec.x,
                                            (int)tracemapRec.y,
                                            (int)tracemapRec.width,
                                            (int)tracemapRec.height), tracemapRec.x, tracemapRec.y - 20, 20, positionColor);
                    }
                }
            }
            //---------------------------------------------------------------------------------

            // Draw reference window edit mode
            if ((layout->refWindow.width > 0) && (layout->refWindow.height > 0))
            {
                if (refWindowEditMode)
                {
                    DrawRectangleRec(layout->refWindow, Fade(BLACK, 0.1f));

                    DrawText(TextFormat("[%i]", (int)layout->refWindow.width),
                             layout->refWindow.x + layout->refWindow.width - MeasureText(TextFormat("[%i]",(int)layout->refWindow.width), 20) - 5,
                             layout->refWindow.y + layout->refWindow.height + 5, 20, positionColor);
                    DrawText(TextFormat("[%i]", (int)layout->refWindow.height),
                             layout->refWindow.x + layout->refWindow.width + 5,
                             layout->refWindow.y + layout->refWindow.height - 20, 20, positionColor);
                }
            }

            // Draw the controls placed on the grid
            // TODO: Support controls editMode vs playMode, just unlock controls and lock edit!
            //----------------------------------------------------------------------------------------
            GuiLock();
            for (int i = 0; i < layout->controlCount; i++)
            {
                if (!layout->controls[i].ap->hidding)
                {
                    Vector2 anchorOffset = (Vector2){ 0, 0 };
                    if (layout->controls[i].ap->id > 0) anchorOffset = (Vector2){ layout->controls[i].ap->x, layout->controls[i].ap->y };
                    Rectangle rec = { anchorOffset.x + layout->controls[i].rec.x, anchorOffset.y + layout->controls[i].rec.y, layout->controls[i].rec.width, layout->controls[i].rec.height };

                    switch (layout->controls[i].type)
                    {
                        case GUI_WINDOWBOX:
                        {
                            GuiFade(0.8f);
                            GuiWindowBox(rec, layout->controls[i].text);
                            GuiFade(1.0f);
                        } break;
                        case GUI_GROUPBOX: GuiGroupBox(rec, layout->controls[i].text); break;
                        case GUI_LINE: 
                        {
                            if (layout->controls[i].text[0] == '\0') GuiLine(rec, NULL);
                            else GuiLine(rec, layout->controls[i].text);
                        } break;
                        case GUI_PANEL:
                        {
                            GuiFade(0.8f);
                            GuiPanel(rec, layout->controls[i].text);
                            GuiFade(1.0f);
                        } break;
                        case GUI_LABEL: GuiLabel(rec, layout->controls[i].text); break;
                        case GUI_BUTTON: GuiButton(rec, layout->controls[i].text); break;
                        case GUI_LABELBUTTON: GuiLabelButton(rec, layout->controls[i].text); break;
                        case GUI_CHECKBOX: GuiCheckBox(rec, layout->controls[i].text, false); break;
                        case GUI_TOGGLE: GuiToggle(rec, layout->controls[i].text, false); break;
                        case GUI_TOGGLEGROUP: GuiToggleGroup(rec, layout->controls[i].text, 1); break;
                        case GUI_COMBOBOX: GuiComboBox(rec, layout->controls[i].text, 1); break;
                        case GUI_DROPDOWNBOX: GuiDropdownBox(rec, layout->controls[i].text, &dropdownBoxActive, false); break;
                        case GUI_TEXTBOX: GuiTextBox(rec, layout->controls[i].text, MAX_CONTROL_TEXT_LENGTH, false); break;
                        case GUI_TEXTBOXMULTI: GuiTextBoxMulti(rec, layout->controls[i].text, MAX_CONTROL_TEXT_LENGTH, false); break;
                        case GUI_VALUEBOX: GuiValueBox(rec, layout->controls[i].text, &valueBoxValue, 42, 100, false); break;
                        case GUI_SPINNER: GuiSpinner(rec, layout->controls[i].text, &spinnerValue, 42, 3, false); break;
                        case GUI_SLIDER: GuiSlider(rec, layout->controls[i].text, NULL, 42, 0, 100); break;
                        case GUI_SLIDERBAR: GuiSliderBar(rec, layout->controls[i].text, NULL, 40, 0, 100); break;
                        case GUI_PROGRESSBAR: GuiProgressBar(rec, layout->controls[i].text, NULL, 40, 0, 100); break;
                        case GUI_STATUSBAR: GuiStatusBar(rec, layout->controls[i].text); break;
                        case GUI_SCROLLPANEL: GuiScrollPanel(rec, layout->controls[i].text, rec, NULL); break;
                        case GUI_LISTVIEW: GuiListView(rec, layout->controls[i].text, &listViewScrollIndex, listViewActive); break;
                        case GUI_COLORPICKER: GuiColorPicker(rec, layout->controls[i].text, RED); break;
                        case GUI_DUMMYREC: GuiDummyRec(rec, layout->controls[i].text); break;
                        default: break;
                    }
                }
            }
            if (!windowOverActive) GuiUnlock();
            //----------------------------------------------------------------------------------------

            // Draw reference window
            //----------------------------------------------------------------------------------------
            anchorSelectedColor = DARKGRAY;
            anchorCircleColor = DARKGRAY;
            if (selectedAnchor == 0)
            {
                if (anchorEditMode) { anchorSelectedColor = ORANGE; anchorCircleColor = ORANGE;}
                DrawRectangle(layout->anchors[0].x - ANCHOR_RADIUS, layout->anchors[0].y - ANCHOR_RADIUS, ANCHOR_RADIUS*2, ANCHOR_RADIUS*2, Fade(anchorSelectedColor, 0.2f));
            }
            if (focusedAnchor == 0)
            {
                anchorCircleColor = BLACK;
                if (anchorEditMode) anchorCircleColor = ORANGE;
            }

            DrawRectangleLines(layout->anchors[0].x - ANCHOR_RADIUS, layout->anchors[0].y - ANCHOR_RADIUS, ANCHOR_RADIUS*2, ANCHOR_RADIUS*2, Fade(anchorCircleColor, 0.5f));
            DrawRectangle(layout->anchors[0].x - ANCHOR_RADIUS - 5, layout->anchors[0].y, ANCHOR_RADIUS*2 + 10, 1, Fade(anchorCircleColor, 0.8f));
            DrawRectangle(layout->anchors[0].x, layout->anchors[0].y - ANCHOR_RADIUS - 5, 1, ANCHOR_RADIUS*2 + 10, Fade(anchorCircleColor, 0.8f));
            //----------------------------------------------------------------------------------------

            // Draw the anchor points
            //----------------------------------------------------------------------------------------
            for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
            {
                if (layout->anchors[i].enabled)
                {
                    if (selectedAnchor > 0 && layout->anchors[i].id == selectedAnchor)
                    {
                        if (layout->anchors[i].hidding)
                        {
                            anchorCircleColor = GRAY;
                            anchorSelectedColor = GRAY;
                        }
                        else
                        {
                            anchorCircleColor = (Color){ 253, 86, 95, 255 }; // LIGHTRED
                            anchorSelectedColor = RED;
                        }
                        if (anchorMoveMode || (anchorEditMode && (focusedAnchor > 0) && (layout->anchors[i].id == focusedAnchor))) anchorSelectedColor = ORANGE;
                        DrawCircle(layout->anchors[i].x, layout->anchors[i].y, ANCHOR_RADIUS, Fade(anchorSelectedColor, 0.2f));
                    }
                    else if (layout->anchors[i].hidding) anchorCircleColor = GRAY;
                    else anchorCircleColor = BLUE;
                    if (focusedAnchor > 0 && layout->anchors[i].id == focusedAnchor)
                    {
                        if (anchorEditMode) anchorCircleColor = ORANGE;
                        else anchorCircleColor = RED;
                    }

                    DrawCircleLines(layout->anchors[i].x, layout->anchors[i].y, ANCHOR_RADIUS, Fade(anchorCircleColor, 0.5f));
                    DrawRectangle(layout->anchors[i].x - ANCHOR_RADIUS - 5, layout->anchors[i].y, ANCHOR_RADIUS*2 + 10, 1, anchorCircleColor);
                    DrawRectangle(layout->anchors[i].x, layout->anchors[i].y - ANCHOR_RADIUS - 5, 1, ANCHOR_RADIUS*2 + 10, anchorCircleColor);
                }
            }
            //----------------------------------------------------------------------------------------

            if (!GuiIsLocked())
            {
                if (!(CheckCollisionPointRec(mouse, paletteState.scrollPanelBounds)))
                {
                    if ((focusedAnchor == -1) && (focusedControl == -1) && !tracemapFocused && !refWindowEditMode)
                    {
                        if (!anchorEditMode)
                        {
                            if (!anchorLinkMode && (selectedAnchor == -1) && (selectedControl == -1) && !tracemapSelected)
                            {
                                // Draw the default rectangle of the control selected
                                GuiLock();
                                GuiFade(0.5f);

                                switch (selectedType)
                                {
                                    case GUI_WINDOWBOX: GuiWindowBox(defaultRec[selectedType], "WINDOW BOX"); break;
                                    case GUI_GROUPBOX: GuiGroupBox(defaultRec[selectedType], "GROUP BOX"); break;
                                    case GUI_LINE: GuiLine(defaultRec[selectedType], NULL); break;
                                    case GUI_PANEL: GuiPanel(defaultRec[selectedType], NULL); break;
                                    case GUI_LABEL: GuiLabel(defaultRec[selectedType], "LABEL TEXT"); break;
                                    case GUI_BUTTON: GuiButton(defaultRec[selectedType], "BUTTON"); break;
                                    case GUI_LABELBUTTON: GuiLabelButton(defaultRec[selectedType], "LABEL_BUTTON"); break;
                                    case GUI_CHECKBOX: GuiCheckBox(defaultRec[selectedType], "CHECK BOX", false); break;
                                    case GUI_TOGGLE: GuiToggle(defaultRec[selectedType], "TOGGLE", false); break;
                                    // WARNING: Selection rectangle for GuiToggleGroup() considers all the control while the function expects only one piece!
                                    // TODO: Review the rectangle defined for ToggleGroup() or review the control itself to receive the complete bounds with (?)
                                    case GUI_TOGGLEGROUP: GuiToggleGroup((Rectangle) { defaultRec[selectedType].x, defaultRec[selectedType].y, defaultRec[selectedType].width/3, defaultRec[selectedType].height }, "ONE;TWO;THREE", 1); break;
                                    case GUI_COMBOBOX: GuiComboBox(defaultRec[selectedType], "ONE;TWO;THREE", 1); break;
                                    case GUI_DROPDOWNBOX: GuiDropdownBox(defaultRec[selectedType], "ONE;TWO;THREE", &dropdownBoxActive, false); break;
                                    case GUI_TEXTBOX: GuiTextBox(defaultRec[selectedType], "TEXT BOX", 7, false); break;
                                    case GUI_TEXTBOXMULTI: GuiTextBoxMulti(defaultRec[selectedType], "TEXT BOX MULTI", 7, false); break;
                                    case GUI_VALUEBOX: GuiValueBox(defaultRec[selectedType], "VALUE BOX", &valueBoxValue, 42, 100, false); break;
                                    case GUI_SPINNER: GuiSpinner(defaultRec[selectedType], "SPINNER", &spinnerValue, 42, 3, false); break;
                                    case GUI_SLIDER: GuiSlider(defaultRec[selectedType], "SLIDER", NULL, 42, 0, 100); break;
                                    case GUI_SLIDERBAR: GuiSliderBar(defaultRec[selectedType], "SLIDER BAR", NULL, 40, 0, 100); break;
                                    case GUI_PROGRESSBAR: GuiProgressBar(defaultRec[selectedType], "PROGRESS BAR", NULL, 40, 0, 100); break;
                                    case GUI_STATUSBAR: GuiStatusBar(defaultRec[selectedType], "STATUS BAR"); break;
                                    case GUI_SCROLLPANEL: GuiScrollPanel(defaultRec[selectedType], NULL, defaultRec[selectedType], NULL); break;
                                    case GUI_LISTVIEW: GuiListView(defaultRec[selectedType], "ONE;TWO;THREE;FOUR", &listViewScrollIndex, listViewActive); break;
                                    case GUI_COLORPICKER: GuiColorPicker(defaultRec[selectedType], NULL, RED); break;
                                    case GUI_DUMMYREC: GuiDummyRec(defaultRec[selectedType], "DUMMY REC"); break;
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
                                DrawText(TextFormat("[%i, %i, %i, %i]", (int)defaultRec[selectedType].x, (int)defaultRec[selectedType].y,
                                    (int)defaultRec[selectedType].width, (int)defaultRec[selectedType].height),
                                    (int)defaultRec[selectedType].x, (int)defaultRec[selectedType].y - 30, 20, Fade(positionColor, 0.5f));

                                // Draw controls name
                                if (IsKeyDown(KEY_N))
                                {
                                    GuiLock();
                                    for (int i = 0; i < layout->controlCount; i++)
                                    {
                                        Rectangle textboxRec = layout->controls[i].rec;
                                        int type = layout->controls[i].type;

                                        // NOTE: Depending on control type, name is drawn in different position
                                        if ((type == GUI_CHECKBOX) || (type == GUI_LABEL) || (type == GUI_SLIDER) || (type == GUI_SLIDERBAR))
                                        {
                                            int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
                                            int textWidth = MeasureText(layout->controls[i].name, fontSize);
                                            if (textboxRec.width < textWidth + 20) textboxRec.width = textWidth + 20;
                                            if (textboxRec.height < fontSize) textboxRec.height += fontSize;
                                        }

                                        if (type == GUI_WINDOWBOX) textboxRec.height = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT;  // Defined inside raygui.h!
                                        else if (type == GUI_GROUPBOX)
                                        {
                                            textboxRec.y -= 10;
                                            textboxRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE)*2;
                                        }

                                        if (layout->controls[i].ap->id > 0)
                                        {
                                            textboxRec.x += layout->controls[i].ap->x;
                                            textboxRec.y += layout->controls[i].ap->y;
                                        }

                                        DrawRectangleRec(textboxRec, WHITE);
                                        GuiTextBox(textboxRec, layout->controls[i].name, MAX_CONTROL_NAME_LENGTH, false);
                                    }

                                    for (int i = 0; i < layout->anchorCount; i++)
                                    {
                                        Rectangle textboxRec = (Rectangle){ layout->anchors[i].x, layout->anchors[i].y,
                                                                            MeasureText(layout->anchors[i].name, GuiGetStyle(DEFAULT, TEXT_SIZE)) + 10, GuiGetStyle(DEFAULT, TEXT_SIZE) + 5 };

                                        DrawRectangleRec(textboxRec, WHITE);
                                        DrawRectangleRec(textboxRec, Fade(ORANGE, 0.1f));
                                        GuiTextBox(textboxRec, layout->anchors[i].name, MAX_ANCHOR_NAME_LENGTH, false);
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
                            DrawRectangle(mouse.x, mouse.y - ANCHOR_RADIUS - 5, 1, ANCHOR_RADIUS*2 + 10, RED);
                        }
                    }
                }

                // Draw focused anchor selector
                if (focusedAnchor != 1)
                {
                    // Anchor links
                    for (int i = 0; i < layout->controlCount; i++)
                    {
                        if (layout->controls[i].ap->id == focusedAnchor)
                        {
                            if (focusedAnchor == 0) DrawLine(layout->controls[i].ap->x, layout->controls[i].ap->y, layout->controls[i].rec.x, layout->controls[i].rec.y, LIGHTGRAY);
                            else if (!layout->controls[i].ap->hidding)
                            {
                                DrawLine(layout->controls[i].ap->x, layout->controls[i].ap->y, layout->controls[i].ap->x + layout->controls[i].rec.x, layout->controls[i].ap->y + layout->controls[i].rec.y, RED);
                            }
                            else DrawLine(layout->controls[i].ap->x, layout->controls[i].ap->y, layout->controls[i].ap->x + layout->controls[i].rec.x, layout->controls[i].ap->y + layout->controls[i].rec.y, GRAY);
                        }
                    }
                }

                // Draw selected anchor
                if (selectedAnchor != -1)
                {
                    // Draw anchor coordinates
                    positionColor = anchorSelectedColor;
                    if (snapMode) positionColor = LIME;
                    if (!dragMoveMode && precisionMode) positionColor = BLUE;

                    if (selectedAnchor > 0)
                    {
                        DrawText(TextFormat("[%i, %i]",
                            (int)(layout->anchors[selectedAnchor].x - layout->refWindow.x),
                            (int)(layout->anchors[selectedAnchor].y - layout->refWindow.y)),
                            layout->anchors[selectedAnchor].x + ANCHOR_RADIUS,
                            layout->anchors[selectedAnchor].y - 38, 20, positionColor);
                    }
                    else
                    {
                        if ((layout->refWindow.width > 0) && (layout->refWindow.height > 0))
                        {
                            DrawText(TextFormat("[%i, %i, %i, %i]",
                                (int)layout->refWindow.x,
                                (int)layout->refWindow.y,
                                (int)layout->refWindow.width,
                                (int)layout->refWindow.height),
                                layout->anchors[selectedAnchor].x + ANCHOR_RADIUS,
                                layout->anchors[selectedAnchor].y - 38, 20, positionColor);
                        }
                        else DrawText(TextFormat("[%i, %i]", 
                                (int)(layout->refWindow.x), 
                                (int)(layout->refWindow.y)), 
                                layout->anchors[selectedAnchor].x + ANCHOR_RADIUS, 
                                layout->anchors[selectedAnchor].y - 38, 20, positionColor);
                    }

                    // Draw anchor links
                    for (int i = 0; i < layout->controlCount; i++)
                    {
                        //if (!layout->controls[i].ap->hidding)
                        if (layout->controls[i].ap->id == selectedAnchor)
                        {
                            if (selectedAnchor == 0) DrawLine(layout->controls[i].ap->x, layout->controls[i].ap->y, layout->controls[i].rec.x, layout->controls[i].rec.y, LIGHTGRAY);
                            else if (!layout->controls[i].ap->hidding)
                            {
                                DrawLine(layout->controls[i].ap->x, layout->controls[i].ap->y, layout->controls[i].ap->x + layout->controls[i].rec.x, layout->controls[i].ap->y + layout->controls[i].rec.y, RED);
                            }
                            else DrawLine(layout->controls[i].ap->x, layout->controls[i].ap->y, layout->controls[i].ap->x + layout->controls[i].rec.x, layout->controls[i].ap->y + layout->controls[i].rec.y, GRAY);
                        }
                    }

                    // Draw anchor link mode
                    if (anchorLinkMode)
                    {
                        if (selectedAnchor == 0) DrawLine(layout->anchors[selectedAnchor].x, layout->anchors[selectedAnchor].y, mouse.x, mouse.y, BLACK);
                        else DrawLine(layout->anchors[selectedAnchor].x, layout->anchors[selectedAnchor].y, mouse.x, mouse.y, RED);
                    }

                    // Draw name edit mode
                    if (nameEditMode)
                    {
                        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(ORANGE, 0.2f));
                        DrawText("Anchor name edit mode", 20, 20, 20, DARKGRAY);

                        int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
                        int textWidth = MeasureText(layout->anchors[selectedAnchor].name, fontSize);
                        Rectangle textboxRec = (Rectangle){ layout->anchors[selectedAnchor].x, layout->anchors[selectedAnchor].y, textWidth + 40, fontSize + 5 };

                        if (textboxRec.width < (textWidth + 40)) textboxRec.width = textWidth + 40;
                        if (textboxRec.height < fontSize) textboxRec.height += fontSize;

                        if (GuiTextBox(textboxRec, layout->anchors[selectedAnchor].name, MAX_ANCHOR_NAME_LENGTH, nameEditMode)) nameEditMode = !nameEditMode;
                    }
                }

                // Draw selected control from palette
                if (selectedControl != -1)
                {
                    // Selection rectangle
                    selectedControlColor = RED;
                    if (!dragMoveMode && resizeMode) selectedControlColor = BLUE;

                    Rectangle selectedRec = layout->controls[selectedControl].rec;
                    if (layout->controls[selectedControl].type == GUI_WINDOWBOX) selectedRec.height = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT;  // Defined inside raygui.h!
                    else if (layout->controls[selectedControl].type == GUI_GROUPBOX)
                    {
                        selectedRec.y -= 10;
                        selectedRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE)*2.0f;
                    }

                    if (layout->controls[selectedControl].ap->id > 0)
                    {
                        selectedRec.x += layout->controls[selectedControl].ap->x;
                        selectedRec.y += layout->controls[selectedControl].ap->y;
                    }

                    DrawRectangleRec(selectedRec, Fade(selectedControlColor, 0.3f));

                    // Control Link
                    if (layout->controls[selectedControl].ap->id > 0) DrawLine(layout->controls[selectedControl].ap->x, layout->controls[selectedControl].ap->y, selectedRec.x, selectedRec.y, RED);

                    // Linking
                    if (anchorLinkMode) DrawLine(selectedRec.x, selectedRec.y, mouse.x, mouse.y, RED);

                    // Control Coordinates
                    positionColor = MAROON;
                    if (useGlobalPos) positionColor = RED;
                    if (snapMode) positionColor = LIME;
                    if (!dragMoveMode && precisionMode) positionColor = BLUE;

                    if (!useGlobalPos)
                    {
                        if (layout->controls[selectedControl].ap->id > 0)
                        {
                            DrawText(TextFormat("[%i, %i, %i, %i]",
                                (int)(layout->controls[selectedControl].rec.x),
                                (int)(layout->controls[selectedControl].rec.y),
                                (int)layout->controls[selectedControl].rec.width,
                                (int)layout->controls[selectedControl].rec.height),
                                selectedRec.x, selectedRec.y - 30, 20, positionColor);
                        }
                        else DrawText(TextFormat("[%i, %i, %i, %i]",
                            (int)(selectedRec.x - layout->refWindow.x),
                            (int)(selectedRec.y - layout->refWindow.y),
                            (int)layout->controls[selectedControl].rec.width,
                            (int)layout->controls[selectedControl].rec.height),
                            selectedRec.x, selectedRec.y - 30, 20, positionColor);
                    }
                    else
                    {
                        DrawText(TextFormat("[%i, %i, %i, %i]",
                            (int)(selectedRec.x - layout->refWindow.x),
                            (int)(selectedRec.y - layout->refWindow.y),
                            (int)layout->controls[selectedControl].rec.width,
                            (int)layout->controls[selectedControl].rec.height),
                            selectedRec.x, selectedRec.y - 30, 20, positionColor);
                    }

                    // Text edit
                    if (textEditMode || showIconPanel)
                    {
                        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(SKYBLUE, 0.2f));
                        DrawText("Control text edit mode", 20, 25, 20, DARKGRAY);

                        Rectangle textboxRec = layout->controls[selectedControl].rec;

                        // Make sure text could be written, no matter if overflows control
                        int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
                        int textWidth = MeasureText(layout->controls[selectedControl].text, fontSize);
                        if (textboxRec.width < (textWidth + 40)) textboxRec.width = textWidth + 40;     // TODO: Why additional space required to work with GuiTextBox()?
                        if (textboxRec.height < fontSize) textboxRec.height += fontSize;

                        if (layout->controls[selectedControl].type == GUI_WINDOWBOX) textboxRec.height = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT;  // Defined inside raygui.h
                        else if (layout->controls[selectedControl].type == GUI_GROUPBOX)
                        {
                            textboxRec.y -= 10;
                            textboxRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE)*2.0f;
                        }

                        if (layout->controls[selectedControl].ap->id > 0)
                        {
                            textboxRec.x += layout->controls[selectedControl].ap->x;
                            textboxRec.y += layout->controls[selectedControl].ap->y;
                        }

                        // Draw a GuiTextBox()/GuiTextBoxMulti() for text edition
                        if (layout->controls[selectedControl].type == GUI_TEXTBOXMULTI)
                        {
                            if (GuiTextBoxMulti(textboxRec, layout->controls[selectedControl].text, MAX_CONTROL_TEXT_LENGTH, textEditMode)) textEditMode = !textEditMode;
                        }
                        else if (GuiTextBox(textboxRec, layout->controls[selectedControl].text, MAX_CONTROL_TEXT_LENGTH, textEditMode)) textEditMode = !textEditMode;

                        // Check if icon panel must be shown
                        if ((strlen(layout->controls[selectedControl].text) == 1) && (layout->controls[selectedControl].text[0] == '#'))
                        {
                            showIconPanel = true;

                            // Draw icons selector
                            selectedIcon = GuiToggleGroup((Rectangle) { (int)textboxRec.x, (int)textboxRec.y + (int)textboxRec.height + 10, 18, 18 }, toggleIconsText, selectedIcon);
                            if (selectedIcon > 0)
                            {
                                strcpy(layout->controls[selectedControl].text, TextFormat("#%03i#\0", selectedIcon));
                                showIconPanel = false;
                                textEditMode = true;
                                selectedIcon = 0;
                            }
                        }
                    }

                    // Name edit
                    if (nameEditMode)
                    {
                        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(GREEN, 0.2f));
                        DrawText("Control name edit mode", 20, 20, 20, DARKGRAY);

                        Rectangle textboxRec = layout->controls[selectedControl].rec;

                        int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
                        int textWidth = MeasureText(layout->controls[selectedControl].name, fontSize);
                        if (textboxRec.width < textWidth + 40) textboxRec.width = textWidth + 40;
                        if (textboxRec.height < fontSize) textboxRec.height += fontSize;

                        if (layout->controls[selectedControl].type == GUI_WINDOWBOX) textboxRec.height = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT;  // Defined inside raygui.h
                        else if (layout->controls[selectedControl].type == GUI_GROUPBOX)
                        {
                            textboxRec.y -= 10;
                            textboxRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE)*2.0f;
                        }

                        if (layout->controls[selectedControl].ap->id > 0)
                        {
                            textboxRec.x += layout->controls[selectedControl].ap->x;
                            textboxRec.y += layout->controls[selectedControl].ap->y;
                        }

                        if (GuiTextBox(textboxRec, layout->controls[selectedControl].name, MAX_CONTROL_NAME_LENGTH, nameEditMode)) nameEditMode = !nameEditMode;
                    }
                }
                //else //selectedControl == 1
                {
                    // Draw controls IDs for layout order edition
                    if (orderEditMode)
                    {
                        for (int i = layout->controlCount - 1; i >= 0; i--)
                        {
                            if (layout->controls[i].ap->id > 0)
                            {
                                DrawText(TextFormat("[%i]", layout->controls[i].id),
                                    layout->controls[i].rec.x + layout->controls[i].ap->x + layout->controls[i].rec.width,
                                    layout->controls[i].rec.y + layout->controls[i].ap->y - 10, 10, BLUE);
                            }
                            else DrawText(TextFormat("[%i]", layout->controls[i].id), layout->controls[i].rec.x + layout->controls[i].rec.width, layout->controls[i].rec.y - 10, 10, BLUE);
                        }
                    }
                }

                // Draw focused control selector
                if (focusedControl != -1)
                {
                    // Draw focused rectangle
                    Rectangle focusRec = layout->controls[focusedControl].rec;

                    if (layout->controls[focusedControl].type == GUI_WINDOWBOX) focusRec.height = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT;  // Defined inside raygui.h!
                    else if (layout->controls[focusedControl].type == GUI_GROUPBOX)
                    {
                        focusRec.y -= 10;
                        focusRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE)*2.0f;
                    }

                    if (layout->controls[focusedControl].ap->id > 0)
                    {
                        focusRec.x += layout->controls[focusedControl].ap->x;
                        focusRec.y += layout->controls[focusedControl].ap->y;
                    }

                    if (focusedControl != selectedControl) DrawRectangleRec(focusRec, Fade(RED, 0.1f));

                    DrawRectangleLinesEx(focusRec, 1, MAROON);

                    if (layout->controls[focusedControl].ap->id > 0) DrawLine(layout->controls[focusedControl].ap->x, layout->controls[focusedControl].ap->y, focusRec.x, focusRec.y, RED);
                }

                // Draw control scale mode selector
                if (mouseScaleReady && (selectedControl >= 0))
                {
                    Rectangle selectedRec = layout->controls[selectedControl].rec;

                    if (layout->controls[selectedControl].ap->id > 0)
                    {
                        selectedRec.x += layout->controls[selectedControl].ap->x;
                        selectedRec.y += layout->controls[selectedControl].ap->y;
                    }

                    DrawRectangleLinesEx(selectedRec, 2, RED);
                    DrawTriangle((Vector2) { selectedRec.x + selectedRec.width - SCALE_BOX_CORNER_SIZE, selectedRec.y + selectedRec.height },
                                 (Vector2) { selectedRec.x + selectedRec.width, selectedRec.y + selectedRec.height },
                                 (Vector2) { selectedRec.x + selectedRec.width, selectedRec.y + selectedRec.height - SCALE_BOX_CORNER_SIZE }, RED);
                }

                // Draw reference window lines
                if ((layout->refWindow.width > 0) && (layout->refWindow.height > 0)) DrawRectangleLinesEx(layout->refWindow, 1, Fade(BLACK, 0.7f));

                // GUI: Controls Selection Palette
                // NOTE: It uses GuiLock() to lock controls behaviour and just limit them to selection
                //----------------------------------------------------------------------------------------
                GuiControlsPalette(&paletteState);

                // Update ScrollPanel bounds in case window is resized
                paletteState.scrollPanelBounds = (Rectangle){ GetScreenWidth() - 160, workAreaOffsetY, 160, GetScreenHeight() - workAreaOffsetY };
                //----------------------------------------------------------------------------------------
            }

            // GUI: Help panel
            //----------------------------------------------------------------------------------------
            if (helpActive) GuiHelpPanel(20, 20, "[F1] Tool Shortcuts", helpLines, HELP_LINES_COUNT);
            //----------------------------------------------------------------------------------------

            // NOTE: If some overlap window is open and main window is locked, we draw a background rectangle
            if (GuiIsLocked()) DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), 0.85f));

            // WARNING: Before drawing the windows, we unlock them
            GuiUnlock();
            
            // GUI: Main toolbar panel (file and visualization)
            //----------------------------------------------------------------------------------
            //GuiMainToolbar(&mainToolbarState);
            //----------------------------------------------------------------------------------

            // GUI: Layout Code Generation Window
            //----------------------------------------------------------------------------------------
            GuiWindowCodegen(&windowCodegenState);

            if (windowCodegenState.generateCodePressed)
            {
                showExportFileDialog = true;
                windowCodegenState.windowCodegenActive = false;
                windowCodegenState.generateCodePressed = false;
            }
            //----------------------------------------------------------------------------------------

            // GUI: About Window
            //----------------------------------------------------------------------------------------
            windowAboutState.position = (Vector2){ GetScreenWidth()/2 - windowAboutState.windowWidth/2, GetScreenHeight()/2 - windowAboutState.windowHeight/2 };
            GuiWindowAbout(&windowAboutState);
            //----------------------------------------------------------------------------------------

            // GUI: New Layout Window (save)
            //----------------------------------------------------------------------------------------
            if (windowResetActive)
            {
                int message = GuiMessageBox((Rectangle) { GetScreenWidth()/2 - 120, GetScreenHeight()/2 - 48, 248, 96 }, "Creating new layout", "Do you want to save the current layout?", "Yes;No");

                if (message == 0) windowResetActive = false;
                else if (message == 1)  // Yes
                {
                    showSaveFileDialog = true;
                    resetProgram = true;
                    windowResetActive = false;
                }
                else if (message == 2)  // No
                {
                    resetProgram = true;
                    windowResetActive = false;
                }
            }
            //----------------------------------------------------------------------------------------

            // GUI: Exit Window
            //----------------------------------------------------------------------------------------
            if (windowExitActive)
            {
                int message = GuiMessageBox((Rectangle) { GetScreenWidth()/2 - 120, GetScreenHeight()/2 - 48, 248, 96 }, "#159#Closing rGuiLayout", "Do you want to save before quitting?", "Yes;No");

                if (message == 0) windowExitActive = false;
                else if (message == 1)  // Yes
                {
                    showSaveFileDialog = true;
                    windowExitActive = false;
                    exitWindow = true;
                }
                else if (message == 2) exitWindow = true;
            }
            //----------------------------------------------------------------------------------------

            // Draw status bar bottom with debug information
            //--------------------------------------------------------------------------------------------
            GuiStatusBar((Rectangle){ 0, GetScreenHeight() - 24, 126, 24}, TextFormat("MOUSE: (%i, %i)", (int)mouse.x, (int)mouse.y));
            GuiStatusBar((Rectangle){ 124, GetScreenHeight() - 24, 81, 24}, (snapMode? "SNAP: ON" : "SNAP: OFF"));
            GuiStatusBar((Rectangle){ 204, GetScreenHeight() - 24, 145, 24}, TextFormat("CONTROLS COUNT: %i", layout->controlCount));
            GuiStatusBar((Rectangle){ 348, GetScreenHeight() - 24, 100, 24}, TextFormat("GRID SIZE: %i", gridSpacing*gridSubdivisions));

            if (selectedControl != -1)
            {
                int defaultPadding = GuiGetStyle(STATUSBAR, TEXT_PADDING);
                int defaultTextAlign = GuiGetStyle(STATUSBAR, TEXT_ALIGNMENT);
                GuiSetStyle(STATUSBAR, TEXT_PADDING, 10);
                GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
                GuiStatusBar((Rectangle){ 348, GetScreenHeight() - 24, GetScreenWidth() - 348, 24},
                             TextFormat("SELECTED CONTROL: #%03i  |  %s  |  REC (%i, %i, %i, %i)  |  %s",
                                        selectedControl, TextToUpper(controlTypeName[layout->controls[selectedControl].type]),
                                        (int)layout->controls[selectedControl].rec.x, (int)layout->controls[selectedControl].rec.y,
                                        (int)layout->controls[selectedControl].rec.width, (int)layout->controls[selectedControl].rec.height,
                                        layout->controls[selectedControl].name));
                GuiSetStyle(STATUSBAR, TEXT_PADDING, defaultPadding);
                GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, defaultTextAlign);
            }
            else GuiStatusBar((Rectangle){ 447, GetScreenHeight() - 24, GetScreenWidth() - 348, 24}, NULL);
            //--------------------------------------------------------------------------------------------

            // GUI: Load File Dialog (and loading logic)
            //----------------------------------------------------------------------------------------
            if (showLoadFileDialog)
            {
#if defined(CUSTOM_MODAL_DIALOGS)
                int result = GuiFileDialog(DIALOG_MESSAGE, "Load raygui layout file ...", inFileName, "Ok", "Just drag and drop your .rgl layout file!");
#else
                int result = GuiFileDialog(DIALOG_OPEN, "Load raygui layout file", inFileName, "*.rgl", "raygui Layout Files (*.rgl)");
#endif
                if (result == 1)
                {
                    // Load layout file
                    GuiLayout *tempLayout = LoadLayout(inFileName);

                    if (tempLayout != NULL)
                    {
                        memcpy(layout, tempLayout, sizeof(GuiLayout));

                        // HACK: When leaving scope, tempLayout internal pointer references are lost,
                        // so we manually reset those references to layout internals
                        // TODO: Probably this system should be designed in a diferent way...
                        for (int i = 0; i < layout->controlCount; i++) layout->controls[i].ap = &layout->anchors[tempLayout->controls[i].ap->id];

                        for (int i = 0; i < MAX_UNDO_LEVELS; i++) memcpy(&undoLayouts[i], layout, sizeof(GuiLayout));
                        currentUndoIndex = 0;
                        firstUndoIndex = 0;

                        UnloadLayout(tempLayout);

                        SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, GetFileName(inFileName)));
                        saveChangesRequired = false;
                    }
                    else inFileName[0] = '\0';
                }

                if (result >= 0) showLoadFileDialog = false;
            }
            //----------------------------------------------------------------------------------------

            // GUI: Save File Dialog (and saving logic)
            //----------------------------------------------------------------------------------------
            if (showSaveFileDialog)
            {
#if defined(CUSTOM_MODAL_DIALOGS)
                int result = GuiFileDialog(DIALOG_TEXTINPUT, "Save raygui layout file...", outFileName, "Ok;Cancel", NULL);
#else
                int result = GuiFileDialog(DIALOG_SAVE, "Save raygui layout file...", outFileName, "*.rgl", "raygui Layout Files (*.rgl)");
#endif
                if (result == 1)
                {
                    // Save file: outFileName
                    // Check for valid extension and make sure it is
                    if ((GetFileExtension(outFileName) == NULL) || !IsFileExtension(outFileName, ".rgl")) strcat(outFileName, ".rgl\0");

                    // Save layout file
                    SaveLayout(layout, outFileName);

                    strcpy(inFileName, outFileName);
                    SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, GetFileName(inFileName)));
                    saveChangesRequired = false;

                #if defined(PLATFORM_WEB)
                    // Download file from MEMFS (emscripten memory filesystem)
                    // NOTE: Second argument must be a simple filename (we can't use directories)
                    emscripten_run_script(TextFormat("saveFileFromMEMFSToDisk('%s','%s')", outFileName, GetFileName(outFileName)));
                #endif
                }

                if (result >= 0) showSaveFileDialog = false;
            }
            //----------------------------------------------------------------------------------------

            // GUI: Export File Dialog (and saving logic)
            //----------------------------------------------------------------------------------------
            if (showExportFileDialog)
            {
                if (windowCodegenState.codeTemplateActive == 0) strcpy(outFileName, TextFormat("gui_%s.c", config.name));
                else if (windowCodegenState.codeTemplateActive == 1) strcpy(outFileName, TextFormat("gui_%s.h", config.name));

#if defined(CUSTOM_MODAL_DIALOGS)
                int result = GuiFileDialog(DIALOG_TEXTINPUT, "Export layout as code file...", outFileName, "Ok;Cancel", NULL);
#else
                int result = GuiFileDialog(DIALOG_SAVE, "Export layout as code file...", outFileName, "*.c;*.h", "Code Files");
#endif
                if (result == 1)
                {
                    // Check for valid extension and make sure it is
                    if ((GetFileExtension(outFileName) == NULL) || 
                        (!IsFileExtension(outFileName, ".c") && !IsFileExtension(outFileName, ".h"))) strcat(outFileName, ".h\0");

                    // Write code string to file
                    FILE *ftool = fopen(outFileName, "wt");
                    fprintf(ftool, windowCodegenState.codeText);
                    fclose(ftool);

                #if defined(PLATFORM_WEB)
                    // Download file from MEMFS (emscripten memory filesystem)
                    // NOTE: Second argument must be a simple filename (we can't use directories)
                    emscripten_run_script(TextFormat("saveFileFromMEMFSToDisk('%s','%s')", outFileName, GetFileName(outFileName)));
                #endif
                }

                if (result >= 0) showExportFileDialog = false;
            }
            //----------------------------------------------------------------------------------------

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
    UnloadLayout(layout);       // Unload raygui layout
    UnloadTexture(tracemap);    // Unload tracemap texture (if loaded)

    free(undoLayouts);          // Free undo layouts array
    free(windowCodegenState.codeText);  // Free loaded codeText memory

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module functions definition
//----------------------------------------------------------------------------------
#if defined(VERSION_ONE)
// Show command line usage info
static void ShowCommandLineInfo(void)
{
    printf("\n//////////////////////////////////////////////////////////////////////////////////\n");
    printf("//                                                                              //\n");
    printf("// %s v%s - %s                 //\n", toolName, toolVersion, toolDescription);
    printf("// powered by raylib v%s and raygui v%s                                   //\n", RAYLIB_VERSION, RAYGUI_VERSION);
    printf("// more info and bugs-report: github.com/raylibtech/rtools                      //\n");
    printf("// feedback and support:      ray[at]raylibtech.com                             //\n");
    printf("//                                                                              //\n");
    printf("// Copyright (c) 2017-2022 raylib technologies (@raylibtech)                    //\n");
    printf("//                                                                              //\n");
    printf("//////////////////////////////////////////////////////////////////////////////////\n\n");

    printf("USAGE:\n\n");
    printf("    > rguilayout [--help] --input <filename.ext> [--output <filename.ext>]\n");
    printf("                 [--format <styleformat>] [--edit-prop <property> <value>]\n");

    printf("\nOPTIONS:\n\n");
    printf("    -h, --help                      : Show tool version and command line usage help\n");
    printf("    -i, --input <filename.ext>      : Define input file.\n");
    printf("                                      Supported extensions: .rgl\n");
    printf("    -o, --output <filename.ext>     : Define output file.\n");
    printf("                                      Supported extensions: .c, .h\n");
    printf("    -t, --template <filename.ext>   : Define code template for output.\n");
    printf("                                      Supported extensions: .c, .h\n\n");

    printf("\nEXAMPLES:\n\n");
    printf("    > rguilayout --input mytool.rgl --output mytools.h\n");
}

// Process command line input
static void ProcessCommandLine(int argc, char *argv[])
{
    // CLI required variables
    bool showUsageInfo = false;         // Toggle command line usage info

    char inFileName[512] = { 0 };       // Input file name
    char outFileName[512] = { 0 };      // Output file name
    char templateFile[512] = { 0 };     // Template file name

    int outputFormat = 0;               // Supported output formats

    // Process command line arguments
    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
        {
            showUsageInfo = true;
        }
        else if ((strcmp(argv[i], "-i") == 0) || (strcmp(argv[i], "--input") == 0))
        {
            // Check for valid argument and valid file extension
            if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
            {
                if (IsFileExtension(argv[i + 1], ".rgl"))
                {
                    strcpy(inFileName, argv[i + 1]);    // Read input filename
                }
                else LOG("WARNING: Input file extension not recognized\n");

                i++;
            }
            else LOG("WARNING: No input file provided\n");
        }
        else if ((strcmp(argv[i], "-o") == 0) || (strcmp(argv[i], "--output") == 0))
        {
            if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
            {
                if (IsFileExtension(argv[i + 1], ".c") ||
                    IsFileExtension(argv[i + 1], ".h"))
                {
                    strcpy(outFileName, argv[i + 1]);   // Read output filename
                }
                else LOG("WARNING: Output file extension not recognized\n");

                i++;
            }
            else LOG("WARNING: No output file provided\n");
        }
        else if ((strcmp(argv[i], "-t") == 0) || (strcmp(argv[i], "--template") == 0))
        {
            if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
            {
                if (IsFileExtension(argv[i + 1], ".c") ||
                    IsFileExtension(argv[i + 1], ".h"))
                {
                    strcpy(templateFile, argv[i + 1]);   // Read template filename
                }
                else LOG("WARNING: Template file extension not recognized\n");

                i++;
            }
            else LOG("WARNING: No template file provided\n");
        }

        // TODO: Support codegen options: exportAnchors, defineRecs, fullComments...
    }

    // Process input file
    if (inFileName[0] != '\0')
    {
        // Set a default name for output in case not provided
        if (outFileName[0] == '\0') strcpy(outFileName, "output.c");

        LOG("\nInput file:       %s", inFileName);
        LOG("\nOutput file:      %s", outFileName);

        // Support .rlg layout processing to generate .c
        GuiLayout *layout = LoadLayout(inFileName);

        GuiLayoutConfig config = { 0 };
        memset(&config, 0, sizeof(GuiLayoutConfig));
        strcpy(config.name, "window_codegen");
        strcpy(config.version, toolVersion);
        strcpy(config.company, "raylib technologies");
        strcpy(config.description, "tool description");
        config.exportAnchors = true;
        config.defineRecs = false;
        config.fullComments = true;

        // Generate C code for gui layout->controls
        char *guiTemplateCustom = NULL;
        if (templateFile[0] != '\0') guiTemplateCustom = LoadFileText(templateFile);

        unsigned char *toolstr = NULL;
        if (guiTemplateCustom != NULL)
        {
            toolstr = GenLayoutCode(guiTemplateCustom, *layout, config);
            free(guiTemplateCustom);
        }
        else toolstr = GenLayoutCode(guiTemplateStandardCode, *layout, config);

        FILE *ftool = fopen(outFileName, "wt");
        fprintf(ftool, toolstr);    // Write code string to file
        fclose(ftool);

        UnloadLayout(layout);
        free(toolstr);
    }

    if (showUsageInfo) ShowCommandLineInfo();
}
#endif      // VERSION_ONE

//--------------------------------------------------------------------------------------------
// Load/Save/Export data functions
//--------------------------------------------------------------------------------------------
// Load layout from text file
// NOTE: If NULL provided, an empty layout is initialized
static GuiLayout *LoadLayout(const char *fileName)
{
    GuiLayout *layout = (GuiLayout *)calloc(1, sizeof(GuiLayout));

    ResetLayout(layout);    // Init some default values

    if (fileName != NULL)   // Try loading text layout file
    {
        FILE *rglFile = fopen(fileName, "rt");
        if (rglFile != NULL)
        {
            char buffer[256] = { 0 };

            int anchorCounter = 0;
            char anchorName[MAX_ANCHOR_NAME_LENGTH] = { 0 };

            fgets(buffer, 256, rglFile);

            while (!feof(rglFile))
            {
                switch (buffer[0])
                {
                case 'r':
                {
                    sscanf(buffer, "r %f %f %f %f", &layout->refWindow.x, &layout->refWindow.y, &layout->refWindow.width, &layout->refWindow.height);
                } break;
                case 'a':
                {
                    int enabled = 0;
                    sscanf(buffer, "a %d %s %d %d %d",
                        &layout->anchors[anchorCounter].id,
                        anchorName,
                        &layout->anchors[anchorCounter].x,
                        &layout->anchors[anchorCounter].y,
                        &enabled);

                    layout->anchors[anchorCounter].enabled = (enabled ? true : false);
                    strcpy(layout->anchors[anchorCounter].name, anchorName);

                    if (layout->anchors[anchorCounter].enabled) layout->anchorCount++;
                    anchorCounter++;
                } break;
                case 'c':
                {
                    int anchorId = 0;
                    sscanf(buffer, "c %d %d %s %f %f %f %f %d %[^\n]s",
                        &layout->controls[layout->controlCount].id,
                        &layout->controls[layout->controlCount].type,
                        layout->controls[layout->controlCount].name,
                        &layout->controls[layout->controlCount].rec.x,
                        &layout->controls[layout->controlCount].rec.y,
                        &layout->controls[layout->controlCount].rec.width,
                        &layout->controls[layout->controlCount].rec.height,
                        &anchorId,
                        layout->controls[layout->controlCount].text);

                    layout->controls[layout->controlCount].ap = &layout->anchors[anchorId];
                    layout->controlCount++;
                } break;
                default: break;
                }

                fgets(buffer, 256, rglFile);
            }

            for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
            {
                layout->anchors[i].x += layout->anchors[0].x;
                layout->anchors[i].y += layout->anchors[0].y;
            }

            fclose(rglFile);
        }
    }

    return layout;
}

// Unload layout
static void UnloadLayout(GuiLayout *layout)
{
    free(layout);
}

// Reset layout to default values
static void ResetLayout(GuiLayout *layout)
{
    // Set anchor points to default values
    for (int i = 0; i < MAX_ANCHOR_POINTS; i++)
    {
        layout->anchors[i].id = i;
        layout->anchors[i].x = 0;
        layout->anchors[i].y = 0;
        layout->anchors[i].enabled = false;
        layout->anchors[i].hidding = false;
        memset(layout->anchors[i].name, 0, MAX_ANCHOR_NAME_LENGTH);

        if (i == 0) strcpy(layout->anchors[i].name, "anchorMain");
        else strcpy(layout->anchors[i].name, TextFormat("anchor%02i", i));
    }

    // Initialize layout controls data
    for (int i = 0; i < MAX_GUI_CONTROLS; i++)
    {
        layout->controls[i].id = 0;
        layout->controls[i].type = 0;
        layout->controls[i].rec = (Rectangle){ 0, 0, 0, 0 };
        memset(layout->controls[i].text, 0, MAX_CONTROL_TEXT_LENGTH);
        memset(layout->controls[i].name, 0, MAX_CONTROL_NAME_LENGTH);
        layout->controls[i].ap = &layout->anchors[0];  // By default, set parent anchor
    }

    layout->refWindow = (Rectangle){ 0, 0, -1, -1 };
    layout->anchorCount = 0;
    layout->controlCount = 0;
}

// Save layout information as text file
static void SaveLayout(GuiLayout *layout, const char *fileName)
{
    #define RGL_FILE_VERSION_TEXT "2.5"

    FILE *rglFile = fopen(fileName, "wt");

    if (rglFile != NULL)
    {
        // Write some description comments
        fprintf(rglFile, "#\n# rgl layout text file (v%s) - raygui layout file generated using rGuiLayout\n#\n", RGL_FILE_VERSION_TEXT);
        fprintf(rglFile, "# Number of controls:     %i\n#\n", layout->controlCount);
        fprintf(rglFile, "# Ref. window:    r <x> <y> <width> <height>\n");
        fprintf(rglFile, "# Anchor info:    a <id> <name> <posx> <posy> <enabled>\n");
        fprintf(rglFile, "# Control info:   c <id> <type> <name> <rectangle> <anchor_id> <text>\n#\n");

        fprintf(rglFile, "r %i %i %i %i\n", (int)layout->refWindow.x, (int)layout->refWindow.y, (int)layout->refWindow.width, (int)layout->refWindow.height);
        fprintf(rglFile, "a %03i %s %i %i %i\n", layout->anchors[0].id, layout->anchors[0].name, layout->anchors[0].x, layout->anchors[0].y, layout->anchors[0].enabled);

        for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
        {
            fprintf(rglFile, "a %03i %s %i %i %i\n",
                                layout->anchors[i].id,
                                layout->anchors[i].name,
                                (int)(layout->anchors[i].x - layout->refWindow.x),
                                (int)(layout->anchors[i].y - layout->refWindow.y),
                                layout->anchors[i].enabled);
        }

        for (int i = 0; i < layout->controlCount; i++)
        {
            fprintf(rglFile, "c %03i %i %s %i %i %i %i %i %s\n",
                                layout->controls[i].id,
                                layout->controls[i].type,
                                layout->controls[i].name,
                                (int)layout->controls[i].rec.x,
                                (int)layout->controls[i].rec.y,
                                (int)layout->controls[i].rec.width,
                                (int)layout->controls[i].rec.height,
                                layout->controls[i].ap->id,
                                layout->controls[i].text);
        }

        fclose(rglFile);
    }

/*
    if (binary)
    {
#define RGL_FILE_VERSION_BINARY 200

        FILE *rglFile = fopen(fileName, "wb");

        if (rglFile != NULL)
        {
            // Layout File Structure (.rgl)
            // ------------------------------------------------------
            // Offset  | Size    | Type       | Description
            // ------------------------------------------------------
            // 0       | 4       | char       | Signature: "rGL "
            // 4       | 2       | short      | Version: 200
            // 6       | 2       | short      | reserved
            //
            // 8       | x       | GuiLayout  | GuiLayout data

            char signature[5] = "rGL ";
            short version = RGL_FILE_VERSION_BINARY;
            short reserved = 0;

            fwrite(signature, 1, 4, rglFile);
            fwrite(&version, 1, sizeof(short), rglFile);
            fwrite(&reserved, 1, sizeof(short), rglFile);

            fwrite(layout, 1, sizeof(GuiLayout), rglFile);

            fclose(rglFile);
        }
    }
*/
}

// Draw help panel with the provided lines
static void GuiHelpPanel(int posX, int posY, const char *title, const char **helpLines, int helpLinesCount)
{
    int nextLineY = 0;
    int lineSpacing = 20;

    DrawRectangleRec((Rectangle) { posX, posY, 300, helpLinesCount*lineSpacing + 20 }, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    GuiGroupBox((Rectangle) { posX, posY, 300, helpLinesCount*lineSpacing + 20 }, title);
    nextLineY += 12;

    for (int i = 0; i < helpLinesCount; i++)
    {
        if (helpLines[i] == NULL) GuiLine((Rectangle) { posX, posY + nextLineY, 300, 12 }, helpLines[i]);
        else if (helpLines[i][0] == '-') GuiLine((Rectangle) { posX, posY + nextLineY, 300, lineSpacing }, helpLines[i] + 1);
        else GuiLabel((Rectangle) { posX + 12, posY + nextLineY, 0, lineSpacing }, helpLines[i]);

        if (helpLines[i] == NULL) nextLineY += 12;
        else nextLineY += lineSpacing;
    }
}