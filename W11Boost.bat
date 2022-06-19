@echo off
title W11Tweak by https://github.com/nermur

REM Disables GPS services, which always run even if there's no GPS hardware installed.
set /A disable_geolocation=0

REM Printers are heavily exploitable, avoid using one if possible.
set /A disable_printer_support=0

REM Disable Explorer's thumbnail border shadows.
set /A disable_thumbnail_shadows=0

REM Ensures Windows' audio ducking/attenuation is disabled.
set /A disable_audio_reduction=0

REM Routing through IPv6 is worse than IPv4 in some areas (higher latency/ping).
set /A disable_ipv6=0

REM Undermines software that clear the clipboard automatically.
set /A disable_clipboard_history=1

REM More comprehensive than GRC's InSpectre tool, but still doesn't interfere with anti-cheats (such as Vanguard).
set /A disable_mitigations=1

REM Disables power saving features for network switches to increase their reliability.
set /A network_adapter_tweaks=1

REM Makes disks using the default file system (NTFS) faster, but disables File History and File Access Dates.
set /A ntfs_tweaks=1

REM Disables Sticky, Filter, and Toggle Keys.
set /A avoid_key_annoyances=1

REM If you don't want to install apps using the Microsoft Store from other devices or a web browser.
set /A disable_remote_msstore_installs=1

REM Use NVIDIA ShadowPlay, AMD ReLive, or OBS Studio instead.
set /A disable_game_dvr=1

reg.exe query HKU\S-1-5-19 || (
	echo ==== Error ====
	echo Right click on this file and select 'Run as administrator'
	echo Press any key to exit...
	Pause>nul
	exit /b
)

REM If these are disabled, Windows Update will break and so will this script
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\AppXSvc" /v "Start" /t REG_DWORD /d 3 /f
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\ClipSVC" /v "Start" /t REG_DWORD /d 3 /f
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\TokenBroker" /v "Start" /t REG_DWORD /d 3 /f
REM Specifically breaks Windows Store if disabled previously (by you)
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\StorSvc" /v "Start" /t REG_DWORD /d 3 /f
sc.exe start AppXSvc
sc.exe start ClipSVC
sc.exe start StorSvc

REM Required for System Restore functionality
net start VSS
powershell.exe -Command "Enable-ComputerRestore -Drive 'C:\'"

cls
echo.
echo ==== Current settings ====
echo.
echo avoid_key_annoyances = %avoid_key_annoyances%
echo disable_game_dvr = %disable_game_dvr%
echo disable_geolocation = %disable_geolocation%
echo disable_audio_reduction = %disable_audio_reduction%
echo disable_ipv6 = %disable_ipv6%
echo disable_mitigations = %disable_mitigations%
echo disable_printer_support = %disable_printer_support%
echo disable_thumbnail_shadows = %disable_thumbnail_shadows%
echo network_adapter_tweaks = %network_adapter_tweaks%
echo ntfs_tweaks = %ntfs_tweaks%
echo 
echo.
Pause
cd %SystemRoot%\System32

REM Won't make a restore point if there's already one within the past 24 hours
WMIC.exe /Namespace:\\root\default Path SystemRestore Call CreateRestorePoint "W11Tweak", 100, 7

REM Allow PowerShell scripts in current directory
powershell.exe -Command "Get-ChildItem *.ps*1 -recurse | Unblock-File"

REM Bitsum Highest Performance profile cannot install if any Power Plans were previously removed
powercfg -restoredefaultschemes
REM Sleep mode achieves the same goal while not hammering the primary hard drive, but will break in power outages/surges; regardless, leaving a PC unattended is bad. Also fixes "Fast startup" problems by disabling it
powercfg.exe /hibernate on
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Power" /V HiberbootEnabled /T REG_DWORD /D 0 /F

if %avoid_key_annoyances%==1 (
	reg.exe add "HKCU\Control Panel\Accessibility\StickyKeys" /v "Flags" /t REG_SZ /d 50 /f
	reg.exe add "HKCU\Control Panel\Accessibility\ToggleKeys" /v "Flags" /t REG_SZ /d 58 /f
	reg.exe add "HKCU\Control Panel\Accessibility\Keyboard Response" /v "Flags" /t REG_SZ /d 122 /f
)

