#pragma once

#include <string>
#include <vector>
#include <directxtk/SimpleMath.h>
#include <memory>
#include <array>

#include "SkeletalMeshSection.h"
#include "Material.h"
#include "Bone.h"
#include "Animation.h"

class SkeletalMesh
{
private:
	using Matrix = DirectX::SimpleMath::Matrix;

private:
	// resource
	std::shared_ptr<std::vector<SkeletalMeshSection>> m_meshes;
	std::shared_ptr<std::vector<Material>> m_materials;
	std::shared_ptr<std::vector<Animation>> m_animations;
	std::shared_ptr<std::vector<BoneInfo>> m_boneInfos;

	// instance
	std::wstring m_name;
	Matrix m_world;
	std::vector<Bone> m_bones;
	std::array<Matrix, 128> m_models;
	int m_animationIndex = 0;
	float m_animationTimer = 0.0f;

public:
	SkeletalMesh(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const char* fileName, const Matrix& world = Matrix::Identity);
	SkeletalMesh(const SkeletalMesh& other) = default;
	SkeletalMesh(const SkeletalMesh& other, const Matrix& world);
	SkeletalMesh& operator=(const SkeletalMesh& other) = default;
	SkeletalMesh(SkeletalMesh&& other) noexcept;
	SkeletalMesh& operator=(SkeletalMesh&& other) noexcept;

public:
	const std::wstring& GetName() const;
	const std::vector<SkeletalMeshSection>& GetMeshes() const;
	const std::vector<Material>& GetMaterials() const;
	const Matrix& GetWorld() const;
	const std::array<Matrix, 128>& GetModels() const;

	void SetWorld(const Matrix& world);
};