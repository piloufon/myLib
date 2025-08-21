# TODO

## General Improvements
- Add hint `[[likely]]` & `[[unlikely]]` where appropriate.
- Create separate README.md for each lib, with usage, dependecies, ...
---

## WindowManager
- Remove `include.h` (include only in `.cpp` and keep `namespace std { /* what's used */ }` in `.h`).  
- Add handling in `WndProc` for:
  - `DXGI_STATUS_OCCLUDED`
  - `DXGI_STATUS_UNOCCLUDED`

---

## FileManager
- Remove `include.h` (include only in `.cpp` and keep `namespace std { /* what's used */ }` in `.h`).  
- Create static functions for easier usage.  
- Presence of TODO in `FileManager.h`.

---

## Input
- Remove `include.h` (include only in `.cpp` and keep `namespace std { /* what's used */ }` in `.h`).

---

## LogManager
- Create a way to **disable log file creation** if one of the macros is defined:  
  - `NOLOGFILE`  
  - `NOTEMPFILE`  
  - `NOPERMFIL`  
 This should completely disable logging when the macros are active.

---

## DX12

### 🔹 Must do now
- In factory initialization, add:
  ```cpp
  RegisterOcclusionStatusWindow (from IDXGIFactory2) to handle window occlusion status.
- **BIG TODO** : add D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV (most important and difficult cause ring buffer, etc... dynamicly too (that's why ring buf) per-frame)
### 🔹 To consider / explore later

 - Feature & capability checks:
   - Use IDXGIFactory5::CheckFeatureSupport to check if the OS supports all required DX12 features.
   - Use ID3D12Device::CheckFeatureSupport to check hardware capabilities (RenderTarget, DepthStencil, ShaderResource, CUDA, etc.).
  - Descriptor optimizations:
    - Understand and (if needed) try ID3D12Device::CopyDescriptors for better performance when managing descriptor heaps.
  - Depth-Stencil View
    - Explore D3D12_DSV_DIMENSION 
  - Continue reading https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nn-d3d12-id3d12device
