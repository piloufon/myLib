# TODO

## General Improvements
- Add hint `[[likely]]` & `[[unlikely]]` where appropriate.

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

### 🔹 To consider / explore later

 - Feature & capability checks:
   - Use IDXGIFactory5::CheckFeatureSupport to check if the OS supports all required DX12 features.
   - Use ID3D12Device::CheckFeatureSupport to check hardware capabilities (RenderTarget, DepthStencil, ShaderResource, CUDA, etc.).
  - Descriptor optimizations:
    - Understand and try ID3D12Device::CopyDescriptors for better performance when managing descriptor heaps.
  - Sampler
    - Explore ID3D12Device::CreateSampler and D3D12_SAMPLER_DESC/D3D12_STATIC_SAMPLER_DESC
  - Continue reading https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nn-d3d12-id3d12device