if %disable_game_dvr%==1 (
	reg.exe add "HKCU\System\GameConfigStore" /v "GameDVR_Enabled" /t REG_DWORD /d 0 /f
	reg.exe add "HKLM\Software\Policies\Microsoft\Windows\GameDVR" /v "AllowGameDVR" /t REG_DWORD /d 0 /f
	reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\GameDVR" /v "AppCaptureEnabled" /t REG_DWORD /d 0 /f
	reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\GameDVR" /v "HistoricalCaptureEnabled" /t REG_DWORD /d 0 /f
	reg.exe add "HKCU\Software\Microsoft\GameBar" /v "UseNexusForGameBarEnabled" /t REG_DWORD /d 0 /f
	reg.exe add "HKCU\Software\Microsoft\GameBar" /v "ShowStartupPanel" /t REG_DWORD /d 0 /f
)

if %disable_geolocation%==1 (
	sc.exe stop lfsvc
	reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\lfsvc" /v "Start" /t REG_DWORD /d 4 /f
	reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\LocationAndSensors" /v "DisableLocation" /t REG_DWORD /d 1 /f
	reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\LocationAndSensors" /v "DisableWindowsLocationProvider" /t REG_DWORD /d 1 /f
	reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\LocationAndSensors" /v "DisableLocationScripting" /t REG_DWORD /d 1 /f
)

if %disable_printer_support%==1 (
	reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\Spooler" /v "Start" /t REG_DWORD /d 4 /f
	reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\PrintNotify" /v "Start" /t REG_DWORD /d 4 /f
)

if %network_adapter_tweaks%==1 (
	powershell.exe -Command ".\network_adapter_tweaks.ps1"
)

if %disable_clipboard_history%==1 (
	reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\System" /v "AllowClipboardHistory" /t REG_DWORD /d 0 /f
	reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\System" /v "AllowCrossDeviceClipboard" /t REG_DWORD /d 0 /f
)

if %disable_mitigations%==1 (
	reg.exe add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management" /v FeatureSettingsOverride /t REG_DWORD /d 3 /f 
	reg.exe add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management" /v FeatureSettingsOverrideMask /t REG_DWORD /d 3 /f
	REM Use the faster but less secure Hyper-V scheduler.
	bcdedit.exe /set hypervisorschedulertype classic
	REM Allow Intel TSX.
	reg.exe add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Kernel" /v DisableTsx /t REG_DWORD /d 0 /f
	powershell.exe -Command "Set-ProcessMitigation -PolicyFilePath disable_system_exploit_mitigations.xml"
	reg.exe add "HKLM\SYSTEM\CurrentControlSet\Control\DeviceGuard\Scenarios\CredentialGuard" /v "Enabled" /t REG_DWORD /d 0 /f
	reg.exe add "HKLM\SYSTEM\CurrentControlSet\Control\DeviceGuard\Scenarios\HypervisorEnforcedCodeIntegrity" /v "Enabled" /t REG_DWORD /d 0 /f
)

if %disable_ipv6%==1 (
	sc.exe stop iphlpsvc
	sc.exe stop IpxlatCfgSvc
	reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\iphlpsvc" /v Start /t REG_DWORD /d 4 /f
	reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\IpxlatCfgSvc" /v Start /t REG_DWORD /d 4 /f
	reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce" /v "disable_ipv6" /t REG_SZ /f /d "powershell -Command Set-NetAdapterBinding -Name '*' -DisplayName 'Internet Protocol Version 6 (TCP/IPv6)' -Enabled 0"
)

if %disable_remote_msstore_installs%==1 (
	reg.exe add "HKLM\Software\Policies\Microsoft\PushToInstall" /v "DisablePushToInstall" /t REG_DWORD /d "1" /f
	schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\PushToInstall\Registration"
)

if %ntfs_tweaks%==1 (
	fsutil behavior set disablelastaccess 3
	fsutil behavior set encryptpagingfile 0
	reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\FileHistory" /v "Disabled" /t REG_DWORD /d 1 /f
	reg.exe add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\I/O System" /v "IoBlockLegacyFsFilters" /t REG_DWORD /d 1 /f
	schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\FileHistory\File History (maintenance mode)"
)

