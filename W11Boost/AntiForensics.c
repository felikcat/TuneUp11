#include "Common.h"
#include <GPEdit.h>

int install_privacy_mode() {
  HRESULT hr = init_gp_object(hKey);

  // Do not analyze apps' execution time data.
  set_dword(hKey, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib",
            L"Disable Performance Counters", 1);

  // Do not keep track of recently opened files.
  set_dword(hKey,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
            L"NoRecentDocsHistory", 1);

  // Disable Superfetch.
  set_dword(hKey, L"SYSTEM\\CurrentControlSet\\Services\\SysMain", L"Start", 4);
  set_dword(
      hKey,
      L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Management",
      L"EnableSuperfetch", 0);

  // Disable Prefetch.
  set_dword(
      hKey,
      L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management\\PrefetchParameters",
      L"EnablePrefetcher", 0);

  // Do not use "Last Access Time Stamp Updates" by default; apps can still
  // explicitly update these timestamps for themself.
  wchar_t disableFileTimestamps[] =
      L"fsutil.exe behavior set disablelastaccess 3";
  start_command_and_wait(disableFileTimestamps);

  // Disable "Application Impact Telemetry".
  set_dword(hKey, L"SOFTWARE\\Policies\\Microsoft\\Windows\\AppCompat",
            L"AITEnable", 0);

  // Disable "Program Compatibility Assistant"
  set_dword(hKey, L"SOFTWARE\\Policies\\Microsoft\\Windows\\AppCompat",
            L"DisablePCA", 1);

  // Disable "Application Compatibility Engine"
  set_dword(hKey, L"SOFTWARE\\Policies\\Microsoft\\Windows\\AppCompat",
            L"DisableEngine", 1);

  // Disable "SwitchBack Compatibility Engine"
  set_dword(hKey, L"SOFTWARE\\Policies\\Microsoft\\Windows\\AppCompat",
            L"SbEnable", 0);

  // Disable "User Steps Recorder"
  set_dword(hKey, L"SOFTWARE\\Policies\\Microsoft\\Windows\\AppCompat",
            L"DisableUAR", 1);

  // Disable "Inventory Collector"
  set_dword(hKey, L"SOFTWARE\\Policies\\Microsoft\\Windows\\AppCompat",
            L"DisableInventory", 1);

  // Disable "Program Compatibility Assistant" service
  set_dword(hKey, L"SYSTEM\\CurrentControlSet\\Services", L"PcaSvc", 4);

  // Disable PerfTrack.
  set_dword(
      hKey,
      L"SOFTWARE\\Policies\\Microsoft\\Windows\\WDI\\{9c5a40da-b965-4fc3-8781-88dd50a6299d}",
      L"ScenarioExecutionEnabled", 0);
  set_dword(hKey, L"SOFTWARE\\Policies\\Microsoft\\Messenger\\Client", L"CEIP",
            2);

  // [P1] Disable tracking of application startups.
  set_dword(hKey, L"SOFTWARE\\Policies\\Microsoft\\Windows\\EdgeUI",
            L"DisableMFUTracking", 1);

  // Fully disable the activity feed.
  set_dword(hKey, L"SOFTWARE\\Policies\\Microsoft\\Windows\\System",
            L"EnableActivityFeed", 0);
  set_dword(hKey, L"(SOFTWARE\\Policies\\Microsoft\\Windows\\System",
            L"PublishUserActivities", 0);
  set_dword(hKey, L"(SOFTWARE\\Policies\\Microsoft\\Windows\\System",
            L"UploadUserActivities", 0);

  // Do not search disks to attempt fixing a missing shortcut.
  set_dword(hKey,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer)",
            L"LinkResolveIgnoreLinkInfo", 1);
  set_dword(hKey,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer)",
            L"NoResolveSearch", 1);
  set_dword(hKey,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer)",
            L"NoResolveTrack", 1);

  // Disable Device Search History.
  set_dword(hKey,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\SearchSettings",
            L"IsDeviceSearchHistoryEnabled", 0);

  // By default Windows does not automatically back-up the registry, but just in
  // case they change this..
  set_dword(
      hKey,
      L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Configuration Manager",
      L"EnablePeriodicBackup", 0);

  // [P2] Disable tracking of application startups.
  set_dword(hKey,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
            L"Start_TrackProgs", 0);
  set_dword(hKey, L"Software\\Policies\\Microsoft\\Windows\\EdgeUI)",
            L"DisableMFUTracking", 1);

  gp_cleanup(hr);
  return 0;
};
