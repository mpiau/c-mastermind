#pragma once


#include "components/components.h"

// Forward declaration
struct ComponentHeader;

bool component_header_exists( enum ComponentId id );

struct ComponentHeader *component_header_optget( enum ComponentId id ); // return NULL if doesn't exist

void component_header_make( struct ComponentHeader *widget, enum ComponentId id, bool enabled );

