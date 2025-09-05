#pragma once

#include "../Common/WinApp.h"

class SwapChainApp :
	public WinApp
{
public:
	void Initialize() override;
	
private:
	void Update() override;
	void Render() override;
};