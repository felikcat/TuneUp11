#ifndef COMMON_H
#define COMMON_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h> // Always first
#include <combaseapi.h>
#include <prsht.h>
#include <gpedit.h> // Requires prsht.h first; not unused
#include <stdbool.h>
#include <curl/curl.h>
#include <Knownfolders.h>
#include <shlobj_core.h>

extern const CLSID _CLSID_GroupPolicyObject;
extern const IID _IID_IGroupPolicyObject;
extern const CLSID _CLSID_GPESnapIn;

extern GUID RegistryID;
extern IGroupPolicyObject *pGPO;
extern HKEY hKey;
extern HKEY hSubKey;

typedef struct {
  char *string;
  bool error;
} utf8_result;

int install_privacy_mode();
utf8_result wide_string_to_utf8(const wchar_t *wide_string);
int log_registry(const wchar_t *subKey, const wchar_t *valueName,
                 const char *typeName);
LSTATUS set_dword(HKEY hKey, const wchar_t *subKey, const wchar_t *valueName,
                  const DWORD value);

LSTATUS set_string(HKEY hKey, const wchar_t *subKey, const wchar_t *valueName,
                   const wchar_t *value);
LSTATUS set_environment(HKEY hKey, const wchar_t *valueName,
                        const wchar_t *value);
LSTATUS remove_subkey(HKEY hKey, const wchar_t *subKey,
                      const wchar_t *valueName);
void gp_cleanup(HRESULT hr);
int start_command_and_wait(wchar_t *cmdLine);
bool append_wchar_t(const wchar_t *original, const wchar_t *append,
                    wchar_t *result);
int download_file(const wchar_t *url, const wchar_t *destination);
int install_microsoft_store();
void restorepoint_prep();
int create_restore_point();
int gp_edits();
wchar_t *get_log_directory();
PWSTR get_windows_path();
int disable_sleep();
HRESULT init_gp_object();
int install_appx_support();

#endif // COMMON_H
