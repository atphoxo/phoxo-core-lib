#pragma once

/// @cond
#include <windows.h>
#include <comdef.h> // for SDK app, must include this before include GDI+
#include <shlwapi.h>
#pragma comment (lib, "Shlwapi.lib")

#include <cmath>
#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <span>
#include <optional>
#include <algorithm>
#include <execution>

#ifndef GDIPVER
    #define GDIPVER 0x0110
#endif
#include <GdiPlus.h>
#pragma comment (lib, "GdiPlus.lib")
/// @endcond

//-------------------------------------------------------------------------------------
#define _PHOXO_NAMESPACE(name) namespace name {
#define _PHOXO_BEGIN  namespace phoxo {
#define _PHOXO_INTERNAL_BEGIN  namespace internal {
#define _PHOXO_EFFECT_BEGIN  namespace effect {
#define _PHOXO_NAMESPACE_END   }

_PHOXO_BEGIN

using GPointF = Gdiplus::PointF;
using std::unique_ptr, std::make_unique;

_PHOXO_NAMESPACE_END

//-------------------------------------------------------------------------------------
#include "color.h"
#include "utils.h"
#include "math.h"
#include "bitmap_hdc.h"
#include "file_ext.h"
#include "pixel_func.h"
#include "progress_listener.h"

#include "wic/wic_interface.h"
#include "wic/wic_interface2.h"
#include "wic/wic_bitmap_lock.h"
#include "wic/wic_orientation_tag.h"
#include "wic/wic_metadata_iterator.h"
#include "wic/wic_metadata.h"
#include "wic/wic_factory.h" // Uses global WIC factory (g_factory)
#include "wic/wic_system_codec.h" // Uses global WIC factory (g_factory)
