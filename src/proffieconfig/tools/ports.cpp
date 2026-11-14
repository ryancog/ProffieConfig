#include "ports.hpp"
#include <filesystem>
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/tools/ports.cpp
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

#if __linux__
#include <sstream>

#include <linux/netlink.h>
#include <sys/eventfd.h>
#include <sys/poll.h>
#include <sys/socket.h>
#endif

#include "log/context.hpp"
#include "utils/defer.hpp"

namespace ports {

namespace {

thread_local logging::Logger *logger{nullptr};

#if __linux__
namespace fs = std::filesystem;

std::string file_string(const fs::path& path) {
    std::ostringstream ostream;
    std::ifstream istream(path);
    ostream << istream.rdbuf();
    return ostream.str();
}
#endif

} // namespace

// TODO: The only reason that notification/listening and the actual data parsing
// is seperated is because I started with Win32 and SetupAPI, but used Config
// Manager for the notification, so that seperation continued.
//
// It seems like it'd be possible and reasonable to make this more efficient,
// but it's already done this way, and it's not a big enough of a deal for me to
// want to try to "fix" it, especially since getting boards is still based
// around the idea of fetching a list.

void Listener::activate() {
#   if _WIN32
    CM_NOTIFY_FILTER filter;
    filter.cbSize = sizeof filter;
    filter.Flags = CM_NOTIFY_FILTER_FLAG_ALL_INTERFACE_CLASSES;
    filter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEINTERFACE;
    std::memset(
        &filter.u.DeviceInterface.ClassGuid,
        0,
        sizeof filter.u.DeviceInterface.ClassGuid
    );
    CM_Register_Notification(
        &filter,
        this,
        pnp_notify_cb,
        &m_pnp_notify_handle
    );
    on_port_update();
#   elif __APPLE__
    m_io_notify_port = IONotificationPortCreate(kIOMasterPortDefault);
    CFRunLoopAddSource(
        CFRunLoopGetCurrent(),
        IONotificationPortGetRunLoopSource(m_io_notify_port),
        kCFRunLoopDefaultMode
    );
    IOServiceAddMatchingNotification(
        m_io_notify_port,
        kIOFirstMatchNotification,
        IOServiceMatching("IOUSBHostDevice"),
        io_notify_cb,
        this,
        &m_io_device_iter
    );
    // It may take a moment for the serial object to get setup, so an extra
    // notification needs to be setup for that, in addition to the usual usb
    // matching for bootloader.
    IOServiceAddMatchingNotification(
        m_io_notify_port,
        kIOFirstMatchNotification,
        IOServiceMatching("IOSerialBSDClient"),
        io_notify_cb,
        this,
        &m_io_serial_iter
    );
    IOServiceAddMatchingNotification(
        m_io_notify_port,
        kIOTerminatedNotification,
        IOServiceMatching("IOUSBHostDevice"),
        io_notify_cb,
        this,
        &m_io_removed_iter
    );
    // Run initial pumps
    io_notify_cb(this, m_io_device_iter);
    io_notify_cb(this, m_io_serial_iter);
    io_notify_cb(this, m_io_removed_iter);
#   elif __linux__
    m_stop_evt = eventfd(0, EFD_CLOEXEC);
    if (0 > m_stop_evt)
        return;

    m_thread = std::thread([this] { poll_thread_func(); });
#   endif
}

void Listener::deactivate() {
#   if _WIN32
    CM_Unregister_Notification(m_pnp_notify_handle);
#   elif __APPLE__
    IOObjectRelease(m_io_device_iter);
    IOObjectRelease(m_io_serial_iter);
    IOObjectRelease(m_io_removed_iter);

    CFRunLoopRemoveSource(
        CFRunLoopGetCurrent(),
        IONotificationPortGetRunLoopSource(m_io_notify_port),
        kCFRunLoopDefaultMode
    );

    IONotificationPortDestroy(m_io_notify_port);
#   elif __linux__
    uint64 evt_data{1};
    write(m_stop_evt, &evt_data, sizeof evt_data);

    if (m_thread.joinable())
        m_thread.join();

    close(m_stop_evt);
#   endif
}

#if _WIN32
DWORD CALLBACK Listener::pnp_notify_cb(
    HCMNOTIFICATION,
    PVOID arg,
    CM_NOTIFY_ACTION,
    PCM_NOTIFY_EVENT_DATA,
    DWORD
) {
    reinterpret_cast<Listener *>(arg)->on_port_update();
    return ERROR_SUCCESS;
}
#elif __APPLE__
void Listener::io_notify_cb(void *arg, io_iterator_t iter) {
    // Drain the iterator to re-arm the notification
    while (not false) {
        auto object{IOIteratorNext(iter)};
        if (object == IO_OBJECT_NULL)
            break;

        IOObjectRelease(object);
    }

    reinterpret_cast<Listener *>(arg)->on_port_update();
}
#elif __linux__
void Listener::poll_thread_func() {
    // Listen to kernel uevents to monitor for add/remove events.
    auto nl_socket{socket(
        AF_NETLINK,
        SOCK_DGRAM | SOCK_CLOEXEC,
        NETLINK_KOBJECT_UEVENT
    )};
    if (0 > nl_socket) {
        logging::Context::getGlobal().quickLog(
            logging::Severity::Warn,
            "ports::Listener::poll_thread_func()",
            "Creating uevent socket failed: " + std::to_string(errno)
        );
        return;
    }

    defer { close(nl_socket); };

    sockaddr_nl sockaddr;
    sockaddr.nl_family = AF_NETLINK;
    // Should be set to zero
    sockaddr.nl_pad = 0;
    // Let the kernel figure out the port id.
    sockaddr.nl_pid = 0;
    // It's probably documented somewhere, but looking at the linux kernel
    // sources, uevents are broadcasted to the group for bit 0.
    sockaddr.nl_groups = 1;

    if (0 > bind(nl_socket, (struct sockaddr *)&sockaddr, sizeof sockaddr)) {
        logging::Context::getGlobal().quickLog(
            logging::Severity::Warn,
            "ports::Listener::poll_thread_func()",
            "Binding uevent socket failed: " + std::to_string(errno)
        );
        return;
    }

    // Setup done, do initial update.
    on_port_update();

    std::array pollfds{
        pollfd{.fd=nl_socket, .events=POLLIN, .revents=0},
        pollfd{.fd=m_stop_evt, .events=POLLIN, .revents=0},
    };

    auto buf{std::make_unique<std::array<char, 0x4000>>()};
    while (not false) {
        if (0 > poll(pollfds.data(), pollfds.size(), -1)) {
            if (errno == EINTR)
                continue;

            break;
        }

        // Check if it was the stop event, otherwise it's the netlink.
        if (pollfds[1].revents)
            break;

        auto bytes_recvd{recv(
            nl_socket,
            buf->data(),
            buf->size(),
            MSG_DONTWAIT
        )};
        if (0 > bytes_recvd)
            continue;

        // Lazy way to make things more readable.
        UEventMap entries;

        for (size buf_idx{0}; buf_idx < bytes_recvd;) {
            cstring entry(&buf->data()[buf_idx]);
            std::string_view view(entry);

            defer {
                // Length plus jump over the null
                buf_idx += view.length() + 1;
            };

            // First string isn't a key-value format and doesn't have any data
            // the entries don't.
            if (buf_idx == 0)
                continue;

            auto equal_pos{view.find('=')};
            entries[view.substr(0, equal_pos)] = view.substr(equal_pos + 1);
        }

        if (
                (entries["ACTION"] == "add" and
                 entries["SUBSYSTEM"] == "usb" and
                 entries["DEVTYPE"] == "usb_device") or
                (entries["ACTION"] == "add" and
                 entries["SUBSYSTEM"] == "tty") or
                (entries["ACTION"] == "remove" and
                 entries["SUBSYSTEM"] == "usb" and
                 entries["DEVTYPE"] == "usb_device")
           ) on_port_update();
    }
}
#endif

USBID Enumerated::id() const {
#   if _WIN32
    BOOL reg_success{};

    // See how much to allocate to retrieve.
    DWORD reg_length{};
    reg_success = SetupDiGetDeviceRegistryPropertyA(
        m_info_set,
        m_info_data,
        SPDRP_HARDWAREID,
        nullptr,
        nullptr,
        0,
        &reg_length
    );
    if (not reg_success) {
        // It seems counterintuitive, but this function still returns
        // this error even with the buffer null and size 0? Whatever...
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            return {};
    }

    char *buf{new char[reg_length]};
    defer { delete[] buf; };

    // Fetch data
    DWORD reg_type{};
    reg_success = SetupDiGetDeviceRegistryPropertyA(
        m_info_set,
        m_info_data,
        SPDRP_HARDWAREID,
        &reg_type,
        (PBYTE)buf,
        reg_length,
        nullptr
    );
    if (not reg_success)
        return {};

    // Sanity check the data type, so we know the null-termination is
    // there, otherwise Bad Things might happen.
    if (reg_type != REG_MULTI_SZ)
        return {};

    // Then test each entry to see if one matching the VID and PID
    // can be found.
    std::string_view view;
    for (
            // Iteration begins at the buffer start, the first string
            std::size_t buf_idx{0};
            // Create a view, which will automatically find the null
            // terminator, then test to see if the string is empty, if
            // it is, that's the end.
            view = &buf[buf_idx], not view.empty();
            // Jump to the next string in the buffer
            buf_idx += view.length() + 1
        ) {
        constexpr std::string_view VID_MARK{"VID_"};
        constexpr std::string_view PID_MARK{"&PID_"};

        const auto vid_mark_pos{view.find(VID_MARK)};
        if (vid_mark_pos == std::string::npos)
            continue;

        const auto pid_mark_pos{view.find(PID_MARK)};
        if (pid_mark_pos == std::string::npos)
            continue;

        const auto vid_pos{vid_mark_pos + VID_MARK.length()};
        const auto pid_pos{pid_mark_pos + PID_MARK.length()};

        const auto vid_end_pos{pid_mark_pos};
        // If there isn't another "&", then npos is the right value.
        const auto pid_end_pos{view.find('&', pid_pos)};

        const auto from_chars{[&](
            std::size_t begin,
            std::size_t end,
            uint16& v
        ) {
            auto [ptr, ec]{std::from_chars(
                &view[begin],
                end == std::string::npos
                    ? view.data() + view.size()
                    : &view[end],
                v,
                16
            )};
            return ec == std::errc();
        }};

        uint16 vid{};
        if (not from_chars(vid_pos, vid_end_pos, vid))
            continue;

        uint16 pid{};
        if (not from_chars(pid_pos, pid_end_pos, pid))
            continue;

        return {.vid_=vid, .pid_=pid};
    }

    // ID couldn't be retrieved.
    return {};
#   elif __APPLE__
    const auto vid{property_uint16(m_obj, CFSTR("idVendor"))};
    const auto pid{property_uint16(m_obj, CFSTR("idProduct"))};
    if (not vid or not pid)
        return {};

    return {.vid_=*vid, .pid_=*pid};
#   elif __linux__
    const auto str_to_uint16{[](const std::string& str, uint16& v) {
        auto [ptr, ec]{std::from_chars(
            str.data(),
            str.data() + str.size(),
            v,
            16
        )};
        return ec == std::errc();
    }};
    auto vid_str{file_string(m_path / "idVendor")};
    auto pid_str{file_string(m_path / "idProduct")};

    uint16 vid{};
    if (not str_to_uint16(vid_str, vid))
        return {};

    uint16 pid{};
    if (not str_to_uint16(pid_str, pid))
        return {};

    return {.vid_=vid, .pid_=pid};
#   else
    return {};
#   endif
}

std::optional<std::string> Enumerated::serial_path() const {
#   if _WIN32
    // Open the registry key for the device, in which we can access properties
    // not exposed by the SetupAPI constants.
    HKEY hkey{SetupDiOpenDevRegKey(
        m_info_set,
        m_info_data,
        DICS_FLAG_GLOBAL,
        0,
        DIREG_DEV,
        KEY_READ
    )};
    if (hkey == INVALID_HANDLE_VALUE) {
        std::error_code ec(
            static_cast<int32>(GetLastError()),
            std::system_category()
        );
        logger->error("Failed opening device registry: " + ec.message());
        return {};
    }

    std::error_code ec;
    DWORD reg_length{};
    char *buf{nullptr};
    defer { delete[] buf; };

    auto get_value{[&] -> std::error_code {
        auto err{RegGetValueA(
            hkey,
            nullptr,
            "PortName",
            RRF_RT_REG_SZ,
            nullptr,
            buf,
            &reg_length
        )};
        return {
            static_cast<int32>(err),
            std::system_category()
        };
    }};
    if (
            // Read to get length, alloc buf, then read again
            (ec = get_value()) or
            (buf = new char[reg_length], ec = get_value())
       ) {
        logger->error("Failed reading device port: " + ec.message());
        return {};
    }

    return buf;
#   elif __APPLE__
    // Go down the hierarchy to find the callout device.
    io_iterator_t cu_iter{IO_OBJECT_NULL};
    IORegistryEntryCreateIterator(
        m_obj,
        kIOServicePlane,
        kIORegistryIterateRecursively,
        &cu_iter
    );
    defer { IOObjectRelease(cu_iter); };

    std::optional<std::string> cu_dev;
    while (not false) {
        auto entry{IOIteratorNext(cu_iter)};
        if (entry == IO_OBJECT_NULL)
            break;

        defer { IOObjectRelease(entry); };

        if (not IOObjectConformsTo(entry, "IOSerialBSDClient"))
            continue;

        cu_dev = property_string(entry, CFSTR("IOCalloutDevice"));
        if (cu_dev)
            break;
    }

    return cu_dev;
#   elif __linux__
    // Look for interfaces, which are directories that start with the device
    // identifier and a colon.
    const auto interface_prefix{m_path.filename().string() + ':'};

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(m_path, ec)) {
        if (not entry.is_directory())
            continue;

        auto filename{entry.path().filename().string()};
        if (not filename.starts_with(interface_prefix))
            continue;

        const auto ttys_dir{entry.path() / "tty"};
        for (const auto& tty_ent : fs::directory_iterator(ttys_dir, ec))
            return "/dev/" + tty_ent.path().filename().string();
    }

