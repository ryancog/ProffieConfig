#include "vcredist.hpp" 
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * launcher/vcredist.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <commctrl.h>
#include <winerror.h>

namespace {

HRESULT CALLBACK task_dialog_cb(
    _In_ HWND hwnd,
    _In_ UINT msg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_ LONG_PTR lpRefData
);

} // namespace

// The launcher is the first ProffieConfig code that will run on a user's
// computer, so this function in particular is potentially the first. Before all
// else, it needs to make sure that the Visual C++ redist is installed,
// otherwise things might just... crash.
// See GH#34 and the following link:
// https://developercommunity.visualstudio.com/t/Access-violation-with-std::mutex::lock-a/10664660#T-N10668856
bool check_vcredist() {
    HKEY hkey{nullptr};

    // This doesn't disappear whenever it's uninstalled (which seems silly but
    // okay Microsoft), but who realistically is uninstalling it?
    auto err{RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        R"(SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\X64)",
        0,
        KEY_READ,
        &hkey
    )};
    if (err != ERROR_SUCCESS)
        return false;

    const auto get_reg_dword{[&](cstring value, DWORD& dword) -> bool {
        DWORD len{sizeof dword};

        auto err{RegGetValueA(
            hkey,
            nullptr,
            value,
            RRF_RT_REG_DWORD,
            nullptr,
            &dword,
            &len
        )};

        return ERROR_SUCCESS == err;
    }};

    DWORD installed{};
    DWORD major{};
    DWORD minor{};

    if (
            not get_reg_dword("Installed", installed) or
            not get_reg_dword("Major", major) or
            not get_reg_dword("Minor", minor)
       ) return false;

    // Check for Visual Studio 2022 VCRedist
    //
    // Major Version 14 has been kept for a while now, across multiple major
    // Visual Studio versions.
    // Minor Version 44 corresponds to the latest for Visual Studio 2022,
    // version 17.14.
    //
    // See the following link for information regarding VS -> vcredist version
    // mapping:
    // https://learn.microsoft.com/en-us/lifecycle/faq/visual-c-faq
    return installed and major >= 14 and minor >= 44;
}

void show_redist_message() {
    TASKDIALOGCONFIG dialog_config;
    std::memset(&dialog_config, 0, sizeof dialog_config);
    dialog_config.cbSize = sizeof dialog_config;
    dialog_config.dwFlags = TDF_ENABLE_HYPERLINKS;
    dialog_config.pfCallback = task_dialog_cb;
    dialog_config.pszMainIcon = TD_INFORMATION_ICON;
    dialog_config.pszWindowTitle = L"Missing Visual C++ Redistributables";
    dialog_config.pszContent =
        L"ProffieConfig requires the Microsoft Visual C++ Redistributables.\n"
        L"Either they are not installed, or they are out of date.\n"
        L"Please install them, and then try again.\n"
        L"\n"
        L"Visit the link below for the latest download from Microsoft:\n"
        LR"(<a href="https://aka.ms/vc14/vc_redist.x64.exe">https://aka.ms/vc14/vc_redist.x64.exe</a>)";

    TaskDialogIndirect(
        &dialog_config,
        nullptr,
        nullptr,
        nullptr
    );
}

namespace {

HRESULT CALLBACK task_dialog_cb(
    _In_ HWND hwnd,
    _In_ UINT msg,
    _In_ WPARAM /*wParam*/,
    _In_ LPARAM lParam,
    _In_ LONG_PTR /*lpRefData*/
) {
    if (msg == TDN_HYPERLINK_CLICKED) {
        ShellExecuteW(
            hwnd,
            L"open",
            (LPCWSTR)lParam,
            nullptr,
            nullptr,
            SW_SHOWNORMAL
        );
    }

    return S_OK;
}

} // namespace


