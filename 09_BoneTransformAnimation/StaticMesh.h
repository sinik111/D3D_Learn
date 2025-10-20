#pragma once

#include <string>
#include <vector>
#include <directxtk/SimpleMath.h>

#include "StaticMeshSection.h"
#include "Material.h"

struct aiNode;

class StaticMesh
{
private:
	using Matrix = DirectX::SimpleMath::Matrix;

	class Node
	{
	private:
		std::string m_name;
		Node* m_parent;
		std::vector<Node*> m_children;
		std::vector<unsigned int> m_meshes;
		// transform

	public:
		Node(const char* name, Node* parent, unsigned int numChildren, aiNode** children,
			unsigned int numMeshes, unsigned int* meshes);

		~Node();
	};

private:
	std::wstring m_name;
	Node* m_rootNode;
	std::vector<StaticMeshSection> m_meshes;
	std::vector<Material> m_materials;
	Matrix m_world;

public:
	StaticMesh(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const char* fileName, const Matrix& world = Matrix::Identity);
	// 이동생성자 필요한데 vector::reserve로 일단 터지진 않음
	~StaticMesh();

public:
	const std::wstring& GetName() const;
	const std::vector<StaticMeshSection>& GetMeshes() const;
	const std::vector<Material>& GetMaterials() const;
	const Matrix& GetWorld() const;

	void SetWorld(const Matrix& world);
};