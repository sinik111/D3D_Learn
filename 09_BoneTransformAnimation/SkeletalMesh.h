#pragma once

#include <string>
#include <vector>
#include <directxtk/SimpleMath.h>
#include <memory>
#include <array>

#include "SkeletalMeshSection.h"
#include "Material.h"
#include "Skeleton.h"

struct SkeletalMeshResource;
struct aiNode;

class SkeletalMesh
{
private:
	using Matrix = DirectX::SimpleMath::Matrix;

private:
	// resource
	std::shared_ptr<SkeletalMeshResource> m_resource;

	// instance
	std::wstring m_name;
	Matrix m_world;
	std::vector<Bone> m_skeleton;
	std::array<Matrix, 32> m_skeletonPose;
	size_t m_animationIndex = 0;
	float m_animationProgressTime = 0.0f;

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
	const std::array<Matrix, 32>& GetSkeletonPose() const;

	void SetWorld(const Matrix& world);

public:
	void Update(float deltaTime);
	void PlayAnimation(size_t index);
};