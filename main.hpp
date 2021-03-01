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

		virtualTable = *reinterpret_cast<LPVOID**>(instance);
		if (!virtualTable) return eResult::failedGetVT;

		DWORD dwProtection;
		if (!VirtualProtect(reinterpret_cast<LPVOID>
			(virtualTable + index), 4, PAGE_READWRITE, &dwProtection))
			return eResult::failedProtection;

		originalFunction = virtualTable[index];
		virtualTable[index] = function;

		if (VirtualProtect(reinterpret_cast<LPVOID>
			(virtualTable + index), 4, dwProtection, &dwProtection))
			return eResult::failedProtection;

		return eResult::successHooked;
	}

	Type call(eConvention callingConvention, Args... functionArguments)
	{
		switch (callingConvention)
		{
			case eConvention::cdeclcall: return reinterpret_cast
				<Type(__cdecl*)(Args...)>(originalFunction)(functionArguments...);
			case eConvention::stdcall: return reinterpret_cast
				<Type(__stdcall*)(Args...)>(originalFunction)(functionArguments...);
			case eConvention::thiscall: return reinterpret_cast
				<Type(__thiscall*)(Args...)>(originalFunction)(functionArguments...);
			case eConvention::fastcall: return reinterpret_cast
				<Type(__fastcall*)(Args...)>(originalFunction)(functionArguments...);
		}
	}

	eResult remove()
	{
		if (virtualTable != nullptr &&
			originalFunction != nullptr)
		{
			DWORD dwProtection;
			if (VirtualProtect(reinterpret_cast<LPVOID>
				(virtualTable + hookIndex), 4, PAGE_READWRITE, &dwProtection))
				return eResult::failedProtection;

			virtualTable[hookIndex] = originalFunction;

			if (VirtualProtect(reinterpret_cast<LPVOID>
				(virtualTable + hookIndex), 4, dwProtection, &dwProtection))
				return eResult::failedProtection;

			return eResult::successHooked;
		}
		return eResult::missingOrigOrVT;
	}
};
