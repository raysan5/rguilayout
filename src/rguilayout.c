/*******************************************************************************************
*
*   rGuiLayout v4.1 - A simple and easy-to-use raygui layouts editor
*
*   FEATURES:
*       - 25 gui controls to define your immmediate-mode gui layout
*       - Place controls and move/scale them freely
*       - Snap to grid mode for maximum precission
*       - Link controls to anchors for better organization
*       - Edit controls text and code name (for exported variables)
*       - Icon selection panel on control text edition
*       - Load/Save your layouts as .rgl text files
*       - Supports .png image loading to be used as tracemap
*       - Support for custom code templates to generate your controls code
*       - Export layout directly as plain C code, ready to edit and compile
*       - Multiple GUI styles available with support for custom ones (.rgs)
*
*   DESIGN DECISIONS:
*       - Anchors can not reference other anchors, one control can only reference one anchor
*       - layout.anchor[0] is used as the refWindow reference position for all the controls and other anchors
*         it is used to define the [0, 0] of the application, a global offset, it's computed on every anchor/control on loading and saving
*
*   LIMITATIONS/NOTES:
*       - Code is a bit old and comboluted, it uses multiple flags to identify states (edit control, edit anchor, edit text...)
*         and controls/anchors are selected by index, probably using pointer would simplify some parts of the code
*
*   POSSIBLE IMPROVEMENTS:
*       - Support multiple controls selection -> Requires some redesing. Considerations:
*           > Can anchors be selected along controls?
*           > How to register selected elements? -> Use anchor.selected/control.selected properties?
*           > When drawing a rectangle for selection, consider partial contained elements?
*           > When a group of elements is selected, what can we do with it? Only move? Scale?
*           > Moving a group of elements, how affects anchors?
*       - Support multiple open layouts in TABS and/or same TAB? --> Export selected controls?
*       - Allow exporting layout as an image, including layout info as a PNG chunk?
*       - CLI: Support additional codegen options: exportAnchors, defineRecs, fullComments...
*
*   CONFIGURATION:
*       #define CUSTOM_MODAL_DIALOGS
*           Use custom raygui generated modal dialogs instead of native OS ones
*           NOTE: Avoids including tinyfiledialogs depencency library
*
*   VERSIONS HISTORY:
*       4.2  (xx-Nov-2024)  ADDED: New welcome/about window
*                           ADDED: Configuration file support: config.ini
*
*       4.1  (06-Apr-2024)  ADDED: Issue report window
*                           REMOVED: Sponsors window
*                           REVIEWED: Main toolbar and help window
*                           UPDATED: Using raylib 5.1-dev and raygui 4.1-dev
*
*       4.0  (26-Sep-2023)  ADDED: Support macOS builds (x86_64 + arm64)
*                           REMOVED: workArea hack for screen-space controls position
*                           REDESIGNED: Reference window definition (anchor[0])
*                           REVIEWED: Controls palette panel
*                           REVIEWED: Layout code export window
*                           REVIEWED: Layout code generator (.c/.h)
*                           REVIEWED: Layout loading and saving (.rgl)
*                           REVIEWED: Regenerated tool imagery
*                           UPDATED: Using raygui 4.0 and latest raylib 4.6-dev
*
*       3.1  (13-Dec-2022)  ADDED: Welcome window with sponsors info
*                           REDESIGNED: Main toolbar to add tooltips
*                           REVIEWED: Help window implementation
*
*       3.0  (26-Oct-2022)  ADDED: Main toolbar, consistent with other tools, many options!
*                           ADDED: View options: Control recs, names, layers order
*                           ADDED: Support custom code templates on code generation
*                           ADDED: Sponsor window for tools support
*                           ADDED: Multiple UI styles selection
*                           REVIEWED: Codegen window font and templates
*                           UPDATED: Using raylib 4.5-dev and raygui 3.5-dev
*                           Source code re-licensed as open-source
*
*       2.5  (05-Jan-2022)  UPDATED: Using raylib 4.0 and raygui 3.1
*       2.0  (15-Sep-2019)  Rewriten from scratch
*       1.0  (14-May-2018)  First release
*
*   DEPENDENCIES:
*       raylib 5.5-dev          - Windowing/input management and drawing
*       raygui 4.5-dev          - Immediate-mode GUI controls with custom styling and icons
*       rpng 1.5                - PNG chunks management
*       tinyfiledialogs 3.19.1  - Open/save file dialogs, it requires linkage with comdlg32 and ole32 libs
*
*   BUILDING:
*     - Windows (MinGW-w64):
*       gcc -o rguilayout.exe rguilayout.c external/tinyfiledialogs.c -s -Iexternal /
*           -lraylib -lopengl32 -lgdi32 -lcomdlg32 -lole32 -std=c99
*
*     - Linux (GCC):
*       gcc -o rguilayout rguilayout.c external/tinyfiledialogs.c -s -Iexternal -no-pie -D_DEFAULT_SOURCE /
*           -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
*
*   ADDITIONAL NOTES:
*       On PLATFORM_ANDROID and PLATFORM_WEB file dialogs are not available
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
*   Copyright (c) 2017-2025 raylib technologies (@raylibtech) / Ramon Santamaria (@raysan5)
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
#define TOOL_VERSION            "4.1"
#define TOOL_DESCRIPTION        "A simple and easy-to-use raygui layouts editor"
#define TOOL_DESCRIPTION_BREAK  "A simple and easy-to-use\nraygui layouts editor"
#define TOOL_RELEASE_DATE       "Apr.2024"
#define TOOL_LOGO_COLOR         0x7da9b9ff
#define TOOL_CONFIG_FILENAME    "rguilayout.ini"

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

// NOTE: Some redefines are required to support icons panel drawing
#define RAYGUI_GRID_ALPHA                 0.1f
#define RAYGUI_TEXTSPLIT_MAX_ITEMS        256
#define RAYGUI_TEXTSPLIT_MAX_TEXT_SIZE   4096
#define RAYGUI_TOGGLEGROUP_MAX_ITEMS      256
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                         // Required for: IMGUI controls

#undef RAYGUI_IMPLEMENTATION                // Avoid including raygui implementation again

#define GUI_MAIN_TOOLBAR_IMPLEMENTATION
#include "gui_main_toolbar.h"               // GUI: Main toolbar

#define GUI_WINDOW_HELP_IMPLEMENTATION
#include "gui_window_help.h"                // GUI: Help Window

#define GUI_WINDOW_ABOUT_IMPLEMENTATION
#include "gui_window_about.h"               // GUI: About Window

#define GUI_FILE_DIALOGS_IMPLEMENTATION
#include "gui_file_dialogs.h"               // GUI: File Dialogs

#include "rguilayout.h"                     // Gui layout types definition

#define GUI_PANEL_CONTROLS_PALETTE_IMPLEMENTATION
#include "gui_panel_controls_palette.h"    // GUI: Controls Palette

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

#define RINI_IMPLEMENTATION
#include "external/rini.h"                  // Config file values reader/writer

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

#define MOVEMENT_FRAME_SPEED         2      // Controls movement speed in pixels per frame

#define MAX_UNDO_LEVELS             64      // Undo levels supported for the ring buffer
#define UNDO_SNAPSHOT_FRAMERATE     90      // Frames to take a new layout snapshot (if changes done), @60 fps = 1.5sec

#define MAX_ICONS_AVAILABLE        220      // Max raygui icons displayed on icon selector

#define MAX_ELEMENTS_SELECTION      64      // Max elements selected

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// NOTE: Most of the structs are defined in "rguilayout.h"

// Tracemap type
typedef struct {
    Texture2D texture;
    Rectangle rec;

    bool focused;
    bool selected;
    bool visible;
    bool locked;
    float alpha;
} Tracemap;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const char *toolName = TOOL_NAME;
static const char *toolVersion = TOOL_VERSION;
static const char *toolDescription = TOOL_DESCRIPTION;

// NOTE: Max length depends on OS, in Windows MAX_PATH = 256
static char inFileName[512] = { 0 };        // Input file name (required in case of drag & drop over executable)
static char outFileName[512] = { 0 };       // Output file name (required for file save/export)

static bool saveChangesRequired = false;    // Flag to notice save changes are required

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

//----------------------------------------------------------------------------------
// Program main entry point
//----------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
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
    const int screenHeight = 720;

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

    bool showWindowActive = false;          // Check for any blocking window active

    // Grid control variables
    int gridSpacing = 8;                    // Grid minimum spacing in pixels (between every subdivision)
    int gridSubdivisions = 3;               // Grid subdivisions (total size for every big line is gridSpacing*gridSubdivisions)
    int gridSnapDelta = 1;                  // Grid snap minimum value in pixels
    int moveFrameCounter = 0;               // Movement frames counter
    int moveFrameSpeed = 1;                 // Movement speed per frame

    // Control edit modes
    // NOTE: [E] - Exclusive mode operation, all other modes blocked
    bool dragMoveMode = false;              // [E] Control drag mode
    bool resizeMode = false;                // [E] Control size mode ((controlSelected != -1) + KEY_LEFT_ALT)
    bool mouseScaleMode = false;            // [E] Control is being scaled by mouse
    bool mouseScaleReady = false;           // Mouse is on position to start control scaling
    bool textEditMode = false;              // [E] Control text edit mode (KEY_T)
    bool nameEditMode = false;              // [E] Control name edit mode (KEY_N)
    bool orderLayerMode = false;            // Control order edit mode ((focusedControl != -1) + KEY_LEFT_ALT)
    bool precisionEditMode = false;         // Control precision edit mode (KEY_LEFT_SHIFT)

    // Anchor edit modes
    bool anchorEditMode = false;            // [E] Anchor edition mode
    bool anchorLinkMode = false;            // [E] Anchor linkage mode
    bool anchorMoveMode = false;            // [E] Anchor move mode

    // Ref window edit mode
    bool refWindowEditMode = false;         // [E] Refence window edit mode

    // Toggle global controls position info
    // NOTE: It could be useful at some moment (but not usual)
    bool showGlobalPosition = false;        // Control global position mode

    // Controls variables
    int selectedControl = -1;               // Control selected on layout
    int focusedControl = -1;                // Control focused on layout
    int selectedType = GUI_WINDOWBOX;       // Control type selected on panel

    // Anchors variables
    int selectedAnchor = -1;
    int focusedAnchor = -1;
    GuiAnchorPoint auxAnchor = { MAX_ANCHOR_POINTS, 0, 0, 0 };  // Auxiliar anchor, required to move anchors without moving controls

    Vector2 panOffset = { 0 };
    Vector2 prevPosition = { 0 };

    // Multiselection variables
    bool multiSelectMode = false;           // [E] Multiselection mode
    Rectangle multiSelectRec = { 0 };
    //Vector2 multiSelectStartPos = { 0 };
    //int multiSelectControls[MAX_ELEMENTS_SELECTION] = { -1 };
    //int multiSelectCount = 0;

    // Define colors to be aligned with style selected
    // Colors used for the different modes, states and elements actions
    //-------------------------------------------------------------------------------------------------
    Color colEditControlTextOverlay = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED));  // Control text edit mode, screen overlay (Fade: 0.2f)
    Color colEditControlNameOverlay = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_PRESSED));  // Control name edit mode, screen overlay (Fade: 0.2f)
    Color colEditControlNameBackRec = WHITE;    // Control name edit mode, back rectangle
    Color colEditAnchorNameOverlay = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_PRESSED));   // Anchor name edit mode, screen overlay (Fade: 0.2f)
    Color colShowControlRecs = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED));         // Control rectangles mode (Fade: 0.2f / Line: Fade: 0.7f)

    //Color colControlCreationCursor = RED;       // Control creation cursor (NOT USED)
    Color colControlFocused = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED));        // Control focused (mouse over it)
    Color colControlSelected = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_PRESSED));       // Control selected
    Color colControlSelectedResize = BLUE;      // Control resize mode (keyboard, RCTRL + ARROWS)
    Color colControlRecTextDefault = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED)); // Control position text (no snap mode)
    Color colControlRecTextSnap = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_PRESSED));    // Control position text (snap mode)
    Color colControlRecTextGlobal = MAROON;     // Control position text (global pos)
    Color colControlRecTextPrecision = BLUE;    // Control position text (precision mode - RSHIFT)

    Color colAnchorCreation = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED));  // Anchor creation cursor (A)
    Color colAnchorDefault = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED));         // Anchor default (not focused or selected)
    Color colAnchorFocused = GetColor(GuiGetStyle(BUTTON, TEXT_COLOR_PRESSED));           // Anchor focused (not filling)
    Color colAnchorSelected = GetColor(GuiGetStyle(BUTTON, BASE_COLOR_PRESSED));          // Anchor selected (with filling)
    Color colAnchorEditMode = ORANGE;           // Anchor selected and edit mode (A over focused anchor)
    Color colAnchorLinkLine = GetColor(GuiGetStyle(BUTTON, TEXT_COLOR_PRESSED));          // Anchor link lines

    Color colAnchor0 = DARKGRAY;                // Anchor 0 (refWindow)
    Color colAnchorLinkLine0 = GetColor(GuiGetStyle(BUTTON, TEXT_COLOR_DISABLED));        // Anchor 0 link lines (refWindow)

    Color colAnchorHidden = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_DISABLED));         // Anchor hidden controls mode
    Color colAnchorLinkLineHidden = GetColor(GuiGetStyle(BUTTON, TEXT_COLOR_DISABLED));   // Anchor hidden control link lines

    Color colRefWindow = BLACK;                 // Ref Window rectangle (Fade: 0.1f)
    Color colRefWindowText = DARKGRAY;          // Ref Window position text

    Color colTracemapFocused = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED));       // Tracemap focused (base + lines)
    Color colTracemapSelected = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_PRESSED));      // Tracemap selected (base + lines)
    Color colTracemapLocked = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_DISABLED));       // Tracemap locked (border lines)
    Color colTracemapResize = BLUE;             // Tracemap resize mode (keyboard, RCTRL + ARROWS)
    //-------------------------------------------------------------------------------------------------

    // Init default layout
    //-------------------------------------------------------------------------
    GuiLayout *layout = NULL;

    if (inFileName[0] != '\0')          // Load dropped file if provided
    {
        // NOTE: Anchors and control screen offset is already considered by refWindow (anchor[0])
        layout = LoadLayout(inFileName);

        SetWindowTitle(TextFormat("%s v%s | File: %s", toolName, toolVersion, GetFileName(inFileName)));
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
    Tracemap tracemap = { 0 };
    tracemap.visible = true;
    tracemap.alpha = 0.7f;

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
    char toggleIconsText[MAX_ICONS_AVAILABLE*6] = { 0 };  // 16 icons per line

    for (int i = 0; i < MAX_ICONS_AVAILABLE; i++)
    {
        // NOTE: Every icon requires 6 text characters: "#001#;"
        if ((i + 1)%16 == 0) strncpy(toggleIconsText + 6*i, TextFormat("#%03i#\n", i), 6);
        else strncpy(toggleIconsText + 6*i, TextFormat("#%03i#;", i), 6);
    }

    toggleIconsText[MAX_ICONS_AVAILABLE*6 - 1] = '\0';
    //-------------------------------------------------------------------------

    // Layout code generation configuration
    //------------------------------------------------------------------------------------
    GuiLayoutConfig guiConfig = { 0 };
    strcpy(guiConfig.name, "window_codegen");
    strcpy(guiConfig.version, toolVersion);
    strcpy(guiConfig.company, "raylib technologies");
    strcpy(guiConfig.description, "tool description");
    guiConfig.template = NULL;
    guiConfig.exportAnchors = false;
    guiConfig.defineRecs = false;
    guiConfig.defineTexts = false;
    guiConfig.fullComments = false;
    guiConfig.exportButtonFunctions = false;

    GuiLayoutConfig prevGuiConfig = { 0 };
    memcpy(&prevGuiConfig, &guiConfig, sizeof(GuiLayoutConfig));
    //------------------------------------------------------------------------------------

    // GUI: Main toolbar panel (file and visualization)
    //-----------------------------------------------------------------------------------
    GuiMainToolbarState mainToolbarState = InitGuiMainToolbar();
    //-----------------------------------------------------------------------------------

    // GUI: Help panel
    //-----------------------------------------------------------------------------------
    GuiWindowHelpState windowHelpState = InitGuiWindowHelp();
    //-----------------------------------------------------------------------------------

    // GUI: About Window
    //-----------------------------------------------------------------------------------
    GuiWindowAboutState windowAboutState = InitGuiWindowAbout();
    //-----------------------------------------------------------------------------------

    // GUI: Issue Report Window
    //-----------------------------------------------------------------------------------
    bool showIssueReportWindow = false;
    //-----------------------------------------------------------------------------------

    // GUI: Controls Selection Palette
    //-----------------------------------------------------------------------------------
    GuiPanelControlsPaletteState windowControlsPaletteState = InitGuiPanelControlsPalette();

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

    // GUI: Exit Window
    //-----------------------------------------------------------------------------------
    bool showExitWindow = false;
    bool closeWindow = false;
    //-----------------------------------------------------------------------------------

    // GUI: Reset Layout Window
    //-----------------------------------------------------------------------------------
    bool showResetWindow = false;
    bool resetLayout = false;
    //-----------------------------------------------------------------------------------

    // GUI: Custom file dialogs
    //-----------------------------------------------------------------------------------
    bool showLoadFileDialog = false;
    bool showSaveFileDialog = false;
    bool showExportFileDialog = false;
    bool showLoadTracemapDialog = false;
    bool showLoadTemplateDialog = false;
    //-----------------------------------------------------------------------------------

#if defined(PLATFORM_DESKTOP)
    // Read application init configuration (if available)
    //-------------------------------------------------------------------------------------
    int windowMaximized = 0;
    if (FileExists(TextFormat("%s/%s", GetApplicationDirectory(), TOOL_CONFIG_FILENAME)))
    {
        rini_data appConfig = rini_load(TextFormat("%s/%s", GetApplicationDirectory(), TOOL_CONFIG_FILENAME));

        // Load required config variables
        // NOTE: Keys not found default to 0 value, unless fallback is requested
        //windowAboutState.showSplash = rini_get_value(config, "SHOW_WINDOW_WELCOME");
        mainToolbarState.showControlPanelActive = rini_get_value(appConfig, "SHOW_WINDOW_CONTROLS");
        mainToolbarState.showTooltips = rini_get_value_fallback(appConfig, "SHOW_CONTROL_TOOLTIPS", 1); // Default to 1 if key not found
        mainToolbarState.showGridActive = rini_get_value(appConfig, "SHOW_IMAGE_GRID");
        gridSpacing = rini_get_value(appConfig, "GRID_LINES_SPACING");
        windowMaximized = rini_get_value(appConfig, "INIT_WINDOW_MAXIMIZED");
        mainToolbarState.visualStyleActive = rini_get_value(appConfig, "GUI_VISUAL_STYLE");
        //mainToolbarState.cleanModeActive = rini_get_value(config, "CLEAN_WINDOW_MODE");

        rini_unload(&appConfig);

        // NOTE: Config is automatically saved when application is closed
    }

    // Setup application using config values (or default)
    //if (windowAboutState.showSplash) { windowAboutState.welcomeMode = true; windowAboutState.windowActive = true; }
    //else { windowAboutState.welcomeMode = false; windowAboutState.windowActive = false; }
    if (mainToolbarState.showTooltips) GuiEnableTooltip();
    else GuiDisableTooltip();

    if (gridSpacing == 0) gridSpacing = 16;
    if (windowMaximized == 1) MaximizeWindow();
    //-------------------------------------------------------------------------------------
#endif

    SetTargetFPS(60);       // Set our game desired framerate
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!closeWindow)    // Detect window close button
    {
        // Basic program flow logic
        //----------------------------------------------------------------------------------
        //framesCounter++;                // General usage frames counter
        mouse = GetMousePosition();     // Get mouse position each frame

        // WARNING: ASINCIFY requires this line,
        // it contains the call to emscripten_sleep() for PLATFORM_WEB
        if (WindowShouldClose()) showExitWindow = true;
        //----------------------------------------------------------------------------------

        // Undo layout change logic
        //----------------------------------------------------------------------------------
        // Every second check if current layout has changed and record a new undo state
        if (!dragMoveMode && !orderLayerMode && !resizeMode && !refWindowEditMode &&
            !textEditMode && !showIconPanel && !nameEditMode && !anchorEditMode && !anchorLinkMode && !anchorMoveMode)
        {
            undoFrameCounter++;

            if (undoFrameCounter >= UNDO_SNAPSHOT_FRAMERATE)
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
                        SetWindowTitle(TextFormat("%s v%s | File: %s*", toolName, toolVersion, GetFileName(inFileName)));
                        saveChangesRequired = true;
                    }
                }

                undoFrameCounter = 0;
            }
        }
        else undoFrameCounter = 120;

        // Recover previous layout state from buffer
        if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z)) || mainToolbarState.btnUndoPressed)
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
        if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Y)) || mainToolbarState.btnRedoPressed)
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

                    // WARNING: When layout is loaded, anchor object references are not set, they must be reset manually
                    for (int i = 0; i < layout->controlCount; i++) layout->controls[i].ap = &layout->anchors[tempLayout->controls[i].ap->id];

                    strcpy(inFileName, droppedFiles.paths[0]);
                    SetWindowTitle(TextFormat("%s v%s | File: %s", toolName, toolVersion, GetFileName(inFileName)));

                    for (int i = 0; i < MAX_UNDO_LEVELS; i++) memcpy(&undoLayouts[i], layout, sizeof(GuiLayout));
                    currentUndoIndex = 0;
                    firstUndoIndex = 0;

                    UnloadLayout(tempLayout);
                }
            }
            else if (IsFileExtension(droppedFiles.paths[0], ".png")) // Tracemap image
            {
                if (tracemap.texture.id > 0) UnloadTexture(tracemap.texture);
                tracemap.texture = LoadTexture(droppedFiles.paths[0]);
                tracemap.rec = (Rectangle){ GetMouseX() - tracemap.texture.width/2, GetMouseY() - tracemap.texture.height/2, tracemap.texture.width, tracemap.texture.height};
                tracemap.visible = true;
                tracemap.alpha = 0.7f;

                mainToolbarState.tracemapLoaded = true;
            }
            else if (IsFileExtension(droppedFiles.paths[0], ".rgs")) GuiLoadStyle(droppedFiles.paths[0]);

            UnloadDroppedFiles(droppedFiles);   // Unload filepaths from memory
        }
        //----------------------------------------------------------------------------------

        // Keyboard shortcuts
        //----------------------------------------------------------------------------------
        // Show window: load layout
        if (mainToolbarState.btnNewFilePressed || mainToolbarState.btnCloseFilePressed) showResetWindow = true;

        // Show window: load layout
        if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O)) || mainToolbarState.btnLoadFilePressed) showLoadFileDialog = true;

        // Show dialog: save layout
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_S)) showSaveFileDialog = true;   // SaveAs dialog
        else if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) || mainToolbarState.btnSaveFilePressed)
        {
            if (inFileName[0] == '\0')
            {
                strcpy(outFileName, "layout_name.rgl");
                showSaveFileDialog = true;
            }
            else
            {
                GuiLayout outLayout = { 0 };
                memcpy(&outLayout, layout, sizeof(GuiLayout));

                SaveLayout(&outLayout, inFileName);

                SetWindowTitle(TextFormat("%s v%s | File: %s", toolName, toolVersion, GetFileName(inFileName)));
                saveChangesRequired = false;
            }
        }

        // Show dialog: export layout as code
        if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E)) || mainToolbarState.btnExportFilePressed)
        {
            // Setup code generation config parameters
            strcpy(guiConfig.name, windowCodegenState.toolNameText);
            strcpy(guiConfig.version, windowCodegenState.toolVersionText);
            strcpy(guiConfig.company, windowCodegenState.companyText);
            strcpy(guiConfig.description, windowCodegenState.toolDescriptionText);
            guiConfig.exportAnchors = windowCodegenState.exportAnchorsChecked;
            guiConfig.defineRecs = windowCodegenState.defineRecsChecked;
            guiConfig.defineTexts = windowCodegenState.defineTextsChecked;
            guiConfig.fullComments = windowCodegenState.fullCommentsChecked;
            guiConfig.exportButtonFunctions = windowCodegenState.genButtonFuncsChecked;

            // Select desired code template for generation
            if (windowCodegenState.codeTemplateActive == 0) guiConfig.template = guiTemplateStandardCode;
            else if (windowCodegenState.codeTemplateActive == 1) guiConfig.template = guiTemplateHeaderOnly;
            else if (windowCodegenState.codeTemplateActive == 2) guiConfig.template = windowCodegenState.customTemplate;

            // Clear current codeText and generate new layout code
            RL_FREE(windowCodegenState.codeText);
            if (guiConfig.template != NULL) windowCodegenState.codeText = GenLayoutCode(guiConfig.template, layout, (Vector2){ 0.0f, 0.0f }, guiConfig);

            // Store current config as prevConfig
            memcpy(&prevGuiConfig, &guiConfig, sizeof(GuiLayoutConfig));

            // Activate code generation export window
            windowCodegenState.codePanelScrollOffset = (Vector2){ 0, 0 };
            windowCodegenState.windowActive = true;
        }

        // If codegen window is active, keep checking for changes for code regeneration
        if (windowCodegenState.windowActive)
        {
            // Setup code generation config parameters
            strcpy(guiConfig.name, windowCodegenState.toolNameText);
            strcpy(guiConfig.version, windowCodegenState.toolVersionText);
            strcpy(guiConfig.company, windowCodegenState.companyText);
            strcpy(guiConfig.description, windowCodegenState.toolDescriptionText);
            guiConfig.exportAnchors = windowCodegenState.exportAnchorsChecked;
            guiConfig.defineRecs = windowCodegenState.defineRecsChecked;
            guiConfig.defineTexts = windowCodegenState.defineTextsChecked;
            guiConfig.fullComments = windowCodegenState.fullCommentsChecked;
            guiConfig.exportButtonFunctions = windowCodegenState.genButtonFuncsChecked;

            // Select desired code template for generation
            if (windowCodegenState.codeTemplateActive == 0) guiConfig.template = guiTemplateStandardCode;
            else if (windowCodegenState.codeTemplateActive == 1) guiConfig.template = guiTemplateHeaderOnly;
            else if (windowCodegenState.codeTemplateActive == 2) guiConfig.template = windowCodegenState.customTemplate;

            // Check if config parameter have changed while codegen window is open to regenerate code
            if (memcmp(&prevGuiConfig, &guiConfig, sizeof(GuiLayoutConfig)) != 0)
            {
                // Clear current codeText and generate new layout code
                RL_FREE(windowCodegenState.codeText);
                windowCodegenState.codeText = GenLayoutCode(guiConfig.template, layout, (Vector2){ 0.0f, 0.0f }, guiConfig);

                // Store current config as prevConfig
                memcpy(&prevGuiConfig, &guiConfig, sizeof(GuiLayoutConfig));
            }
        }

        // Show dialog: load tracemap image
        if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_O)) || mainToolbarState.btnLoadTracemapPressed) showLoadTracemapDialog = true;

        // Toggle window: help
        if (IsKeyPressed(KEY_F1)) windowHelpState.windowActive = !windowHelpState.windowActive;

        // Toggle window: about
        if (IsKeyPressed(KEY_F2)) windowAboutState.windowActive = !windowAboutState.windowActive;

        // Toggle window: issue report
        if (IsKeyPressed(KEY_F3)) showIssueReportWindow = !showIssueReportWindow;

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
            else    // Close windows logic
            {
                if (windowAboutState.windowActive) windowAboutState.windowActive = false;
                if (showIssueReportWindow) showIssueReportWindow = false;
                else if (windowCodegenState.windowActive) windowCodegenState.windowActive = false;
                else if (showResetWindow) showResetWindow = false;
                else if (showExitWindow) showExitWindow = false;
#if !defined(PLATFORM_WEB)
                else if ((layout->controlCount <= 0) && (layout->anchorCount <= 1)) closeWindow = true;
                else
                {
                    showExitWindow = !showExitWindow;
                    selectedControl = -1;
                    selectedAnchor = -1;
                }
#endif
            }
        }

        // Check no blocking mode enabled (active window | text edition | name edition)
        // to check the main keyboard shortcuts
        if (!showWindowActive && !textEditMode && !nameEditMode)
        {
            // Toggle Grid mode
            if (IsKeyPressed(KEY_G)) mainToolbarState.showGridActive = !mainToolbarState.showGridActive;

            // Toggle Snap to grid mode
            if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_S))
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

            // Toggle controls rectangles view
            if (IsKeyPressed(KEY_R)) mainToolbarState.showControlRecsActive = !mainToolbarState.showControlRecsActive;

            // Toggle controls name view
            if (IsKeyPressed(KEY_N) && (selectedControl == -1) && (selectedAnchor == -1)) mainToolbarState.showControlNamesActive = !mainToolbarState.showControlNamesActive;

            // Toggle controls order drawing view
            if (IsKeyPressed(KEY_L))
            {
                mainToolbarState.showControlOrderActive = !mainToolbarState.showControlOrderActive;
                orderLayerMode = mainToolbarState.showControlOrderActive;
            }

            // Toggle global position info (anchor reference or global reference)
            if (IsKeyPressed(KEY_F)) showGlobalPosition = !showGlobalPosition;

            if (IsKeyPressed(KEY_H) && tracemap.selected) mainToolbarState.hideTracemapActive = true;

            // Toggle anchor mode editing (on key down)
            anchorEditMode = IsKeyDown(KEY_A);

            // Toggle precision move/scale mode
            precisionEditMode = IsKeyDown(KEY_LEFT_SHIFT);

            // Toggle control resize mode
            resizeMode = IsKeyDown(KEY_LEFT_CONTROL);

            // Toggle tracemap lock mode
            if (IsKeyPressed(KEY_SPACE) && mainToolbarState.tracemapLoaded)
            {
                if (tracemap.selected) tracemap.locked = true;
                else if (tracemap.locked) tracemap.locked = false;
            }

            // Check modes requiring LEFT_CONTROL modifier
            if (IsKeyDown(KEY_LEFT_CONTROL))
            {
                // Open reset window
                if (IsKeyPressed(KEY_N)) showResetWindow = true;
            }

            // Grid spacing customization
            // TODO: Expose grid spacing config in a visual way
            if (IsKeyDown(KEY_RIGHT_ALT))
            {
                if (IsKeyPressed(KEY_RIGHT)) gridSubdivisions++;
                else if (IsKeyPressed(KEY_LEFT)) gridSubdivisions--;

                if (IsKeyPressed(KEY_UP)) gridSpacing++;
                else if (IsKeyPressed(KEY_DOWN)) gridSpacing--;

                gridSnapDelta = gridSpacing;
            }
        }
        //----------------------------------------------------------------------------------

        // Main toolbar logic
        //----------------------------------------------------------------------------------
        windowControlsPaletteState.panelActive = mainToolbarState.showControlPanelActive;

        //styleFrameCounter++;
        //if ((styleFrameCounter%120) == 0) mainToolbarState.visualStyleActive++;
        //if (mainToolbarState.visualStyleActive > 11) mainToolbarState.visualStyleActive = 0;

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

            // Update colors for the style
            // Colors used for the different modes, states and elements actions
            //-------------------------------------------------------------------------------------------------
            colEditControlTextOverlay = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED));  // Control text edit mode, screen overlay (Fade: 0.2f)
            colEditControlNameOverlay = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_PRESSED));  // Control name edit mode, screen overlay (Fade: 0.2f)
            colEditControlNameBackRec = WHITE;    // Control name edit mode, back rectangle
            colEditAnchorNameOverlay = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_PRESSED));   // Anchor name edit mode, screen overlay (Fade: 0.2f)
            colShowControlRecs = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED));         // Control rectangles mode (Fade: 0.2f / Line: Fade: 0.7f)

            //colControlCreationCursor = RED;       // Control creation cursor (NOT USED)
            colControlFocused = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED));        // Control focused (mouse over it)
            colControlSelected = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_PRESSED));       // Control selected
            colControlSelectedResize = BLUE;      // Control resize mode (keyboard, RCTRL + ARROWS)
            colControlRecTextDefault = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED)); // Control position text (no snap mode)
            colControlRecTextSnap = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_PRESSED));    // Control position text (snap mode)
            colControlRecTextGlobal = MAROON;     // Control position text (global pos)
            colControlRecTextPrecision = BLUE;    // Control position text (precision mode - RSHIFT)

            colAnchorCreation = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED));  // Anchor creation cursor (A)
            colAnchorDefault = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED));         // Anchor default (not focused or selected)
            colAnchorFocused = GetColor(GuiGetStyle(BUTTON, BASE_COLOR_PRESSED));           // Anchor focused (not filling)
            colAnchorSelected = GetColor(GuiGetStyle(BUTTON, TEXT_COLOR_PRESSED));          // Anchor selected (with filling)
            colAnchorEditMode = ORANGE;           // Anchor selected and edit mode (A over focused anchor)
            colAnchorLinkLine = GetColor(GuiGetStyle(BUTTON, TEXT_COLOR_PRESSED));          // Anchor link lines

            colAnchor0 = DARKGRAY;                // Anchor 0 (refWindow)
            colAnchorLinkLine0 = GetColor(GuiGetStyle(BUTTON, TEXT_COLOR_DISABLED));        // Anchor 0 link lines (refWindow)

            colAnchorHidden = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_DISABLED));         // Anchor hidden controls mode
            colAnchorLinkLineHidden = GetColor(GuiGetStyle(BUTTON, TEXT_COLOR_DISABLED));   // Anchor hidden control link lines

            colRefWindow = BLACK;                 // Ref Window rectangle (Fade: 0.1f)
            colRefWindowText = DARKGRAY;          // Ref Window position text

            colTracemapFocused = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_FOCUSED));       // Tracemap focused (base + lines)
            colTracemapSelected = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_PRESSED));      // Tracemap selected (base + lines)
            colTracemapLocked = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_DISABLED));       // Tracemap locked (border lines)
            colTracemapResize = BLUE;             // Tracemap resize mode (keyboard, RCTRL + ARROWS)
            //-------------------------------------------------------------------------------------------------

            mainToolbarState.prevVisualStyleActive = mainToolbarState.visualStyleActive;
        }

        // Help options logic
        if (mainToolbarState.btnHelpPressed) windowHelpState.windowActive = true;
        if (mainToolbarState.btnAboutPressed) windowAboutState.windowActive = true;
        if (mainToolbarState.btnIssuePressed) showIssueReportWindow = true;
        //----------------------------------------------------------------------------------

        // Layout edition logic
        //----------------------------------------------------------------------------------------------
        // Check no blocking mode enabled (active window | text edition | name edition)
        if (!showWindowActive && !nameEditMode && !textEditMode)
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
            if (!CheckCollisionPointRec(mouse, windowControlsPaletteState.panelBounds))
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
                int offsetX = (int)defaultRec[selectedType].x%gridSnapDelta;
                int offsetY = (int)defaultRec[selectedType].y%gridSnapDelta;

                if (offsetX >= gridSpacing/2) defaultRec[selectedType].x += (gridSpacing - offsetX);
                else defaultRec[selectedType].x -= offsetX;

                if (offsetY >= gridSpacing/2) defaultRec[selectedType].y += (gridSpacing - offsetY);
                else defaultRec[selectedType].y -= offsetY;
            }

            // Check mouse is in the "work screen area" and not on toolbars/panels
            if (CheckCollisionPointRec(mouse, (Rectangle){ 0, 40, GetScreenWidth(), GetScreenHeight() - 64 }) &&
                !CheckCollisionPointRec(mouse, windowControlsPaletteState.panelBounds))
            {
                if (!dragMoveMode)
                {
                    focusedControl = -1;

                    // Focus control logic
                    if (!anchorEditMode && (focusedAnchor == -1))
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
                    if ((focusedAnchor == -1) &&
                        (selectedAnchor == -1) &&
                        (selectedControl == -1) &&
                        !multiSelectMode &&
                        !tracemap.focused &&
                        !tracemap.selected)
                    {
                        // Create control
                        if (!anchorEditMode && !anchorLinkMode)
                        {
                            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                            {
                                // Control basic configuration
                                layout->controls[layout->controlCount].id = layout->controlCount;
                                layout->controls[layout->controlCount].type = selectedType;
                                layout->controls[layout->controlCount].rec = defaultRec[selectedType];  // Use default rectangle for control creation

                                // NOTE: HACK: GuiToggleGroup() considers width per element instead of full control
                                if (layout->controls[layout->controlCount].type == GUI_TOGGLEGROUP)
                                {
                                    layout->controls[layout->controlCount].rec = (Rectangle){ defaultRec[selectedType].x, defaultRec[selectedType].y,
                                                                                              defaultRec[selectedType].width/3.0f, defaultRec[selectedType].height };
                                }

                                // Control text definition: default label
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

                                // Control text definition: control options
                                if ((layout->controls[layout->controlCount].type == GUI_TOGGLEGROUP)
                                    || (layout->controls[layout->controlCount].type == GUI_COMBOBOX)
                                    || (layout->controls[layout->controlCount].type == GUI_DROPDOWNBOX)
                                    || (layout->controls[layout->controlCount].type == GUI_LISTVIEW))
                                {
                                    strcpy(layout->controls[layout->controlCount].text, "ONE;TWO;THREE");
                                }

                                // Control name definition (type + count)
                                strcpy(layout->controls[layout->controlCount].name,
                                       TextFormat("%s%03i", controlTypeName[layout->controls[layout->controlCount].type], layout->controlCount));

                                // Control anchor point definition: Default to anchor[0] = (0, 0)
                                layout->controls[layout->controlCount].ap = &layout->anchors[0];

                                // Control anchor point redefinition if created inside a GuiWindowBox(),
                                // anchor the new control to the GuiWindowBox() anchor
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

                                // Create anchor for GuiWindowBox() or GuiGroupBox() controls
                                if ((layout->anchorCount < MAX_ANCHOR_POINTS) &&
                                    ((layout->controls[layout->controlCount].type == GUI_WINDOWBOX) ||
                                     (layout->controls[layout->controlCount].type == GUI_GROUPBOX)))
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
                    if (orderLayerMode)
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

                    if (CheckCollisionPointRec(GetMousePosition(), rec) &&
                        CheckCollisionPointRec(GetMousePosition(), (Rectangle){ rec.x + rec.width - SCALE_BOX_CORNER_SIZE,
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
                        if ((layout->controls[selectedControl].type == GUI_WINDOWBOX) &&
                            (rec.height < RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*2)) rec.height = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT*2;

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
                                if ((layout->controls[selectedControl].ap->id != 0) && IsKeyPressed(KEY_U))
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
                                    anchorLinkMode = true;          // Enable anchor link mode
                                }
                                else if (IsKeyReleased(KEY_T))      // Enable text edit mode
                                {
                                    strcpy(prevText, layout->controls[selectedControl].text);
                                    textBoxCursorIndex = (int)strlen(layout->controls[selectedControl].text);
                                    textEditMode = true;
                                }
                                else if (IsKeyReleased(KEY_N))      // Enable name edit mode
                                {
                                    strcpy(prevName, layout->controls[selectedControl].name);
                                    textBoxCursorIndex = (int)strlen(layout->controls[selectedControl].name);
                                    nameEditMode = true;
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

            // TODO: Controls multi-selection and edition logic
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
                        // TODO: Check if full control rec is contained in multiSelectRec
                        // WARNING: multiSelectRec is in screen-space, we must consider rec, anchor and refWindow!
                        if (CheckCollisionRecs(multiSelectRec, layout->controls[i].rec))
                        {
                            multiSelectControls[multiSelectCount] = i;
                            multiSelectCount++;
                        }
                    }

                    if (multiSelectCount == 0) multiSelectMode = false;
                    else
                    {
                        // Calculate multi-selection rectangle
                        multiSelectRec = (Rectangle){ 16384, 16384, 0, 0 };
                        Vector2 maxValue = { 0 };

                        for (int i = 0; i < multiSelectCount; i++)
                        {
                            // Get minimum value
                            if (layout->controls[multiSelectControls[i]].rec.x < multiSelectRec.x) multiSelectRec.x = layout->controls[multiSelectControls[i]].rec.x;
                            if (layout->controls[multiSelectControls[i]].rec.y < multiSelectRec.y) multiSelectRec.y = layout->controls[multiSelectControls[i]].rec.y;

                            // Get maximum value
                            if ((layout->controls[multiSelectControls[i]].rec.x + layout->controls[multiSelectControls[i]].rec.width) > maxValue.x) maxValue.x = layout->controls[multiSelectControls[i]].rec.x + layout->controls[multiSelectControls[i]].rec.width;
                            if ((layout->controls[multiSelectControls[i]].rec.y + layout->controls[multiSelectControls[i]].rec.height) > maxValue.y) maxValue.y = layout->controls[multiSelectControls[i]].rec.y + layout->controls[multiSelectControls[i]].rec.height;
                            multiSelectRec.width = maxValue.x - multiSelectRec.x;
                            multiSelectRec.height = maxValue.y - multiSelectRec.y;
                        }

                        multiSelectMode = false;
                    }
                }
            }

            // Reset multi selection
            if ((multiSelectCount > 0) && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
            {
                for (int i = 0; i < 20; i++) multiSelectControls[i] = -1;
                multiSelectCount = 0;
            }

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
            // Anchor selection logic: mouse over anchor to focus
            if (!dragMoveMode)
            {
                focusedAnchor = -1;

                // NOTE: Allowing selection of reference anchor[0]
                for (int i = 0; i < MAX_ANCHOR_POINTS; i++)
                {
                    if (layout->anchors[i].enabled)
                    {
                        // Checks if mouse is over an anchor
                        if (CheckCollisionPointCircle(mouse, (Vector2){ layout->anchors[i].x, layout->anchors[i].y }, ANCHOR_RADIUS))
                        {
                            focusedAnchor = i;
                        }
                    }
                }
            }

            // New anchor creation logic (if possible)
            if ((focusedControl == -1) && (focusedAnchor == -1))
            {
                // Conditions to check
                if (anchorEditMode &&
                    !anchorLinkMode &&
                    !tracemap.focused &&
                    !tracemap.selected &&
                    (layout->anchorCount < MAX_ANCHOR_POINTS))
                {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                    {
                        // NOTE: layout.anchor[0] is reference point
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

            // Select/unselect focused anchor logic
            if (!CheckCollisionPointRec(mouse, (Rectangle){ 0, 0, GetScreenWidth(), 40 }) &&            // Avoid maintoolbar
                (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)))
            {
                selectedAnchor = focusedAnchor;
                if (anchorLinkMode) selectedAnchor = -1;
            }

            // Actions with one anchor selected
            if ((selectedAnchor != -1) && !CheckCollisionPointRec(mouse, (Rectangle){ 0, 0, GetScreenWidth(), 40 })) // Avoid top bar
            {
                // Link anchor
                if (!anchorLinkMode)
                {
                    if (refWindowEditMode)
                    {
                        // NOTE: In the case of ref-window, we don't link to controls, just define size
                        layout->refWindow.width = mouse.x - layout->refWindow.x;
                        layout->refWindow.height = mouse.y  - layout->refWindow.y;

                        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
                        {
                            if (layout->refWindow.width < 0) layout->refWindow.width = -1;
                            if (layout->refWindow.height < 0) layout->refWindow.height = -1;
                            refWindowEditMode = false;
                        }
                    }
                    else    // Not refWindowEditMode
                    {
                        if (dragMoveMode)
                        {
                            //if (selectedAnchor == 0) anchorEditMode = false;

                            // Move anchor without moving controls
                            if (anchorMoveMode && !anchorEditMode)
                            {
                                for (int i = 0; i < layout->controlCount; i++)
                                {
                                    if (layout->controls[i].ap->id == MAX_ANCHOR_POINTS)    // Auxiliar anchor id
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
                                        if (layout->controls[i].ap->id == MAX_ANCHOR_POINTS)  // Auxiliar anchor id
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
                        else    // Not anchor drag-move mode
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
                            else    // Not anchor resize mode
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

                                if (selectedAnchor == 0) layout->refWindow = (Rectangle){ layout->anchors[0].x, layout->anchors[0].y, layout->refWindow.width, layout->refWindow.height };

                                // Activate anchor position edit mode
                                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) dragMoveMode = true;
                                else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
                                {
                                    if (selectedAnchor == 0) refWindowEditMode = true;  // Activate ref window edit
                                    else anchorLinkMode = true;  // Activate anchor link mode
                                }
                                else if (IsKeyPressed(KEY_H)) layout->anchors[selectedAnchor].hidding = !layout->anchors[selectedAnchor].hidding;   // Hide/Unhide anchors
                                else if (IsKeyPressed(KEY_U) && (selectedAnchor > 0))                   // Unlinks controls from selected anchor
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
                    }
                }
            }

            if (anchorLinkMode && (selectedAnchor == -1) && IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) anchorLinkMode = false;
            //----------------------------------------------------------------------------------------------

            // Tracemap edition logic
            //----------------------------------------------------------------------------------------------
            if (tracemap.visible && !tracemap.locked)
            {
                tracemap.focused = false;
                if (CheckCollisionPointRec(mouse, tracemap.rec) && (focusedControl == -1) && (focusedAnchor == -1)) tracemap.focused = true;

                // Check if mouse is in the screen work area
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                    CheckCollisionPointRec(mouse, (Rectangle){ 0, 40, GetScreenWidth(), GetScreenHeight() - 64 }))  tracemap.selected = tracemap.focused;

                if (tracemap.selected)
                {
                    if (dragMoveMode)
                    {
                        int offsetX = (int)mouse.x%gridSpacing;
                        int offsetY = (int)mouse.y%gridSpacing;

                        tracemap.rec.x = prevPosition.x + (mouse.x - panOffset.x);
                        tracemap.rec.y = prevPosition.y + (mouse.y - panOffset.y);

                        if (mainToolbarState.snapModeActive)
                        {
                            if (offsetX >= gridSpacing/2) mouse.x += (gridSpacing - offsetX);
                            else mouse.x -= offsetX;

                            if (offsetY >= gridSpacing/2) mouse.y += (gridSpacing - offsetY);
                            else mouse.y -= offsetY;

                            offsetX = (int)tracemap.rec.x%gridSpacing;
                            offsetY = (int)tracemap.rec.y%gridSpacing;

                            if (offsetX >= gridSpacing/2) tracemap.rec.x += (gridSpacing - offsetX);
                            else tracemap.rec.x -= offsetX;

                            if (offsetY >= gridSpacing/2) tracemap.rec.y += (gridSpacing - offsetY);
                            else tracemap.rec.y -= offsetY;
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
                                    tracemap.rec.height += gridSnapDelta;
                                    tracemap.rec.width += gridSnapDelta;
                                }
                                else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_UP))
                                {
                                    tracemap.rec.height -= gridSnapDelta;
                                    tracemap.rec.width -= gridSnapDelta;
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
                                        tracemap.rec.height += gridSnapDelta;
                                        tracemap.rec.width += gridSnapDelta;
                                    }
                                    else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_UP))
                                    {
                                        tracemap.rec.height -= gridSnapDelta;
                                        tracemap.rec.width -= gridSnapDelta;
                                    }

                                    moveFrameCounter = 0;
                                }
                            }

                            //tracemap.height = tracemap.rec.height;
                            //tracemap.width = tracemap.rec.width;
                        }
                        else
                        {
                            // Move map with arrows
                            int offsetX = (int)tracemap.rec.x%gridSnapDelta;
                            int offsetY = (int)tracemap.rec.y%gridSnapDelta;

                            if (precisionEditMode)
                            {
                                if (IsKeyPressed(KEY_RIGHT))  tracemap.rec.x += (gridSnapDelta - offsetX);
                                else if (IsKeyPressed(KEY_LEFT))
                                {
                                    if (offsetX == 0) offsetX = gridSnapDelta;
                                    tracemap.rec.x -= offsetX;
                                }

                                if (IsKeyPressed(KEY_DOWN)) tracemap.rec.y += (gridSnapDelta - offsetY);
                                else if (IsKeyPressed(KEY_UP))
                                {
                                    if (offsetY == 0) offsetY = gridSnapDelta;
                                    tracemap.rec.y -= offsetY;
                                }

                                moveFrameCounter = 0;
                            }
                            else
                            {
                                moveFrameCounter++;

                                if ((moveFrameCounter%moveFrameSpeed) == 0)
                                {
                                    if (IsKeyDown(KEY_RIGHT)) tracemap.rec.x += (gridSnapDelta - offsetX);
                                    else if (IsKeyDown(KEY_LEFT))
                                    {
                                        if (offsetX == 0) offsetX = gridSnapDelta;
                                        tracemap.rec.x -= offsetX;
                                    }

                                    if (IsKeyDown(KEY_DOWN)) tracemap.rec.y += (gridSnapDelta - offsetY);
                                    else if (IsKeyDown(KEY_UP))
                                    {
                                        if (offsetY == 0) offsetY = gridSnapDelta;
                                        tracemap.rec.y -= offsetY;
                                    }

                                    moveFrameCounter = 0;
                                }
                            }
                            //------------------------------------------------------------------

                            // Change tracemap alpha
                            // TODO: Expose tracemap alpha config in a visual way
                            if (IsKeyDown(KEY_RIGHT_ALT))
                            {
                                if (IsKeyDown(KEY_O)) tracemap.alpha += 0.05f;
                                else if (IsKeyDown(KEY_P)) tracemap.alpha -= 0.05f;
                            }

                            // Delete tracemap
                            if (IsKeyPressed(KEY_DELETE))
                            {
                                UnloadTexture(tracemap.texture);
                                tracemap.texture.id = 0;
                                tracemap.rec.x = 0;
                                tracemap.rec.y = 0;

                                tracemap.locked = false;
                                tracemap.focused = false;
                                tracemap.selected = false;
                                tracemap.visible = false;

                                mainToolbarState.tracemapLoaded = false;
                            }

                            // Enable dragMoveMode mode
                            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                            {
                                panOffset = mouse;
                                prevPosition = (Vector2){ tracemap.rec.x, tracemap.rec.y };

                                dragMoveMode = true;
                            }
                        }
                    }
                }
            }
            else
            {
                tracemap.focused = false;
                tracemap.selected = false;
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
        //----------------------------------------------------------------------------------------------

        // Window locks logic
        //----------------------------------------------------------------------------------------------
        // WARNING: If any window is shown, cancel any edition mode
        if (windowHelpState.windowActive ||
            windowAboutState.windowActive ||
            showIssueReportWindow ||
            windowCodegenState.windowActive ||
            showExitWindow ||
            showResetWindow ||
            showLoadFileDialog ||
            showSaveFileDialog ||
            showExportFileDialog ||
            showLoadTracemapDialog ||
            showLoadTemplateDialog)
        {
            nameEditMode = false;
            textEditMode = false;
            resizeMode = false;
            dragMoveMode = false;
            precisionEditMode = false;

            showWindowActive = true;        // There is some window overlap!
        }
        else showWindowActive = false;

        // WARNING: Some windows should lock the main screen controls when shown
        if (showWindowActive) GuiLock();
        else GuiUnlock();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

            // Draw background grid
            Vector2 mouseCell = { 0 };
            if (mainToolbarState.showGridActive)
            {
                // Draw grid covering the screen work area
                GuiGrid((Rectangle){ 0, 40, GetScreenWidth(), GetScreenHeight() - 64 }, NULL, gridSpacing*gridSubdivisions, gridSubdivisions, &mouseCell);
            }

            // Draw tracemap
            //---------------------------------------------------------------------------------
            if (tracemap.visible && (tracemap.texture.id > 0))
            {
                DrawTexturePro(tracemap.texture, (Rectangle){ 0, 0, tracemap.texture.width, tracemap.texture.height }, tracemap.rec, (Vector2){ 0, 0 }, 0.0f, Fade(WHITE, tracemap.alpha));

                if (tracemap.locked)
                {
                    DrawRectangleLinesEx(tracemap.rec, 1, colTracemapLocked);
                }
                else
                {
                    if (tracemap.focused)
                    {
                        DrawRectangleRec(tracemap.rec, Fade(colTracemapFocused, 0.1f));
                        DrawRectangleLinesEx(tracemap.rec, 2, colTracemapFocused);
                    }

                    if (tracemap.selected)
                    {
                        Color colTracemap = colTracemapSelected;
                        if (!dragMoveMode && resizeMode) colTracemap = colTracemapResize;
                        DrawRectangleRec(tracemap.rec, Fade(colTracemap, 0.3f));

                        Color colPositionText = colControlRecTextDefault;
                        if (showGlobalPosition) colPositionText = colControlRecTextGlobal;
                        if (mainToolbarState.snapModeActive) colPositionText = colControlRecTextSnap;
                        if (!dragMoveMode && precisionEditMode) colPositionText = colControlRecTextPrecision;

                        DrawText(TextFormat("[%i, %i, %i, %i]",
                                            (int)tracemap.rec.x - (int)layout->refWindow.x,
                                            (int)tracemap.rec.y - (int)layout->refWindow.y,
                                            (int)tracemap.rec.width,
                                            (int)tracemap.rec.height), tracemap.rec.x, tracemap.rec.y - 20, 20, colPositionText);
                    }
                }
            }
            //---------------------------------------------------------------------------------

            // Draw controls
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
                            GuiSetAlpha(0.7f);
                            GuiWindowBox(rec, layout->controls[i].text);
                            GuiSetAlpha(1.0f);
                        } break;
                        case GUI_GROUPBOX: GuiGroupBox(rec, layout->controls[i].text); break;
                        case GUI_LINE:
                        {
                            if (layout->controls[i].text[0] == '\0') GuiLine(rec, NULL);
                            else GuiLine(rec, layout->controls[i].text);
                        } break;
                        case GUI_PANEL:
                        {
                            GuiSetAlpha(0.7f);
                            GuiPanel(rec, (layout->controls[i].text[0] == '\0')? NULL : layout->controls[i].text);
                            GuiSetAlpha(1.0f);
                        } break;
                        case GUI_LABEL: GuiLabel(rec, layout->controls[i].text); break;
                        case GUI_BUTTON: GuiButton(rec, layout->controls[i].text); break;
                        case GUI_LABELBUTTON: GuiLabelButton(rec, layout->controls[i].text); break;
                        case GUI_CHECKBOX: GuiCheckBox(rec, layout->controls[i].text, NULL); break;
                        case GUI_TOGGLE: GuiToggle(rec, layout->controls[i].text, NULL); break;
                        case GUI_TOGGLEGROUP: GuiToggleGroup(rec, layout->controls[i].text, NULL); break;
                        case GUI_COMBOBOX: GuiComboBox(rec, layout->controls[i].text, NULL); break;
                        case GUI_DROPDOWNBOX: GuiDropdownBox(rec, layout->controls[i].text, &dropdownBoxActive, false); break;
                        case GUI_TEXTBOX: GuiTextBox(rec, layout->controls[i].text, MAX_CONTROL_TEXT_LENGTH, false); break;
                        //case GUI_TEXTBOXMULTI: GuiTextBoxMulti(rec, layout->controls[i].text, MAX_CONTROL_TEXT_LENGTH, false); break;
                        case GUI_VALUEBOX: GuiValueBox(rec, layout->controls[i].text, &valueBoxValue, 42, 100, false); break;
                        case GUI_SPINNER: GuiSpinner(rec, layout->controls[i].text, &spinnerValue, 42, 3, false); break;
                        case GUI_SLIDER: GuiSlider(rec, layout->controls[i].text, NULL, NULL, 0, 100); break;
                        case GUI_SLIDERBAR: GuiSliderBar(rec, layout->controls[i].text, NULL, NULL, 0, 100); break;
                        case GUI_PROGRESSBAR: GuiProgressBar(rec, layout->controls[i].text, NULL, NULL, 0, 100); break;
                        case GUI_STATUSBAR: GuiStatusBar(rec, layout->controls[i].text); break;
                        case GUI_SCROLLPANEL:
                        {
                            GuiSetAlpha(0.7f);
                            GuiScrollPanel(rec, (layout->controls[i].text[0] == '\0')? NULL : layout->controls[i].text, rec, NULL, NULL);
                            GuiSetAlpha(1.0f);
                        } break;
                        case GUI_LISTVIEW: GuiListView(rec, layout->controls[i].text, &listViewScrollIndex, &listViewActive); break;
                        case GUI_COLORPICKER: GuiColorPicker(rec, (layout->controls[i].text[0] == '\0')? NULL : layout->controls[i].text, NULL); break;
                        case GUI_DUMMYREC: GuiDummyRec(rec, layout->controls[i].text); break;
                        default: break;
                    }

                    // View control rectangles mode (avoid containers)
                    if (mainToolbarState.showControlRecsActive && (layout->controls[i].type != GUI_WINDOWBOX) && (layout->controls[i].type != GUI_PANEL) && (layout->controls[i].type != GUI_GROUPBOX))
                    {
                        DrawRectangleRec(rec, Fade(colShowControlRecs, 0.2f));
                        DrawRectangleLinesEx(rec, 1.0f, Fade(colShowControlRecs, 0.7f));
                    }
                }
            }
            if (!showWindowActive) GuiUnlock();
            //----------------------------------------------------------------------------------------

            // Draw anchors and related data
            //----------------------------------------------------------------------------------------
            // Draw anchors
            for (int i = 0; i < MAX_ANCHOR_POINTS; i++)
            {
                if (layout->anchors[i].enabled)
                {
                    if (i == 0)    // Reference anchor
                    {
                        Color colAnchor = colAnchor0;

                        if ((i == focusedAnchor) || (i == selectedAnchor)) DrawRectangle(layout->anchors[0].x - ANCHOR_RADIUS, layout->anchors[0].y - ANCHOR_RADIUS, ANCHOR_RADIUS*2, ANCHOR_RADIUS*2, Fade(colAnchor, 0.2f));
                        DrawRectangleLinesEx((Rectangle){ layout->anchors[0].x - ANCHOR_RADIUS, layout->anchors[0].y - ANCHOR_RADIUS, ANCHOR_RADIUS*2, ANCHOR_RADIUS*2 }, 1.0f, Fade(colAnchor, 0.5f));
                        DrawRectangle(layout->anchors[0].x - ANCHOR_RADIUS - 5, layout->anchors[0].y, ANCHOR_RADIUS*2 + 10, 1, Fade(colAnchor, 0.8f));
                        DrawRectangle(layout->anchors[0].x, layout->anchors[0].y - ANCHOR_RADIUS - 5, 1, ANCHOR_RADIUS*2 + 10, Fade(colAnchor, 0.8f));
                    }
                    else    // Regular anchors
                    {
                        // Logic to properly choose colors
                        Color colAnchor = colAnchorDefault;

                        if (i == focusedAnchor) colAnchor = colAnchorFocused;
                        else if (i == selectedAnchor) colAnchor = colAnchorSelected;

                        if (anchorMoveMode || anchorEditMode) colAnchor = colAnchorEditMode;

                        if (layout->anchors[i].hidding) colAnchor = colAnchorHidden;

                        // Draw anchor circles and lines
                        if ((i == focusedAnchor) || (i == selectedAnchor)) DrawCircle(layout->anchors[i].x, layout->anchors[i].y, ANCHOR_RADIUS, Fade(colAnchor, 0.3f));
                        DrawCircleLines(layout->anchors[i].x, layout->anchors[i].y, ANCHOR_RADIUS, Fade(colAnchor, 0.5f));
                        DrawRectangle(layout->anchors[i].x - ANCHOR_RADIUS - 5, layout->anchors[i].y, ANCHOR_RADIUS*2 + 10, 1, colAnchor);
                        DrawRectangle(layout->anchors[i].x, layout->anchors[i].y - ANCHOR_RADIUS - 5, 1, ANCHOR_RADIUS*2 + 10, colAnchor);
                    }
                }
            }

            // Reference window edit mode lines
            if (refWindowEditMode)
            {
                if ((layout->refWindow.width > 0) && (layout->refWindow.height > 0))
                {
                    DrawRectangleRec(layout->refWindow, Fade(colRefWindow, 0.1f));

                    DrawText(TextFormat("[%i]", (int)layout->refWindow.width),
                        layout->refWindow.x + layout->refWindow.width - MeasureText(TextFormat("[%i]",(int)layout->refWindow.width), 20) - 5,
                        layout->refWindow.y + layout->refWindow.height + 5, 20, colRefWindowText);

                    DrawText(TextFormat("[%i]", (int)layout->refWindow.height),
                        layout->refWindow.x + layout->refWindow.width + 5,
                        layout->refWindow.y + layout->refWindow.height - 20, 20, colRefWindowText);
                }
            }

            // Draw anchor links on anchor focused or selected
            if (focusedAnchor != -1)
            {
                for (int i = 0; i < layout->controlCount; i++)
                {
                    if (layout->controls[i].ap->id == focusedAnchor)
                    {
                        if (focusedAnchor == 0) DrawLine(layout->controls[i].ap->x, layout->controls[i].ap->y, layout->controls[i].rec.x, layout->controls[i].rec.y, colAnchorLinkLine0);
                        else if (!layout->controls[i].ap->hidding)
                        {
                            DrawLine(layout->controls[i].ap->x, layout->controls[i].ap->y, layout->controls[i].ap->x + layout->controls[i].rec.x, layout->controls[i].ap->y + layout->controls[i].rec.y, colAnchorLinkLine);
                        }
                        else DrawLine(layout->controls[i].ap->x, layout->controls[i].ap->y, layout->controls[i].ap->x + layout->controls[i].rec.x, layout->controls[i].ap->y + layout->controls[i].rec.y, colAnchorLinkLineHidden);
                    }
                }
            }

            // Draw anchor links on anchor selected
            if (selectedAnchor != -1)
            {
                for (int i = 0; i < layout->controlCount; i++)
                {
                    if (layout->controls[i].ap->id == selectedAnchor)
                    {
                        if (selectedAnchor == 0) DrawLine(layout->controls[i].ap->x, layout->controls[i].ap->y, layout->controls[i].rec.x, layout->controls[i].rec.y, Fade(colAnchorLinkLine0, 0.5f));
                        else if (!layout->controls[i].ap->hidding)
                        {
                            DrawLine(layout->controls[i].ap->x, layout->controls[i].ap->y, layout->controls[i].ap->x + layout->controls[i].rec.x, layout->controls[i].ap->y + layout->controls[i].rec.y, colAnchorLinkLine);
                        }
                        else DrawLine(layout->controls[i].ap->x, layout->controls[i].ap->y, layout->controls[i].ap->x + layout->controls[i].rec.x, layout->controls[i].ap->y + layout->controls[i].rec.y, colAnchorLinkLine0);
                    }
                }
            }

            // Draw selected anchor coordinates text
            if (selectedAnchor != -1)
            {
                Color colPositionText = colControlRecTextDefault;
                if (showGlobalPosition) colPositionText = colControlRecTextGlobal;
                if (mainToolbarState.snapModeActive) colPositionText = colControlRecTextSnap;
                if (!dragMoveMode && precisionEditMode) colPositionText = colControlRecTextPrecision;

                if (selectedAnchor > 0)
                {
                    DrawText(TextFormat("[%i, %i]",
                        (int)(layout->anchors[selectedAnchor].x - layout->refWindow.x),
                        (int)(layout->anchors[selectedAnchor].y - layout->refWindow.y)),
                        layout->anchors[selectedAnchor].x + ANCHOR_RADIUS,
                        layout->anchors[selectedAnchor].y - 38, 20, colPositionText);
                }
                else    // anchor[0] -> reference window
                {
                    if ((layout->refWindow.width > 0) && (layout->refWindow.height > 0))
                    {
                        DrawText(TextFormat("[%i, %i, %i, %i]",
                            (int)layout->refWindow.x,
                            (int)layout->refWindow.y,
                            (int)layout->refWindow.width,
                            (int)layout->refWindow.height),
                            layout->anchors[selectedAnchor].x - ANCHOR_RADIUS,
                            layout->anchors[selectedAnchor].y + ANCHOR_RADIUS + 4, 20, colPositionText);
                    }
                    else DrawText(TextFormat("[%i, %i]",
                        (int)(layout->refWindow.x),
                        (int)(layout->refWindow.y)),
                        layout->anchors[selectedAnchor].x - ANCHOR_RADIUS,
                        layout->anchors[selectedAnchor].y + ANCHOR_RADIUS + 4, 20, colPositionText);
                }
            }
            //----------------------------------------------------------------------------------------

            if (!GuiIsLocked())
            {
                // Control selected from panel drawing
                // NOTE: It uses default control rectangle for selected type
                if (CheckCollisionPointRec(mouse, (Rectangle){ 0, 40, (float)GetScreenWidth(), (float)GetScreenHeight() - 64 }) &&
                    !CheckCollisionPointRec(mouse, windowControlsPaletteState.panelBounds))
                {
                    if ((focusedAnchor == -1) && (focusedControl == -1) && !tracemap.focused && !refWindowEditMode && !multiSelectMode)
                    {
                        if (!anchorEditMode)
                        {
                            if (!anchorLinkMode && (selectedAnchor == -1) && (selectedControl == -1) && !tracemap.selected)
                            {
                                // Draw the default rectangle of the control selected
                                GuiLock();
                                GuiSetAlpha(0.5f);

                                switch (selectedType)
                                {
                                    case GUI_WINDOWBOX: GuiWindowBox(defaultRec[selectedType], "WINDOW BOX"); break;
                                    case GUI_GROUPBOX: GuiGroupBox(defaultRec[selectedType], "GROUP BOX"); break;
                                    case GUI_LINE: GuiLine(defaultRec[selectedType], NULL); break;
                                    case GUI_PANEL: GuiPanel(defaultRec[selectedType], NULL); break;
                                    case GUI_LABEL: GuiLabel(defaultRec[selectedType], "LABEL TEXT"); break;
                                    case GUI_BUTTON: GuiButton(defaultRec[selectedType], "BUTTON"); break;
                                    case GUI_LABELBUTTON: GuiLabelButton(defaultRec[selectedType], "LABEL_BUTTON"); break;
                                    case GUI_CHECKBOX: GuiCheckBox(defaultRec[selectedType], "CHECK BOX", NULL); break;
                                    case GUI_TOGGLE: GuiToggle(defaultRec[selectedType], "TOGGLE", NULL); break;
                                    // WARNING: Selection rectangle for GuiToggleGroup() considers all the control while the function expects only one piece!
                                    case GUI_TOGGLEGROUP:
                                    {
                                        GuiToggleGroup((Rectangle){ defaultRec[selectedType].x, defaultRec[selectedType].y, defaultRec[selectedType].width/3.0f, defaultRec[selectedType].height }, "ONE;TWO;THREE", NULL);
                                    } break;
                                    case GUI_COMBOBOX: GuiComboBox(defaultRec[selectedType], "ONE;TWO;THREE", NULL); break;
                                    case GUI_DROPDOWNBOX: GuiDropdownBox(defaultRec[selectedType], "ONE;TWO;THREE", &dropdownBoxActive, false); break;
                                    case GUI_TEXTBOX: GuiTextBox(defaultRec[selectedType], "TEXT BOX", 7, false); break;
                                    //case GUI_TEXTBOXMULTI: GuiTextBoxMulti(defaultRec[selectedType], "TEXT BOX MULTI", 7, false); break;
                                    case GUI_VALUEBOX: GuiValueBox(defaultRec[selectedType], "VALUE BOX", &valueBoxValue, 42, 100, false); break;
                                    case GUI_SPINNER: GuiSpinner(defaultRec[selectedType], "SPINNER", &spinnerValue, 42, 3, false); break;
                                    case GUI_SLIDER: GuiSlider(defaultRec[selectedType], "SLIDER", NULL, NULL, 0, 100); break;
                                    case GUI_SLIDERBAR: GuiSliderBar(defaultRec[selectedType], "SLIDER BAR", NULL, NULL, 0, 100); break;
                                    case GUI_PROGRESSBAR: GuiProgressBar(defaultRec[selectedType], "PROGRESS BAR", NULL, NULL, 0, 100); break;
                                    case GUI_STATUSBAR: GuiStatusBar(defaultRec[selectedType], "STATUS BAR"); break;
                                    case GUI_SCROLLPANEL: GuiScrollPanel(defaultRec[selectedType], NULL, defaultRec[selectedType], NULL, NULL); break;
                                    case GUI_LISTVIEW: GuiListView(defaultRec[selectedType], "ONE;TWO;THREE;FOUR", &listViewScrollIndex, &listViewActive); break;
                                    case GUI_COLORPICKER: GuiColorPicker(defaultRec[selectedType], NULL, NULL); break;
                                    case GUI_DUMMYREC: GuiDummyRec(defaultRec[selectedType], "DUMMY REC"); break;
                                    default: break;
                                }

                                GuiSetAlpha(1.0f);
                                if (!showWindowActive) GuiUnlock();

                                // Draw cursor position
                                Color colPositionText = colControlRecTextDefault;
                                if (mainToolbarState.snapModeActive) colPositionText = colControlRecTextSnap;

                                // Draw anchor position
                                DrawText(TextFormat("[%i, %i, %i, %i]",
                                    (int)defaultRec[selectedType].x - (int)layout->refWindow.x,
                                    (int)defaultRec[selectedType].y - (int)layout->refWindow.y,
                                    (int)defaultRec[selectedType].width,
                                    (int)defaultRec[selectedType].height),
                                    (int)defaultRec[selectedType].x, ((int)defaultRec[selectedType].y < ((int)layout->refWindow.y + 8))? (int)defaultRec[selectedType].y + 30 : (int)defaultRec[selectedType].y - 30, 20, Fade(colPositionText, 0.5f));
                            }
                        }
                        else
                        {
                            // Draw anchor cursor
                            DrawCircleLines(mouse.x, mouse.y, ANCHOR_RADIUS, Fade(colAnchorCreation, 0.5f));
                            DrawRectangle(mouse.x - ANCHOR_RADIUS - 5, mouse.y, ANCHOR_RADIUS*2 + 10, 1, colAnchorCreation);
                            DrawRectangle(mouse.x, mouse.y - ANCHOR_RADIUS - 5, 1, ANCHOR_RADIUS*2 + 10, colAnchorCreation);
                        }
                    }
                }

                // Draw multiselection rectangle
                if (multiSelectMode)
                {
                    DrawRectangleLinesEx(multiSelectRec, 2.0f, colAnchorDefault);
                }

                // Draw anchor edit data
                // NOTE: Only if GUI is not locked!
                if (selectedAnchor != -1)
                {
                    // Draw anchor link mode line to mouse
                    // NOTE: anchor[0] is the reference anchor, applies to all other anchors and controls
                    if (anchorLinkMode && (selectedAnchor != 0))
                    {
                        DrawLine(layout->anchors[selectedAnchor].x, layout->anchors[selectedAnchor].y, mouse.x, mouse.y, colAnchorLinkLine);
                    }

                    // Draw name edit mode
                    if (nameEditMode)
                    {
                        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(colEditAnchorNameOverlay, 0.2f));

                        int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE)*2;
                        int textWidth = MeasureText(layout->anchors[selectedAnchor].name, fontSize);
                        Rectangle textboxRec = (Rectangle){ layout->anchors[selectedAnchor].x, layout->anchors[selectedAnchor].y, textWidth + 40, fontSize + 5 };

                        if (textboxRec.width < (textWidth + 40)) textboxRec.width = (float)textWidth + 40;
                        if (textboxRec.height < fontSize) textboxRec.height += fontSize;

                        if (GuiTextBox(textboxRec, layout->anchors[selectedAnchor].name, MAX_ANCHOR_NAME_LENGTH, nameEditMode)) nameEditMode = !nameEditMode;
                    }
                }

                // Draw selected control
                if (selectedControl != -1)
                {
                    // Selection rectangle
                    Color colControl = colControlSelected;
                    if (!dragMoveMode && resizeMode) colControl = colControlSelectedResize;

                    Rectangle selectedRec = layout->controls[selectedControl].rec;
                    float reportedRecHeight = selectedRec.height;
                    if (layout->controls[selectedControl].type == GUI_WINDOWBOX) selectedRec.height = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT;  // Defined inside raygui.h!
                    else if (layout->controls[selectedControl].type == GUI_GROUPBOX)
                    {
                        selectedRec.y -= 10;
                        selectedRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE)*2.0f;
                    }

                    // WARNING: Adding offset to control rectangle draw in screen coordinates
                    if (layout->controls[selectedControl].ap->id > 0)
                    {
                        selectedRec.x += layout->controls[selectedControl].ap->x;
                        selectedRec.y += layout->controls[selectedControl].ap->y;
                    }

                    DrawRectangleRec(selectedRec, Fade(colControl, 0.3f));

                    // Control Link
                    if (layout->controls[selectedControl].ap->id > 0) DrawLine(layout->controls[selectedControl].ap->x, layout->controls[selectedControl].ap->y, selectedRec.x, selectedRec.y, colAnchorLinkLine);

                    // Linking
                    if (anchorLinkMode) DrawLine(selectedRec.x, selectedRec.y, mouse.x, mouse.y, colAnchorLinkLine);

                    // Draw control text coordinates
                    Color colPositionText = colControlRecTextDefault;
                    if (showGlobalPosition) colPositionText = colControlRecTextGlobal;
                    if (mainToolbarState.snapModeActive) colPositionText = colControlRecTextSnap;
                    if (!dragMoveMode && precisionEditMode) colPositionText = colControlRecTextPrecision;

                    if (!showGlobalPosition)
                    {
                        if (layout->controls[selectedControl].ap->id > 0)
                        {
                            DrawText(TextFormat("[%i, %i, %i, %i]",
                                (int)(layout->controls[selectedControl].rec.x),
                                (int)(layout->controls[selectedControl].rec.y),
                                (int)selectedRec.width,
                                (int)reportedRecHeight),
                                selectedRec.x, selectedRec.y - 30, 20, colPositionText);
                        }
                        else DrawText(TextFormat("[%i, %i, %i, %i]",
                            (int)(selectedRec.x - (int)layout->refWindow.x),
                            (int)(selectedRec.y - (int)layout->refWindow.y),
                            (int)selectedRec.width,
                            (int)reportedRecHeight),
                            selectedRec.x, selectedRec.y - 30, 20, colPositionText);
                    }
                    else
                    {
                        DrawText(TextFormat("[%i, %i, %i, %i]",
                            (int)(selectedRec.x - (int)layout->refWindow.x),
                            (int)(selectedRec.y - (int)layout->refWindow.y),
                            (int)selectedRec.width,
                            (int)reportedRecHeight),
                            selectedRec.x, selectedRec.y - 30, 20, colPositionText);
                    }

                    // Text edit
                    if (textEditMode || showIconPanel)
                    {
                        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(colEditControlTextOverlay, 0.2f));

                        Rectangle textboxRec = layout->controls[selectedControl].rec;

                        // Make sure text could be written, no matter if overflows control
                        int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
                        int textWidth = MeasureText(layout->controls[selectedControl].text, fontSize);
                        if (textboxRec.width < (textWidth + 40)) textboxRec.width = textWidth + 40;
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

                        // Draw a GuiTextBox() for text edition
                        if (layout->controls[selectedControl].type == GUI_TEXTBOXMULTI)
                        {
                            //if (GuiTextBoxMulti(textboxRec, layout->controls[selectedControl].text, MAX_CONTROL_TEXT_LENGTH, textEditMode)) textEditMode = !textEditMode;
                        }
                        else if (GuiTextBox(textboxRec, layout->controls[selectedControl].text, MAX_CONTROL_TEXT_LENGTH, textEditMode)) textEditMode = !textEditMode;

                        // Check if icon panel must be shown
                        if ((strlen(layout->controls[selectedControl].text) == 1) && (layout->controls[selectedControl].text[0] == '#'))
                        {
                            showIconPanel = true;

                            // Draw icons selector
                            GuiToggleGroup((Rectangle){ (int)textboxRec.x, (int)textboxRec.y + (int)textboxRec.height + 10, 18, 18 }, toggleIconsText, &selectedIcon);
                            if (selectedIcon > 0)
                            {
                                strcpy(layout->controls[selectedControl].text, TextFormat("#%03i#\0", selectedIcon));
                                textBoxCursorIndex = (int)strlen(layout->controls[selectedControl].text);
                                showIconPanel = false;
                                textEditMode = true;
                                selectedIcon = 0;
                            }
                        }
                    }

                    // Name edit
                    if (nameEditMode)
                    {
                        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(colEditControlNameOverlay, 0.2f));

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

                // Draw focused control selector
                if (focusedControl != -1)
                {
                    // Draw focused rectangle
                    Rectangle focusedRec = layout->controls[focusedControl].rec;
                    float reportedRecHeight = focusedRec.height;

                    if (layout->controls[focusedControl].type == GUI_WINDOWBOX) focusedRec.height = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT;  // Defined inside raygui.h!
                    else if (layout->controls[focusedControl].type == GUI_GROUPBOX)
                    {
                        focusedRec.y -= 10;
                        focusedRec.height = GuiGetStyle(DEFAULT, TEXT_SIZE)*2.0f;
                    }

                    // WARNING: Adding offset to control rectangle draw in screen coordinates
                    if (layout->controls[focusedControl].ap->id > 0)
                    {
                        focusedRec.x += layout->controls[focusedControl].ap->x;
                        focusedRec.y += layout->controls[focusedControl].ap->y;
                    }

                    if (focusedControl != selectedControl) DrawRectangleRec(focusedRec, Fade(colControlFocused, 0.1f));

                    DrawRectangleLinesEx(focusedRec, 1, colControlFocused);

                    if (layout->controls[focusedControl].ap->id > 0)
                    {
                        DrawLine(layout->controls[focusedControl].ap->x, layout->controls[focusedControl].ap->y, focusedRec.x, focusedRec.y, colAnchorLinkLine);
                    }

                    // Draw control focused text coordinates
                    Color colPositionText = colControlRecTextDefault;
                    if (showGlobalPosition) colPositionText = colControlRecTextGlobal;
                    if (mainToolbarState.snapModeActive) colPositionText = colControlRecTextSnap;
                    if (!dragMoveMode && precisionEditMode) colPositionText = colControlRecTextPrecision;

                    if (!showGlobalPosition)
                    {
                        if (layout->controls[focusedControl].ap->id > 0)
                        {
                            DrawText(TextFormat("[%i, %i, %i, %i]",
                                (int)(layout->controls[focusedControl].rec.x),
                                (int)(layout->controls[focusedControl].rec.y),
                                (int)focusedRec.width,
                                (int)reportedRecHeight),
                                focusedRec.x, focusedRec.y - 30, 20, colPositionText);
                        }
                        else DrawText(TextFormat("[%i, %i, %i, %i]",
                            (int)(focusedRec.x - (int)layout->refWindow.x),
                            (int)(focusedRec.y - (int)layout->refWindow.y),
                            (int)focusedRec.width,
                            (int)reportedRecHeight),
                            focusedRec.x, focusedRec.y - 30, 20, colPositionText);
                    }
                    else
                    {
                        DrawText(TextFormat("[%i, %i, %i, %i]",
                            (int)(focusedRec.x - (int)layout->refWindow.x),
                            (int)(focusedRec.y - (int)layout->refWindow.y),
                            (int)focusedRec.width,
                            (int)reportedRecHeight),
                            focusedRec.x, focusedRec.y - 30, 20, colPositionText);
                    }
                }

                // Draw control scale mode selector
                if ((selectedControl >= 0) && mouseScaleReady)
                {
                    Rectangle rec = layout->controls[selectedControl].rec;

                    // NOTE: We must consider anchor offset!
                    if (layout->controls[selectedControl].ap->id > 0)
                    {
                        rec.x += layout->controls[selectedControl].ap->x;
                        rec.y += layout->controls[selectedControl].ap->y;
                    }

                    DrawRectangleLinesEx(rec, 2, colControlSelected);
                    DrawTriangle((Vector2) { rec.x + rec.width - SCALE_BOX_CORNER_SIZE, rec.y + rec.height },
                                 (Vector2) { rec.x + rec.width, rec.y + rec.height },
                                 (Vector2) { rec.x + rec.width, rec.y + rec.height - SCALE_BOX_CORNER_SIZE }, colControlSelected);
                }

                // Draw reference window lines
                if ((layout->refWindow.width > 0) && (layout->refWindow.height > 0))
                {
                    DrawRectangleLinesEx(layout->refWindow, 1, Fade(colRefWindow, 0.7f));
                }

                // Draw controls IDs for layout order edition
                if (orderLayerMode)
                {
                    for (int i = layout->controlCount - 1; i >= 0; i--)
                    {
                        if (layout->controls[i].ap->id > 0)
                        {
                            DrawTextEx(GuiGetFont(), TextFormat("[%i]", layout->controls[i].id),
                                (Vector2){ layout->controls[i].rec.x + layout->controls[i].ap->x + layout->controls[i].rec.width,
                                layout->controls[i].rec.y + layout->controls[i].ap->y - GuiGetStyle(DEFAULT, TEXT_SIZE) },
                                GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetColor(GuiGetStyle(LABEL, TEXT_COLOR_PRESSED)));
                        }
                        else
                        {
                            DrawTextEx(GuiGetFont(), TextFormat("[%i]", layout->controls[i].id),
                                (Vector2){ layout->controls[i].rec.x + layout->controls[i].rec.width, layout->controls[i].rec.y - GuiGetStyle(DEFAULT, TEXT_SIZE) },
                                GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetColor(GuiGetStyle(LABEL, TEXT_COLOR_PRESSED)));
                        }
                    }
                }

                // Draw controls name if required
                if (mainToolbarState.showControlNamesActive)
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

                        DrawRectangleRec(textboxRec, colEditControlNameBackRec);
                        GuiTextBox(textboxRec, layout->controls[i].name, MAX_CONTROL_NAME_LENGTH, false);
                    }

                    for (int i = 0; i < layout->anchorCount; i++)
                    {
                        Rectangle textboxRec = (Rectangle){ layout->anchors[i].x, layout->anchors[i].y,
                            MeasureText(layout->anchors[i].name, GuiGetStyle(DEFAULT, TEXT_SIZE)) + 10, GuiGetStyle(DEFAULT, TEXT_SIZE) + 5 };

                        DrawRectangleRec(textboxRec, colEditControlNameBackRec);
                        DrawRectangleRec(textboxRec, Fade(colAnchorEditMode, 0.1f));
                        GuiTextBox(textboxRec, layout->anchors[i].name, MAX_ANCHOR_NAME_LENGTH, false);
                    }

                    if (!showWindowActive) GuiUnlock();
                }
            }

            // GUI: Controls Selection Palette
            // NOTE: It uses GuiLock() to lock controls behaviour and just limit them to selection
            //----------------------------------------------------------------------------------------
            GuiPanelControlsPalette(&windowControlsPaletteState);

            mainToolbarState.showControlPanelActive = windowControlsPaletteState.panelActive;

            if (showWindowActive) GuiLock();
            else GuiUnlock();
            //----------------------------------------------------------------------------------------

            // GUI: Main toolbar panel + control/anchor buttons logic
            //----------------------------------------------------------------------------------
            // Get requires properties from main program
            mainToolbarState.controlSelected = selectedControl;
            mainToolbarState.anchorSelected = selectedAnchor;
            mainToolbarState.hideAnchorControlsActive = layout->anchors[selectedAnchor].hidding;
            mainToolbarState.hideTracemapActive = !tracemap.visible;
            mainToolbarState.lockTracemapActive = tracemap.locked;
            mainToolbarState.tracemapAlphaValue = tracemap.alpha;
            mainToolbarState.showControlOrderActive = orderLayerMode;

            GuiMainToolbar(&mainToolbarState);

            // Snap mode setup
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

            // Control: Enable text edit mode if required
            if (mainToolbarState.btnEditTextPressed)
            {
                strcpy(prevText, layout->controls[selectedControl].text);
                textBoxCursorIndex = (int)strlen(layout->controls[selectedControl].text);
                textEditMode = true;
            }

            // Control: Enable name edit mode if required
            if (mainToolbarState.btnEditNamePressed)
            {
                strcpy(prevName, layout->controls[selectedControl].name);
                textBoxCursorIndex = (int)strlen(layout->controls[selectedControl].name);
                nameEditMode = true;
            }

            // Control: Duplicate selected control if required
            if (mainToolbarState.btnDuplicateControlPressed)
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

            // Control: Unlink from anchor
            if (mainToolbarState.btnUnlinkControlPressed)
            {
                if (layout->controls[selectedControl].ap->id != 0)
                {

                    layout->controls[selectedControl].rec.x += layout->controls[selectedControl].ap->x;
                    layout->controls[selectedControl].rec.y += layout->controls[selectedControl].ap->y;
                    layout->controls[selectedControl].ap = &layout->anchors[0];  // Link to default work area anchor
                }
            }

            // Control: Delete if required
            if (mainToolbarState.btnDeleteControlPressed)
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

            // Anchor: Enable name edit mode if required
            if (mainToolbarState.btnEditAnchorNamePressed)
            {
                strcpy(prevName, layout->anchors[selectedAnchor].name);
                nameEditMode = true;
            }

            // Anchor: Set current view mode
            if (selectedAnchor != -1) layout->anchors[selectedAnchor].hidding = mainToolbarState.hideAnchorControlsActive;

            // Anchor: Unlink all linked controls
            if (mainToolbarState.btnUnlinkAnchorControlsPressed)
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

            // Anchor: Delete if required
            if (mainToolbarState.btnDeleteAnchorPressed)
            {
                if (selectedAnchor == 0)
                {
                    layout->anchors[0].x = 0;
                    layout->anchors[0].y = 40;
                    layout->refWindow = (Rectangle){ 0, 40, -1, -1 };
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

            // Tracemap: Load new tracemap -> Already processed on Keyboard shortcuts files logic

            // Tracemap: Setup selected properties
            tracemap.visible = !mainToolbarState.hideTracemapActive;
            tracemap.locked = mainToolbarState.lockTracemapActive;
            tracemap.alpha = mainToolbarState.tracemapAlphaValue;

            if (!tracemap.visible) tracemap.selected = false;

            // Tracemap: Delete current tracemap
            if (mainToolbarState.btnDeleteTracemapPressed)
            {
                UnloadTexture(tracemap.texture);
                tracemap.texture.id = 0;
                tracemap.rec.x = 0;
                tracemap.rec.y = 0;

                tracemap.locked = false;
                tracemap.focused = false;
                tracemap.selected = false;
                tracemap.visible = false;

                mainToolbarState.tracemapLoaded = false;
            }

            // Visuals: Show order mode
            orderLayerMode = mainToolbarState.showControlOrderActive;
            //----------------------------------------------------------------------------------

            // GUI: Status bar
            //--------------------------------------------------------------------------------------------
            GuiSetStyle(STATUSBAR, TEXT_PADDING, 0);
            GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
            GuiStatusBar((Rectangle){ 0, GetScreenHeight() - 24, 160, 24}, TextFormat("CONTROLS COUNT: %i", layout->controlCount));

            // Show edit mode depending on the element/property being edited
            if (textEditMode) GuiStatusBar((Rectangle){ 160 - 1, GetScreenHeight() - 24, 168, 24 }, "EDIT MODE: TEXT");
            else if (nameEditMode) GuiStatusBar((Rectangle){ 160 - 1, GetScreenHeight() - 24, 168, 24 }, "EDIT MODE: NAME");
            else if (orderLayerMode) GuiStatusBar((Rectangle){ 160 - 1, GetScreenHeight() - 24, 168, 24 }, "EDIT MODE: LAYER");
            else if (selectedControl != -1) GuiStatusBar((Rectangle){ 160 - 1, GetScreenHeight() - 24, 168, 24 }, "EDIT MODE: CONTROL");
            else if (anchorEditMode || (selectedAnchor != -1)) GuiStatusBar((Rectangle){ 160 - 1, GetScreenHeight() - 24, 168, 24 }, "EDIT MODE: ANCHOR");
            else if (tracemap.selected)  GuiStatusBar((Rectangle){ 160 - 1, GetScreenHeight() - 24, 168, 24 }, "EDIT MODE: TRACEMAP");
            else GuiStatusBar((Rectangle){ 160 - 1, GetScreenHeight() - 24, 168, 24 }, "EDIT MODE: LAYOUT");

            // Selected control info
            GuiSetStyle(STATUSBAR, TEXT_PADDING, 10);
            GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
            if (selectedControl != -1)
            {
                GuiStatusBar((Rectangle){ 160 + 168 - 2, GetScreenHeight() - 24, 600, 24 },
                    TextFormat("SELECTED CONTROL: %03i  (%i, %i, %i, %i)  | TYPE: %s | NAME: %s", selectedControl,
                        (int)layout->controls[selectedControl].rec.x, (int)layout->controls[selectedControl].rec.y,
                        (int)layout->controls[selectedControl].rec.width, (int)layout->controls[selectedControl].rec.height,
                        TextToUpper(controlTypeName[layout->controls[selectedControl].type]),
                        layout->controls[selectedControl].name));
            }
            else if (selectedAnchor != -1)
            {
                // Count controls linked to an anchor
                int count = 0;
                for (int i = 0; i < layout->controlCount; i++) if (layout->controls[i].ap->id == layout->anchors[selectedAnchor].id) count++;

                GuiStatusBar((Rectangle){ 160 + 168 - 2, GetScreenHeight() - 24, 600, 24 },
                    TextFormat("SELECTED ANCHOR: %02i  (%i, %i) | CONTROLS: %03i | %s", selectedAnchor,
                        (int)layout->anchors[selectedAnchor].x - (int)layout->refWindow.x, (int)layout->anchors[selectedAnchor].y - (int)layout->refWindow.y, count,
                        (int)layout->anchors[selectedAnchor].hidding? "HIDDEN MODE" : "VISIBLE"));
            }
            else if (tracemap.selected)
            {
                GuiStatusBar((Rectangle){ 160 + 168 - 2, GetScreenHeight() - 24, 600, 24 },
                    TextFormat("SELECTED TRACEMAP: (%i, %i, %i, %i) | OPACITY: %i %% | %s",
                        (int)tracemap.rec.x - (int)layout->refWindow.x, (int)tracemap.rec.y - (int)layout->refWindow.y, (int)tracemap.rec.width, (int)tracemap.rec.height,
                        (int)(tracemap.alpha*100.0f), tracemap.locked? "LOCKED" : "UNLOCKED"));
            }
            else GuiStatusBar((Rectangle){ 160 + 168 - 2, GetScreenHeight() - 24, 560, 24 }, "NO CONTROL | ANCHOR | TRACEMAP SELECTED");

            // Environment info, far right position anchor
            GuiSetStyle(STATUSBAR, TEXT_PADDING, 0);
            GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
            GuiStatusBar((Rectangle){ 160 + 168 + 560 - 3, GetScreenHeight() - 24, GetScreenWidth() - 928 - 180 - 100 + 6, 24 }, NULL);
            GuiStatusBar((Rectangle){ GetScreenWidth() - 180 - 100 - 120 + 3, GetScreenHeight() - 24, 120, 24 }, (showGlobalPosition? "POS: GLOBAL" : "POS: RELATIVE"));
            GuiStatusBar((Rectangle){ GetScreenWidth() - 180 - 100 + 2, GetScreenHeight() - 24, 100, 24 }, (mainToolbarState.snapModeActive? "SNAP: ON" : "SNAP: OFF"));
            GuiStatusBar((Rectangle){ GetScreenWidth() - 180, GetScreenHeight() - 24, 180, 24}, TextFormat("GRID: %i px | %i Divs.", gridSpacing*gridSubdivisions, gridSubdivisions));
            GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
            GuiSetStyle(STATUSBAR, TEXT_PADDING, 8);
            //----------------------------------------------------------------------------------------

            // NOTE: If some overlap window is open and main window is locked, we draw a background rectangle
            //if (GuiIsLocked())    // WARNING: It takes one extra frame to process, so we just check required conditions
            if (windowHelpState.windowActive ||
                windowAboutState.windowActive ||
                windowCodegenState.windowActive ||
                showExitWindow ||
                showResetWindow ||
                showIssueReportWindow ||
                showLoadFileDialog ||
                showSaveFileDialog ||
                showExportFileDialog ||
                showLoadTracemapDialog ||
                showLoadTemplateDialog)
            {
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), 0.8f));
            }

            // WARNING: Before drawing the windows, we unlock them
            GuiUnlock();

            // GUI: Help Window
            //----------------------------------------------------------------------------------------
            windowHelpState.windowBounds.height = GetScreenHeight() - 100;
            GuiWindowHelp(&windowHelpState);
            //----------------------------------------------------------------------------------------

            // GUI: About Window
            //----------------------------------------------------------------------------------------
            GuiWindowAbout(&windowAboutState);
            //----------------------------------------------------------------------------------------

            // GUI: Issue Report Window
            //----------------------------------------------------------------------------------------
            if (showIssueReportWindow)
            {
                Rectangle messageBox = { (float)GetScreenWidth()/2 - 300/2, (float)GetScreenHeight()/2 - 190/2 - 20, 300, 190 };
                int result = GuiMessageBox(messageBox, "#220#Report Issue",
                    "Do you want to report any issue or\nfeature request for this program?\n\ngithub.com/raysan5/rguilayout", "#186#Report on GitHub");

                if (result == 1)    // Report issue pressed
                {
                    OpenURL("https://github.com/raysan5/rguilayout/issues");
                    showIssueReportWindow = false;
                }
                else if (result == 0) showIssueReportWindow = false;
            }
            //----------------------------------------------------------------------------------------

            // GUI: Layout Code Generation Window
            //----------------------------------------------------------------------------------------
            GuiWindowCodegen(&windowCodegenState);

            if (windowCodegenState.btnLoadCustomTemplatePressed)
            {
                if (windowCodegenState.customTemplateLoaded)
                {
                    UnloadFileText(windowCodegenState.customTemplate);
                    windowCodegenState.customTemplate = NULL;
                    windowCodegenState.customTemplateLoaded = false;
                }
                else showLoadTemplateDialog = true;
            }

            if (windowCodegenState.btnExportCodePressed)
            {
                if (windowCodegenState.codeTemplateActive == 1) strcpy(outFileName, TextFormat("gui_%s.h", guiConfig.name));
                else strcpy(outFileName, TextFormat("%s.c", guiConfig.name));

                showExportFileDialog = true;
                windowCodegenState.windowActive = false;
                windowCodegenState.btnExportCodePressed = false;
            }
            //----------------------------------------------------------------------------------------

            // GUI: New Layout Window (save)
            //----------------------------------------------------------------------------------------
            if (showResetWindow)
            {
                int message = GuiMessageBox((Rectangle){ GetScreenWidth()/2 - 320/2, GetScreenHeight()/2 - 120/2, 320, 120 }, "#8# Creating new layout", "Do you want to save the current layout?", "Yes;No");

                if (message == 0) showResetWindow = false;
                else if (message == 1)  // Yes
                {
                    strcpy(outFileName, "layout_name.rgl");
                    showSaveFileDialog = true;
                    showResetWindow = false;
                    resetLayout = true;
                }
                else if (message == 2)  // No
                {
                    showResetWindow = false;
                    resetLayout = true;
                }
            }
            //----------------------------------------------------------------------------------------

            // GUI: Exit Window
            //----------------------------------------------------------------------------------------
            if (showExitWindow)
            {
                int result = GuiMessageBox((Rectangle){ GetScreenWidth()/2 - 320/2, GetScreenHeight()/2 - 120/2, 320, 120 }, "#159#Closing rGuiLayout", "Do you want to close without saving?", "Yes;No");

                if (result == 0) showExitWindow = false;
                else if (result == 1) closeWindow = true;
                else if (result == 2)
                {
                    strcpy(outFileName, "layout_name.rgl");
                    showSaveFileDialog = true;
                    showExitWindow = false;
                }
            }
            //----------------------------------------------------------------------------------------

            // GUI: Load File Dialog (and loading logic)
            //----------------------------------------------------------------------------------------
            if (showLoadFileDialog)
            {
#if defined(CUSTOM_MODAL_DIALOGS)
                int result = GuiFileDialog(DIALOG_MESSAGE, "#5#Load raygui layout file", inFileName, "Ok", "Just drag and drop your .rgl layout file!");
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

                        // WARNING: When layout is loaded, anchor object references are not set, they must be reset manually
                        for (int i = 0; i < layout->controlCount; i++) layout->controls[i].ap = &layout->anchors[tempLayout->controls[i].ap->id];

                        // Reinit undo levels for new layout
                        for (int i = 0; i < MAX_UNDO_LEVELS; i++) memcpy(&undoLayouts[i], layout, sizeof(GuiLayout));
                        currentUndoIndex = 0;
                        firstUndoIndex = 0;

                        UnloadLayout(tempLayout);

                        SetWindowTitle(TextFormat("%s v%s | File: %s", toolName, toolVersion, GetFileName(inFileName)));
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
                int result = GuiFileDialog(DIALOG_TEXTINPUT, "#6#Save raygui layout file...", outFileName, "Ok;Cancel", NULL);
                //int result = GuiTextInputBox((Rectangle){ screenWidth/2 - 280/2, screenHeight/2 - 112/2 - 30, 280, 112 }, "#2#Save raygui style file...", NULL, "#2#Save", outFileName, 512, NULL);
#else
                int result = GuiFileDialog(DIALOG_SAVE_FILE, "Save raygui layout file...", outFileName, "*.rgl", "raygui Layout Files (*.rgl)");
#endif
                if (result == 1)
                {
                    // Save file: outFileName
                    // Check for valid extension and make sure it is
                    if (!IsFileNameValid(GetFileName(outFileName))) strcpy(outFileName, "layout.rgl");
                    if ((GetFileExtension(outFileName) == NULL) || !IsFileExtension(outFileName, ".rgl")) strcat(outFileName, ".rgl\0");

                    GuiLayout outLayout = { 0 };
                    memcpy(&outLayout, layout, sizeof(GuiLayout));

                    SaveLayout(&outLayout, outFileName);

                    strcpy(inFileName, outFileName);
                    SetWindowTitle(TextFormat("%s v%s | File: %s", toolName, toolVersion, GetFileName(inFileName)));
                    saveChangesRequired = false;

                #if defined(PLATFORM_WEB)
                    // Download file from MEMFS (emscripten memory filesystem)
                    // NOTE: Second argument must be a simple filename (we can't use directories)
                    // NOTE: Included security check to (partially) avoid malicious code on PLATFORM_WEB
                    if (strchr(outFileName, '\'') == NULL) emscripten_run_script(TextFormat("saveFileFromMEMFSToDisk('%s','%s')", outFileName, GetFileName(outFileName)));
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
                int result = GuiFileDialog(DIALOG_TEXTINPUT, "#7#Export layout as code file...", outFileName, "Ok;Cancel", NULL);
#else
                int result = GuiFileDialog(DIALOG_SAVE_FILE, "Export layout as code file...", outFileName, "*.c;*.h", "Code Files");
#endif
                if (result == 1)
                {
                    // Check for valid extension and make sure it is
                    if (!IsFileNameValid(GetFileName(outFileName))) strcpy(outFileName, "layout_code.c");
                    if ((GetFileExtension(outFileName) == NULL) ||
                        (!IsFileExtension(outFileName, ".c") && !IsFileExtension(outFileName, ".h"))) strcat(outFileName, ".h\0");

                    // Write code string to file
                    FILE *ftool = fopen(outFileName, "wt");
                    fprintf(ftool, windowCodegenState.codeText);
                    fclose(ftool);

                #if defined(PLATFORM_WEB)
                    // Download file from MEMFS (emscripten memory filesystem)
                    // NOTE: Second argument must be a simple filename (we can't use directories)
                    // NOTE: Included security check to (partially) avoid malicious code on PLATFORM_WEB
                    if (strchr(outFileName, '\'') == NULL) emscripten_run_script(TextFormat("saveFileFromMEMFSToDisk('%s','%s')", outFileName, GetFileName(outFileName)));
                #endif
                }

                if (result >= 0) showExportFileDialog = false;
            }
            //----------------------------------------------------------------------------------------

            // GUI: Load Tracemap File Dialog
            //----------------------------------------------------------------------------------------
            if (showLoadTracemapDialog)
            {
#if defined(CUSTOM_MODAL_DIALOGS)
                int result = GuiFileDialog(DIALOG_MESSAGE, "#12#Load tracemap image", inFileName, "Ok", "Just drag and drop your .png tracemap!");
#else
                int result = GuiFileDialog(DIALOG_OPEN_FILE, "Load tracemap image", inFileName, "*.png", "Tracemap Image (*.png)");
#endif
                if (result == 1)
                {
                    // Load layout file
                    Texture2D texture = LoadTexture(inFileName);

                    if (texture.id > 0)
                    {
                        if (tracemap.texture.id > 0) UnloadTexture(tracemap.texture);
                        tracemap.texture = texture;
                        tracemap.rec = (Rectangle){ 48 + layout->refWindow.x, 48 + layout->refWindow.y, tracemap.texture.width, tracemap.texture.height };
                        tracemap.visible = true;
                        tracemap.alpha = 0.7f;

                        mainToolbarState.tracemapLoaded = true;
                    }
                    else inFileName[0] = '\0';
                }

                if (result >= 0) showLoadTracemapDialog = false;
            }
            //----------------------------------------------------------------------------------------

            // GUI: Load Custom Code Template File Dialog
            //----------------------------------------------------------------------------------------
            if (showLoadTemplateDialog)
            {
#if defined(CUSTOM_MODAL_DIALOGS)
                int result = GuiFileDialog(DIALOG_MESSAGE, "#12#Load code template", inFileName, "Ok", "Custom code template\nonly available on desktop version");
#else
                int result = GuiFileDialog(DIALOG_OPEN_FILE, "Load code template", inFileName, "*.h;*.c", "Tracemap Image (*.h, *.c)");
#endif
                if (result == 1)
                {
                    // Load custom code template file
                    windowCodegenState.customTemplate = LoadFileText(inFileName);

                    if (windowCodegenState.customTemplate != NULL)
                    {
                        // TODO: Check if custom template contains required variables

                        windowCodegenState.customTemplateLoaded = true;
                    }
                    else inFileName[0] = '\0';
                }

                if (result >= 0) showLoadTemplateDialog = false;
            }
            //----------------------------------------------------------------------------------------

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadLayout(layout);                   // Unload raygui layout
    UnloadTexture(tracemap.texture);        // Unload tracemap texture (if loaded)

    RL_FREE(undoLayouts);                   // Free undo layouts array (allocated with RL_CALLOC)
    RL_FREE(windowCodegenState.codeText);   // Free loaded codeText memory

#if defined(PLATFORM_DESKTOP)
    // Save application init configuration for next run
    //--------------------------------------------------------------------------------------
    rini_data appConfig = rini_load(NULL);   // Create empty config with 32 entries (RINI_MAX_CONFIG_CAPACITY)

    windowMaximized = (int)IsWindowMaximized();

    // Define header comment lines
    rini_set_comment_line(&appConfig, NULL);   // Empty comment line, but including comment prefix delimiter
    rini_set_comment_line(&appConfig, TextFormat("%s initialization configuration options", TOOL_NAME));
    rini_set_comment_line(&appConfig, NULL);
    rini_set_comment_line(&appConfig, "NOTE: This file is loaded at application startup,");
    rini_set_comment_line(&appConfig, "if file is not found, default values are applied");
    rini_set_comment_line(&appConfig, NULL);

    //rini_set_value(&config, "SHOW_WINDOW_WELCOME", (int)windowAboutState.showSplash, "Show welcome window at initialization");
    rini_set_value(&appConfig, "SHOW_WINDOW_INFO", (int)mainToolbarState.showControlPanelActive, "Show control panel");
    rini_set_value(&appConfig, "SHOW_CONTROL_TOOLTIPS", (int)mainToolbarState.showTooltips, "Show controls tooltips on mouse hover");
    rini_set_value(&appConfig, "INIT_WINDOW_MAXIMIZED", (int)windowMaximized, "Initialize window maximized");
    rini_set_value(&appConfig, "SHOW_IMAGE_GRID", (int)mainToolbarState.showGridActive, "Show image grid");
    rini_set_value(&appConfig, "GRID_LINES_SPACING", (int)gridSpacing, "Grid lines spacing");
    rini_set_value(&appConfig, "GUI_VISUAL_STYLE", (int)mainToolbarState.visualStyleActive, "UI visual style selected");
    //rini_set_value(&config, "CLEAN_WINDOW_MODE", (int)mainToolbarState.cleanModeActive, "Clean window mode enabled");

    rini_save(appConfig, TextFormat("%s/%s", GetApplicationDirectory(), TOOL_CONFIG_FILENAME));
    rini_unload(&appConfig);
    //--------------------------------------------------------------------------------------
#endif

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
    printf("// Copyright (c) 2017-2025 raylib technologies (@raylibtech)                    //\n");
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

    char templateFile[512] = { 0 };     // Template file name

    //int outputFormat = 0;               // Supported output formats

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

        // TODO: CLI: Support codegen options: exportAnchors, defineRecs, fullComments...
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

        GuiLayoutConfig guiConfig = { 0 };
        memset(&guiConfig, 0, sizeof(GuiLayoutConfig));
        strcpy(guiConfig.name, "window_codegen");
        strcpy(guiConfig.version, toolVersion);
        strcpy(guiConfig.company, "raylib technologies");
        strcpy(guiConfig.description, "tool description");
        guiConfig.exportAnchors = true;
        guiConfig.defineRecs = false;
        guiConfig.fullComments = true;

        // Generate C code for gui layout->controls
        char *guiTemplateCustom = NULL;
        if (templateFile[0] != '\0') guiTemplateCustom = LoadFileText(templateFile);

        char *toolstr = NULL;
        if (guiTemplateCustom != NULL)
        {
            toolstr = GenLayoutCode(guiTemplateCustom, layout, (Vector2){ 0, 0 }, guiConfig);
            RL_FREE(guiTemplateCustom);
        }
        else toolstr = GenLayoutCode(guiTemplateStandardCode, layout, (Vector2){ 0, 0 }, guiConfig);

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
            char anchorName[MAX_ANCHOR_NAME_LENGTH] = { 0 };

            fgets(buffer, 256, rglFile);

            while (!feof(rglFile))
            {
                switch (buffer[0])
                {
                    case 'r':
                    {
                        sscanf(buffer, "r %f %f %f %f", &layout->refWindow.x, &layout->refWindow.y, &layout->refWindow.width, &layout->refWindow.height);

                        // NOTE: Reference window defines anchor[0]
                        layout->anchors[0].id = 0;
                        layout->anchors[0].ap = NULL;
                        layout->anchors[0].x = layout->refWindow.x;
                        layout->anchors[0].y = layout->refWindow.y;
                        layout->anchors[0].enabled = true;
                        strcpy(layout->anchors[0].name, "refPoint");

                        layout->anchorCount++;
                    } break;
                    case 'a':
                    {
                        int enabled = 0;
                        sscanf(buffer, "a %d %s %d %d %d",
                            &layout->anchors[layout->anchorCount].id,
                            anchorName,
                            &layout->anchors[layout->anchorCount].x,
                            &layout->anchors[layout->anchorCount].y,
                            &enabled);

                        if (layout->anchors[layout->anchorCount].id > 0)
                        {
                            layout->anchors[layout->anchorCount].enabled = (enabled? true : false);
                            strcpy(layout->anchors[layout->anchorCount].name, anchorName);

                            layout->anchorCount++;
                        }
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

                        // NOTE: refWindow offset (anchor[0]) must be added to controls with no anchor
                        if (anchorId == 0)
                        {
                            layout->controls[layout->controlCount].rec.x += layout->refWindow.x;
                            layout->controls[layout->controlCount].rec.y += layout->refWindow.y;
                        }

                        layout->controlCount++;
                    } break;
                    default: break;
                }

                fgets(buffer, 256, rglFile);
            }

            fclose(rglFile);

            // NOTE: refWindow offset (anchor[0]) must be added to all anchors
            for (int i = 1; i < layout->anchorCount; i++)
            {
                layout->anchors[i].x += layout->refWindow.x;
                layout->anchors[i].y += layout->refWindow.y;
            }
        }
    }
    else
    {
        // Define reference anchor
        layout->anchors[0].x = 0;
        layout->anchors[0].y = 40;
        layout->anchors[0].enabled = true;
        layout->refWindow.x = 0;
        layout->refWindow.y = 40;
        layout->anchorCount = 1;
    }

    return layout;
}

// Unload layout
static void UnloadLayout(GuiLayout *layout)
{
    RL_FREE(layout);
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

        if (i == 0) strcpy(layout->anchors[i].name, "refPoint");
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

    layout->refWindow = (Rectangle){ 0, 40, -1, -1 };
    layout->anchorCount = 0;
    layout->controlCount = 0;
}

// Save layout information as text file
static void SaveLayout(GuiLayout *layout, const char *fileName)
{
    #define RGL_FILE_VERSION_TEXT "4.0"

    FILE *rglFile = fopen(fileName, "wt");

    if (rglFile != NULL)
    {
        // Write some description comments
        fprintf(rglFile, "#\n# rgl layout text file (v%s) - raygui layout file generated using rGuiLayout\n#\n", RGL_FILE_VERSION_TEXT);
        fprintf(rglFile, "# Number of controls:     %i\n#\n", layout->controlCount);
        fprintf(rglFile, "# Ref. window:    r <x> <y> <width> <height>\n");
        fprintf(rglFile, "# Anchor info:    a <id> <name> <posx> <posy> <enabled>\n");
        fprintf(rglFile, "# Control info:   c <id> <type> <name> <rectangle> <anchor_id> <text>\n#\n");

        // Write reference window and reference anchor (anchor[0])
        fprintf(rglFile, "r %i %i %i %i\n", (int)layout->refWindow.x, (int)layout->refWindow.y, (int)layout->refWindow.width, (int)layout->refWindow.height);
        //fprintf(rglFile, "a %03i %s %i %i %i\n", layout->anchors[0].id, layout->anchors[0].name, layout->anchors[0].x, layout->anchors[0].y, layout->anchors[0].enabled);

        for (int i = 1; i < MAX_ANCHOR_POINTS; i++)
        {
            if (layout->anchors[i].enabled)
            {
                fprintf(rglFile, "a %03i %s %i %i %i\n",
                    layout->anchors[i].id,
                    layout->anchors[i].name,
                    (int)(layout->anchors[i].x - (int)layout->refWindow.x),
                    (int)(layout->anchors[i].y - (int)layout->refWindow.y),
                    layout->anchors[i].enabled);
            }
        }

        for (int i = 0; i < layout->controlCount; i++)
        {
            if (layout->controls[i].ap->id == 0)
            {
                fprintf(rglFile, "c %03i %i %s %i %i %i %i %i %s\n",
                    layout->controls[i].id,
                    layout->controls[i].type,
                    layout->controls[i].name,
                    (int)layout->controls[i].rec.x - (int)layout->refWindow.x,
                    (int)layout->controls[i].rec.y - (int)layout->refWindow.y,
                    (int)layout->controls[i].rec.width,
                    (int)layout->controls[i].rec.height,
                    layout->controls[i].ap->id,
                    layout->controls[i].text);
            }
            else
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
        }

        fclose(rglFile);
    }

/*
    if (binary)
    {
        #define RGL_FILE_VERSION_BINARY 400

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
            // Ref window data
            // 8       | 4       | int        | ref window x
            // 12      | 4       | int        | ref window x
            // 16      | 4       | int        | ref window width
            // 20      | 4       | int        | ref window height
            //
            // Anchor info data:    a <id> <name> <posx> <posy> <enabled>\n");
            // 24      | 4       | int        | Anchor count
            // for (i = 1; i < anchorCount; i++)
            // {
            //    x    | 4       | int        | Anchor id
            //    x    | 64      | char       | Anchor name
            //    x    | 4       | int        | Anchor posX
            //    x    | 4       | int        | Anchor posY
            //    x    | 4       | int        | Anchor enabled?
            // }
            //
            // Control info data:   c <id> <type> <name> <rectangle> <anchor_id> <text>\n#\n")
            // x       | 4       | int        | Control count
            // for (i = 0; i < countCount; i++)
            // {
            //    x    | 4       | int        | Control id
            //    x    | 4       | int        | Control type
            //    x    | 64      | char       | Control name
            //    x    | 4       | int        | Control rec x
            //    x    | 4       | int        | Control rec y
            //    x    | 4       | int        | Control rec width
            //    x    | 4       | int        | Control rec height
            //    x    | 4       | int        | Control anchor id
            //    x    | 128     | char       | Control text
            // }

            char signature[5] = "rGL ";
            short version = RGL_FILE_VERSION_BINARY;
            short reserved = 0;

            fwrite(signature, 1, 4, rglFile);
            fwrite(&version, sizeof(short), 1, rglFile);
            fwrite(&reserved, sizeof(short), 1, rglFile);

            int rec[4] = { (int)layout->refWindow.x, (int)layout->refWindow.y, (int)layout->refWindow.width, (int)layout->refWindow.height };
            fwrite(&rec[0], sizeof(int), 1, rglFile);
            fwrite(&rec[1], sizeof(int), 1, rglFile);
            fwrite(&rec[2], sizeof(int), 1, rglFile);
            fwrite(&rec[3], sizeof(int), 1, rglFile);

            fwrite(&layout->anchorCount, sizeof(int), 1, rglFile);

            // WARNING: anchor[0] is already implicit in ref window
            for (int i = 1, x = 0, y = 0, e = 0; i < anchorCount; i++)
            {
                fwrite(&layout->anchors[i].id, sizeof(int), 1, rglFile); // Anchor id
                fwrite(&layout->anchors[i].name, sizeof(char), MAX_ANCHOR_NAME_LENGTH, rglFile); // Anchor name
                x = (int)(layout->anchors[i].x - (int)layout->refWindow.x);
                fwrite(&x, sizeof(int), 1, rglFile); // Anchor posX
                y = (int)(layout->anchors[i].y - (int)layout->refWindow.y),
                fwrite(&y, sizeof(int), 1, rglFile); // Anchor posY
                e = (layout->anchors[i].enabled? 1 : 0);
                fwrite(&e, sizeof(int), 1, rglFile); // Anchor enabled?
            }

            fwrite(&layout->controlCount, sizeof(int), 1, rglFile);

            for (int i = 0; i < countCount; i++)
            {
                fwrite(&layout->controls[i].id, sizeof(int), 1, rglFile); // Control id
                fwrite(&layout->controls[i].type, sizeof(int), 1, rglFile); // Control type
                fwrite(&layout->controls[i].name, sizeof(char), MAX_CONTROL_NAME_LENGTH, rglFile); // Control name
                rec = (Rectangle){ (int)layout->controls[i].rec.x, (int)layout->controls[i].rec.y, (int)layout->controls[i].rec.width, (int)layout->controls[i].rec.height };
                fwrite(&rec[0], sizeof(int), 1, rglFile); // Control rec x
                fwrite(&rec[1], sizeof(int), 1, rglFile); // Control rec y
                fwrite(&rec[2], sizeof(int), 1, rglFile); // Control rec width
                fwrite(&rec[3], sizeof(int), 1, rglFile); // Control rec height
                fwrite(&layout->controls[i].ap->id, sizeof(int), 1, rglFile); // Control anchor id
                fwrite(&layout->controls[i].text, sizeof(char), MAX_CONTROL_TEXT_LENGTH, rglFile); // Control text
            }

            fclose(rglFile);
        }
    }
*/
}

/*
// Check if a rectangle is contained within another
static bool IsRecContainedInRec(Rectangle container, Rectangle rec)
{
    bool result = false;

    if ((rec.x >= container.x) && (rec.y >= container.y) &&
        ((rec.x + rec.width) <= (container.x + container.width)) &&
        ((rec.y + rec.height) <= (container.y + container.height))) result = true;

    return result;
}
*/

