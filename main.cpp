#pragma once

template<typename Type, typename... Args>
class clVirtualTable
{
	UINT hookAddress, hookIndex;
	LPVOID* virtualTable, originalFunction;
public:
	~clVirtualTable()
	{
		this->remove();
	}

	void install(const UINT address, const UINT index, const LPVOID function)
	{
		hookAddress = address;
		hookIndex = index;

		virtualTable = *reinterpret_cast<LPVOID**>
			(*reinterpret_cast<LPDWORD>(address));

		DWORD dwProtection;
		::VirtualProtect(reinterpret_cast<LPVOID>
			(virtualTable + index), 4, PAGE_READWRITE, &dwProtection);

		originalFunction = virtualTable[index];
		virtualTable[index] = function;

		::VirtualProtect(reinterpret_cast<LPVOID>
			(virtualTable + index), 4, dwProtection, &dwProtection);
	}

	Type call(Args... functionArguments)
	{
		return reinterpret_cast<Type(__stdcall*)(Args...)>(originalFunction)(functionArguments...);
	}

	void remove()
	{
		if (virtualTable != nullptr && 
			originalFunction != nullptr)
		{
			DWORD dwProtection;
			::VirtualProtect(reinterpret_cast<LPVOID>
				(virtualTable + hookIndex), 4, PAGE_READWRITE, &dwProtection);

			virtualTable[hookIndex] = originalFunction;

			::VirtualProtect(reinterpret_cast<LPVOID>
				(virtualTable + hookIndex), 4, dwProtection, &dwProtection);
		}
	}
};
