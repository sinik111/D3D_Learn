#pragma once

//#include <string>
//#include <vector>
//#include <directxtk/SimpleMath.h>
//
//#include "StaticMeshSection.h"
//#include "Material.h"
//
//struct aiNode;
//
//class StaticMesh
//{
//private:
//	using Matrix = DirectX::SimpleMath::Matrix;
//
//private:
//	std::wstring m_name;
//	std::vector<StaticMeshSection> m_meshes;
//	std::vector<Material> m_materials;
//	Matrix m_world;
//
//public:
//	StaticMesh(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::string& fileName, const Matrix& world = Matrix::Identity);
//
//public:
//	const std::wstring& GetName() const;
//	const std::vector<StaticMeshSection>& GetMeshes() const;
//	const std::vector<Material>& GetMaterials() const;
//	const Matrix& GetWorld() const;
//
//	void SetWorld(const Matrix& world);
//};

#include <string>
#include <memory>

#include "../Common/StaticMeshData.h"
#include "../Common/MaterialData.h"

struct aiNode;

class StaticMesh
{
private:
	std::shared_ptr<StaticMeshData> m_staticMeshData;
	std::shared_ptr<MaterialData> m_materialData;

public:
	StaticMesh(const std::wstring& filePath);

public:
	void Draw();
};