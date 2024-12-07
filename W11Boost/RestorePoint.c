#include "Common.h"
#include <AccCtrl.h>
#include <AclAPI.h>
#include <SrRestorePtApi.h>

typedef bool(WINAPI *PFN_SETRESTOREPTW)(PRESTOREPOINTINFOW, PSTATEMGRSTATUS);

bool InitializeCOMSecurity() {
  // Create the security descriptor explicitly as follows because
  // CoInitializeSecurity() will not accept the relative security descriptors
  // returned by ConvertStringSecurityDescriptorToSecurityDescriptor().

  SECURITY_DESCRIPTOR securityDesc = {0};
  EXPLICIT_ACCESS ea[5] = {{0}};
  ACL *pAcl = NULL;
  unsigned __int64
      rgSidBA[(SECURITY_MAX_SID_SIZE + sizeof(unsigned __int64) - 1) /
                           sizeof(unsigned __int64)] = {0};
  unsigned __int64
      rgSidLS[(SECURITY_MAX_SID_SIZE + sizeof(unsigned __int64) - 1) /
                           sizeof(unsigned __int64)] = {0};
  unsigned __int64
      rgSidNS[(SECURITY_MAX_SID_SIZE + sizeof(unsigned __int64) - 1) /
              sizeof(unsigned __int64)] = {0};
  unsigned __int64
      rgSidPS[(SECURITY_MAX_SID_SIZE + sizeof(unsigned __int64) - 1) /
                           sizeof(unsigned __int64)] = {0};
  unsigned __int64
      rgSidSY[(SECURITY_MAX_SID_SIZE + sizeof(unsigned __int64) - 1) /
                           sizeof(unsigned __int64)] = {0};
  unsigned long cbSid = 0;
  bool fRet = FALSE;
  unsigned long dwRet = ERROR_SUCCESS;
  HRESULT hrRet = S_OK;

  //
  // This creates a security descriptor that is equivalent to the following
  // security descriptor definition language (SDDL) string:
  //
  //   O:BAG:BAD:(A;;0x1;;;LS)(A;;0x1;;;NS)(A;;0x1;;;PS)(A;;0x1;;;SY)(A;;0x1;;;BA)
  //

  // Initialize the security descriptor.
  fRet = InitializeSecurityDescriptor(&securityDesc,
                                        SECURITY_DESCRIPTOR_REVISION);
  if (!fRet)
    goto exit;

  // Create an administrator group security identifier (SID).
  cbSid = sizeof(rgSidBA);
  fRet =
      CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, rgSidBA, &cbSid);
  if (!fRet)
    goto exit;

  // Create a local service security identifier (SID).
  cbSid = sizeof(rgSidLS);
  fRet = CreateWellKnownSid(WinLocalServiceSid, NULL, rgSidLS, &cbSid);
  if (!fRet)
    goto exit;

  // Create a network service security identifier (SID).
  cbSid = sizeof(rgSidNS);
  fRet = CreateWellKnownSid(WinNetworkServiceSid, NULL, rgSidNS, &cbSid);
  if (!fRet)
    goto exit;

  // Create a personal account security identifier (SID).
  cbSid = sizeof(rgSidPS);
  fRet = CreateWellKnownSid(WinSelfSid, NULL, rgSidPS, &cbSid);
  if (!fRet)
    goto exit;

  // Create a local service security identifier (SID).
  cbSid = sizeof(rgSidSY);
  fRet = CreateWellKnownSid(WinLocalSystemSid, NULL, rgSidSY, &cbSid);
  if (!fRet)
    goto exit;

  // Setup the access control entries (ACE) for COM. You may need to modify
  // the access permissions for your application. COM_RIGHTS_EXECUTE and
  // COM_RIGHTS_EXECUTE_LOCAL are the minimum access rights required.

  ea[0].grfAccessPermissions = COM_RIGHTS_EXECUTE | COM_RIGHTS_EXECUTE_LOCAL;
  ea[0].grfAccessMode = GRANT_ACCESS;
  ea[0].grfInheritance = NO_INHERITANCE;
  ea[0].Trustee.pMultipleTrustee = NULL;
  ea[0].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
  ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
  ea[0].Trustee.ptstrName = (LPTSTR)rgSidBA;

  ea[1].grfAccessPermissions = COM_RIGHTS_EXECUTE | COM_RIGHTS_EXECUTE_LOCAL;
  ea[1].grfAccessMode = GRANT_ACCESS;
  ea[1].grfInheritance = NO_INHERITANCE;
  ea[1].Trustee.pMultipleTrustee = NULL;
  ea[1].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
  ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
  ea[1].Trustee.ptstrName = (LPTSTR)rgSidLS;

  ea[2].grfAccessPermissions = COM_RIGHTS_EXECUTE | COM_RIGHTS_EXECUTE_LOCAL;
  ea[2].grfAccessMode = GRANT_ACCESS;
  ea[2].grfInheritance = NO_INHERITANCE;
  ea[2].Trustee.pMultipleTrustee = NULL;
  ea[2].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
  ea[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea[2].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
  ea[2].Trustee.ptstrName = (LPTSTR)rgSidNS;

  ea[3].grfAccessPermissions = COM_RIGHTS_EXECUTE | COM_RIGHTS_EXECUTE_LOCAL;
  ea[3].grfAccessMode = GRANT_ACCESS;
  ea[3].grfInheritance = NO_INHERITANCE;
  ea[3].Trustee.pMultipleTrustee = NULL;
  ea[3].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
  ea[3].Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea[3].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
  ea[3].Trustee.ptstrName = (LPTSTR)rgSidPS;

  ea[4].grfAccessPermissions = COM_RIGHTS_EXECUTE | COM_RIGHTS_EXECUTE_LOCAL;
  ea[4].grfAccessMode = GRANT_ACCESS;
  ea[4].grfInheritance = NO_INHERITANCE;
  ea[4].Trustee.pMultipleTrustee = NULL;
  ea[4].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
  ea[4].Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea[4].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
  ea[4].Trustee.ptstrName = (LPTSTR)rgSidSY;

  // Create an access control list (ACL) using this ACE list.
  dwRet = SetEntriesInAcl(ARRAYSIZE(ea), ea, NULL, &pAcl);
  if (dwRet != ERROR_SUCCESS || pAcl == NULL) {
    fRet = FALSE;
    goto exit;
  }

  // Set the security descriptor owner to Administrators.
  fRet = SetSecurityDescriptorOwner(&securityDesc, rgSidBA, FALSE);
  if (!fRet)
    goto exit;

  // Set the security descriptor group to Administrators.
  fRet = SetSecurityDescriptorGroup(&securityDesc, rgSidBA, FALSE);
  if (!fRet)
    goto exit;

  // Set the discretionary access control list (DACL) to the ACL.
  fRet = SetSecurityDescriptorDacl(&securityDesc, TRUE, pAcl, FALSE);
  if (!fRet)
    goto exit;

  // Initialize COM. You may need to modify the parameters of
  // CoInitializeSecurity() for your application. Note that an
  // explicit security descriptor is being passed down.

  hrRet = CoInitializeSecurity(
      &securityDesc, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
      RPC_C_IMP_LEVEL_IDENTIFY, NULL, EOAC_DISABLE_AAA | EOAC_NO_CUSTOM_MARSHAL,
      NULL);
  if (FAILED(hrRet)) {
    fRet = FALSE;
    goto exit;
  }

  fRet = TRUE;

exit:

  LocalFree(pAcl);

  return fRet;
}

