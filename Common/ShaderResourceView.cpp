#include "ShaderResourceView.h"

#include <directxtk/WICTextureLoader.h>
#include <directxtk/DDSTextureLoader.h>
#include <DirectXTex.h>
#include <filesystem>
#include <OpenExr/ImfRgbaFile.h>
#include <OpenExr/ImfArray.h>
#include <Imath/ImathBox.h>

void ShaderResourceView::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filePath, TextureType type)
{
	namespace fs = std::filesystem;

	if (type == TextureType::Texture2D)
	{
		auto extension = fs::path(filePath).extension();

		if (extension == ".tga" || extension == ".TGA")
		{
			DirectX::ScratchImage image;
			LoadFromTGAFile(filePath.c_str(), nullptr, image);
			
			CreateShaderResourceView(device.Get(),image.GetImages(), image.GetImageCount(), image.GetMetadata(), &m_shaderResourceView);
		}
        else if (extension == ".exr" || extension == ".EXR")
        {
            // 1. 파일 경로를 std::string으로 변환 (OpenEXR이 std::string/char*를 선호)
            std::string path_s = fs::path(filePath).string();

            int width = 0;
            int height = 0;

            // OpenEXR 배열을 위한 메모리 할당
            Imf::Array2D<Imf::Rgba> pixels;

            // 2. EXR 파일 읽기 (헤더 읽기 및 픽셀 데이터 읽기)
            Imf::RgbaInputFile file(path_s.c_str());
            const Imath::Box2i& dw = file.header().dataWindow();

            width = dw.max.x - dw.min.x + 1;
            height = dw.max.y - dw.min.y + 1;

            // 메모리 할당
            pixels.resizeErase(height, width);

            // 픽셀 읽기
            // frameBuffer 설정: 1은 x-stride, width는 y-stride
            file.setFrameBuffer(&pixels[0][0], 1, width);
            file.readPixels(dw.min.y, dw.max.y);

            // 3. DirectXTex ScratchImage 초기화
            DirectX::ScratchImage image;
            // EXR Rgba는 16비트 Half Float이므로 R16G16B16A16_FLOAT 포맷 사용
            const DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT;

            // ScratchImage 메모리 할당
            image.Initialize2D(format, width, height, 1, 1);

            // ScratchImage의 첫 번째 이미지(서브리소스)에 대한 포인터
            const DirectX::Image* img = image.GetImage(0, 0, 0);

            // 4. OpenEXR 데이터를 ScratchImage 메모리로 복사
            // OpenEXR Rgba 구조체는 R, G, B, A 순서로 4개의 Half(16비트 float)를 가집니다.
            // 이는 R16G16B16A16_FLOAT와 메모리 레이아웃이 일치하므로 직접 복사가 가능합니다.
            // Imf::Rgba는 총 8바이트 (4 * 2바이트 Half)
            size_t rowPitch = width * sizeof(Imf::Rgba);
            size_t slicePitch = height * rowPitch;

            // 픽셀 데이터 복사 (Rgba 배열을 ScratchImage 데이터로 복사)
            memcpy(img->pixels, pixels[0], slicePitch);

            // 5. D3D11 리소스 생성
            // ScratchImage 객체가 이제 유효한 텍스처 데이터를 가지고 있습니다.
            DirectX::CreateShaderResourceView(
                device.Get(),
                image.GetImages(),
                image.GetImageCount(),
                image.GetMetadata(),
                &m_shaderResourceView
            );
        }
        else if (extension == ".dds" || extension == ".DDS")
        {
            DirectX::CreateDDSTextureFromFile(device.Get(), filePath.c_str(), nullptr, &m_shaderResourceView);
        }
        else
		{
			DirectX::CreateWICTextureFromFile(device.Get(), filePath.c_str(), nullptr, &m_shaderResourceView);
		}
	}

	if (type == TextureType::TextureCube)
	{
		DirectX::CreateDDSTextureFromFile(device.Get(), filePath.c_str(), nullptr, &m_shaderResourceView);
	}
}

void ShaderResourceView::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const D3D11_TEXTURE2D_DESC& textureDesc,
	const D3D11_SUBRESOURCE_DATA& subData)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;

	device->CreateTexture2D(&textureDesc, &subData, &texture);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	device->CreateShaderResourceView(texture.Get(), &srvDesc, &m_shaderResourceView);
}

void ShaderResourceView::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture2D, const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc)
{
	device->CreateShaderResourceView(texture2D.Get(), &srvDesc, &m_shaderResourceView);
}

const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& ShaderResourceView::GetShaderResourceView() const
{
	return m_shaderResourceView;
}

ID3D11ShaderResourceView* ShaderResourceView::GetRawShaderResourceView() const
{
	return m_shaderResourceView.Get();
}
