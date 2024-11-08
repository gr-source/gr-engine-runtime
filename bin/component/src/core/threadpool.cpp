#include "core/threadpool.hpp"
#include "core/function.hpp"

void (*Threadpool::CreateInternalTask)(std::function<void()> func);
INSERT_FUNCTION(0, Threadpool::CreateInternalTask)




