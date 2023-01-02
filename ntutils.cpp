#include <optional>

#include "ntutils.h"
#include "ntdefs.h"
#include "common.h"

#define UNICODE_STRING_TO_WSTRING_PARAMS(x) \
    ((x)->Buffer), ((x)->Length / sizeof(wchar_t))

namespace
{
    constexpr static size_t kDefaultBufferSize = 1024 * 10;

    struct ScopedHandle : private NonCopyable
    {
        ScopedHandle(HANDLE handle) noexcept : handle_(handle)
        { }

        ScopedHandle(ScopedHandle&& rhs) noexcept : handle_(rhs.handle_)
        {
            rhs.handle_ = NULL;
        }

        ~ScopedHandle()
        {
            if (handle_)
            {
                CloseHandle(handle_);
            }
        }

        inline HANDLE Get() const
        {
            return handle_;
        }

    private:
        HANDLE handle_;
    };

    std::wstring FileHandleGetKernelName(HANDLE file_handle, void* buf = nullptr, ULONG buf_size = 0)
    {
        std::wstring result;

        // Only handle regular files on disk.
        if (GetFileType(file_handle) != FILE_TYPE_DISK)
        {
            return result;
        }

        std::unique_ptr<char> owned_buffer;
        if (!buf)
        {
            buf_size = kDefaultBufferSize;
            owned_buffer.reset(new char[buf_size]);
            buf = owned_buffer.get();
            if (!buf)
            {
                return result;
            }
        }

        ULONG return_length;
        auto status = NtQueryObject(file_handle, ObjectNameInformation, buf, buf_size, &return_length);
        if (!NT_SUCCESS(status))
        {
            return result;
        }

        auto object_name = (UNICODE_STRING*)buf;
        result.assign(UNICODE_STRING_TO_WSTRING_PARAMS(object_name));

        return result;
    }

    std::optional<TypedGrowableBuffer<SYSTEM_HANDLE_INFORMATION>> RetrieveAllHandles()
    {
        TypedGrowableBuffer<SYSTEM_HANDLE_INFORMATION> buf(kDefaultBufferSize);
        while (true)
        {
            ULONG return_length;
            auto status = NtQuerySystemInformation(SystemHandleInformation, buf.Address(), (ULONG)buf.Size(), &return_length);
            if (status == STATUS_INFO_LENGTH_MISMATCH)
            {
                buf.Grow(return_length);
                continue;
            }
            else if (NT_ERROR(status))
            {
                return {};
            }
            break;
        }
        return std::move(buf);
    }
}

namespace ntutils
{
	std::wstring FilePathToKernelName(const std::wstring& file_path)
	{
        std::wstring result;

        HANDLE file_handle = CreateFileW(file_path.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (file_handle == INVALID_HANDLE_VALUE)
        {
            return result;
        }

        result = FileHandleGetKernelName(file_handle);

        ::CloseHandle(file_handle);
        return result;
	}

    void FindOpenedFileHandles(const std::wstring& target_kernel_name, HandleFindHandler&& handler)
    {
        auto handles_buf = RetrieveAllHandles();
        if (!handles_buf.has_value())
        {
            return;
        }

        auto& handles = handles_buf->Data();
        HANDLE current_process_handle = GetCurrentProcess();
        std::unordered_map<ULONG, ScopedHandle> pids_to_handles;

        for (ULONG i = 0; i < handles.HandleCount; ++i)
        {
            const auto& handle_info = handles.Handles[i];
            auto pid = handle_info.ProcessId;

            // Open or reuse an opened process handle.
            HANDLE process_handle;
            auto proc_handle_iter = pids_to_handles.find(pid);
            if (proc_handle_iter == pids_to_handles.end())
            {
                process_handle = OpenProcess(PROCESS_VM_READ | PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION, FALSE, pid);
                if (!process_handle)
                {
                    continue;
                }
                pids_to_handles.emplace(pid, ScopedHandle(process_handle));
            }
            else
            {
                process_handle = proc_handle_iter->second.Get();
            }

            // Duplicate the remote handle for inspecting locally.
            HANDLE local_handle;
            if (!DuplicateHandle(process_handle, (HANDLE)handle_info.Handle, current_process_handle, &local_handle, 0, 0, DUPLICATE_SAME_ACCESS))
            {
                continue;
            }

            // Manage `local_handle` and close it automatically when the function exits.
            ScopedHandle scoped_local_handle(local_handle);

            TypedGrowableBuffer<char[1]> temp_inspection_buf(kDefaultBufferSize);
            
            // Query the handle object type and filter out anyone other than files.
            {
                ULONG return_length;
                if (NT_ERROR(NtQueryObject(local_handle, ObjectTypeInformation, temp_inspection_buf.Address(), (ULONG)temp_inspection_buf.Size(), &return_length)))
                {
                    continue;
                }

                const auto& object_type_info = reinterpret_cast<TypedGrowableBuffer<OBJECT_TYPE_INFORMATION>*>(&temp_inspection_buf)->Data();
                std::wstring object_type_name(UNICODE_STRING_TO_WSTRING_PARAMS(&object_type_info.Name));
                if (object_type_name != L"File")
                {
                    continue;
                }
            }

            auto file_name = FileHandleGetKernelName(local_handle, temp_inspection_buf.Address(), (ULONG)temp_inspection_buf.Size());
            if (target_kernel_name == file_name)
            {
                if (handler)
                {
                    handler(process_handle, (HANDLE)handle_info.Handle);
                }
            }
        }
    }

    void CloseHandle(HANDLE proc_handle, HANDLE handle)
    {
#pragma warning( push )
#pragma warning( disable : 6387 )
        DuplicateHandle(proc_handle, handle, NULL, NULL, 0, 0, DUPLICATE_CLOSE_SOURCE);
#pragma warning( pop )
    }
}