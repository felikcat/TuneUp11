#define __STDC_WANT_LIB_EXT1__ 1
#include "Common.h"
#include <UserEnv.h>
#include <Shlwapi.h>
#include <fcntl.h>
#include <Shlwapi.h>
#include <shellapi.h>
#include <libloaderapi.h>
#include <wchar.h>
#include <time.h>
#include <stdio.h>

const CLSID _CLSID_GroupPolicyObject = {0xea502722, 0xa23d, 0x11d1, {0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3}};
const IID _IID_IGroupPolicyObject = {0xea502723, 0xa23d, 0x11d1, {0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3}};
const CLSID _CLSID_GPESnapIn = {0x8fc0b734, 0xa0e1, 0x11d1, {0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3}};
GUID RegistryID = {0x35378EAC, 0x683F, 0x11D2, {0xA8, 0x9A, 0x00, 0xC0, 0x4F, 0xBB, 0xCF, 0xA2}};
GUID RegistryID;
IGroupPolicyObject *pGPO;
HKEY hKey;
HKEY hSubKey;
LONG result;

wchar_t *get_log_directory()
{
    wchar_t currentPath[MAX_PATH];
    GetModuleFileNameW(NULL, currentPath, MAX_PATH);

    wchar_t *removeExe = wcsrchr(currentPath, L'\\');
    if (removeExe != NULL)
    {
        *removeExe = L'\0';
    }

    const wchar_t *dirName = L"W11Boost Logs";
    wchar_t *fullPath = (wchar_t *)malloc(MAX_PATH * sizeof(wchar_t));
    swprintf_s(fullPath, MAX_PATH, L"%s\\%s", currentPath, dirName);

    // Ensure double-null termination
    size_t len = wcslen(fullPath);
    if (len + 2 < MAX_PATH)
    {
        fullPath[len + 1] = L'\0';
    }
    return fullPath;
}

utf8_result wide_string_to_utf8(const wchar_t *wide_string)
{
    if (wide_string == NULL || *wide_string == U'\0')
        return (utf8_result){NULL, false};

    size_t wide_length = 0;
    while (wide_string[wide_length] != U'\0')
        wide_length++;

    size_t size_needed = WideCharToMultiByte(CP_UTF8, 0, wide_string, wide_length, NULL, 0, NULL, NULL);
    if (size_needed <= 0)
        return (utf8_result){NULL, false};

    char *result = (char *)malloc(size_needed + 1);
    if (result == NULL)
        return (utf8_result){NULL, false};

    int converted_result =
        WideCharToMultiByte(CP_UTF8, 0, wide_string, (int)wide_length, &result[0], size_needed, NULL, NULL);
    if (converted_result <= 0)
    {
        free(result);
        return (utf8_result){NULL, true};
    }
    result[size_needed] = '\0';
    return (utf8_result){result, false}; // if SUCCESS (0), proceed
}

int log_registry(const wchar_t *subKey, const wchar_t *valueName, const char *typeName)
{
    wchar_t *currentDir = get_log_directory();
    wchar_t logLocation[MAX_PATH];
    swprintf_s(logLocation, MAX_PATH, L"%s\\Registry.log", currentDir);

    FILE *logFile = NULL;
    errno_t err = _wfopen_s(&logFile, logLocation, L"a");
    if (err != 0 || logFile == NULL)
        return EXIT_FAILURE;

    if (logFile)
    {
        time_t now;
        time(&now);

        struct tm timeInfo;
        localtime_s(&timeInfo, &now);

        char timeString[21]; // 19 char + null terminator
        strftime(timeString, sizeof(timeString), "%d-%m-%Y %H:%M:%S", &timeInfo);

        utf8_result narrow_subKey = wide_string_to_utf8(subKey);
        utf8_result narrow_valueName = wide_string_to_utf8(valueName);

        if (!narrow_subKey.error && !narrow_valueName.error)
        {
            fprintf_s(logFile, "%s %s %s\\%s\n", timeString, typeName, narrow_subKey.string, narrow_valueName.string);
        }
    }
    fclose(logFile);
    free(currentDir);
    return EXIT_SUCCESS;
}

