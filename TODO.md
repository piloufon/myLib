Add hint [[likely]] & [[unlikely]] 
# WindowManager
Should get rid off the include.h (include in .cpp and namespace std{/*What's used*/;} in .h)
Add in WndProc case DXGI_STATUS_OCCLUDED & DXGI_STATUS_UNOCCLUDED
# FileManger
Should get rid off the include.h (include in .cpp and namespace std{/*What's used*/;} in .h)\
Should also create static function for easier use\
TODO in the FileManager.h
# Input
Should get rid off the include.h (include in .cpp and namespace std{/*What's used*/;} in .h)
# LogManger
Should create a way to not create log file if NOLOGFILE / NOTEMPFILE / NOPERMFIL with #ifdef (will completely disable the use of them)
# DX12
Add in the init of Factory RegisterOcclusionStatusWindow (from IDXGIFactory2)


*Not really important/urgent -> using IDXGIFactory5::CheckFeatureSupport to check if OS has every function needed. And far after ID3D12Device::CheckFeatureSupport to check what abilities the hardware has (=> has RenderTarget, DepthStencil, ou ShaderResource, CUDA, ...)*
*Also, I need to understand and try to use ID3D12Device::CopyDescriptors for better performance*
