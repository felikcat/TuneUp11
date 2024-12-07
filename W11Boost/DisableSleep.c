#include "Common.h"

int disable_sleep(void)
{
    HRESULT hr = init_gp_object(hKey);

    // Globally disable hibernation
    set_dword(hKey, L"SYSTEM\\CurrentControlSet\\Control\\Power", L"HibernateEnabledDefault", 0);

    // Turn off hybrid sleep (on battery)
    set_dword(hKey, L"Software\\Policies\\Microsoft\\Power\\PowerSettings\\94ac6d29-73ce-41a6-809f-6363ba21b47e",
              L"DCSettingIndex", 0);

    // Turn off hybrid sleep (plugged in)
    set_dword(hKey, L"Software\\Policies\\Microsoft\\Power\\PowerSettings\\94ac6d29-73ce-41a6-809f-6363ba21b47e",
              L"ACSettingIndex", 0);

    // Never idle to sleep (on battery)
    set_dword(hKey, L"Software\\Policies\\Microsoft\\Power\\PowerSettings\\29F6C1DB-86DA-48C5-9FDB-F2B67B1F44DA",
              L"DCSettingIndex", 0);

    // Never idle to sleep (plugged in)
    set_dword(hKey, L"Software\\Policies\\Microsoft\\Power\\PowerSettings\\29F6C1DB-86DA-48C5-9FDB-F2B67B1F44DA",
              L"ACSettingIndex", 0);

    // Never idle to hibernate (on battery)
    set_dword(hKey, L"Software\\Policies\\Microsoft\\Power\\PowerSettings\\9D7815A6-7EE4-497E-8888-515A05F02364",
              L"DCSettingIndex", 0);

    // Never idle to hibernate (plugged in)
    set_dword(hKey, L"Software\\Policies\\Microsoft\\Power\\PowerSettings\\9D7815A6-7EE4-497E-8888-515A05F02364",
              L"ACSettingIndex", 0);

    // Never unattended idle to sleep (on battery)
    set_dword(hKey, L"Software\\Policies\\Microsoft\\Power\\PowerSettings\\7bc4a2f9-d8fc-4469-b07b-33eb785aaca0",
              L"DCSettingIndex", 0);

    // Never unattended idle to sleep (plugged in)
    set_dword(hKey, L"Software\\Policies\\Microsoft\\Power\\PowerSettings\\7bc4a2f9-d8fc-4469-b07b-33eb785aaca0",
              L"ACSettingIndex", 0);

    // Disable the Hibernate entry in the Power Menu
    set_dword(hKey, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FlyoutMenuSettings",
              L"ShowHibernateOption", 0);

    // Disable the Sleep entry in the Power Menu
    set_dword(hKey, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FlyoutMenuSettings", L"ShowSleepOption",
              0);

    gp_cleanup(hr);
    RegCloseKey(hKey);
    return 0;
}
