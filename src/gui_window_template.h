/*******************************************************************************************
*
*   $(GUILAYOUT_NAME_PASCALCASE) v$(GUILAYOUT_VERSION) - $(GUILAYOUT_DESCRIPTION)
*
*   MODULE USAGE:
*       #define GUI_$(GUILAYOUT_NAME_UPPERCASE)_IMPLEMENTATION
*       #include "gui_$(GUILAYOUT_NAME).h"
*
*       INIT: Gui$(GUILAYOUT_NAME_PASCALCASE)State state = InitGui$(GUILAYOUT_NAME_PASCALCASE)();
*       DRAW: Gui$(GUILAYOUT_NAME_PASCALCASE)(&state);
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

#ifndef GUI_$(GUILAYOUT_NAME_UPPERCASE)_H
#define GUI_$(GUILAYOUT_NAME_UPPERCASE)_H

$(GUILAYOUT_STRUCT_TYPE)

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
$(GUILAYOUT_FUNCTIONS_DECLARATION_H)

#ifdef __cplusplus
}
#endif

#endif // GUI_$(GUILAYOUT_NAME_UPPERCASE)_H

/***********************************************************************************
*
*   GUI_$(GUILAYOUT_NAME_UPPERCASE) IMPLEMENTATION
*
************************************************************************************/
#if defined(GUI_$(GUILAYOUT_NAME_UPPERCASE)_IMPLEMENTATION)

#include "raygui.h"

$(GUILAYOUT_FUNCTION_INITIALIZE_H)

$(GUILAYOUT_FUNCTION_DRAWING_H)

#endif // GUI_$(GUILAYOUT_NAME_UPPERCASE)_IMPLEMENTATION