if %disable_thumbnail_shadows%==1 (
	reg.exe add "HKCR\SystemFileAssociations\image" /v "Treatment" /t REG_DWORD /d 0 /f
	reg.exe add "HKCR\SystemFileAssociations\image" /v "TypeOverlay" /t REG_SZ /d "" /f
)

if %disable_audio_reduction%==1 (
	reg.exe add "HKCU\SOFTWARE\Microsoft\Multimedia\Audio" /v "UserDuckingPreference" /t REG_DWORD /d "3" /f
	reg.exe delete "HKCU\SOFTWARE\Microsoft\Internet Explorer\LowRegistry\Audio\PolicyConfig\PropertyStore" /f
)


reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer" /v "NoLowDiskSpaceChecks" /t REG_DWORD /d 1 /f
reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer" /v "LinkResolveIgnoreLinkInfo" /t REG_DWORD /d 1 /f

REM Don't search disks to attempt fixing a missing shortcut.
reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer" /v "NoResolveSearch" /t REG_DWORD /d 1 /f

REM Don't search all paths related to the missing shortcut.
reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer" /v "NoResolveTrack" /t REG_DWORD /d 1 /f

REM Don't waste CPU cycles to remove thumbnail caches.
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\VolumeCaches\Thumbnail Cache" /v "Autorun" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Explorer\VolumeCaches\Thumbnail Cache" /v "Autorun" /t REG_DWORD /d 0 /f

REM Don't check for an active connection through Microsoft's servers
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\NlaSvc\Parameters\Internet" /v EnableActiveProbing /t REG_DWORD /d 0 /f

reg.exe add "HKLM\SYSTEM\ControlSet001\Services\DiagTrack" /v "Start" /t REG_DWORD /d 4 /f
reg.exe add "HKLM\SYSTEM\ControlSet001\Services\dmwappushservice" /v "Start" /t REG_DWORD /d 4 /f
reg.exe add "HKLM\SYSTEM\ControlSet001\Control\WMI\Autologger\AutoLogger-Diagtrack-Listener" /v "Start" /t REG_DWORD /d 0 /f

REM Ask OneDrive to only generate network traffic if signed in to OneDrive.
reg.exe add "HKLM\SOFTWARE\Microsoft\OneDrive" /v "PreventNetworkTrafficPreUserSignIn" /t REG_DWORD /d 1 /f

REM Ask nicely to stop sending diagnostic data to Microsoft.
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\DataCollection" /v "AllowTelemetry" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\DataCollection" /v "AllowTelemetry" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\DataCollection" /v "DisableEnterpriseAuthProxy" /t REG_DWORD /d 1 /f
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\DataCollection" /v "DisableOneSettingsDownloads" /t REG_DWORD /d 1 /f
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\DataCollection" /v "DisableTelemetryOptInChangeNotification" /t REG_DWORD /d 1 /f
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\DataCollection" /v "DoNotShowFeedbackNotifications" /t REG_DWORD /d 1 /f
reg.exe add "HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Policies\DataCollection" /v "AllowTelemetry" /t REG_DWORD /d 0 /f

REM Disable "Application Compatibility Engine".
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\AppCompat" /v "DisableEngine" /t REG_DWORD /d 1 /f
REM Disable "Application Telemetry".
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\AppCompat" /v "AITEnable" /t REG_DWORD /d 0 /f
REM Disable "Program Compatibility Assistant".
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\AppCompat" /v "DisablePCA" /t REG_DWORD /d 1 /f
REM Disable "SwitchBack Compatibility Engine".
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\AppCompat" /v "SbEnable" /t REG_DWORD /d 0 /f
REM Disable user steps recorder.
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\AppCompat" /v "DisableUAR" /t REG_DWORD /d 1 /f

reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Privacy" /v "TailoredExperiencesWithDiagnosticDataEnabled" /t REG_DWORD /d 0 /f

REM Disable Autoplay on all disk types.
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\Explorer" /v "NoDriveTypeAutoRun" /t REG_DWORD /d 255 /f

