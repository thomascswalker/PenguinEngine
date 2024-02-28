#pragma once

#include "Logging.h"

template <class Out, class In>
constexpr Out* Cast(In InValue)
{
    return dynamic_cast<Out*>(InValue);
}