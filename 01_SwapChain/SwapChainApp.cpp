#include "SwapChainApp.h"

#include <directxtk/SimpleMath.h>

using DirectX::SimpleMath::Color;

void SwapChainApp::Initialize()
{
	WinApp::Initialize();
}

void SwapChainApp::OnRender()
{
	m_graphicsDevice.BeginDraw({ 0.9f, 1.0f, 0.9f, 1.0f });

	m_graphicsDevice.EndDraw();
}