REM Disable WER (Windows Error Reporting).
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\Windows Error Reporting" /v "Disabled" /t REG_DWORD /d 1 /f
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\Windows Error Reporting" /v "AutoApproveOSDumps" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\Windows Error Reporting" /v "DontSendAdditionalData" /t REG_DWORD /d 1 /f
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Windows Error Reporting\QueueReporting"

REM Disallow execution of experiments by Microsoft.
reg.exe add "HKLM64\SOFTWARE\Microsoft\PolicyManager\current\device\System" /v "AllowExperimentation" /t REG_DWORD /d 0 /f

REM Disable tracking of application startups.
reg.exe add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced" /v "Start_TrackProgs" /t REG_DWORD /d 0 /f

REM Disable all Content Delivery Manager features, which stops automatic installation of advertised apps among others.
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v "ContentDeliveryAllowed" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v "FeatureManagementEnabled" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v "OemPreInstalledAppsEnabled" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v "PreInstalledAppsEnabled" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v "PreInstalledAppsEverEnabled" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v "RemediationRequired" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v "RotatingLockScreenEnabled" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v "RotatingLockScreenOverlayEnabled" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v "SilentInstalledAppsEnabled" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v "SoftLandingEnabled" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v "SystemPaneSuggestionsEnabled" /t REG_DWORD /d 0 /f

REM Disable SmartScreen, it delays the launch of software and is better done by other anti-malware software (like Kaspersky).
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\System" /v "EnableSmartScreen" /t REG_DWORD /d 0 /f
reg.exe add "HKLM64\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer" /v "SmartScreenEnabled" /t REG_SZ /d "Off" /f
reg.exe add "HKLM64\SOFTWARE\Microsoft\Windows\CurrentVersion\AppHost" /v "EnableWebContentEvaluation" /t REG_DWORD /d 0 /f

REM Disable legacy PowerShell.
powershell.exe -Command "Disable-WindowsOptionalFeature -NoRestart -Online -FeatureName "MicrosoftWindowsPowerShellV2Root""
powershell.exe -Command "Disable-WindowsOptionalFeature -NoRestart -Online -FeatureName "MicrosoftWindowsPowerShellV2""

REM Increasing overall system/DPC latency for the sake of minimal power saving is bad.
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Control\Power\PowerThrottling" /v "PowerThrottlingOff" /t REG_DWORD /d 1 /f

REM Automated file cleanup (without user interaction) is a bad idea; Storage Sense only runs on low-disk space events.
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\StorageSense" /v "AllowStorageSenseGlobal" /t REG_DWORD /d 0 /f
reg.exe delete "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\StorageSense" /f

