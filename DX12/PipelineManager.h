#pragma once
#include "include.h"
#include "ShaderManager.h"


// TOOD : Multiple RooSignatur and not only one
namespace DX12 {


	class PipelineManager {
	public:
		bool InitializePipelineManager(ComPtr<ID3D12Device10> device, const std::string& path = std::string());
		void Shutdown();

		bool SetRootSignature(const std::string& name);
		bool CreateMaterialPSO(const std::string& nameMaterial, const std::string& vs, const std::string& ps);

		void BindPSO(ComPtr<ID3D12GraphicsCommandList10>& cmdList, const std::string& nameMaterial);
		void BindRootSignature(ComPtr<ID3D12GraphicsCommandList10>& cmdList);

		ShaderManager& GetShaderManager() { return shaderManager; }
	private:
		ShaderManager shaderManager;
		ComPtr<ID3D12Device10> device;


		ComPtr<ID3D12RootSignature> rootSignature;
		std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> PSOs;
	};
}

