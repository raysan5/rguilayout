# rGuiLayout Usage

rGuiLayout is a tool to create `raygui` layouts visually.

Layouts basically consist of **controls** and those controls could be linked to **anchors** for convenience.

rGuiLayout supports several edit modes depending on the element to edit, basically 3 edition elements are available:

 - [Controls](#controls)
 - [Anchors](#anchors)
 - [Tracemap](#tracemap)

Once a layout is build it can be saved as a `.rgl` file for further loading or exported as code, as a `.c`/`.h` file. 

Code is generated from the layout and multiple configuration options are available. Check [Code Generation](#code-generation)

There are also multiple layout configuration options available to help on that task.

## Layout Edit Options

### Grid and Snap mode
 
There is a grid to place layout elements (controls, anchors, tracemap). 
Grid size and subdivisions can be configured with a shortcut: 
   
  > `RIGHT_ALT + LEFT/RIGHT`: Select division pixel size
  > `RIGHT_ALT + UP/DOWN`: Set grid spacing (considering subdivisions)
  
Snap mode can be activated to snap elements placement to grid subdivisions:

  > `LEFT_ALT + S`: Toggle snap mode

### Undo/Redo

Multiple levels of Undo/Redo are supported for the edited layout, the current implementation takes a snapshot of current state every 2 seconds and a new undo state is saved if the layout changed.
  
### Control rectangles view
 
All control rectangles can be enabled to get a colored rectangle overlay over all controls, it could be useful for controls placement and alignment.
The rectangle overlay is not applied over two special container controls: `WindowBox` and `GroupBox`.

  > Shortcut: `R`
 
### Control names view

All control have names assigned that will be used on code generation. It's possible to see all controls names at onece for convenience, to detect possible duplicates or inconsistencies.

  > Shortcut: `N` (with no control/anchor selected)

### Layers view

Controls follow a drawing order. It's possible to change that order focusing the controls and using the mouse wheel up/down if the layer view mode is activated.

  > Shortcut: `L`

## Controls

Controls are the base elements for the layouts, layouts contain controls.

### Creation
 
Control creation mode is the default mode, a control is selected from Controls Panel and it can be placed anywhere in the grid.
To change between controls `MOUSE_WHEEL` can be used to move selection up and down.
Controls can not be placed in the same position of already placed controls, actually, when moving mouse over an already placed control it gets automatically focused for selection.
 
### Selection
 
To select an already placed control, move mouse cursor over the control rectangle, it gets focused and with `MOUSE_LEFT_BUTTON` click it can be selected.

NOTE: Some control focuse/selection rectangles could not cover all control area. For example CheckBox control rectangle only considers check box but not optional text.

### Edition: Movement
 
Selected controls can be moved with the mouse but it's also possible to move them with keyboard for more precision.

`LEFT_SHIFT + ARROWS` allow moving selected controls by grid subdivision increments when snap mode is active and by 1px increments if snap is not active.

NOTE: Snap mode can be toggled with `LEFT_ALT + S`.

### Edition: Scale
 
Selected control can be scaled with mouse selecting the bottom-right corner of the control rectangle, a scaling rectangle appears with a corner triangle for scaling.

It's also possible to scale selected control with keyboard.

`LEFT_CONTROL + ARROWS` allow scaling selected control by grid subdivision increments when snap mode is active and by 1px increments if snap is not active.
 
### Edition: Anchor linkage

Controls can be linked to anchors, so, when moving an anchor all linked controls are moved together and control position depends on anchor position.
When exporting layout to code and anchors are exported, controls positions depend their anchors. It makes it easy to move groups of controls together.
To link a control to an anchor just focus/select the cotrol and drag a line with `MOURE_RIGHT_BUTTON`.
 
### Edition: Property: `text`

Most controls contain a text label. That text can be edited directly in the tool.
To enable text editing mode, select a control and press `T` key, a text box will appear to introduce new text. `ESCAPE` key cancels text editing, returning to previous text and `ENTER` key validates introduced text.

 > NOTE 1: Some controls support up to two text labels (`Slider`, `SliderBar`, `ProgressBar`), only left label text editing is supported in those controls.

 > NOTE 2: Some controls do not support text labels but text define control selection options (`ToggleGroup`, `ComboBox`, `DropdownBox`, `ListView`), in those cases instroduced text represent those control options, the different options must be separated by `;` character to be properly processed.

 > NOTE 3: raygui support icons at the beggining of text. If the first text character introduced on text editing is `#`, a icons selector panel will show up to choose an icon from the raygui available ones.

### Edition: Property: `name`

Controls can be named. `name` actually defines the code generated variable name for the control required properties. That name could represent a part of the variables names.
It's really useful and recommended to name the controls properly for code generation.

Here it is a table showing the default names at creation and the code generated variables from name for the different types of controls:

| Control     |   DefaultName    | Generated variables (sufixes)  |
|:-----------:|:---------------- |:------------------------------ |
| WINDOWBOX   |   WindowBox001   |    `bool {name}Active`         | 
| GROUPBOX    |   GroupBox001    |    -                           |
| LINE        |   Line001        |    -                           |
| PANEL       |   Panel001       |    -                           |
| LABEL       |   Label001       |    -                           |
| BUTTON      |   Button001      |    `bool {name}Pressed`        |
| TOGGLE      |   Toggle001      |    `bool {name}Active`         |
| TOGGLEGROUP |   ToggleGroup001 |    `int {name}Active`          |
| CHECKBOX    |   CheckBox001    |    `bool {name}Checked`        |
| COMBOBOX    |   ComboBox001    |    `int {name}Active`          |
| DROPDOWNBOX |   DropdownBox001 |    `int {name}Active`,<br>`bool {name}EditMode`      |
| SPINNER     |   Spinner001     |    `int {name}Value`,<br>`bool {name}EditMode`       |
| VALUEBOX    |   ValueBox001    |    `int {name}Value`,<br>`bool {name}EditMode`       |
| TEXTBOX     |   TextBox001     |    `char {name}Text[128]`,<br>`bool {name}EditMode`  |
| SLIDER      |   Slider001      |    `float {name}Value`         |
| SLIDERBAR   |   SliderBar001   |    `float {name}Value`         |
| PROGRESSBAR |   ProgressBar001 |    `float {name}Value`         |
| STATUSBAR   |   StatusBar001   |    -                           |
| SCROLLPANEL |   ScrollPanel001 |    `Rectangle {name}View`,<br>`Vector2 {name}Offset`,<br>`Vector2 {name}BoundsOffset` |
| LISTVIEW    |   ListView001    |    `int {name}Active`,<br>`int {name}ScrollIndex`    |
| COLORPICKER |   ColorPicker001 |    `Color {name}Value`         |
| DUMMYREC    |   DummyRec001    |    -

 > NOTE: The number at the end of the default name represents the layout drawing order for the control.

Depending on the code generation options, additional variables can be created:

 - Export Anchors: Additional variables for anchors are created: `Vector2 {anchor_name}`. Linked controls position use it on drawing.
 - Define Rectangles: Controls rectangles are defined as a Rectangles array: `Rectangle layoutRecs[n]`.
 - Defint text as const: All controls text is defined as const: `const char *{name}Text` or `const char *{name}TextList[]`.
 - Generate button functions: Button functions are generated to be directly called on button press, instead of creating `bool {name}Pressed`.
 
### Deletion

Selected controls can be deleted with `DELETE` key.

## Anchors

Anchors define relative positions for layouts and they are very useful to move multiple control linked to an anchor all together.
When layout is exported to code, anchors can be exported and code considers them with all the controls attached as relative positions,
very useful to place complete layouts on the screen on target product.

### Creation
 
Anchor creation mode can be enabled pressing down `A` key, cursor will change into an anchor that could be placed anywhere to further link controls to it.
 
### Selection
 
Anchors can be selected like any other control with `MOUSE_LEFT_BUTTON` click over them.
 
### Edition: Movement
 
Anchors can be moved like any other control but when moving an anchor all linked controls will be moved with it.
To move an anchor independently of the controls linked, press down `A` key while moving the anchor.
 
### Edition: Link/Unlink Controls
 
To link control to an anchor, press `MOURE_RIGHT_BUTTON` over an anchor/control and move it the control/anchor to be linked.
To unlink all controls from an anchor, select the anchor and press `U`, it will unlink all controls from that anchor.
To unlink a single control, select the control to unlink and press `U`. 
 
### Edition: Hide/Show Linked Controls
 
All the controls linked to an anchor can be hidden, select an anchor and press `H` to toggle controls hidding.
 
### Deletion
 
Anchors can be deleted by selecting it and pressing `DELETE` key, once an anchor is deleted all controls are unlinked from that anchor. 

## Tracemap

Tracemap are reference images (`.png`) that can be used as background to draw control over it, or just use them as a reference when building a new layout. 

### Loading

A new tracemap can be loaded with drag&drop of a `.png` image over the work area or pressing `KEY_LEFT_CONTROL + KEY_LEFT_SHIFT + KEY_O`.
 
### Selection

Tracemap can be selected placing mouse over it and with a `MOUSE_LEFT_BUTTON` click.
 
### Edition: Movement

Selected tracemap can be moved with the mouse but it's also possible to move it with keyboard for more precision.

`LEFT_SHIFT + ARROWS` allow moving selected controls by grid subdivision increments when snap mode is active and by 1px increments if snap is not active.

NOTE: Snap mode can be toggled with `LEFT_ALT + S`.

### Edition: Opacity

Tracemap has a 70% opacity by default but this value can be changed with `RIGHT_ALT + O/P`.
 
### Edition: Lock/Unlock

Tracemap can be locked to allow placing controls over it, a selected tracemap can be locked with `SPACE` key. To unlock tracemap, there is an option on main toolbar.
 
### Delete

To delete a tracemap, select it and press `DELETE` key.

## Reference Window

The reference window can be used to define a workable area for the project an a different origin coordinates point for its elements.
For rGuiLayout v3.0 the reference window options have been disabled. 

## Code Generation

Layout can be exported as code pressing `LEFT_CONTROL + E`. On Code Generation Window is possible to pre-visualized the exported result and also configure several exporting options.

Some basic layout properties can be defined to be later used in code comments: `Name`, `Version`, `Company` and `Short Description`.

Those values are placed in code template where the following variables are defined:

```c
// General layout config variables
$(GUILAYOUT_NAME)               // Layout name
$(GUILAYOUT_NAME_UPPERCASE)     // Layout name UPPERCASE
$(GUILAYOUT_NAME_PASCALCASE)    // Layout name PascalCase
$(GUILAYOUT_VERSION)            // Layout version
$(GUILAYOUT_DESCRIPTION)        // Layout description
$(GUILAYOUT_WINDOW_WIDTH)       // Layout window width
$(GUILAYOUT_WINDOW_HEIGHT)      // Layout window height
```

### Code: Standard Template (.c)

The standard generation template is a basic plain C file using raylib, the following template variables are replaced as required:

```c
// Standard C file template variables
$(GUILAYOUT_INITIALIZATION_C)           // Initialize gui variables
$(GUILAYOUT_DRAWING_C)                  // Draw gui controls
$(GUILAYOUT_FUNCTION_DECLARATION_C)     // Declare required functions (buttons)
$(GUILAYOUT_FUNCTION_DEFINITION_C)      // Define required functions (empty body)
```

### Code: Portable Template (.h)

This template defines a single-file header-only code file (.h) that can be included in any project as a portable layout.
Usage information is provided in the same file header and basically consists of including the file an calling two functions:

```c
// Include layout module 
#define GUI_TEST_LAYOUT_IMPLEMENTATION
#include "gui_layout_name.h"

// On app initialization:
GuiLayoutNameState state = InitGuiLayoutName();

// On app drawing:
GuiLayoutName(&state);
```

The following template variables are replaced on code generation:

```
// Header-only style template variables
$(GUILAYOUT_STRUCT_TYPE)                // Define gui state struct
$(GUILAYOUT_FUNCTIONS_DECLARATION_H)    // Declare Init/Draw functions
$(GUILAYOUT_FUNCTION_INITIALIZE_H)      // Initialize gui state struct function
$(GUILAYOUT_FUNCTION_DRAWING_H)         // Draw gui function
```

### Code: Custom Template (.c/.h)

When defining a custom template for code generation, any of the previously mentioned variables can be defined in the custom code to be replaced on code generation.

Custom template can be loaded in Code Generation window to see the results.