    return {};
#   else
    return {};
#   endif
}

#if __APPLE__
std::optional<uint16> property_uint16(
    io_registry_entry_t entry,
    CFStringRef key
) {
    CFTypeRef property{IORegistryEntryCreateCFProperty(
        entry,
        key,
        kCFAllocatorDefault,
        0
    )};
    if (property == nil)
        return std::nullopt;

    defer { CFRelease(property); };

    if (CFGetTypeID(property) != CFNumberGetTypeID())
        return std::nullopt;

    CFNumberRef number{(CFNumberRef)property};

    SInt32 value{};
    if (CFNumberGetValue(number, kCFNumberSInt32Type, &value))
        return value;

    return std::nullopt;
}

std::optional<std::string> property_string(
    io_registry_entry_t entry,
    CFStringRef key
) {
    CFTypeRef property{IORegistryEntryCreateCFProperty(
        entry,
        key,
        kCFAllocatorDefault,
        0
    )};
    if (property == nil)
        return std::nullopt;

    defer { CFRelease(property); };

    if (CFGetTypeID(property) != CFStringGetTypeID())
        return std::nullopt;

    CFStringRef string{(CFStringRef)property};
    std::string ret;

    ret.resize(CFStringGetLength(string));
    auto converted{CFStringGetCString(
        string,
        ret.data(),
        ret.length() + 1,
        kCFStringEncodingUTF8
    )};
    if (not converted)
        return std::nullopt;

    return ret;
}
#endif

