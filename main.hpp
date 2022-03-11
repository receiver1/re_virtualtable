#pragma once

enum class eResult
{
	failedProtection,
	failedGetVT,
	successHooked,
	missingOrigOrVT
};

enum class eConvention
{
	cdeclcall,
	stdcall,
	thiscall,
	fastcall
};

template<typename Type, typename... Args>
class clVirtualTable
{
	UINT hookIndex;
	LPVOID* virtualTable, originalFunction;
public:
	~clVirtualTable()
	{
		this->remove();
	}

	eResult install(const UINT address, const UINT index, const LPVOID function)
	{
		const LPVOID instance = *reinterpret_cast<LPVOID*>(address);
		return this->install(instance, index, function);
	}

	eResult install(const LPVOID instance, const UINT index, const LPVOID function)
	{
		hookIndex = index;
		
		// This is a check of the availability of the region
		// This is so important, because your program will crash
		// if the region is unavailable and you will try to write or read its.
		::MEMORY_BASIC_INFORMATION mbi;
		::VirtualQuery(instance, &mbi, sizeof(mbi));

		if (mbi.State == MEM_COMMIT && (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)))
		{
			virtualTable = *reinterpret_cast<LPVOID**>(instance);
			if (!virtualTable) return eResult::failedGetVT;

			DWORD dwProtection;
			if (!VirtualProtect(reinterpret_cast<LPVOID>
				(virtualTable + index), 4, PAGE_READWRITE, &dwProtection))
				return eResult::failedProtection;

			originalFunction = virtualTable[index];
			virtualTable[index] = function;

			if (!VirtualProtect(reinterpret_cast<LPVOID>
				(virtualTable + index), 4, dwProtection, &dwProtection))
				return eResult::failedProtection;

			return eResult::successHooked;
		}
	}

	Type call(eConvention callingConvention, Args... functionArguments)
	{
		auto result{ reinterpret_cast<Type(__cdecl*)(Args...)>(originalFunction)(functionArguments...) };
		switch (callingConvention)
		{
		case eConvention::stdcall: 
			result = reinterpret_cast<Type(__stdcall*)(Args...)>(originalFunction)(functionArguments...);
			break;
		case eConvention::thiscall: 
			result = reinterpret_cast<Type(__thiscall*)(Args...)>(originalFunction)(functionArguments...);
			break;
		case eConvention::fastcall: 
			result = reinterpret_cast<Type(__fastcall*)(Args...)>(originalFunction)(functionArguments...);
		}
		return result;
	}

	eResult remove()
	{
		if (virtualTable != nullptr &&
			originalFunction != nullptr)
		{
			DWORD dwProtection;
			if (!VirtualProtect(reinterpret_cast<LPVOID>
				(virtualTable + hookIndex), 4, PAGE_READWRITE, &dwProtection))
				return eResult::failedProtection;

			virtualTable[hookIndex] = originalFunction;

			if (!VirtualProtect(reinterpret_cast<LPVOID>
				(virtualTable + hookIndex), 4, dwProtection, &dwProtection))
				return eResult::failedProtection;

			return eResult::successHooked;
		}
		return eResult::missingOrigOrVT;
	}
};
