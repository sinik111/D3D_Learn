#pragma once

#include <string>
#include <vector>
#include <directxtk/SimpleMath.h>
#include <memory>

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
	BoneMatrixArray m_skeletonPose;
	size_t m_animationIndex = 0;
	float m_animationProgressTime = 0.0f;
	bool m_isRigid = false;

public:
	SkeletalMesh(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::string& fileName, const Matrix& world = Matrix::Identity);

public:
	const std::wstring& GetName() const;
	const std::vector<SkeletalMeshSection>& GetMeshes() const;
	const std::vector<Material>& GetMaterials() const;
	const Matrix& GetWorld() const;
	const BoneMatrixArray& GetSkeletonPose() const;
	const BoneMatrixArray& GetBoneOffsets() const;
	bool IsRigid() const;

	void SetWorld(const Matrix& world);

public:
	void Update(float deltaTime);
	void PlayAnimation(size_t index);
};