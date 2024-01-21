#pragma once

#include "core_types.h"
#include "keyboard_inputs.h"
#include "events.h"

void gameloop_emit_key( enum KeyInput const input );
void gameloop_emit_event( enum EventType type, struct EventData *data );
