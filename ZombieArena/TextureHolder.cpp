#include "pch.h"
#include "TextureHolder.h"
#include <assert.h>

TextureHolder* TextureHolder::m_s_Instance = nullptr;

TextureHolder::TextureHolder()
{
	assert(m_s_Instance == nullptr);
	m_s_Instance = this;
}

Texture& TextureHolder::GetTexture(string const& filename)
{
	auto& m = m_s_Instance->m_Textures;

	// Iterator for KeyValuePair and and search for it in the file
	auto keyValuePair = m.find(filename);

	// Check for match and return the texture if so
	if (keyValuePair != m.end())
	{
		return keyValuePair->second;
	}
	else
	{	
		// Create new KeyValuePair
		auto& texture = m[filename];
		// And load it
		texture.loadFromFile(filename);
		return texture;
	}
}
