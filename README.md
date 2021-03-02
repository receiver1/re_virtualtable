# re_virtualtable
#### Description:
Virtual Table Hooker. Tested only with DirectX 9.
#### Example of usage:
```C++
clVirtualTable<HRESULT, LPDIRECT3DDEVICE9> endSceneHook;
clVirtualTable<HRESULT, LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*> resetHook;

HRESULT __stdcall endSceneHooked(LPDIRECT3DDEVICE9 pDevice)
{
    // Some actions...
    return endSceneHook.call(eConvention::stdcall, pDevice);
}

HRESULT __stdcall resetHooked(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pParams)
{
    // Some actions...
    return resetHook.call(eConvention::stdcall, pDevice, pParams); 
}

// Entry point
const UINT hookAddress{ 0xC97C28U }; // GTA:SA device
endSceneHook.install(hookAddress, 42, &endSceneHooked)
resetHook.install(hookAddress, 16, &resetHooked);
```
