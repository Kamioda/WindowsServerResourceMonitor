#include "ComInitManager.hpp"
#include <comdef.h>

ComInitManager::ComInitManager() { CoInitialize(nullptr); }

ComInitManager::~ComInitManager() { CoUninitialize(); }
