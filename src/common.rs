use chrono::{Datelike, Timelike, Utc};
use fltk::app;
use windows_sys::Win32::System::SystemServices::MAXIMUM_ALLOWED;
use windows_sys::Win32::System::Threading::{OpenProcess, OpenProcessToken};
use std::error::Error;
use std::os::raw::c_void;
use std::fs::{self, OpenOptions};
use std::io::Write;
use std::path::PathBuf;
use std::ptr::null_mut;
use windows::Win32::Foundation::ERROR_SUCCESS;
use windows::Win32::System::Registry::{REG_DWORD, REG_SZ, RegCreateKeyW, RegSetValueExW};
use windows_sys::Win32::Foundation::INVALID_HANDLE_VALUE;
use windows_sys::Win32::Security::{
    DuplicateTokenEx, SECURITY_ATTRIBUTES, SecurityImpersonation,
    TOKEN_ALL_ACCESS, TOKEN_DUPLICATE, TokenImpersonation,
};
use windows::core::PCWSTR;
use windows::{
    Win32::System::{
        Com::{CLSCTX_INPROC_SERVER, COINIT_APARTMENTTHREADED, CoCreateInstance, CoInitializeEx},
        GroupPolicy::{
            CLSID_GroupPolicyObject, GPO_OPEN_LOAD_REGISTRY, GPO_SECTION_MACHINE,
            IGroupPolicyObject, REGISTRY_EXTENSION_GUID,
        },
        Registry::RegCloseKey,
    },
    core::GUID,
};
use winsafe::{
    self as w, HKEY, RegistryValue,
    co::{self, KNOWNFOLDERID},
    prelude::*,
};

pub fn get_windows_path(folder_id: &KNOWNFOLDERID) -> Result<String, Box<dyn Error>> {
    let the_path = w::SHGetKnownFolderPath(folder_id, co::KF::DEFAULT, None)?;
    Ok(the_path)
}

pub fn set_dword_gpo(
    hkey: windows::Win32::System::Registry::HKEY,
    subkey: PCWSTR,
    value_name: PCWSTR,
    value: u32,
) -> Result<(), Box<dyn Error>> {
    unsafe {
        let mut new_key: windows::Win32::System::Registry::HKEY = hkey;

        let result = RegCreateKeyW(new_key, subkey, &mut new_key);
        if result != ERROR_SUCCESS {
            return Err(format!("[set_dword_gpo] Failed to create key: {:?}", result).into());
        }

        let bytes = value.to_ne_bytes();

        let result = RegSetValueExW(new_key, value_name, 0, REG_DWORD, Some(&bytes));
        if result != ERROR_SUCCESS {
            return Err(format!("[set_dword_gpo] Failed to set key value: {:?}", result).into());
        }

        let result = RegCloseKey(new_key);
        if result != ERROR_SUCCESS {
            return Err(format!("[set_dword_gpo] Failed to close key: {:?}", result).into());
        }
    }
    Ok(())
}

pub fn set_string_gpo(
    hkey: windows::Win32::System::Registry::HKEY,
    subkey: PCWSTR,
    value_name: PCWSTR,
    value: PCWSTR,
) -> Result<(), Box<dyn Error>> {
    unsafe {
        let mut new_key: windows::Win32::System::Registry::HKEY = hkey;

        let result = RegCreateKeyW(new_key, subkey, &mut new_key);
        if result != ERROR_SUCCESS {
            return Err(format!("[set_string_gpo] Failed to create key: {:?}", result).into());
        }

        let bytes = value.as_wide();
        let length = bytes.len().checked_mul(2).unwrap();
        let bytes_cast: *const u8 = bytes.as_ptr().cast();
        let slice = std::slice::from_raw_parts(bytes_cast, length);

        let result = RegSetValueExW(new_key, value_name, 0, REG_SZ, Some(slice));
        if result.is_err() {
            return Err(format!("[set_string_gpo] Failed to set key: {:?}", result).into());
        }

        let result = RegCloseKey(new_key);
        if result.is_err() {
            return Err(format!("[set_string_gpo] Failed to close key: {:?}", result).into());
        }
    }
    Ok(())
}

