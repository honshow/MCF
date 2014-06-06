// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_MCFBUILD_HPP_
#define MCFBUILD_MCFBUILD_HPP_

#include <array>
#include <cstddef>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../MCF/StdMCF.hpp"
#include "../MCF/Core/String.hpp"
#include "../MCF/Core/Utilities.hpp"
#include "../MCF/Core/Exception.hpp"

#include "Localization.hpp"

namespace MCFBuild {

typedef std::array<unsigned char, 32> Sha256;

}

#define FORMAT_THROW(code, msg)	MCF_THROW(code, ::MCFBuild::FormatString(msg))

#endif
