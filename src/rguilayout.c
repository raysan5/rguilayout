/*******************************************************************************************
*
*   rGuiLayout v3.0 - A simple and easy-to-use raygui layouts editor
*
*   CONFIGURATION:
*
*   #define CUSTOM_MODAL_DIALOGS
*       Use custom raygui generated modal dialogs instead of native OS ones
*       NOTE: Avoids including tinyfiledialogs depencency library
*
*   VERSIONS HISTORY:
*       3.0  (xx-Oct-2022)  ADDED: Main toolbar, consistent with other tools
*                           ADDED: Sponsor window for tools support
*                           ADDED: Multiple UI styles selection
*                           REVIEWED: Codegen window font and templates
*                           Updated to raylib 4.5-dev and raygui 3.5-dev
*                           Source code re-licensed as open-source
*       2.5  (05-Jan-2022)  Updated to raylib 4.0 and raygui 3.1
*       2.0  (15-Sep-2019)  Rewriten from scratch
*       1.0  (14-May-2018)  First release
*
*   DEPENDENCIES:
*       raylib 4.5-dev          - Windowing/input management and drawing
*       raygui 3.5-dev          - Immediate-mode GUI controls with custom styling and icons
*       rpng 1.0                - PNG chunks management
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
*   NOTE: On PLATFORM_ANDROID and PLATFORM_WEB file dialogs are not available
*
*   DEVELOPERS:
*       Ramon Santamaria (@raysan5):    Supervision, review, redesign, update and maintenance.
*       Sergio Martinez (@anidealgift): Developer and designer (v2.0 - Jan.2019)
*       Adria Arranz (@Adri102):        Developer and designer (v1.0 - Jun.2018)
*       Jordi Jorba (@KoroBli):         Developer and designer (v1.0 - Jun.2018)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2017-2022 raylib technologies (@raylibtech) / Ramon Santamaria (@raysan5)
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

#define TOOL_NAME               "rGuiLayout"
#define TOOL_SHORT_NAME         "rGL"
#define TOOL_VERSION            "3.0"
#define TOOL_DESCRIPTION        "A simple and easy-to-use raygui layouts editor"
#define TOOL_RELEASE_DATE       "Oct.2022"
#define TOOL_LOGO_COLOR         0x7da9b9ff

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

// NOTE: Some redefines are required to support icons panel drawing
#define RAYGUI_GRID_ALPHA                 0.1f
#define RAYGUI_TEXTSPLIT_MAX_ELEMENTS     256
#define RAYGUI_TEXTSPLIT_MAX_TEXT_SIZE   4096
#define RAYGUI_TOGGLEGROUP_MAX_ELEMENTS   256
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                         // Required for: IMGUI controls

#undef RAYGUI_IMPLEMENTATION                // Avoid including raygui implementation again

#define GUI_MAIN_TOOLBAR_IMPLEMENTATION
#include "gui_main_toolbar.h"               // GUI: Main toolbar

#define GUI_WINDOW_ABOUT_IMPLEMENTATION
#include "gui_window_about.h"               // GUI: About Window

#define GUI_WINDOW_SPONSOR_IMPLEMENTATION
#include "gui_window_sponsor.h"             // GUI: Sponsor Window

#define GUI_FILE_DIALOGS_IMPLEMENTATION
#include "gui_file_dialogs.h"               // GUI: File Dialogs

#include "rguilayout.h"                     // Gui layout types definition

#define GUI_WINDOW_CONTROLS_PALETTE_IMPLEMENTATION
#include "gui_window_controls_palette.h"    // GUI: Controls Palette

#define CODEGEN_IMPLEMENTATION
#include "codegen.h"                        // Code generation functions
#include "code_templates.h"                 // Code template files (char buffers)

#define GUI_WINDOW_CODEGEN_IMPLEMENTATION
#include "gui_window_codegen.h"             // GUI: Code Generation Window

// raygui embedded styles
// NOTE: Included in the same order as selector
#define MAX_GUI_STYLES_AVAILABLE   12       // NOTE: Included light style
#include "styles/style_jungle.h"            // raygui style: jungle
#include "styles/style_candy.h"             // raygui style: candy
#include "styles/style_lavanda.h"           // raygui style: lavanda
#include "styles/style_cyber.h"             // raygui style: cyber
#include "styles/style_terminal.h"          // raygui style: terminal
#include "styles/style_ashes.h"             // raygui style: ashes
#include "styles/style_bluish.h"            // raygui style: bluish
#include "styles/style_dark.h"              // raygui style: dark
#include "styles/style_cherry.h"            // raygui style: cherry
#include "styles/style_sunny.h"             // raygui style: sunny
#include "styles/style_enefete.h"           // raygui style: enefete

// WARNING: When compiling in raylib DLL mode, this include generates missing symbols issues: zinflate, sdefl_bound, zsdeflate
// because those symbols are provided by raylib but are moved to the external DLL
// Otherwise, when compiling in static mode, defining RPNG_DEFLATE_IMPLEMENTATION generated symbol duplicates
#define RPNG_IMPLEMENTATION
//#define RPNG_DEFLATE_IMPLEMENTATION
#include "external/rpng.h"                  // PNG chunks management

// Standard C libraries
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
#if defined(SUPPORT_LOG_INFO) && defined(_DEBUG)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

#define ANCHOR_RADIUS               20      // Default anchor radius
#define MIN_CONTROL_SIZE            10      // Minimum control size
#define SCALE_BOX_CORNER_SIZE       12      // Scale box bottom-right corner square size

#define MOVEMENT_FRAME_SPEED         2      // Controls movement speed in pixels per frame: TODO: Review

#define MAX_UNDO_LEVELS             32      // Undo levels supported for the ring buffer

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Layout editing mode enabled
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

#define HELP_LINES_COUNT    36

// Tool help info
static const char *helpLines[HELP_LINES_COUNT] = {
    "F1 - Show Help window",
    "F2 - Show About window",
    "F3 - Show Sponsor window",
    "-File Options",
    "LCTRL + N - New layout file (.rgl)",
    "LCTRL + O - Open layout file (.rgl)",
    "LCTRL + S - Save layout file (.rgl)",
    "LCTRL + E - Export layout to code",
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
    "LCTRL + G - Toggle grid mode",
    "LALT + S - Toggle snap to grid mode",
    //"RALT + UP/DOWN - Grid spacing + snap",
    "LCTRL + F - Toggle control position info (global/anchor)",     // TODO: Really?
    "SPACE - Toggle tracemap Lock/Unlock",
};

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
#if defined(PLATFORM_DESKTOP)
static void ShowCommandLineInfo(void);                      // Show command line usage info
static void ProcessCommandLine(int argc, char *argv[]);     // Process command line input
#endif

// Init/Load/Save/Export data functions
static GuiLayout *LoadLayout(const char *fileName);         // Load raygui layout: empty (NULL) or from file (.rgl)
static void UnloadLayout(GuiLayout *layout);                // Unload raygui layout
static void ResetLayout(GuiLayout *layout);                 // Reset layout to default values
static void SaveLayout(GuiLayout *layout, const char *fileName);     // Save raygui layout as text file (.rgl)

// Auxiliar functions
static int GuiWindowHelp(Rectangle bounds, const char *title, const char **helpLines, int helpLinesCount); // Draw help window with the provided lines

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
#if defined(PLATFORM_DESKTOP)
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
                strcpy(inFileName, argv[1]);        // Read input filename to open with gui interface
            }
        }
        else
        {
            ProcessCommandLine(argc, argv);
            return 0;
        }
    }
#endif  // PLATFORM_DESKTOP
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
    SetWindowMinSize(1280, 720);
    SetExitKey(0);

    // Code font generation for embedding
    // WARNING: It requires SUPPORT_FILEFORMAT_TTF enabled by raylib
    //Font codeFont = LoadFontEx("resources/gohufont-14.ttf", 14, NULL, 0);
    //ExportFontAsCode(codeFont, "gohufont.h");

    // General pourpose variables
    Vector2 mouse = { 0, 0 };               // Mouse position

    // Work area to place components (full screen by default)
    Rectangle workArea = { 0, 40, GetScreenWidth(), GetScreenHeight() - 40 - 24 };
    bool windowOverActive = false;          // Check for any blocking window active

    // Grid control variables
    int gridSpacing = 8;                    // Grid minimum spacing in pixels (between every subdivision)
    int gridSubdivisions = 3;               // Grid subdivisions (total size for every big line is gridSpacing*gridSubdivisions)
    int gridSnapDelta = 1;                  // Grid snap minimum value in pixels
    int moveFrameCounter = 0;               // Movement frames counter
    int moveFrameSpeed = 1;                 // Movement speed per frame

    // Control edit modes
    // NOTE: [E] - Exclusive mode operation, all other modes blocked
    bool dragMoveMode = false;              // [E] Control drag mode
    bool textEditMode = false;              // [E] Control text edit mode (KEY_T)
    bool nameEditMode = false;              // [E] Control name edit mode (KEY_N)
    bool resizeMode = false;                // [E] Control size mode ((controlSelected != -1) + KEY_LEFT_ALT)
    bool orderEditMode = false;             // Control order edit mode ((focusedControl != -1) + KEY_LEFT_ALT)
    bool precisionEditMode = false;         // Control precision edit mode (KEY_LEFT_SHIFT)
    bool mouseScaleMode = false;            // [E] Control is being scaled by mouse
    bool mouseScaleReady = false;           // Mouse is on position to start control scaling

    // Anchor edit modes
    bool anchorEditMode = false;            // [E] Anchor edition mode
    bool anchorLinkMode = false;            // [E] Anchor linkage mode
    bool anchorMoveMode = false;            // [E] Anchor move mode

    // Ref window edit mode
    bool refWindowEditMode = false;         // [E] Refence window edit mode

    // Toggle global controls position info
    // NOTE: It could be useful at some moment (but not usual)
    bool showGlobalPosition = false;        // Control global position mode

    // TODO: Check exclusive modes (work on its own) and combinable modes (can work in combination with other)
    // Replace all bool values by enumerator value, it should simplify code...
    //int layoutEditMode = NONE;              // Layout edition mode

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

    // TODO: Support multiple controls selection
    // Multiselection variables
    //bool multiSelectMode = false;           // [E] Multiselection mode
    //Rectangle multiSelectRec = { 0 };
    //Vector2 multiSelectStartPos = { 0 };
    //int multiSelectControls[20] = { -1 };
    //int multiSelectCount = 0;

    // Init default layout
    //-------------------------------------------------------------------------
    GuiLayout *layout = NULL;
    
    if (inFileName[0] != '\0')          // Load dropped file if provided
    {
        layout = LoadLayout(inFileName);
        SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, GetFileName(inFileName)));
    }
    else layout = LoadLayout(NULL);     // Load empty layout

    // Previous text/name, required when cancel editing
    char prevText[MAX_CONTROL_TEXT_LENGTH] = { 0 };
    char prevName[MAX_CONTROL_NAME_LENGTH] = { 0 };
    //-------------------------------------------------------------------------

    // Undo/redo system variables
    //-------------------------------------------------------------------------
    GuiLayout *undoLayouts = (GuiLayout *)RL_CALLOC(MAX_UNDO_LEVELS, sizeof(GuiLayout));   // Layouts array
    int currentUndoIndex = 0;
    int firstUndoIndex = 0;
    int lastUndoIndex = 0;
    int undoFrameCounter = 0;

    // Init undo/redo system with current layout
    for (int i = 0; i < MAX_UNDO_LEVELS; i++) memcpy(&undoLayouts[i], layout, sizeof(GuiLayout));
    //-------------------------------------------------------------------------

    // Tracemap (background image for reference) variables
    Texture2D tracemap = { 0 };
    Rectangle tracemapRec = { 0 };
    bool tracemapBlocked = false;
    bool tracemapFocused = false;
    bool tracemapSelected = false;
    float tracemapFade = 0.5f;
    Color tracemapColor = RED;

    // Controls temp variables (default values)
    int dropdownBoxActive = 0;
    int spinnerValue = 0;
    int valueBoxValue = 0;
    int listViewScrollIndex = 0;
    int listViewActive = 0;

    // Icons panel variables
    //-------------------------------------------------------------------------
    bool showIconPanel = false;             // Show icon panel for selection

    int selectedIcon = 0;                   // Current icon selected
    char toggleIconsText[16*14*6] = { 0 };  // 14 lines with 16 icons per line -> TODO: Review if more icons are added!
    for (int i = 0; i < 16*14; i++)
    {
        // NOTE: Every icon requires 6 text characters: "#001#;"
        if ((i + 1)%16 == 0) strncpy(toggleIconsText + 6*i, TextFormat("#%03i#\n", i), 6);
        else strncpy(toggleIconsText + 6*i, TextFormat("#%03i#;", i), 6);
    }
    toggleIconsText[16*14*6 - 1] = '\0';
    //-------------------------------------------------------------------------

    // Layout code generation configuration
    //------------------------------------------------------------------------------------
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
    //------------------------------------------------------------------------------------

    // GUI: Main toolbar panel (file and visualization)
    //-----------------------------------------------------------------------------------
    GuiMainToolbarState mainToolbarState = InitGuiMainToolbar();
    //-----------------------------------------------------------------------------------

    // GUI: About Window
    //-----------------------------------------------------------------------------------
    GuiWindowAboutState windowAboutState = InitGuiWindowAbout();
    //-----------------------------------------------------------------------------------
    
    // GUI: Sponsor Window
    //-----------------------------------------------------------------------------------
    GuiWindowSponsorState windowSponsorState = InitGuiWindowSponsor();
    //-----------------------------------------------------------------------------------

    // GUI: Controls Selection Palette
    //-----------------------------------------------------------------------------------
    GuiWindowControlsPaletteState windowControlsPaletteState = InitGuiWindowControlsPalette();

    // Rectangles used on controls preview drawing, copied from palette
    // NOTE: [x, y] position is set on mouse movement and considering snap mode
    Rectangle defaultRec[CONTROLS_PALETTE_COUNT] = { 0 };
    for (int i = 0; i < CONTROLS_PALETTE_COUNT; i++)
    {
        defaultRec[i].width = windowControlsPaletteState.controlRecs[i].width;
        defaultRec[i].height = windowControlsPaletteState.controlRecs[i].height;
    }
    //-----------------------------------------------------------------------------------

    // GUI: Layout Code Generation Window
    //-----------------------------------------------------------------------------------
    GuiWindowCodegenState windowCodegenState = InitGuiWindowCodegen();
    //-----------------------------------------------------------------------------------

    // GUI: Help panel
    //-----------------------------------------------------------------------------------
    bool windowHelpActive = false;
    //-----------------------------------------------------------------------------------

    // GUI: Exit Window
    //-----------------------------------------------------------------------------------
    bool windowExitActive = false;
    bool closeWindow = false;
    //-----------------------------------------------------------------------------------

    // GUI: Reset Layout Window
    //-----------------------------------------------------------------------------------
    bool windowResetActive = false;
    bool resetLayout = false;
    //-----------------------------------------------------------------------------------

    // GUI: Custom file dialogs
    //-----------------------------------------------------------------------------------
    bool showLoadFileDialog = false;
    bool showSaveFileDialog = false;
    bool showExportFileDialog = false;
    bool showLoadTracemapDialog = false;
    //-----------------------------------------------------------------------------------

    SetTargetFPS(60);       // Set our game desired framerate
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!closeWindow)    // Detect window close button
    {
        // WARNING: ASINCIFY requires this line,
        // it contains the call to emscripten_sleep() for PLATFORM_WEB
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
            FilePathList droppedFiles = LoadDroppedFiles();

            // Supports loading .rgl layout files (text or binary) and .png tracemap images
            if (IsFileExtension(droppedFiles.paths[0], ".rgl"))
            {
                GuiLayout *tempLayout = LoadLayout(droppedFiles.paths[0]);

                if (tempLayout != NULL)
                {
                    memcpy(layout, tempLayout, sizeof(GuiLayout));

                    // HACK: When leaving scope, tempLayout internal pointer references are lost,
                    // so we manually reset those references to layout internals
                    // TODO: Probably this system should be designed in a diferent way...
                    for (int i = 0; i < layout->controlCount; i++) layout->controls[i].ap = &layout->anchors[tempLayout->controls[i].ap->id];

                    strcpy(inFileName, droppedFiles.paths[0]);
                    SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, GetFileName(inFileName)));

                    for (int i = 0; i < MAX_UNDO_LEVELS; i++) memcpy(&undoLayouts[i], layout, sizeof(GuiLayout));
                    currentUndoIndex = 0;
                    firstUndoIndex = 0;

                    UnloadLayout(tempLayout);
                }
            }
            else if (IsFileExtension(droppedFiles.paths[0], ".png")) // Tracemap image
            {
                if (tracemap.id > 0) UnloadTexture(tracemap);
                tracemap = LoadTexture(droppedFiles.paths[0]);
                tracemapRec = (Rectangle){30, 30, tracemap.width, tracemap.height};
            }
            else if (IsFileExtension(droppedFiles.paths[0], ".rgs")) GuiLoadStyle(droppedFiles.paths[0]);

            UnloadDroppedFiles(droppedFiles);   // Unload filepaths from memory
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

        // Show dialog: export style file (.rgs, .png, .h)
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E)) showExportFileDialog = true;

        // Toggle window: help
        if (IsKeyPressed(KEY_F1)) windowHelpActive = !windowHelpActive;

        // Toggle window: about
        if (IsKeyPressed(KEY_F2)) windowAboutState.windowActive = !windowAboutState.windowActive;

        // Toggle window: sponsor
        if (IsKeyPressed(KEY_F3)) windowSponsorState.windowActive = !windowSponsorState.windowActive;

        // Show closing window on ESC
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
                if (windowSponsorState.windowActive) windowSponsorState.windowActive = false;
                else if (windowCodegenState.windowActive) windowCodegenState.windowActive = false;
                else if (windowResetActive) windowResetActive = false;
#if !defined(PLATFORM_WEB)
                else if ((layout->controlCount <= 0) && (layout->anchorCount <= 1)) closeWindow = true;
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
            // Enables or disables mainToolbarState.snapModeActive if not in textEditMode
            if (IsKeyPressed(KEY_S))
            {
                mainToolbarState.snapModeActive = !mainToolbarState.snapModeActive;
                if (mainToolbarState.snapModeActive)
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
            if (IsKeyPressed(KEY_F)) showGlobalPosition = !showGlobalPosition;      // Toggle global position info (anchor reference or global reference)
            if (IsKeyPressed(KEY_G)) mainToolbarState.showGridActive = !mainToolbarState.showGridActive;              // Toggle Grid mode

            anchorEditMode = IsKeyDown(KEY_A);              // Toggle anchor mode editing (on key down)
            orderEditMode = IsKeyDown(KEY_LEFT_ALT);        // Toggle controls drawing order

            precisionEditMode = IsKeyDown(KEY_LEFT_SHIFT);      // Toggle precision move/scale mode
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
                    config.exportButtonFunctions = windowCodegenState.genButtonFuncsChecked;

                    memcpy(&prevConfig, &config, sizeof(GuiLayoutConfig));

                    RL_FREE(windowCodegenState.codeText);
                    windowCodegenState.codeText = GenLayoutCode(guiTemplateStandardCode, *layout, config);
                    windowCodegenState.windowActive = true;
                }
            }

            // Change grid spacing
            // TODO: Look for a better mechanism  --> Project config window
            /*
            if (IsKeyDown(KEY_RIGHT_ALT))
            {
                if (IsKeyPressed(KEY_UP)) gridSpacing++;
                else if (IsKeyPressed(KEY_DOWN)) gridSpacing--;

                gridSnapDelta = gridSpacing;
            }
            */
        }
        
        // Main toolbar logic
        //----------------------------------------------------------------------------------
        windowControlsPaletteState.windowActive = mainToolbarState.showControlPanelActive;
        
        // Visual options logic
        if (mainToolbarState.visualStyleActive != mainToolbarState.prevVisualStyleActive)
        {
            // Reset to default internal style
            // NOTE: Required to unload any previously loaded font texture
            GuiLoadStyleDefault();

            switch (mainToolbarState.visualStyleActive)
            {
                case 1: GuiLoadStyleJungle(); break;
                case 2: GuiLoadStyleCandy(); break;
                case 3: GuiLoadStyleLavanda(); break;
                case 4: GuiLoadStyleCyber(); break;
                case 5: GuiLoadStyleTerminal(); break;
                case 6: GuiLoadStyleAshes(); break;
                case 7: GuiLoadStyleBluish(); break;
                case 8: GuiLoadStyleDark(); break;
                case 9: GuiLoadStyleCherry(); break;
                case 10: GuiLoadStyleSunny(); break;
                case 11: GuiLoadStyleEnefete(); break;
                default: break;
            }

            mainToolbarState.prevVisualStyleActive = mainToolbarState.visualStyleActive;
        }

        // Help options logic
        if (mainToolbarState.btnHelpPressed) windowHelpActive = true;                   // Help button logic
        if (mainToolbarState.btnAboutPressed) windowAboutState.windowActive = true;     // About window button logic
        if (mainToolbarState.btnSponsorPressed) windowSponsorState.windowActive = true; // User sponsor logic
        //----------------------------------------------------------------------------------

        // Basic program flow logic
        //----------------------------------------------------------------------------------
        mouse = GetMousePosition();

        if (IsWindowResized())
        {
            workArea.width = GetScreenWidth();
            workArea.height = GetScreenHeight() - 40 - 24;
            windowControlsPaletteState.windowBounds.x = workArea.width - windowControlsPaletteState.windowBounds.width;
            windowControlsPaletteState.windowBounds.y = 40;
            windowControlsPaletteState.windowBounds.height = workArea.height;
        }

        // Code generation window logic
        //----------------------------------------------------------------------------------
        if (windowCodegenState.windowActive)
        {
            strcpy(config.name, windowCodegenState.toolNameText);
            strcpy(config.version, windowCodegenState.toolVersionText);
            strcpy(config.company, windowCodegenState.companyText);
            strcpy(config.description, windowCodegenState.toolDescriptionText);
            config.exportAnchors = windowCodegenState.exportAnchorsChecked;
            config.defineRecs = windowCodegenState.defineRecsChecked;
            config.defineTexts = windowCodegenState.defineTextsChecked;
            config.fullComments = windowCodegenState.fullCommentsChecked;
            config.exportButtonFunctions = windowCodegenState.genButtonFuncsChecked;

            if ((currentCodeTemplate != windowCodegenState.codeTemplateActive) ||
                (memcmp(&config, &prevConfig, sizeof(GuiLayoutConfig)) != 0))
            {
                const unsigned char *template = NULL;
                if (windowCodegenState.codeTemplateActive == 0) template = guiTemplateStandardCode;
                else if (windowCodegenState.codeTemplateActive >= 1) template = guiTemplateHeaderOnly;
                //else if (windowCodegenState.codeTemplateActive == 2) template = LoadFileText(/*custom_template*/);
                currentCodeTemplate = windowCodegenState.codeTemplateActive;

                RL_FREE(windowCodegenState.codeText);
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
            if (mainToolbarState.snapModeActive && !anchorLinkMode)
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
            if (!CheckCollisionPointRec(mouse, windowControlsPaletteState.scrollPanelBounds))
            {
                if (focusedControl == -1) windowControlsPaletteState.selectedControl -= GetMouseWheelMove();

                if (windowControlsPaletteState.selectedControl < GUI_WINDOWBOX) windowControlsPaletteState.selectedControl = GUI_WINDOWBOX;
                else if (windowControlsPaletteState.selectedControl > GUI_DUMMYREC) windowControlsPaletteState.selectedControl = GUI_DUMMYREC;

                selectedType = windowControlsPaletteState.selectedControl;
            }
            //----------------------------------------------------------------------------------------------

            // Controls selection and edition logic
            //----------------------------------------------------------------------------------------------
            defaultRec[selectedType].x = mouse.x - defaultRec[selectedType].width/2;
            defaultRec[selectedType].y = mouse.y - defaultRec[selectedType].height/2;

            if (mainToolbarState.snapModeActive)
            {
                // TODO: Review depending on the Grid size and position

                int offsetX = (int)defaultRec[selectedType].x%gridSnapDelta;
                int offsetY = (int)defaultRec[selectedType].y%gridSnapDelta;

                if (offsetX >= gridSpacing/2) defaultRec[selectedType].x += (gridSpacing - offsetX);
                else defaultRec[selectedType].x -= offsetX;

                if (offsetY >= gridSpacing/2) defaultRec[selectedType].y += (gridSpacing - offsetY);
                else defaultRec[selectedType].y -= offsetY;
            }

            // Work area logic (controls placement and selection)
            if (CheckCollisionPointRec(mouse, workArea) &&
                !CheckCollisionPointRec(mouse, windowControlsPaletteState.windowBounds))
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

                                            if (mainToolbarState.snapModeActive)
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
                if (!mouseScaleReady && !CheckCollisionPointRec(mouse, (Rectangle){ 0, 0, GetScreenWidth(), 40 }) &&
                    (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)))
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

                            if (mainToolbarState.snapModeActive)
                            {
                                int offsetX = (int)controlPosX%gridSnapDelta;
                                int offsetY = (int)controlPosY%gridSnapDelta;

                                if (offsetX >= gridSnapDelta/2) controlPosX += (gridSnapDelta - offsetX);
                                else controlPosX -= offsetX;

                                if (offsetY >= gridSnapDelta/2) controlPosY += (gridSnapDelta - offsetY);
                                else controlPosY -= offsetY;
                            }

                            if (showGlobalPosition && (layout->controls[selectedControl].ap->id != 0))
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

                                if (precisionEditMode)
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

                                if (showGlobalPosition && (layout->controls[selectedControl].ap->id != 0))
                                {
                                    controlPosX += layout->controls[selectedControl].ap->x;
                                    controlPosY += layout->controls[selectedControl].ap->y;
                                }

                                int offsetX = (int)controlPosX%gridSnapDelta;
                                int offsetY = (int)controlPosY%gridSnapDelta;

                                if (precisionEditMode)
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

                                if (showGlobalPosition && (layout->controls[selectedControl].ap->id != 0))
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

                                    if (showGlobalPosition && (layout->controls[selectedControl].ap->id != 0))
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
            if (!CheckCollisionPointRec(mouse, (Rectangle){ 0, 0, GetScreenWidth(), 40 }) && 
                (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)))
            {
                selectedAnchor = focusedAnchor;
                if (anchorLinkMode) selectedAnchor = -1;
            }

            // Actions with one anchor selected
            if ((selectedAnchor != -1) && !CheckCollisionPointRec(mouse, (Rectangle){ 0, 0, GetScreenWidth(), 40 }))
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
                                if (precisionEditMode)
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

                        if (mainToolbarState.snapModeActive)
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
                            if (precisionEditMode)
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

                            if (precisionEditMode)
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
                            if (precisionEditMode)
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

        // Reset program logic
        //----------------------------------------------------------------------------------------------
        if (resetLayout)
        {
            focusedAnchor = -1;
            selectedAnchor = -1;
            focusedControl = -1;
            selectedControl = -1;

            resizeMode = false;
            dragMoveMode = false;
            precisionEditMode = false;
            nameEditMode = false;
            textEditMode = false;
            showIconPanel = false;

            ResetLayout(layout);

            strcpy(inFileName, "\0");
            SetWindowTitle(TextFormat("%s v%s", toolName, toolVersion));

            for (int i = 0; i < MAX_UNDO_LEVELS; i++) memcpy(&undoLayouts[i], layout, sizeof(GuiLayout));
            currentUndoIndex = 0;
            firstUndoIndex = 0;

            resetLayout = false;
        }
        
        // WARNING: If any window is shown, cancel any edition mode
        if (windowAboutState.windowActive ||
            windowSponsorState.windowActive ||
            windowCodegenState.windowActive ||
            windowResetActive ||
            windowExitActive ||
            windowHelpActive ||
            showLoadFileDialog ||
            showSaveFileDialog ||
            showExportFileDialog)
        {
            nameEditMode = false;
            textEditMode = false;
            resizeMode = false;
            dragMoveMode = false;
            precisionEditMode = false;

            windowOverActive = true;        // There is some window overlap!
        }
        else windowOverActive = false;

        // WARNING: Some windows should lock the main screen controls when shown
        if (windowOverActive) GuiLock();
        else GuiUnlock();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

            // Draw background grid
            if (mainToolbarState.showGridActive) GuiGrid(workArea, NULL, gridSpacing*gridSubdivisions, gridSubdivisions);

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
                        if (showGlobalPosition) positionColor = RED;
                        if (mainToolbarState.snapModeActive) positionColor = LIME;
                        if (!dragMoveMode && precisionEditMode) positionColor = BLUE;
                        DrawText(TextFormat("[%i, %i, %i, %i]",
                                            (int)tracemapRec.x - (int)workArea.x,
                                            (int)tracemapRec.y - (int)workArea.y,
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
                            GuiFade(0.7f);
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
                            GuiPanel(rec, (layout->controls[i].text[0] == '\0')? NULL : layout->controls[i].text);
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
                        case GUI_SCROLLPANEL: GuiScrollPanel(rec, (layout->controls[i].text[0] == '\0') ? NULL : layout->controls[i].text, rec, NULL); break;
                        case GUI_LISTVIEW: GuiListView(rec, layout->controls[i].text, &listViewScrollIndex, listViewActive); break;
                        case GUI_COLORPICKER: GuiColorPicker(rec, (layout->controls[i].text[0] == '\0') ? NULL : layout->controls[i].text, RED); break;
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
                if (CheckCollisionPointRec(mouse, workArea) &&
                    !CheckCollisionPointRec(mouse, windowControlsPaletteState.windowBounds))
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
                                    case GUI_TOGGLEGROUP: GuiToggleGroup((Rectangle){ defaultRec[selectedType].x, defaultRec[selectedType].y, defaultRec[selectedType].width/3, defaultRec[selectedType].height }, "ONE;TWO;THREE", 1); break;
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
                                if (mainToolbarState.snapModeActive) positionColor = LIME;
                                DrawText(TextFormat("[%i, %i, %i, %i]", 
                                    (int)defaultRec[selectedType].x - (int)workArea.x, 
                                    (int)defaultRec[selectedType].y - (int)workArea.y,
                                    (int)defaultRec[selectedType].width, 
                                    (int)defaultRec[selectedType].height),
                                    (int)defaultRec[selectedType].x, ((int)defaultRec[selectedType].y < ((int)workArea.y + 8))? (int)defaultRec[selectedType].y + 30 : (int)defaultRec[selectedType].y - 30, 20, Fade(positionColor, 0.5f));

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
                    if (mainToolbarState.snapModeActive) positionColor = LIME;
                    if (!dragMoveMode && precisionEditMode) positionColor = BLUE;

                    if (selectedAnchor > 0)
                    {
                        DrawText(TextFormat("[%i, %i]",
                            (int)(layout->anchors[selectedAnchor].x - layout->refWindow.x - (int)workArea.x),
                            (int)(layout->anchors[selectedAnchor].y - layout->refWindow.y - (int)workArea.y)),
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
                    if (showGlobalPosition) positionColor = RED;
                    if (mainToolbarState.snapModeActive) positionColor = LIME;
                    if (!dragMoveMode && precisionEditMode) positionColor = BLUE;

                    if (!showGlobalPosition)
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
                            (int)(selectedRec.x - layout->refWindow.x - workArea.x),
                            (int)(selectedRec.y - layout->refWindow.y - workArea.y),
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
                            selectedIcon = GuiToggleGroup((Rectangle){ (int)textboxRec.x, (int)textboxRec.y + (int)textboxRec.height + 10, 18, 18 }, toggleIconsText, selectedIcon);
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
                GuiWindowControlsPalette(&windowControlsPaletteState);

                mainToolbarState.showControlPanelActive = windowControlsPaletteState.windowActive;

                // Update ScrollPanel bounds in case window is resized
                windowControlsPaletteState.scrollPanelBounds = (Rectangle){ GetScreenWidth() - 170, workArea.y, 170, GetScreenHeight() - workArea.y - 24 };
                //----------------------------------------------------------------------------------------
            }
            
            // NOTE: If some overlap window is open and main window is locked, we draw a background rectangle
            if (GuiIsLocked()) DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), 0.85f));

            // WARNING: Before drawing the windows, we unlock them
            GuiUnlock();
            
            // GUI: Main toolbar panel
            //----------------------------------------------------------------------------------
            mainToolbarState.controlSelected = (selectedControl >= 0);
            mainToolbarState.anchorSelected = (selectedAnchor >= 0);
            mainToolbarState.tracemapSelected = tracemapSelected;

            GuiMainToolbar(&mainToolbarState);
            //----------------------------------------------------------------------------------
            
            // GUI: About Window
            //----------------------------------------------------------------------------------------
            GuiWindowAbout(&windowAboutState);
            //----------------------------------------------------------------------------------------
            
            // GUI: Sponsor Window
            //----------------------------------------------------------------------------------------
            GuiWindowSponsor(&windowSponsorState);
            //----------------------------------------------------------------------------------------

            // GUI: Help Window
            //----------------------------------------------------------------------------------------
            Rectangle helpWindowBounds = { (float)GetScreenWidth()/2 - 330/2, (float)GetScreenHeight()/2 - 700.0f/2, 330, 0 };
            if (windowHelpActive) windowHelpActive = GuiWindowHelp(helpWindowBounds, GuiIconText(ICON_HELP, TextFormat("%s Shortcuts", TOOL_NAME)), helpLines, HELP_LINES_COUNT);
            //----------------------------------------------------------------------------------------

            // GUI: Layout Code Generation Window
            //----------------------------------------------------------------------------------------
            GuiWindowCodegen(&windowCodegenState);

            if (windowCodegenState.btnGenerateCodePressed)
            {
                showExportFileDialog = true;
                windowCodegenState.windowActive = false;
                windowCodegenState.btnGenerateCodePressed = false;
            }
            //----------------------------------------------------------------------------------------

            // GUI: New Layout Window (save)
            //----------------------------------------------------------------------------------------
            if (windowResetActive)
            {
                int message = GuiMessageBox((Rectangle){ GetScreenWidth()/2 - 120, GetScreenHeight()/2 - 48, 248, 96 }, "Creating new layout", "Do you want to save the current layout?", "Yes;No");

                if (message == 0) windowResetActive = false;
                else if (message == 1)  // Yes
                {
                    showSaveFileDialog = true;
                    resetLayout = true;
                    windowResetActive = false;
                }
                else if (message == 2)  // No
                {
                    resetLayout = true;
                    windowResetActive = false;
                }
            }
            //----------------------------------------------------------------------------------------

            // GUI: Exit Window
            //----------------------------------------------------------------------------------------
            if (windowExitActive)
            {
                int result = GuiMessageBox((Rectangle){ GetScreenWidth()/2 - 320/2, GetScreenHeight()/2 - 120/2, 320, 120 }, "#159#Closing rGuiLayout", "Do you want to close without saving?", "Yes;No");

                if ((result == 0) || (result == 1)) closeWindow = true;
                else if (result == 2) 
                {
                    showSaveFileDialog = true;
                    windowExitActive = false;
                }
            }
            //----------------------------------------------------------------------------------------

            // GUI: Status bar
            //--------------------------------------------------------------------------------------------
            GuiStatusBar((Rectangle){ 0, GetScreenHeight() - 24, 126, 24}, TextFormat("MOUSE: (%i, %i)", (int)mouse.x, (int)mouse.y));
            GuiStatusBar((Rectangle){ 124, GetScreenHeight() - 24, 81, 24}, (mainToolbarState.snapModeActive? "SNAP: ON" : "SNAP: OFF"));
            GuiStatusBar((Rectangle){ 204, GetScreenHeight() - 24, 145, 24}, TextFormat("CONTROLS COUNT: %i", layout->controlCount));
            GuiStatusBar((Rectangle){ 348, GetScreenHeight() - 24, 100, 24}, TextFormat("GRID SIZE: %i", gridSpacing*gridSubdivisions));

            if (selectedControl != -1)
            {
                int defaultPadding = GuiGetStyle(STATUSBAR, TEXT_PADDING);
                int defaultTextAlign = GuiGetStyle(STATUSBAR, TEXT_ALIGNMENT);
                GuiSetStyle(STATUSBAR, TEXT_PADDING, 10);
                GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
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
            //----------------------------------------------------------------------------------------

            // GUI: Load File Dialog (and loading logic)
            //----------------------------------------------------------------------------------------
            if (showLoadFileDialog)
            {
#if defined(CUSTOM_MODAL_DIALOGS)
                int result = GuiFileDialog(DIALOG_MESSAGE, "Load raygui layout file ...", inFileName, "Ok", "Just drag and drop your .rgl layout file!");
#else
                int result = GuiFileDialog(DIALOG_OPEN_FILE, "Load raygui layout file", inFileName, "*.rgl", "raygui Layout Files (*.rgl)");
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
                //int result = GuiTextInputBox((Rectangle){ screenWidth/2 - 280/2, screenHeight/2 - 112/2 - 30, 280, 112 }, "#2#Save raygui style file...", NULL, "#2#Save", outFileName, 512, NULL);
#else
                int result = GuiFileDialog(DIALOG_SAVE_FILE, "Save raygui layout file...", outFileName, "*.rgl", "raygui Layout Files (*.rgl)");
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
                //if (windowCodegenState.codeTemplateActive == 0) strcpy(outFileName, TextFormat("gui_%s.c", config.name));
                //else if (windowCodegenState.codeTemplateActive == 1) strcpy(outFileName, TextFormat("gui_%s.h", config.name));

#if defined(CUSTOM_MODAL_DIALOGS)
                int result = GuiFileDialog(DIALOG_TEXTINPUT, "Export layout as code file...", outFileName, "Ok;Cancel", NULL);
#else
                int result = GuiFileDialog(DIALOG_SAVE_FILE, "Export layout as code file...", outFileName, "*.c;*.h", "Code Files");
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

    RL_FREE(undoLayouts);       // Free undo layouts array (allocated with RL_CALLOC)
    RL_FREE(windowCodegenState.codeText);  // Free loaded codeText memory

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module functions definition
//----------------------------------------------------------------------------------
#if defined(PLATFORM_DESKTOP)
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
            RL_FREE(guiTemplateCustom);
        }
        else toolstr = GenLayoutCode(guiTemplateStandardCode, *layout, config);

        FILE *ftool = fopen(outFileName, "wt");
        fprintf(ftool, toolstr);    // Write code string to file
        fclose(ftool);

        UnloadLayout(layout);
        RL_FREE(toolstr);
    }

    if (showUsageInfo) ShowCommandLineInfo();
}
#endif      // PLATFORM_DESKTOP