pub fn set_dword(
    hkey: &HKEY,
    subkey: &str,
    value_name: &str,
    value: u32,
) -> Result<(), Box<dyn Error>> {
    let o_subkey = subkey;
    let hkey_text = get_hkey_text(hkey)?;

    let (subkey, _) = hkey
        .RegCreateKeyEx(
            subkey,
            None,
            co::REG_OPTION::NON_VOLATILE,
            co::KEY::WRITE,
            None,
        )
        .expect(&format!(
            "Failed to open a DWORD in key: {}\\{}\\{}",
            hkey_text, o_subkey, value_name
        ));
    subkey
        .RegSetValueEx(Some(value_name), RegistryValue::Dword(value.clone()))
        .expect(&format!(
            "Failed to set a DWORD in key: {}\\{}\\{} -> {}",
            hkey_text, o_subkey, value_name, value
        ));

    match log_registry(hkey, o_subkey, value_name, &value.to_string(), "DWORD") {
        Ok(_) => Ok(()),
        Err(e) => Err(format!(
            "Failed to log DWORD change for key: {}\\{}\\{} -> {} -> Error: {}",
            hkey_text, o_subkey, value_name, value, e
        )
        .into()),
    }
}

pub fn set_string(
    hkey: &HKEY,
    subkey: &str,
    value_name: &str,
    value: &str,
) -> Result<(), Box<dyn Error>> {
    let o_subkey = subkey;
    let hkey_text = get_hkey_text(hkey).unwrap();

    let (subkey, _) = hkey
        .RegCreateKeyEx(
            subkey,
            None,
            co::REG_OPTION::NON_VOLATILE,
            co::KEY::WRITE,
            None,
        )
        .expect(&format!(
            "Failed to open String key: {}\\{}\\{}",
            hkey_text, o_subkey, value_name
        ));
    let value = value.to_string();
    subkey
        .RegSetValueEx(Some(value_name), RegistryValue::Sz(value.clone()))
        .expect(&format!(
            "Failed to set String value in key: {}\\{}\\{}",
            hkey_text, o_subkey, value_name
        ));

    log_registry(hkey, o_subkey, value_name, &value, "String").expect(&format!(
        "Failed to log String change for key: {}\\{}\\{} -> {}",
        hkey_text, o_subkey, value_name, value
    ));
    Ok(())
}

pub fn remove_subkey(hkey: &HKEY, subkey: &str) -> Result<(), Box<dyn Error>> {
    let o_subkey = subkey;
    let hkey_text = get_hkey_text(hkey).unwrap();

    match hkey.RegDeleteTree(Some(subkey)) {
        Ok(_) => Ok(()),
        Err(e) if e == w::co::ERROR::FILE_NOT_FOUND => Ok(()),
        Err(e) => Err(Box::new(e)),
    }
    .expect(&format!(
        "Failed to delete subkey: {}\\{}",
        hkey_text, o_subkey
    ));

    log_registry(hkey, o_subkey, "->", "", "Removed")?;
    Ok(())
}

pub fn check_dword(
    hkey: &HKEY,
    subkey: &str,
    value_name: &str,
    expected_value: u32,
) -> Result<bool, Box<dyn Error>> {
    let o_subkey = subkey;
    let hkey_text = get_hkey_text(hkey).unwrap();

    let subkey = match hkey.RegGetValue(Some(subkey), Some(value_name)) {
        Ok(value) => value,
        Err(e) if e == w::co::ERROR::FILE_NOT_FOUND => return Ok(false),
        Err(e) => {
            return Err(format!(
                "Failed to open key for DWORD check: {}\\{}\\{} -> Error: {}",
                hkey_text, o_subkey, value_name, e
            )
            .into());
        }
    };

    if let RegistryValue::Dword(value) = subkey {
        if value != expected_value {
            Ok(false)
        } else {
            Ok(true)
        }
    } else {
        Err("Expected DWORD value".into())
    }
}

fn get_hkey_text(hkey: &HKEY) -> Result<&str, Box<dyn Error>> {
    let result = if *hkey == HKEY::LOCAL_MACHINE {
        "HKEY_LOCAL_MACHINE"
    } else if *hkey == HKEY::CURRENT_USER {
        "HKEY_CURRENT_USER"
    } else {
        "UNKNOWN_HKEY"
    };

    Ok(result)
}

