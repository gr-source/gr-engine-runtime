#include "core/event.hpp"
#include "core/function.hpp"

void (*Event::AddInternalEvent)(std::function<void()> func);
INSERT_FUNCTION(0, Event::AddInternalEvent)



