#pragma once

#include <memory>
#include <string>
#include <directxtk/SimpleMath.h>

class ShaderResourceView;
struct Material;
enum class MaterialKey : unsigned long long;

namespace MaterialHelper
{
	constexpr unsigned char WHITE_DATA[4]{ 255, 255, 255, 255 };
	constexpr unsigned char BLACK_DATA[4]{ 0, 0, 0, 0 };
	constexpr unsigned char FLAT_DATA[4]{ 128, 128, 255, 255 };

	void SetupTextureSRV(std::shared_ptr<ShaderResourceView>& srv, const Material& material, MaterialKey key,
		const std::wstring& dummyName, const unsigned char colorData[4]);
	void SetupMaterialVector(DirectX::SimpleMath::Vector4& v, const Material& material, MaterialKey key);
	void SetupMaterialScalar(float& f, const Material& material, MaterialKey key);
}