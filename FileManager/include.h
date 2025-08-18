#pragma once
#include <Windows.h>
#include <string>
#include <map>
#include <variant>
#include <optional>
#include <cassert>
#include <concepts>
#include <stdexcept>
#include <vector>



using std::vector;
using std::string;
using std::wstring;
using std::to_string;
using std::to_wstring;
using std::variant;
using std::map;
using std::move;

#include "..\LogManager\LogManager.h"

#ifdef CreateFile
	#undef CreateFile
#endif
#ifdef CreateDirectory
	#undef CreateDirectory
#endif
#ifdef DeleteFile
	#undef DeleteFile
#endif
