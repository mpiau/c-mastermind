#include "core_types.h"
#include "terminal.h"

// Widgets should be graphical only !!
// No widget -> the game can still run the same
// Ex: removing the fps bar widgte doesn't stop the game from calculating the FPS.

struct Widget;

bool widgets_init( void );

bool widgets_hook( struct Widget *widget );
void widgets_unhook( struct Widget const *widget );
bool widgets_is_hook( struct Widget const *widget );

// Executed on all hooked widgets :
void widgets_frame( void );
void widgets_screen_resized( void /* ... */ );
void widgets_hide( void );
void widgets_show( void );
