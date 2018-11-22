#include "raylib.h"

#ifndef GUI_WINDOW_EXPORT_CODE_H
#define GUI_WINDOW_EXPORT_CODE_H

// Gui window structure declaration
typedef struct {
    Vector2 position;
    bool active;
    int width;
    int height;
    bool chkLicenseChecked;
} GuiWindowAboutState;

typedef struct {
    // Main window properties
    bool active;
    Rectangle rec;
    // Name text
    int nameTextSize;  
    unsigned char nameText[32];    
    bool nameTextEdit;
    // Version text
    int versionTextSize;
    unsigned char versionText[16];
    bool versionTextEdit;
    // Window size values
    int windowWidth;
    bool windowWidthEdit;
    int windowHeight;
    bool windowHeightEdit;
    // Company text
    int companyTextSize;
    unsigned char companyText[32];
    bool companyTextEdit;
    // Description text
    int descriptionTextSize;
    unsigned char descriptionText[256];
    bool descriptionTextEdit;
    // Export options
    bool exportRects;
    bool exportText;
    bool exportAnchors;
    bool exportVariables;
    bool exportComments;
    bool cropToWindow;
    // Export button
    bool exportButtonPressed;
    
} GuiWindowExportCodeState;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
GuiWindowExportCodeState InitGuiWindowExportCode(void);
void GuiWindowExportCode (GuiWindowExportCodeState *state);

#ifdef __cplusplus
}
#endif

#endif // GUI_WINDOW_EXPORT_CODE_H

/***********************************************************************************
*
*   GUI_WINDOW_EXPORT_CODE IMPLEMENTATION
*
************************************************************************************/
#if defined(GUI_WINDOW_EXPORT_CODE_IMPLEMENTATION)

#include "raygui.h"

// Init Window About
GuiWindowAboutState InitGuiWindowAbout(void)
{
    GuiWindowAboutState state = {
        .position = (Vector2){ GetScreenWidth()/2 - 330/2, GetScreenHeight()/2 - 380/2 },
        .active = false,
        .width = 330,
        .height = 380,
        .chkLicenseChecked = true
    };
    
    return state;
}

GuiWindowExportCodeState InitGuiWindowExportCode(void)
{
    GuiWindowAboutState state = { 0 };
    // Main window properties
    state.active = false;
    state.rec = (Rectangle){ GetScreenWidth()/2 - 200, GetScreenHeight()/2 - 128 , 400, 256 };
    // Name text
    state.nameTextSize = 32;
    strcpy(state.nameText, "layout_file_name");
    state.nameTextEdit = false;
    // Version text
    state.versionTextSize = 16;
    strcpy(state.versionText, "layout_file_name");
    state.versionTextEdit = false;
    // Window size values
    state.windowWidth = 0;
    state.windowWidthEdit = false;
    state.windowHeight = 0;
    state.windowHeightEdit = false;
    // Company text
    state.companyTextSize = 32;
    strcpy(state.companyText, "layout_file_name");
    state.companyTextEdit = false;
    // Description text
    state.descriptionTextSize = 256;
    strcpy(state.descriptionText, "tool description");
    state.descriptionTextEdit = false;
    // Export options    
    state,exportRects = false;
    state.exportText = false;
    state.exportAnchors = false;
    state.exportVariables = false;
    state.exportComments = false;
    state.cropToWindow = false;
    // Export button
    exportButtonPressed = false;
    
    return state;
}

