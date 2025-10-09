#pragma once

#include <string>
#include <vector>

#include "Mesh.h"
#include "Material.h"

struct aiNode;

class Model
{
private:
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
	Node* m_rootNode;
	std::vector<Mesh> m_meshes;
	std::vector<Material> m_materials;

public:
	Model(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const char* fileName);
	~Model();

public:
	const std::vector<Mesh>& GetMeshes() const;
	const std::vector<Material>& GetMaterials() const;
};