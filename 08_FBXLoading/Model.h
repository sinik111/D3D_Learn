#pragma once

#include <string>
#include <vector>
#include <directxtk/SimpleMath.h>

#include "Mesh.h"
#include "Material.h"

struct aiNode;

class Model
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
	std::vector<Mesh> m_meshes;
	std::vector<Material> m_materials;
	Matrix m_world;

public:
	Model(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const char* fileName, const Matrix& world = Matrix::Identity);
	// todo: move constructor
	~Model();

public:
	const std::wstring& GetName() const;
	const std::vector<Mesh>& GetMeshes() const;
	const std::vector<Material>& GetMaterials() const;
	const Matrix& GetWorld() const;
};