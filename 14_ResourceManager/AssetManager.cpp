#include "AssetManager.h"

AssetManager& AssetManager::Get()
{
	static AssetManager s_instance;

	return s_instance;
}
