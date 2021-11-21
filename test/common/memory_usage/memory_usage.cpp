/*
    Copyright (c) 2005-2021 Intel Corporation

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "memory_usage.h"

#include "common/utils_assert.h"

#if (_WIN32 && !__TBB_WIN8UI_SUPPORT) || TBB_CYGWIN_USE_WIN32
#include <windows.h>
#include <psapi.h>
#if _MSC_VER
#pragma comment(lib, "psapi")
#endif

#elif __unix__ || __sun
#include <sys/resource.h>
#include <unistd.h>
#include <sys/utsname.h> /* for uname */
#include <errno.h>       /* for use in LinuxKernelVersion() */

// Parse file utility for THP info
#include "src/tbbmalloc/shared_utils.h"

#elif __APPLE__ && !__ARM_ARCH
#include <unistd.h>
#include <mach/mach.h>
#include <AvailabilityMacros.h>
#if MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_6 || __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_8_0
#include <mach/shared_region.h>
#else
#include <mach/shared_memory_server.h>
#endif
#if SHARED_TEXT_REGION_SIZE || SHARED_DATA_REGION_SIZE
const size_t shared_size = SHARED_TEXT_REGION_SIZE+SHARED_DATA_REGION_SIZE;
#else
const size_t shared_size = 0;
#endif

#endif /* OS selection */

namespace utils {

    //! Return estimate of number of bytes of memory that this program is currently using.
    /* Returns 0 if not implemented on platform. */
    std::size_t GetMemoryUsage(MemoryStatType stat) {
        utils::suppress_unused_warning(stat);
#if __TBB_WIN8UI_SUPPORT || defined(WINAPI_FAMILY)
        return 0;
#elif _WIN32 || TBB_CYGWIN_USE_WIN32
        PROCESS_MEMORY_COUNTERS mem;
        bool status = GetProcessMemoryInfo(GetCurrentProcess(), &mem, sizeof(mem)) != 0;
        ASSERT(status, NULL);
        return stat == currentUsage ? mem.PagefileUsage : mem.PeakPagefileUsage;
#elif __unix__
        long unsigned size = 0;
        FILE* fst = fopen("/proc/self/status", "r");
        ASSERT(fst != nullptr, NULL);
        const int BUF_SZ = 200;
        char buf_stat[BUF_SZ];
        const char* pattern = stat == peakUsage ? "VmPeak: %lu" : "VmSize: %lu";
        while (NULL != fgets(buf_stat, BUF_SZ, fst)) {
            if (1 == sscanf(buf_stat, pattern, &size)) {
                ASSERT(size, "Invalid value of memory consumption.");
                break;
            }
        }
        // VmPeak is available in kernels staring 2.6.15
        if (stat != peakUsage || LinuxKernelVersion() >= 2006015)
            ASSERT(size, "Invalid /proc/self/status format, pattern not found.");
        fclose(fst);
        return size * 1024;
#elif __APPLE__ && !__ARM_ARCH
        // TODO: find how detect peak virtual memory size under macOS
        if (stat == peakUsage)
            return 0;
        kern_return_t status;
        task_basic_info info;
        mach_msg_type_number_t msg_type = TASK_BASIC_INFO_COUNT;
        status = task_info(mach_task_self(), TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &msg_type);
        ASSERT(status == KERN_SUCCESS, NULL);
        return info.virtual_size - shared_size;
#else
        return 0;
#endif
    }

} // namespace utils
