#pragma once

class Utils {
public:
	static DWORD GetPID(const std::wstring& procname);
	static DWORD GetBase(DWORD PID, const std::wstring& modulename);

	static float GetCPULoad();
	static bool SetPrivilege(LPCWSTR lpszPrivilege, BOOL bEnablePrivilege);

private:
	static float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks);
	static unsigned long long FileTimeToInt64(const FILETIME& ft);
};