void enumerate(const std::function<void(const Enumerated&)>& cb) {
    logger = &logging::Context::getGlobal().createLogger("ports::enumerate()");
    defer { logger = nullptr; };

#   if _WIN32
    // Retrieve the device info list for all connected USB devices.
    HDEVINFO info_set{SetupDiGetClassDevsA(
        nullptr,
        "USB",
        nullptr,
        DIGCF_ALLCLASSES | DIGCF_PRESENT
    )};
    if (info_set == INVALID_HANDLE_VALUE) {
        std::error_code ec(
            static_cast<int32>(GetLastError()),
            std::system_category()
        );
        logger->error("Failed retrieving USB device info list: " + ec.message());
        return;
    }

    // Make sure it's cleaned up once we're done here.
    defer {
        if (not SetupDiDestroyDeviceInfoList(info_set)) {
            std::error_code ec(
                static_cast<int32>(GetLastError()),
                std::system_category()
            );
            logger->warn("Failed destroying device info list: " + ec.message());
        }
    };

    SP_DEVINFO_DATA info_data;
    info_data.cbSize = sizeof info_data;
    for (DWORD idx{0}; SetupDiEnumDeviceInfo(info_set, idx, &info_data); ++idx) {
        // The generic USBDevice setup class GUID isn't defined anywhere?
        constexpr GUID GUID_DEVCLASS_USBDEVICE{0x88bae032L, 0x5a81, 0x49f0, {0xbc, 0x3d, 0xa4, 0xff, 0x13, 0x82, 0x16, 0xd6}};

        // ProffieOS serial will enumerate as a COM Port, under DEVCLASS_PORTS,
        // and bootloader will show up under the generic DEVCLASS_USBDEVICE.
        if (
                info_data.ClassGuid != GUID_DEVCLASS_PORTS and
                info_data.ClassGuid != GUID_DEVCLASS_USBDEVICE
           ) continue;

        Enumerated enumerated;
        enumerated.m_info_set = info_set;
        enumerated.m_info_data = &info_data;
        cb(enumerated);
    }

    std::error_code ec(
        static_cast<int32>(GetLastError()),
        std::system_category()
    );
    if (ec.value() != ERROR_NO_MORE_ITEMS)
        logger->warn("Failed enumerating USB devices: " + ec.message());
#   elif __APPLE__
    io_iterator_t iter{IO_OBJECT_NULL};
    IOServiceGetMatchingServices(
        kIOMasterPortDefault,
        IOServiceMatching("IOUSBHostDevice"),
        &iter
    );
    defer { IOObjectRelease(iter); };

    while (not false) {
        auto entry{IOIteratorNext(iter)};
        if (entry == IO_OBJECT_NULL)
            break;

        defer { IOObjectRelease(entry); };

        Enumerated enumerated;
        enumerated.m_obj = entry;
        cb(enumerated);
    }
#   elif __linux__
    // Search the sysfs
    std::error_code ec;
    fs::path usb_devices("/sys/bus/usb/devices/");
    for (const auto& entry : fs::directory_iterator(usb_devices, ec)) {
        // Skip interfaces, which are denoted with `:`
        if (entry.path().filename().string().contains(':'))
            continue;

        Enumerated enumerated;
        enumerated.m_path = entry.path();
        cb(enumerated);
    }
#   endif
}

} // namespace ports

