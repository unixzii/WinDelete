#pragma once

#include "AboutDialogContent.g.h"

namespace winrt::WinDeleteNG::implementation
{
    struct AboutDialogContent : AboutDialogContentT<AboutDialogContent>
    {
        AboutDialogContent();
    };
}

namespace winrt::WinDeleteNG::factory_implementation
{
    struct AboutDialogContent : AboutDialogContentT<AboutDialogContent, implementation::AboutDialogContent>
    {
    };
}
