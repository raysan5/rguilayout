/*******************************************************************************************
*
*   Visualization v1.0.0 - Visualization tool bar
*
*   MODULE USAGE:
*       #define GUI_VISUALIZATION_IMPLEMENTATION
*       #include "gui_visualization.h"
*
*       INIT: GuiVisualizationState state = InitGuiVisualization();
*       DRAW: GuiVisualization(&state);
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

// WARNING: raygui implementation is expected to be defined before including this header
#undef RAYGUI_IMPLEMENTATION
#include "raygui.h"

#ifndef GUI_VISUALIZATION_H
#define GUI_VISUALIZATION_H

typedef struct {
    Vector2 visualizationAnchor;
    
    bool centerPressed;
    bool fullScreenPressed;
    float zoomValue;
    int scalingActive;
    int helpersActive;
    int backgroundActive;
    int channelsActive;

    // Custom state variables (depend on development software)
    // NOTE: This variables should be added manually if required

} GuiVisualizationState;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Internal Module Functions Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
GuiVisualizationState InitGuiVisualization(void);
void GuiVisualization(GuiVisualizationState *state);

#ifdef __cplusplus
}
#endif

#endif // GUI_VISUALIZATION_H

/***********************************************************************************
*
*   GUI_VISUALIZATION IMPLEMENTATION
*
************************************************************************************/
#if defined(GUI_VISUALIZATION_IMPLEMENTATION)

#include "raygui.h"

GuiVisualizationState InitGuiVisualization(void)
{
    GuiVisualizationState state = { 0 };

    state.visualizationAnchor = (Vector2){ 0, 0 };
    
    state.centerPressed = false;
    state.fullScreenPressed = false;
    state.zoomValue = 0.0f;
    state.scalingActive= 0;
    state.helpersActive= 0;
    state.backgroundActive= 0;
    state.channelsActive= 0;

    // Custom variables initialization

    return state;
}

void GuiVisualization(GuiVisualizationState *state)
{
    const char *centerText = "#0";
    const char *fullScreenText = "#0";
    const char *zoomText = "ZOOM:";
    const char *scalingText = "# Point; # Bilinear";
    const char *helpersText = "#0;#1;#2";
    const char *backgroundText = "#0;#1;#2;#3";
    const char *channelsText = "#0;#1;#2;#3;#4";
    
    GuiPanel((Rectangle){ state->visualizationAnchor.x + 0, state->visualizationAnchor.y + -1, 1000, 40 });
    state->centerPressed = GuiButton((Rectangle){ state->visualizationAnchor.x + 15, state->visualizationAnchor.y + 7, 25, 24 }, centerText); 
    state->fullScreenPressed = GuiButton((Rectangle){ state->visualizationAnchor.x + 959, state->visualizationAnchor.y + 7, 24, 24 }, fullScreenText); 
    state->zoomValue = GuiSliderBar((Rectangle){ state->visualizationAnchor.x + 82, state->visualizationAnchor.y + 14, 200, 10 }, zoomText, state->zoomValue, 0, 100, true);
    state->scalingActive = GuiToggleGroup((Rectangle){ state->visualizationAnchor.x + 372, state->visualizationAnchor.y + 9, 60, 20 }, scalingText, state->scalingActive);
    state->helpersActive = GuiToggleGroup((Rectangle){ state->visualizationAnchor.x + 828, state->visualizationAnchor.y + 9, 20, 20 }, helpersText, state->helpersActive);
    state->backgroundActive = GuiToggleGroup((Rectangle){ state->visualizationAnchor.x + 540, state->visualizationAnchor.y + 9, 20, 20 }, backgroundText, state->backgroundActive);
    state->channelsActive = GuiToggleGroup((Rectangle){ state->visualizationAnchor.x + 672, state->visualizationAnchor.y + 9, 20, 20 }, channelsText, state->channelsActive);
}

#endif // GUI_VISUALIZATION_IMPLEMENTATION
