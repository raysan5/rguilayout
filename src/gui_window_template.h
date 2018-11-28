/*******************************************************************************************
*
*   raygui layout - $(GUILAYOUT_NAME)
*
*   MODULE USAGE:
*       #define GUI_$(GUILAYOUT_NAME)_IMPLEMENTATION
*       #include "gui_$(GUILAYOUT_NAME).h"
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

#ifndef $(GUILAYOUT_NAME)_H
#define $(GUILAYOUT_NAME)_H

$(GUILAYOUT_STRUCT_TYPE)

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
$(GUILAYOUT_FUNCTIONS_DECLARATION)

#ifdef __cplusplus
}
#endif

#endif // $(GUILAYOUT_NAME)_H

/***********************************************************************************
*
*   $(GUILAYOUT_NAME) IMPLEMENTATION
*
************************************************************************************/
#if defined($(GUILAYOUT_NAME)_IMPLEMENTATION)

#include "raygui.h"

$(GUILAYOUT_FUNCTIONS_DEFINITION)

#endif // $(GUILAYOUT_NAME)_IMPLEMENTATION