#include <texture.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#ifdef RENDER_OPENGL

Texture::Texture() {
	width_ = 0;
	height_ = 0;
	channels_ = 0;
	loaded_ = false;
	texture_id_ = -1;
}

Texture::~Texture() {
	if (loaded_ && texture_id_ != -1) {
		glDeleteTextures(1, &texture_id_);
		loaded_ = false;
	}
}
bool Texture::LoadTexture(std::string src) {
	if (src.empty()) return false;

	//Get img data
	stbi_set_flip_vertically_on_load(true);


	data_ = stbi_load(src.c_str(), &width_, &height_, &channels_, 0);
	if (data_ == nullptr) {
		printf("Error loading texture from file %s\n", src.c_str());
		return false;
	}

	if (channels_ == 1) { format = GL_RED; }
	else if (channels_ == 3) { format = GL_RGB; }
	else if (channels_ == 4) { format = GL_RGBA; }

	//Generate buffer from data with needed values
	glGenTextures(1, &texture_id_);
	glBindTexture(GL_TEXTURE_2D, texture_id_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, data_);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Free old data
	stbi_image_free(data_);

	//Set the file name from the source
	char search_start = '/';
	char search_end = '.';
	size_t init_pos = -1;
	size_t end_pos = -1;

	//Find last ocurrence of the separator
	init_pos = src.rfind(search_start);
	end_pos = src.rfind(search_end);

	if (init_pos != -1) {
		name_ = src.substr(init_pos + 1, end_pos - (init_pos + 1));
	}
	else { name_ = "unnamed"; }

	//Mark as loaded and save the source route
	loaded_ = true;
	src_ = src;


	return true;
}

bool Texture::LoadTextureNoInit(std::string src) {
	if (src.empty()) return false;

	//Get img data
	stbi_set_flip_vertically_on_load(true);

	data_ = stbi_load(src.c_str(), &width_, &height_, &channels_, 0);
	if (data_ == nullptr) {
		printf("Error loading texture from file %s\n", src.c_str());
		return false;
	}

	if (channels_ == 1) { format = GL_RED; }
	else if (channels_ == 3) { format = GL_RGB; }
	else if (channels_ == 4) { format = GL_RGBA; }
	src_ = src;

	//Set the file name from the source
	char search_start = '/';
	char search_end = '.';
	size_t init_pos = -1;
	size_t end_pos = -1;

	//Find last ocurrence of the separator
	init_pos = src.rfind(search_start);
	end_pos = src.rfind(search_end);

	if (init_pos != -1) {
		name_ = src.substr(init_pos + 1, end_pos - (init_pos + 1));
	}
	else { name_ = "unnamed"; }

	return true;
}

bool Texture::InitTexture(){
	if (!loaded_) {
		//Generate buffer from data with needed values
		glGenTextures(1, &texture_id_);
		glBindTexture(GL_TEXTURE_2D, texture_id_);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, data_);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		//Free old data
		stbi_image_free(data_);

		//Mark as loaded and save the source route
		loaded_ = true;

		return true;
	}
	return false;
}



bool Texture::LoadCubemapTexture(std::string filepath) {
	if (filepath.empty()) {
		return false;
	}

	glGenTextures(1, &texture_id_);
	glBindTexture(GL_TEXTURE_2D, texture_id_);

	stbi_uc* data = nullptr;
	data = stbi_load(filepath.c_str(), &width_, &height_, &channels_, 0);
	if (data != nullptr) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else { printf("Cubemap texture failed to load at path: %s\n", filepath.c_str()); }

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);

	src_ = filepath;

	loaded_ = true;
	return true;
}

bool Texture::FreeTexture() {
	if (loaded_ && texture_id_ != -1) {
		glDeleteTextures(1, &texture_id_);
		loaded_ = false;
	}
return true;
}
#endif

#ifdef RENDER_DIRECTX11
Texture::Texture() {
	width_ = 0;
	height_ = 0;
	channels_ = 0;
	loaded_ = false;
}

Texture::~Texture() {
	if (loaded_ && texture_ != nullptr) {
		//texture_->Release();
		loaded_ = false;
	}
}

bool Texture::LoadTexture(ID3D11Device* dev, std::string src) {
	if (src.empty()) return false;

	//Get img data
	stbi_set_flip_vertically_on_load(true);

	desired_channels_ = 4;
	unsigned char* data_ = stbi_load(src.c_str(), &width_, &height_, &channels_, desired_channels_);
	if (data_ == nullptr) {
		printf("Error loading texture from file %s\n ", src.c_str());
		return false;
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width_;
	desc.Height = height_;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	//desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA subRes = {};
	subRes.pSysMem = data_;
	subRes.SysMemPitch = width_ * desired_channels_;
	subRes.SysMemSlicePitch = 0;

	HRESULT hr= dev->CreateTexture2D(&desc, &subRes, texture_.GetAddressOf());
	if (FAILED(hr)){printf("Texture not created: %#010x\n", hr);}

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = dev->CreateSamplerState(&sampDesc, &sampler_);
	if (FAILED(hr)) { printf("Sampler not created: %#010x\n", hr); }

	// Create texture view 
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = dev->CreateShaderResourceView(texture_.Get(), &srvDesc, shader_resource_view_.GetAddressOf());
	if (FAILED(hr))
	{
		printf("Shader resource view not created\n");
		printf("%#010x\n", hr);
	}

	//Free old data
	stbi_image_free(data_);

	//Mark as loaded and save the source route
	loaded_ = true;
	src_ = src;

	return true;
}

bool Texture::LoadCubemapTexture(ID3D11Device* dev, std::string filepath) {
	if (filepath.empty()) {
		return false;
	}


	stbi_uc* data = nullptr;
	desired_channels_ = 4;
	data = stbi_load(filepath.c_str(), &width_, &height_, &channels_, desired_channels_);

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width_;
	desc.Height = height_;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA subRes = {};
	subRes.pSysMem = data;
	subRes.SysMemPitch = width_ * desired_channels_;
	subRes.SysMemSlicePitch = 0;

	HRESULT hr = dev->CreateTexture2D(&desc, &subRes, texture_.GetAddressOf());
	if (FAILED(hr))
	{
		printf("Texture not created\n");
		printf("%#010x\n", hr);
	}

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));


	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = dev->CreateShaderResourceView(texture_.Get(), &srvDesc, shader_resource_view_.GetAddressOf());
	if (FAILED(hr))
	{
		printf("Shader resource view not created\n");
		printf("%#010x\n", hr);
	}

	stbi_image_free(data);

	src_ = filepath;

	loaded_ = true;
	return true;
	}

bool Texture::FreeTexture() {
	if (loaded_ && texture_ != nullptr) {
		texture_->Release();
		loaded_ = false;
	}
	return true;
}
#endif 