LSTATUS set_dword(HKEY hKey, const wchar_t *subKey, const wchar_t *valueName, const DWORD value)
{
    HKEY hSubKey;

    result = RegCreateKeyExW(hKey, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, NULL);

    if (result != ERROR_SUCCESS)
    {
        fprintf_s(stderr, "RegCreateKeyExW failed; code: %lu\n", result);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    result = RegSetValueExW(hSubKey, valueName, 0, REG_DWORD, (const BYTE *)&value, sizeof(DWORD));

    if (result != ERROR_SUCCESS)
    {
        fprintf_s(stderr, "RegSetValueExW failed; code: %lu\n", result);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    const char *typeName = " - DWORD: ";
    log_registry(subKey, valueName, typeName);

    result = RegCloseKey(hSubKey);

    if (result != ERROR_SUCCESS)
    {
        fprintf_s(stderr, "RegCloseKey failed; code: %lu\n", result);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    return result;
}

LSTATUS set_string(HKEY hKey, const wchar_t *subKey, const wchar_t *valueName, const wchar_t *value)
{
    HKEY hSubKey;
    result = RegCreateKeyExW(hKey, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, NULL);

    if (result != 0)
    {
        fprintf_s(stderr, "RegCreateKeyExW failed; code: %lu\n", result);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    result = RegSetValueExW(hSubKey, valueName, 0, REG_SZ, (const BYTE *)value, sizeof(wchar_t));

    if (result != 0)
    {
        fprintf_s(stderr, "RegSetValueExW failed; code: %lu\n", result);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    result = RegCloseKey(hSubKey);

    if (result != 0)
    {
        fprintf_s(stderr, "RegCloseKey failed; code: %lu\n", result);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    const char *typeName = " - SZ: ";
    log_registry(subKey, valueName, typeName);
    return EXIT_SUCCESS;
}

LSTATUS set_environment(HKEY hKey, const wchar_t *valueName, const wchar_t *value)
{
    const wchar_t *subKey = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
    result = RegOpenKeyExW(hKey, subKey, 0, KEY_ALL_ACCESS, &hSubKey);

    if (result == ERROR_SUCCESS)
        result = RegSetValueExW(hSubKey, valueName, 0, REG_EXPAND_SZ, (const BYTE *)value,
                                (wcslen(value) * sizeof(wchar_t)));

    const char *typeName = " - EXPAND_SZ: ";
    log_registry(subKey, valueName, typeName);

    result = RegCloseKey(hSubKey);
    return result;
}

LSTATUS remove_subkey(HKEY hKey, const wchar_t *subKey, const wchar_t *valueName)
{
    HKEY hSubKey;
    result = RegOpenKeyExW(hKey, subKey, 0, KEY_WRITE, &hSubKey);

    if (result == ERROR_SUCCESS)
        result = RegDeleteValueW(hSubKey, valueName);

    const char *typeName = " - Removed subkey: ";
    log_registry(subKey, valueName, typeName);

    result = RegCloseKey(hSubKey);
    return result;
}

void gp_cleanup(HRESULT hr)
{
    RegCloseKey(hKey);

    if (SUCCEEDED(hr))
        hr = pGPO->lpVtbl->Save(pGPO, TRUE, TRUE, &RegistryID, (GUID *)&_CLSID_GPESnapIn);

    // Apply new policy objects to the registry
    RefreshPolicyEx(TRUE, RP_FORCE);

    if (pGPO)
        pGPO->lpVtbl->Release(pGPO);
}

int start_command_and_wait(wchar_t *cmdLine)
{
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    SecureZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    SecureZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessW(NULL, cmdLine, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        fprintf_s(stderr, "CreateProcessW failed; code: %lu\n", GetLastError());
        return EXIT_FAILURE;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return EXIT_SUCCESS;
}

bool append_wchar_t(const wchar_t *original, const wchar_t *append, wchar_t *result)
{
    wcscpy_s(result, MAX_PATH, original);
    if (SUCCEEDED(PathAppendW(result, append)))
        return true;

    return false;
}

PWSTR get_windows_path(const KNOWNFOLDERID *folderId)
{
    PWSTR path = NULL;
    SHGetKnownFolderPath(folderId, 0, NULL, &path);
    return path;
}

HRESULT init_gp_object()
{
    HRESULT hr = S_OK; 
    hKey = HKEY_CURRENT_USER;

    // The apartment thread model is required for GPOs
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        fprintf(stderr, "CoInitializeEx failed: 0x%08lx\n", hr);
        return hr;
    }

    hr = CoCreateInstance(&_CLSID_GroupPolicyObject, NULL, CLSCTX_INPROC_SERVER, &_IID_IGroupPolicyObject, (LPVOID *)&pGPO);
    if (FAILED(hr))
    {
        fprintf(stderr, "CoCreateInstance failed: 0x%08lx\n", hr);
        CoUninitialize();
        return hr;
    }

    hr = pGPO->lpVtbl->OpenLocalMachineGPO(pGPO, GPO_OPEN_LOAD_REGISTRY);
    if (FAILED(hr))
    {
        fprintf(stderr, "OpenLocalMachineGPO failed: 0x%08lx\n", hr);
        pGPO->lpVtbl->Release(pGPO);
        CoUninitialize();
        return hr;
    }

    hr = pGPO->lpVtbl->GetRegistryKey(pGPO, GPO_SECTION_USER, &hKey);
    if (FAILED(hr))
    {
        fprintf(stderr, "GetRegistryKey failed: 0x%08lx\n", hr);
        pGPO->lpVtbl->Release(pGPO);
        CoUninitialize();
        return hr;
    }

    return hr;
}
