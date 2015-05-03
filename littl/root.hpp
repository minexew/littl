#pragma once

//* the littl base
#include "Base.hpp"
#include "Utf8.hpp"

#include "Allocator.hpp"
#include "Thread.hpp"

//* depends on: Allocator
#include "Array.hpp"

//* depends on: Array
#include "List.hpp"

//* depends on: List
#include "String.hpp"
#include "Exception.hpp"
#include "Map.hpp"
#include "Stack.hpp"

//* depends on: String
#include "Stream.hpp"
#include "StreamBuffer.hpp"

//* depends on: BaseIO
#include "Console.hpp"
#include "File.hpp"
#include "Http.hpp"