void restorepoint_prep() {
  HKEY hKey = HKEY_LOCAL_MACHINE;

  long result = RegCreateKeyExW(
      hKey, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SystemRestore", 0,
      NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, NULL);

  if (result == ERROR_SUCCESS) {
    unsigned long value = 0;
    RegSetValueExW(hSubKey, L"SystemRestorePointCreationFrequency", 0,
                   REG_DWORD, (const BYTE *)&value, sizeof(value));
  }

  result = RegCreateKeyExW(
      hKey, L"SOFTWARE\\Policies\\Microsoft\\Windows NT\\SystemRestore", 0, NULL,
      REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, NULL);

  if (result == ERROR_SUCCESS) {
    RegDeleteValueW(hSubKey, L"DisableConfig");
    RegDeleteValueW(hSubKey, L"DisableSR");
  }
  RegCloseKey(hKey);
}

// If the user disabled System Restore via Group Policies, that'll revert on its
// own
void restorepoint_prep_revert() {
  HKEY hKey = HKEY_LOCAL_MACHINE;

  long result = RegCreateKeyExW(
      hKey, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SystemRestore", 0,
      NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, NULL);

  if (result == ERROR_SUCCESS) {
    RegDeleteValueW(hSubKey, L"SystemRestorePointCreationFrequency");
  }

  RegCloseKey(hKey);
}

int create_restore_point() {
  STATEMGRSTATUS SMgrStatus;

  // COINIT_MULTITHREADED seems to cause race conditions
  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (FAILED(hr))
    return EXIT_FAILURE;

  bool fRet = InitializeCOMSecurity();
  if (FAILED(fRet))
    return EXIT_FAILURE;

  HMODULE hSrClient = LoadLibraryW(L"srclient.dll");
  if (NULL == hSrClient)
    return EXIT_FAILURE;

  RESTOREPOINTINFOW RestorePtInfo = {.dwEventType = BEGIN_SYSTEM_CHANGE,
                                     .dwRestorePtType = APPLICATION_INSTALL,
                                     .llSequenceNumber = 0};

  wcscpy_s(RestorePtInfo.szDescription, _countof(RestorePtInfo.szDescription),
           L"Installed W11Boost");

  PFN_SETRESTOREPTW fnSRSetRestorePointW =
      (PFN_SETRESTOREPTW)GetProcAddress(hSrClient, "SRSetRestorePointW");
  if (NULL == fnSRSetRestorePointW) {
    goto exit;
    return EXIT_FAILURE;
  }

  restorepoint_prep();

  // Start System Restore point
  fRet = fnSRSetRestorePointW(&RestorePtInfo, &SMgrStatus);

  if (!fRet) { // Either SR is off, or restore point creation failed outright
    goto exit;
    return EXIT_FAILURE;
  }

  RestorePtInfo.dwEventType = END_SYSTEM_CHANGE;
  RestorePtInfo.llSequenceNumber = SMgrStatus.llSequenceNumber;

  // End System Restore point
  fRet = fnSRSetRestorePointW(&RestorePtInfo, &SMgrStatus);

  restorepoint_prep_revert();

exit:
  if (hSrClient != NULL) {
    FreeLibrary(hSrClient);
    hSrClient = NULL;
  }

  return 0;
}
