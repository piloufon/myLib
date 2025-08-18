#pragma once

#define NOMINMAX
#include <Windows.h>
#include <string>
#include <map>
#include <variant>
#include <optional>
#include <cassert>
#include <concepts>
#include <stdexcept>
#include <vector>

#pragma comment(lib, "gdi32.lib")


using std::vector;
using std::string;
using std::wstring;
using std::to_string;
using std::to_wstring;
using std::variant;
using std::map;
using std::move;

#include "..\Input\Input.h"
