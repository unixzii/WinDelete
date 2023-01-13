#include "pch.h"
#include "AboutDialogContent.xaml.h"
#if __has_include("AboutDialogContent.g.cpp")
#include "AboutDialogContent.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::WinDeleteNG::implementation
{
    AboutDialogContent::AboutDialogContent()
    {
        InitializeComponent();
    }
}
