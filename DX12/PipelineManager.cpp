#include "PipelineManager.h"

namespace DX12 {
    bool PipelineManager::InitializePipelineManager(ComPtr<ID3D12Device10> device, const std::string& path) {
        if (!device) {
            LOG_ERROR(L"PipelineManager - InitializePipelineManager", L"ComPtr device is a nullptr");
            return false;
        }
        
        this->device = device;
        
        if (!path.empty()) shaderManager = ShaderManager(path);
        else shaderManager = ShaderManager();

        return true;
    }
    void PipelineManager::Shutdown() {
        for (auto& [name, pos] : PSOs) {
            pos.Reset();
        }
        
        rootSignature.Reset();

        device.Reset();
    }

    bool PipelineManager::SetRootSignature(const std::string& name) {
        if (!shaderManager.LoadShader(name)) [[unlikely]] {
            LOG_WARNING(L"PipelineManager - SetRootSignature", L"Couldn't load the shader");
            return false;
        }

        if (FAILED(device->CreateRootSignature(0, shaderManager.GetShader(name).data(), shaderManager.GetShader(name).size(), IID_PPV_ARGS(&rootSignature)))) [[unlikely]] {
            LOG_WARNING(L"PipelineManager - SetRootSignature", L"Couldn't execute CreateRootSignature");
            return false;
        }

        return true;
    }
    bool PipelineManager::CreateMaterialPSO(const std::string& nameMaterial, const std::string& vs, const std::string& ps) {
        if (!rootSignature) {
            LOG_WARNING(L"PipelineManager - CreateMaterialPSO", L"Should have a valid rootSignature");
            return false;
        }
        const auto& vertexShader = shaderManager.GetShader(vs);
        const auto& pixelShader = shaderManager.GetShader(ps);
        
        D3D12_INPUT_ELEMENT_DESC vertexLayout[] = {
            { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "Color", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        
        D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsod{  // TODO : Better understanding bc I forgot a lot and jsut copy/paste from a previous project
          .pRootSignature = rootSignature.Get(),
          .VS{
              .pShaderBytecode = vertexShader.data(),
              .BytecodeLength = vertexShader.size()
          },
          .PS{
              .pShaderBytecode = pixelShader.data(),
              .BytecodeLength = pixelShader.size()
          },
          .DS{
              .pShaderBytecode = nullptr,
              .BytecodeLength = 0
          },
          .HS{
              .pShaderBytecode = nullptr,
              .BytecodeLength = 0
          },
          .GS{
              .pShaderBytecode = nullptr,
              .BytecodeLength = 0
          },
          .StreamOutput{
              .pSODeclaration = nullptr,
              .NumEntries = 0,
              .pBufferStrides = nullptr,
              .NumStrides = 0,
              .RasterizedStream = 0
          },
          .BlendState{
              .AlphaToCoverageEnable = FALSE,
              .IndependentBlendEnable = FALSE,
              .RenderTarget = {{
                  .BlendEnable = FALSE,
                  .LogicOpEnable = FALSE,
                  .SrcBlend = D3D12_BLEND_ONE,
                  .DestBlend = D3D12_BLEND_ZERO,
                  .BlendOp = D3D12_BLEND_OP_ADD,
                  .SrcBlendAlpha = D3D12_BLEND_ZERO,
                  .DestBlendAlpha = D3D12_BLEND_ZERO,
                  .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                  .LogicOp = D3D12_LOGIC_OP_NOOP,
                  .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL
              }}
          },
          .SampleMask = 0xFFFFFFFF,
          .RasterizerState{
              .FillMode = D3D12_FILL_MODE_SOLID,
              .CullMode = D3D12_CULL_MODE_BACK,
              .FrontCounterClockwise = FALSE,
              .DepthBias = 0,
              .DepthBiasClamp = 0.0f,
              .SlopeScaledDepthBias = 0.0f,
              .DepthClipEnable = FALSE,
              .MultisampleEnable = FALSE,
              .AntialiasedLineEnable = FALSE
          },
          .DepthStencilState{
              .DepthEnable = FALSE,
              .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
              .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
              .StencilEnable = FALSE,
              .FrontFace{
                  .StencilFailOp = D3D12_STENCIL_OP_KEEP,
                  .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
                  .StencilPassOp = D3D12_STENCIL_OP_KEEP
              }
          },
          .InputLayout{
              .pInputElementDescs = vertexLayout,
              .NumElements = _countof(vertexLayout)
          },
          .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
          .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
          .NumRenderTargets = 1,
          .RTVFormats = { DXGI_FORMAT_R8G8B8A8_UNORM },
          .DSVFormat = DXGI_FORMAT_UNKNOWN,
          .SampleDesc{
              .Count = 1,
              .Quality = 0
          },
          .NodeMask = 0,
          .CachedPSO{
              .pCachedBlob = nullptr,
              .CachedBlobSizeInBytes = 0
          },
          .Flags = D3D12_PIPELINE_STATE_FLAG_NONE
        };
        

        if (FAILED(device->CreateGraphicsPipelineState(&gfxPsod, IID_PPV_ARGS(&PSOs[nameMaterial])))) [[unlikely]] {
            LOG_ERROR(L"PipelineManager - CreateMaterialPSO", L"Couldn't execute CreateGraphicsPipelineState");
        }

        return true;
    }
    
    void PipelineManager::BindPSO(ComPtr<ID3D12GraphicsCommandList10>& cmdList, const std::string& nameMaterial) {
        if (!PSOs.contains(nameMaterial)) {
            LOG_ERROR("PipelineManager - BindPSO", "POSss doesn't contains the materialName : " + nameMaterial);
            return;
        }
        
        cmdList->SetPipelineState(PSOs[nameMaterial].Get());
    }
    void PipelineManager::BindRootSignature(ComPtr<ID3D12GraphicsCommandList10>& cmdList) {
        if (!rootSignature) {
            LOG_WARNING(L"PipelineManager - BindRootSignature", L"Should have a valid rootSignature");
            return;
        }

        cmdList->SetGraphicsRootSignature(rootSignature.Get());
    }
}