fn log_registry(
    hkey: &HKEY,
    subkey: &str,
    value_name: &str,
    value: &str,
    type_name: &str,
) -> Result<(), Box<dyn Error>> {
    let hkey_text = get_hkey_text(hkey)?;

    // Can't use &KNOWNFOLDERID::Desktop because we're running as TrustedInstaller.
    let desktop_dir = get_windows_path(&KNOWNFOLDERID::PublicDesktop)?;
    let mut log_path = PathBuf::from(desktop_dir);
    log_path.push("W11Boost Logs");

    if !log_path.exists() {
        fs::create_dir_all(&log_path).map_err(|e| {
            format!(
                "Failed to create log directory: {} - {}",
                log_path.display(),
                e
            )
        })?;
    }

    let now = Utc::now();
    let time_info = format!(
        "{:02}/{:02}/{} {:02}:{:02}:{:02}",
        now.day(),
        now.month(),
        now.year(),
        now.hour(),
        now.minute(),
        now.second()
    );

    let log_entry = format!(
        "{} -> {}\\{}\\{}\\{} -> {}\n",
        time_info, hkey_text, subkey, value_name, type_name, value
    );

    log_path.push("Registry.log");

    let mut file = OpenOptions::new()
        .create(true)
        .append(true)
        .open(&log_path)
        .map_err(|e| {
            format!(
                "Failed to open/create log file: {} - {}",
                log_path.display(),
                e
            )
        })?;

    // Write log entry with explicit error handling
    file.write_all(log_entry.as_bytes()).map_err(|e| {
        format!(
            "Failed to write to log file: {} - {}",
            log_path.display(),
            e
        )
    })?;

    Ok(())
}

pub fn center() -> (i32, i32) {
    (
        (app::screen_size().0 / 2.0) as i32,
        (app::screen_size().1 / 2.0) as i32,
    )
}

pub fn init_registry_gpo(
    mut hkey: windows::Win32::System::Registry::HKEY,
) -> Result<(windows::Win32::System::Registry::HKEY, IGroupPolicyObject), Box<dyn Error>> {
    unsafe {
        // The apartment thread model is required for GPOs.
        let result = CoInitializeEx(None, COINIT_APARTMENTTHREADED);
        if result.is_err() {
            return Err(format!("Failed to run CoInitalizeEx: {:?}", result).into());
        }
        let gpo: IGroupPolicyObject =
            CoCreateInstance(&CLSID_GroupPolicyObject, None, CLSCTX_INPROC_SERVER)
                .expect("Failed to create GPO object");

        gpo.OpenLocalMachineGPO(GPO_OPEN_LOAD_REGISTRY)
            .expect("Failed to open local machine GPO");

        gpo.GetRegistryKey(GPO_SECTION_MACHINE, &mut hkey)
            .expect("GetRegistryKey failed");

        Ok((hkey, gpo))
    }
}

pub fn save_registry_gpo(
    hkey: windows::Win32::System::Registry::HKEY,
    gpo: IGroupPolicyObject,
) -> Result<(), Box<dyn Error>> {
    let mut snap_guid = GUID::from_u128(0x0f6b957e_509e_11d1_a7cc_0000f87571e3);
    let mut registry_guid = REGISTRY_EXTENSION_GUID;
    unsafe {
        gpo.Save::<bool, bool>(
            true.into(),
            false.into(),
            &mut registry_guid,
            &mut snap_guid,
        )
        .expect("Failed to save GPO changes");
    }

    let result = unsafe { RegCloseKey(hkey) };
    if result.is_err() {
        eprintln!("RegCloseKey failed");
    }

    Ok(())
}

pub fn create_access_token_from_pid(process_id: u32) -> Result<*mut c_void, Box<dyn Error>> {
    let mut dup_token = INVALID_HANDLE_VALUE;
    unsafe {
        let process = OpenProcess(MAXIMUM_ALLOWED, 0, process_id);
        if !process.is_null() {
            let mut token = INVALID_HANDLE_VALUE;
            OpenProcessToken(process, TOKEN_DUPLICATE, &mut token);

            let attributes = SECURITY_ATTRIBUTES {
                nLength: size_of::<SECURITY_ATTRIBUTES>() as u32,
                lpSecurityDescriptor: null_mut(),
                bInheritHandle: 0,
            };

            DuplicateTokenEx(
                token,
                TOKEN_ALL_ACCESS,
                &attributes,
                SecurityImpersonation,
                TokenImpersonation,
                &mut dup_token,
            );
        }
    }

    Ok(dup_token)
}
