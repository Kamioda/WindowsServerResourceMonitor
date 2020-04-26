#pragma once
#include "../Common/CommandLineManager.hpp"

namespace CommandLineManager = CommandLineManagerA;
#ifdef UNICODE
namespace CharsetManager = CommandLineManagerW;
#else
namespace CharsetManager = CommandLineManagerA;
#endif