// Gui export code
void GuiWindowExportCode(GuiWindowExportCodeState *state)
{
    if (state->active)
    {
        state->rec = (Rectangle){ GetScreenWidth()/2 - 200, GetScreenHeight()/2 - 128 , 400, 256 };
        
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), 0.85f));
        // Main window
        state->active = !GuiWindowBox(state->rec, "Export Code Options");
        // Name text
        GuiLabel((Rectangle){state->rec.x + 10, state->rec.y + 35, 65, 25 }, "Name:")
        if (GuiTextBox((Rectangle){ state->rec.x + 75, state->rec.y + 35, 135, 25 }, state->nameText, state->nameTextSize, state->nameTextEdit)) state->nameTextEdit = !state->nameTextEdit;
        // Version text
        GuiLabel((Rectangle){state->rec.x + 225, state->rec.y + 35, 50, 25 }, "Version:")
        if (GuiTextBox((Rectangle){ state->rec.x + 275, state->rec.y + 35, 115, 25 }, state->versionText, state->versionTextSize, state->versionTextEdit)) state->versionTextEdit = !state->versionTextEdit;
        // Window size values
        GuiLabel((Rectangle){state->rec.x + 10, state->rec.y + 65, 65, 25 }, "Window size:")
        if (GuiValueBox((Rectangle){ state->rec.x + 75, state->rec.y + 65, 60, 25 }, &state->windowWidth, 400, 1000, state->windowWidthEdit)) state->windowWidthEdit = !state->windowWidthEdit;
        GuiLabel((Rectangle){state->rec.x + 140, state->rec.y + 65, 10, 25 }, "x:")
        if (GuiValueBox((Rectangle){ state->rec.x + 150, state->rec.y + 65, 60, 25 }, &state->windowHeight, 400, 1000, state->windowHeightEdit)) state->windowHeightEdit = !state->windowHeightEdit;
        // Company text
        GuiLabel((Rectangle){state->rec.x + 225, state->rec.y + 65, 50, 25 }, "Company:")
        if(GuiTextBox((Rectangle){ state->rec.x + 275, state->rec.y + 65, 115, 25 }, state->companyText, state->companyTextSize, state->companyTextEdit)) state->companyTextEdit = !state->companyTextEdit;
        // Description multitext
        GuiLabel((Rectangle){state->rec.x + 10, state->rec.y + 95, 65, 25 }, "Description:")
        if (GuiTextBoxMulti((Rectangle){ state->rec.x + 75, state->rec.y + 95, 315, 55 }, state->descriptionText, state->descriptionTextSize, state->descriptionTextEdit)) state->descriptionTextEdit = !state->descriptionTextEdit;
        // Export options
            // Define rectangles
            state->exportRects = GuiCheckBoxEx((Rectangle){ state->rec.x + 10, state->rec.y + 160, 15, 15 }, state->exportRects, "Define Rectangles");
            // Define text const
            state->exportText = GuiCheckBoxEx((Rectangle){ state->rec.x + 10, state->rec.y + 180, 15, 15 }, state->exportText, "Define text const");
            // Define Export anchors
            state->exportAnchors = GuiCheckBoxEx((Rectangle){ state->rec.x + 10, state->rec.y + 200, 15, 15 }, state->exportAnchors, "Export anchors");
            // Full variables
            state->exportVariables = GuiCheckBoxEx((Rectangle){ exportWindowPos.x + 140, exportWindowPos.y + 160, 15, 15 }, state->exportVariables, "Full variables");
            // Full comments
            state->exportComments = GuiCheckBoxEx((Rectangle){ state->rec.x + 140, state->rec.y + 180, 15, 15 }, state->exportComments, "Full comments");
            // Crop to window
            state->cropToWindow = GuiCheckBoxEx((Rectangle){ state->rec.x + 140, state->rec.y + 200, 15, 15 }, state->cropToWindow, "Crop to Window");
        // Export button
        exportButtonPressed = GuiButton((Rectangle){ state->rec.x + 275, state->rec.y + 185, 115, 30 }, "Export Code");  
    }      
}

// Gui about window
void GuiWindowAbout(GuiWindowAboutState *state)
{    
    // NOTE: const string literals are most-probably stored in read-only data section
    const char *lblNameVersionText = "rFXGen v1.0";
    const char *lblDateText = "(Dec. 2018)";
    const char *lblDescriptionText = "A simple and easy-to-use sounds generator";
    const char *lblUsedLibsText = "Used libraries:";
    const char *linkraylibText = "www.raylib.com";
    const char *linkGitraylibText = "github.com/raysan5/raylib";
    const char *linkGitrayguiText = "github.com/raysan5/raygui";
    const char *lblDevelopersText = "Developers:";
    const char *lblDev01Text = "- Ramon Santamaria (              )";
    const char *linkDev01Text = "@raysan5";
    const char *lblCopyrightText = "Copyright (c) 2019 raylib technologies (                 )";
    const char *linkraylibtech = "@raylibtech";
    const char *lblMoreInfoText = "More info:";
    const char *linkToolWebText = "www.raylibtech.com/rfxgen";
    const char *lblSupportText = "Support:";
    const char *linkMailText = "ray@raylibtech.com";
    
    const int toolColor = 0x5197d4ff;

    if (state->active)
    {
        
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), 0.85f));

        state->active = !GuiWindowBox((Rectangle){ state->position.x + 0, state->position.y + 0, state->width, state->height }, "About rFxGen");
        
        // TODO: Create a color rectangle or color-config panel...
        DrawRectangle(state->position.x + 1, state->position.y + 5 + 20, 330 - 2, 90 - 5, GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_DISABLED)));
        