REM Disable these scheduler tasks to keep performance and bandwidth usage more consistent.
schtasks.exe /Change /DISABLE /TN "\Microsoft\Office\OfficeTelemetryAgentFallBack"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Office\OfficeTelemetryAgentLogOn"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\AppID\SmartScreenSpecific"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Application Experience\Microsoft Compatibility Appraiser"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Application Experience\PcaPatchDbTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Application Experience\ProgramDataUpdater"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Application Experience\StartupAppTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\ApplicationData\DsSvcCleanup"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\AppxDeploymentClient\Pre-staged app cleanup"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Autochk\Proxy"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\CertificateServicesClient\UserTask-Roam"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Chkdsk\ProactiveScan"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Clip\License Validation"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\CloudExperienceHost\CreateObjectTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Customer Experience Improvement Program\Consolidator"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Customer Experience Improvement Program\KernelCeipTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Customer Experience Improvement Program\UsbCeip"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Defrag\ScheduledDefrag"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Diagnosis\Scheduled"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\DiskCleanup\SilentCleanup"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\DiskDiagnostic\Microsoft-Windows-DiskDiagnosticDataCollector"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\DiskFootprint\Diagnostics"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\DiskFootprint\StorageSense"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Feedback\Siuf\DmClient"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Feedback\Siuf\DmClientOnScenarioDownload"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\File Classification Infrastructure\Property Definition Sync"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\FileHistory\File History (maintenance mode)"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\HelloFace\FODCleanupTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\InstallService\ScanForUpdates"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\InstallService\ScanForUpdatesAsUser"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\InstallService\SmartRetry"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\LanguageComponentsInstaller\ReconcileLanguageResources"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Location\Notifications"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Location\WindowsActionDialog"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Maintenance\WinSAT"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Maps\MapsToastTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Maps\MapsUpdateTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Mobile Broadband Accounts\MNO Metadata Parser"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\MUI\LPRemove"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Multimedia\SystemSoundsService"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\NetTrace\GatherNetworkInfo"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\PI\Sqm-Tasks"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Plug and Play\Device Install Reboot Required"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Power Efficiency Diagnostics\AnalyzeSystem"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Printing\EduPrintProv"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Ras\MobilityManager"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\RecoveryEnvironment\VerifyWinRE"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Registry\RegIdleBackup"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\RemoteAssistance\RemoteAssistanceTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\SettingSync\BackgroundUploadTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\SettingSync\NetworkStateChangeTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Setup\SetupCleanupTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Shell\FamilySafetyMonitor"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Shell\FamilySafetyRefreshTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Shell\IndexerAutomaticMaintenance"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\SoftwareProtectionPlatform\SvcRestartTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\SoftwareProtectionPlatform\SvcRestartTaskLogon"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\SoftwareProtectionPlatform\SvcRestartTaskNetwork"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Speech\HeadsetButtonPress"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Speech\SpeechModelDownloadTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Sysmain\ResPriStaticDbSync"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Sysmain\WsSwapAssessmentTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\UpdateOrchestrator\Schedule Scan Static Task"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\UpdateOrchestrator\Schedule Scan"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\UpdateOrchestrator\UpdateModelTask"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\UpdateOrchestrator\USO_UxBroker"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\USB\Usb-Notifications"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\WDI\ResolutionHost"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Windows Error Reporting\QueueReporting"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Windows Filtering Platform\BfeOnServiceStartTypeChange"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\WindowsUpdate\Scheduled Start"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\WindowsUpdate\sih"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\WOF\WIM-Hash-Management"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\WOF\WIM-Hash-Validation"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Work Folders\Work Folders Logon Synchronization"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\Work Folders\Work Folders Maintenance Work"
schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\WS\WSTask"

reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce" /v "removetask1" /t REG_SZ /f /d "schtasks.exe /Delete /F /TN \Microsoft\Windows\RetailDemo\CleanupOfflineContent"
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce" /v "removetask2" /t REG_SZ /f /d "schtasks.exe /Delete /F /TN \Microsoft\Windows\Setup\SetupCleanupTask"

attrib +R %WinDir%\System32\SleepStudy\UserNotPresentSession.etl

if exist "%WinDir%\Microsoft.NET\Framework\v2.0.50727\ngen.exe" (
	reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Run" /v "DOTNET20_Optimize1" /t REG_SZ /f /d "schtasks.exe /Create /Delay 0000:02 /TR \"cmd /c start /min %WinDir%\Microsoft.NET\Framework\v2.0.50727\ngen.exe ExecuteQueuedItems\" /RU Administrator /TN NETOptimize1 /SC ONLOGON /IT"
	reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Run" /v "DOTNET20_Optimize2" /t REG_SZ /f /d "schtasks.exe /Create /Delay 0000:02 /TR \"cmd /c start /min %WinDir%\Microsoft.NET\Framework64\v2.0.50727\ngen.exe ExecuteQueuedItems\" /RU Administrator /TN DOTNET20_Optimize3 /SC ONLOGON /IT"
)
if exist "%WinDir%\Microsoft.NET\Framework\v4.0.30319\ngen.exe" (
	reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Run" /v "DOTNET40_Optimize1" /t REG_SZ /f /d "schtasks.exe /Create /Delay 0000:02 /TR \"cmd /c start /min %WinDir%\Microsoft.NET\Framework\v4.0.30319\ngen.exe ExecuteQueuedItems\" /RU Administrator /TN NETOptimize2 /SC ONLOGON /IT"
	reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Run" /v "DOTNET40_Optimize2" /t REG_SZ /f /d "schtasks.exe /Create /Delay 0000:02 /TR \"cmd /c start /min %WinDir%\Microsoft.NET\Framework64\v4.0.30319\ngen.exe ExecuteQueuedItems\" /RU Administrator /TN DOTNET40_Optimize3 /SC ONLOGON /IT"
	schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\.NET Framework\.NET Framework NGEN v4.0.30319 64 Critical"
	schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\.NET Framework\.NET Framework NGEN v4.0.30319 64"
	schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\.NET Framework\.NET Framework NGEN v4.0.30319 Critical"
	schtasks.exe /Change /DISABLE /TN "\Microsoft\Windows\.NET Framework\.NET Framework NGEN v4.0.30319"
)

