#pragma once
#define NOMINMAX

#include <Windows.h>
#include <string>
#include <unordered_map>
#include <chrono>
#include <cassert>
#include <concepts>
#include <stdexcept>
#include <vector>
#include <functional>
#include <algorithm>
#pragma comment(lib, "user32.lib")

#ifdef max
	#undef max
#endif
