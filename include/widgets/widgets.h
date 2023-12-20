#include "core_types.h"
#include "terminal.h"

struct Widget;

bool widgets_hook( struct Widget *widget );
void widgets_unhook( struct Widget const *widget );
bool widgets_is_hook( struct Widget const *widget );

// Executed on all hooked widgets :
void widgets_frame( void );
void widgets_screen_resized( void /* ... */ );
void widgets_hide( void );
void widgets_show( void );