#if defined(VERSION_ONE)
        GuiLabel((Rectangle){ state->position.x + 85, state->position.y + 60, 200, 25 }, FormatText("%s ONE %s", lblNameVersionText, lblDateText));
#else
        GuiLabel((Rectangle){ state->position.x + 85, state->position.y + 60, 200, 25 }, FormatText("%s ZERO %s", lblNameVersionText, lblDateText));
#endif

        GuiLabel((Rectangle){ state->position.x + 85, state->position.y + 80, 225, 20 }, lblDescriptionText);

        GuiLine((Rectangle){ state->position.x + 0, state->position.y + 100, 330, 20 }, 1);
        GuiLabel((Rectangle){ state->position.x + 10, state->position.y + 110, 126, 25 }, lblUsedLibsText);


        if (GuiLabelButton((Rectangle){ state->position.x + 155, state->position.y + 130, 126, 25 }, linkraylibText)) { OpenURL("https://www.raylib.com/"); }
        if (GuiLabelButton((Rectangle){ state->position.x + 155, state->position.y + 150, 165, 25 }, linkGitraylibText)) { OpenURL("https://github.com/raysan5/raylib"); }
        if (GuiLabelButton((Rectangle){ state->position.x + 155, state->position.y + 170, 165, 25 }, linkGitrayguiText)) { OpenURL("https://github.com/raysan5/raygui"); }

        GuiLine((Rectangle){ state->position.x + 10, state->position.y + 200, 310, 20 }, 1);
        GuiLabel((Rectangle){ state->position.x + 10, state->position.y + 210, 80, 25 }, lblDevelopersText);
        GuiLabel((Rectangle){ state->position.x + 20, state->position.y + 230, 180, 25 }, lblDev01Text);
        if (GuiLabelButton((Rectangle){ state->position.x + 130, state->position.y + 230, 56, 25 }, linkDev01Text)) { OpenURL("https://twitter.com/raysan5"); }
        GuiLine((Rectangle){ state->position.x + 10, state->position.y + 250, 310, 20 }, 1);
        GuiLabel((Rectangle){ state->position.x + 10, state->position.y + 265, 289, 25 }, lblCopyrightText);
        if (GuiLabelButton((Rectangle){ state->position.x + 215, state->position.y + 265, 76, 25 }, linkraylibtech)) { OpenURL("https://twitter.com/raylibtech"); }
        GuiLabel((Rectangle){ state->position.x + 10, state->position.y + 285, 80, 25 }, lblMoreInfoText);
        GuiLabel((Rectangle){ state->position.x + 10, state->position.y + 305, 80, 25 }, lblSupportText);
        if (GuiLabelButton((Rectangle){ state->position.x + 95, state->position.y + 285, 165, 25 }, linkToolWebText)) { OpenURL("https://www.raylibtech.com/"); }
        if (GuiLabelButton((Rectangle){ state->position.x + 95, state->position.y + 305, 165, 25 }, linkMailText)) { OpenURL("mailto:ray@raylibtech.com"); }
        GuiLine((Rectangle){ state->position.x + 0, state->position.y + 325, 330, 20 }, 1);
#if defined(VERSION_ONE)
        state->chkLicenseChecked = GuiCheckBoxEx((Rectangle){ state->position.x + 10, state->position.y + 350, 15, 15 }, state->chkLicenseChecked, "License Agreement (EULA)");
#else
        GuiDisable();state->chkLicenseChecked = GuiCheckBoxEx((Rectangle){ state->position.x + 10, state->position.y + 350, 15, 15 }, state->chkLicenseChecked, "License Agreement (EULA)"); GuiEnable();
        if (GuiButton((Rectangle){ state->position.x + 175, state->position.y + 345, 70, 25 }, "Be ONE!")) { OpenURL("https://raylibtech.itch.io/rfxgen"); }
#endif
        if (GuiButton((Rectangle){ state->position.x + 250, state->position.y + 345, 70, 25 }, "Close")) state->active = false;
    }
}

#endif // GUI_WINDOW_ABOUT_IMPLEMENTATION