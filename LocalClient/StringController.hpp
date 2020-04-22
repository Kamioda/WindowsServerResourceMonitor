#pragma once
#include "CommandLineManager.h"

namespace CommandLineManager = CommandLineManagerA;
#ifdef UNICODE
namespace CharsetManager = CommandLineManagerW;
#else
namespace CharsetManager = CommandLineManagerA;
#endif
