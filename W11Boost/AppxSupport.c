#include "Common.h"

int write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    FILE* stream = (FILE*)userdata;
    return fwrite(ptr, size, nmemb, stream);
}

int install_appx_support() {
    CURL* curl = curl_easy_init();

    PWSTR desktopPath = get_windows_path(&FOLDERID_Desktop);
    WCHAR fullPath[MAX_PATH];
    swprintf(fullPath, MAX_PATH, L"%s\\Microsoft.DesktopAppInstaller_8wekyb3d8bbwe.msixbundle", desktopPath);

    FILE* file = NULL;
    errno_t err = _wfopen_s(&file, fullPath, L"wb");
    if (err != 0 || file == NULL) {
        free(file);
        return EXIT_FAILURE;
    }

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL,
            "https://github.com/microsoft/winget-cli/releases/latest/download/Microsoft.DesktopAppInstaller_8wekyb3d8bbwe.msixbundle");

        curl_easy_setopt(curl, CURLOPT_USERAGENT,
            "Mozilla/5.0 (Windows NT 10.0; WOW64; x64) AppleWebKit/537.36 (KHTML, like Gecko) "
            "Chrome/130.0.6556.192 Safari/537.36");

        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Required due to GitHub redirecting
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 10000L); // 10000 miliseconds -> 10 seconds

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        CURLcode res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        fclose(file);

        if (res != CURLE_OK)
            return EXIT_FAILURE;
    }

    wchar_t installAppx[] = L"powershell.exe Add-AppxPackage ([Environment]::GetFolderPath(\"Desktop\") + \"\Microsoft.DesktopAppInstaller_8wekyb3d8bbwe.msixbundle\")";
    start_command_and_wait(installAppx);

    return EXIT_SUCCESS;
}
