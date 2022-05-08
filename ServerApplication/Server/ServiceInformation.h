﻿#pragma once
#include <Windows.h>
#include <tchar.h>

namespace ServiceInfo {
	constexpr TCHAR* Name = _T("ResMonSrvSvc");
	constexpr TCHAR* DisplayName = _T("Windows Resource Monitor Server");
	constexpr TCHAR* Description = _T("Get computer resource with running Windows Server");
	constexpr DWORD StartType = SERVICE_AUTO_START;
	constexpr bool DelayedStart = true;
	// LocalSystem    : NULL
	// LocalService   : NT AUTHORITY\\LocalService
	// NetworkService : NT AUTHORITY\\NetworkService
	constexpr const TCHAR* ExecutionUser = NULL;
	constexpr bool RequireAdministrator = true;
}