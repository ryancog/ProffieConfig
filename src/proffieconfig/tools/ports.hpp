#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/tools/ports.hpp
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

#if _WIN32
#include <cfgmgr32.h>
#include <setupapi.h>
#elif __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#elif __linux__
#include <filesystem>
#include <thread>
#include <unordered_map>
#endif

namespace ports {

struct Listener {
    virtual ~Listener() = default;

    void activate();
    void deactivate();

    virtual void on_port_update() = 0;

private:
#   if _WIN32
    static DWORD CALLBACK pnp_notify_cb(
        HCMNOTIFICATION,
        PVOID,
        CM_NOTIFY_ACTION,
        PCM_NOTIFY_EVENT_DATA,
        DWORD
    );

    HCMNOTIFICATION m_pnp_notify_handle;
#   elif __APPLE__
    static void io_notify_cb(void *, io_iterator_t);

    IONotificationPortRef m_io_notify_port;
    io_iterator_t m_io_device_iter{IO_OBJECT_NULL};
    io_iterator_t m_io_serial_iter{IO_OBJECT_NULL};
    io_iterator_t m_io_removed_iter{IO_OBJECT_NULL};
#   elif __linux__
    void poll_thread_func();

    using UEventMap = std::unordered_map<std::string_view, std::string_view>;

    std::thread m_thread;
    int m_stop_evt;
#   endif
};

struct USBID {
    uint16 vid_{0};
    uint16 pid_{0};

    bool operator==(const USBID&) const = default;

    operator bool() const {
        return vid_ != 0 and pid_ != 0;
    }
};

struct Enumerated {
    /**
     * Returns the USBID for the enumerated device.
     * May return null USBID if it couldn't be fetched.
     */
    [[nodiscard]] USBID id() const;

    /**
     * Return the serial path (COM Port, Callout Device, etc.) for the device,
     * or nullopt if there isn't one.
     */
    [[nodiscard]] std::optional<std::string> serial_path() const;

private:
    friend void enumerate(const std::function<void(const Enumerated&)>&);

#   if __APPLE__
    static std::optional<uint16> property_uint16(
        io_registry_entry_t entry,
        CFStringRef key
    );
    static std::optional<std::string> property_string(
        io_registry_entry_t entry,
        CFStringRef key
    );
#   endif

#   if _WIN32
    HDEVINFO m_info_set;
    PSP_DEVINFO_DATA m_info_data;
#   elif __APPLE__
    io_object_t m_obj;
#   elif __linux__
    std::filesystem::path m_path;
#   endif
};

void enumerate(const std::function<void(const Enumerated&)>& cb);

} // namespace ports

