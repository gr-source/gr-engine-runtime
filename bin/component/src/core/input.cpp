#include "core/input.hpp"
#include "core/function.hpp"

bool (*Input::IskeyPress)(InputKey key);
INSERT_FUNCTION(1, Input::IskeyPress);

void (*Input::SetCursorPos)(const Vector2 &value);
INSERT_FUNCTION(2, Input::SetCursorPos);

Vector2 (*Input::GetCursorPos)();
INSERT_FUNCTION(3, Input::GetCursorPos);

bool (*Input::IsMousePress)(InputKey key);
INSERT_FUNCTION(4, Input::IsMousePress);




