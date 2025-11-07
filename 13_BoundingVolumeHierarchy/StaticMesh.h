#pragma once

#include <string>
#include <vector>
#include <directxtk/SimpleMath.h>
#include <DirectXCollision.h>
#include <memory>

#include "StaticMeshSection.h"
#include "Material.h"

struct StaticMeshResource;
struct aiNode;

class StaticMesh
{
private:
	using Matrix = DirectX::SimpleMath::Matrix;

private:
	// resource
	std::shared_ptr<StaticMeshResource> m_resource;

	// instance
	std::wstring m_name;
	Matrix m_world;

public:
	StaticMesh(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::string& fileName, const Matrix& world = Matrix::Identity);

public:
	const std::wstring& GetName() const;
	const std::vector<StaticMeshSection>& GetMeshes() const;
	const std::vector<Material>& GetMaterials() const;
	const Matrix& GetWorld() const;
	DirectX::BoundingBox GetWorldBoundingBox() const;

	void SetWorld(const Matrix& world);
};