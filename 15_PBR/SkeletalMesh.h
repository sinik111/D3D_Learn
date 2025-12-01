#pragma once

#include <string>
#include <vector>
#include <directxtk/SimpleMath.h>
#include <memory>

#include "../Common/ShaderConstant.h"
#include "../Common/ShaderResourceView.h"
#include "../Common/SkeletonData.h"

class SkeletalMeshData;
class MaterialData;
class AnimationData;

class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class VertexShader;
class PixelShader;
class ShaderResourceView;
class InputLayout;
class SamplerState;

struct aiNode;

class SkeletalMesh
{
private:
	// assets
	std::shared_ptr<SkeletalMeshData> m_skeletalMeshData;
	std::shared_ptr<MaterialData> m_materialData;
	std::shared_ptr<AnimationData> m_animationData;
	std::shared_ptr<SkeletonData> m_skeletonData;

	// resources
	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	std::shared_ptr<ConstantBuffer> m_materialBuffer;
	std::shared_ptr<ConstantBuffer> m_worldTransformBuffer;
	std::shared_ptr<ConstantBuffer> m_bonePoseBuffer;
	std::shared_ptr<ConstantBuffer> m_boneOffsetBuffer;
	std::shared_ptr<VertexShader> m_finalPassVertexShader;
	std::shared_ptr<VertexShader> m_shadowPassVertexShader;
	std::shared_ptr<PixelShader> m_finalPassPixelShader;
	std::shared_ptr<PixelShader> m_shadowPassPixelShader;
	std::vector<TextureSRVs> m_textureSRVs;
	std::shared_ptr<InputLayout> m_inputLayout;
	std::shared_ptr<SamplerState> m_samplerState;

	// instance
	std::vector<MaterialBuffer> m_materialCBs;
	WorldTransformBuffer m_worldTransformCB;
	std::vector<Bone> m_skeleton;
	BoneMatrixArray m_skeletonPose;
	size_t m_animationIndex = 0;
	float m_animationProgressTime = 0.0f;

public:
	SkeletalMesh(const std::wstring& filePath);

public:
	void SetWorld(const DirectX::SimpleMath::Matrix& world);

public:
	void Update(float deltaTime);
	void PlayAnimation(size_t index);
	void Draw(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& deviceContext);
};