//--------------------------------------------------------------------------------------------
// Load/Save/Export data functions
//--------------------------------------------------------------------------------------------
// Load layout from text file
// NOTE: If NULL provided, an empty layout is initialized
static GuiLayout *LoadLayout(const char *fileName)
{
    GuiLayout *layout = (GuiLayout *)RL_CALLOC(1, sizeof(GuiLayout));

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
    #define RGL_FILE_VERSION_TEXT "3.0"

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

// Draw help window with the provided lines
static int GuiWindowHelp(Rectangle bounds, const char *title, const char **helpLines, int helpLinesCount)
{
    int nextLineY = 0;

    // Calculate window height if not externally provided a desired height
    if (bounds.height == 0) bounds.height = (float)(helpLinesCount*24 + 24);

    int windowHelpActive = !GuiWindowBox(bounds, title);
    nextLineY += (24 + 2);

    for (int i = 0; i < helpLinesCount; i++)
    {
        if (helpLines[i] == NULL) GuiLine((Rectangle){ bounds.x, bounds.y + nextLineY, 330, 12 }, helpLines[i]);
        else if (helpLines[i][0] == '-') GuiLine((Rectangle){ bounds.x, bounds.y + nextLineY, 330, 24 }, helpLines[i] + 1);
        else GuiLabel((Rectangle){ bounds.x + 12, bounds.y + nextLineY, bounds.width, 24 }, helpLines[i]);

        if (helpLines[i] == NULL) nextLineY += 12;
        else nextLineY += 24;
    }

    return windowHelpActive;
}