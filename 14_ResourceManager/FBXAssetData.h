#pragma once

#include <memory>
#include <string>

#include "AssetData.h"

class StaticMeshData;
class MaterialData;

enum class FBXAssetKind
{
	Static,
	Rigid,
	Skinning
};

class FBXAssetData :
	public AssetData
{
private:
	FBXAssetKind m_kind;
	std::shared_ptr<StaticMeshData> m_staticMesh;
	std::shared_ptr<MaterialData> m_material;

public:
	void Create(FBXAssetKind kind, const std::wstring& filePath);

	std::shared_ptr<StaticMeshData> GetStaticMeshData() const;
	std::shared_ptr<MaterialData> GetMaterialData() const;

private:
	void LoadStaticMesh(const std::wstring& filePath);
};