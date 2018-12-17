/*******************************************************************************************
*
*   raygui layout - $(GUILAYOUT_NAME)
*
*   MODULE USAGE:
*       #define GUI_$(GUILAYOUT_NAME_UPPER)_IMPLEMENTATION
*       #include "gui_$(GUILAYOUT_NAME_LOWER).h"
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

#ifndef GUI_$(GUILAYOUT_NAME_UPPER)_H
#define GUI_$(GUILAYOUT_NAME_UPPER)_H

$(GUILAYOUT_STRUCT_TYPE)

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
$(GUILAYOUT_FUNCTIONS_DECLARATION_H)

#ifdef __cplusplus
}
#endif

#endif // GUI_$(GUILAYOUT_NAME_UPPER)_H

/***********************************************************************************
*
*   GUI_$(GUILAYOUT_NAME_UPPER) IMPLEMENTATION
*
************************************************************************************/
#if defined(GUI_$(GUILAYOUT_NAME_UPPER)_IMPLEMENTATION)

#include "raygui.h"

$(GUILAYOUT_FUNCTION_INITIALIZE_H)

$(GUILAYOUT_FUNCTION_DRAWING_H)

#endif // GUI_$(GUILAYOUT_NAME_UPPER)_IMPLEMENTATION