REM Disable "Customer Experience Improvement Program"; also implies turning off the Inventory Collector.
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\AppV\CEIP" /v "CEIPEnable" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Microsoft\SQMClient\Windows" /v "CEIPEnable" /t REG_DWORD /d 0 /f
reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Messenger\Client" /v "CEIP" /t REG_DWORD /d 2 /f

reg.exe add "HKLM\SOFTWARE\Policies\Microsoft\Windows\System" /v "PublishUserActivities" /t REG_DWORD /d 0 /f

REM Disable "Delivery Optimization".
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\DoSvc" /v Start /t REG_DWORD /d 4 /f
REM Disables "Diagnostic Policy Service"; logs tons of information to be sent off and analyzed by Microsoft, and in some cases caused noticeable performance slowdown.
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\DPS" /v Start /t REG_DWORD /d 4 /f
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\PcaSvc" /v Start /t REG_DWORD /d 4 /f

REM Use sane defaults for these sensitive settings, incase a modded Windows screwed them up.
bcdedit.exe /deletevalue useplatformclock
bcdedit.exe /deletevalue uselegacyapicmode
bcdedit.exe /deletevalue x2apicpolicy
bcdedit.exe /set disabledynamictick yes
bcdedit.exe /set uselegacyapicmode no

REM Don't draw the Windows logo for faster boot times.
bcdedit.exe /set bootuxdisabled on

REM A worthless security measure, just use BitLocker.
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management" /v ClearPageFileAtShutdown /t REG_DWORD /d 0 /f

REM Don't log events without warnings or errors.
auditpol.exe /set /category:* /Success:disable

REM Game scheduler tweaks; doubles GPU priority, then sets I/O and CPU priority to High.
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile\Tasks\Games" /v "GPU Priority" /t REG_DWORD /d 16 /f
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile\Tasks\Games" /v "SFIO Priority" /t REG_SZ /d "High" /f
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile\Tasks\Games" /v "Scheduling Category" /t REG_SZ /d "High" /f

REM Don't delay startup of programs.
reg.exe add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Serialize" /v "Startupdelayinmsec" /t REG_DWORD /d 0 /f

REM Decrease shutdown time.
reg.exe add "HKCU\Control Panel\Desktop" /v WaitToKillAppTimeOut /t REG_SZ /d 2000 /f
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Control" /v WaitToKillServiceTimeout /t REG_SZ /d 2000 /f
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Control" /v HungAppTimeout /t REG_SZ /d 2000 /f
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Control" /v AutoEndTasks /t REG_SZ /d 1 /f
reg.exe add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System" /v DisableShutdownNamedPipe /t REG_DWORD /d 1 /f

REM Clean out font cache; incase font cache was corrupted before running this script.
:FontCache
sc stop "FontCache"
reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\FontCache" /v "Start" /t REG_DWORD /d 4 /f
sc query FontCache | findstr /I /C:"STOPPED" 
if not %errorlevel%==0 (goto FontCache)

REM Grant access rights to current user for "%WinDir%\ServiceProfiles\LocalService" folder and contents.
icacls.exe "%WinDir%\ServiceProfiles\LocalService" /grant "%UserName%":F /C /T /Q
REM Delete font cache.
del /A /F /Q "%WinDir%\ServiceProfiles\LocalService\AppData\Local\FontCache\*FontCache*"
del /A /F /Q "%WinDir%\System32\FNTCACHE.DAT"

reg.exe add "HKLM\SYSTEM\CurrentControlSet\Services\FontCache" /v "Start" /t REG_DWORD /d 2 /f

taskkill.exe /IM explorer.exe /F
start explorer.exe
echo.
echo Your PC will restart after a key is pressed; required to fully apply changes
echo.
Pause
shutdown.exe /r /t 00