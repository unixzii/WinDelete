#pragma once

#include <Windows.h>
#include <string>
#include <functional>

namespace ntutils
{
    std::wstring FilePathToKernelName(const std::wstring& file_path);

    using HandleFindHandler = std::function<void(HANDLE proc_handle, HANDLE remote_file_handle)>;
    void FindOpenedFileHandles(const std::wstring& target_kernel_name, HandleFindHandler&& handler);

    void CloseHandle(HANDLE proc_handle, HANDLE handle);
};
