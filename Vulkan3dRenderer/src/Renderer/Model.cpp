#include "Model.h"

#include "Buffer.h"
#include "Utils.h"
#include "Log.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

// std
#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Descriptors.h"

namespace std
{
	template<>
	struct hash<Vipera::Model::Vertex>
	{
		size_t operator()(Vipera::Model::Vertex const& vertex) const
		{
			size_t seed = 0;
			Vipera::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace Vipera
{
	Model::Model(Device& device, const Builder& builder)
		: m_Device{ device }
	{
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	Model::~Model()
	{
		vkDestroySampler(m_Device.device(), m_TextureSampler, nullptr);
		vkDestroyImageView(m_Device.device(), m_TextureImageView, nullptr);
		vkDestroyImage(m_Device.device(), m_TextureImage, nullptr);
	}

	std::unique_ptr<Model> Model::CreateModelFromFile(Device& device, const std::string& filepath)
	{
		Builder builder{};
		builder.loadModel(filepath);

		VIPERA_CORE_INFO("vertex count: {}", builder.vertices.size())

		return std::make_unique<Model>(device, builder);
	}

	void Model::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		Buffer stagingBuffer{
			m_Device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		vertexBuffer = std::make_unique<Buffer>(
			m_Device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_Device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	void Model::createIndexBuffers(const std::vector<uint32_t>& indices)
	{
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer)
		{
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		Buffer stagingBuffer{
			m_Device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		indexBuffer = std::make_unique<Buffer>(
			m_Device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_Device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
	}

	void Model::draw(VkCommandBuffer commandBuffer)
	{
		if (hasIndexBuffer)
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		else
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	void Model::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer)
		{
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
		//vkCmdBindDescriptorSets(commandBuffer, 1, )
	}

	std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });

		return attributeDescriptions;
	}

	void Model::Builder::loadModel(const std::string& filepath)
	{
		//position, color, normal, uv
		tinyobj::attrib_t attrib;
		//index values 
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
		{
			throw std::runtime_error("Can not load model");
		}

		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				if (index.vertex_index >= 0)
				{
					vertex.position = 
					{
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					vertex.color =
					{
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2]
					};
				}

				if (index.normal_index >= 0)
				{
					vertex.normal =
					{
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}

				if (index.texcoord_index >= 0)
				{
					vertex.uv =
					{
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1] //OBJ specific 0 - bottom, vulkan otherwise
					};
				}

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}

	void Model::createTexture(const std::string& filepath) {
		int texWidth, texHeight, channels;
		stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);

		//VkDeviceSize imageSize = texWidth * texHeight * 4;
		VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

		VkDeviceSize pixelsSize = texWidth * texHeight * 4;

		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		m_Device.createBuffer(pixelsSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);
		void* data;
		vkMapMemory(m_Device.device(), stagingBufferMemory, 0, pixelsSize, 0, &data);
		memcpy(data, pixels, pixelsSize);
		vkUnmapMemory(m_Device.device(), stagingBufferMemory);
		stbi_image_free(pixels);

		VkDeviceMemory textureMemory = VK_NULL_HANDLE;
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width = static_cast<uint32_t>(texWidth);
		imageCreateInfo.extent.height = static_cast<uint32_t>(texHeight);
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.flags = 0;

		m_Device.createImageWithInfo(imageCreateInfo, 0, m_TextureImage, textureMemory);
		m_Device.transitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		m_Device.copyBufferToImage(stagingBuffer, m_TextureImage, texWidth, texHeight, 1);
		m_Device.transitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		vkDestroyBuffer(m_Device.device(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device.device(), stagingBufferMemory, nullptr);
	}

	void Model::createTextureImageView() {
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = m_TextureImage;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_Device.device(), &imageViewCreateInfo, nullptr, &m_TextureImageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}
	}

	void Model::createTextureSampler() {
		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.anisotropyEnable = VK_TRUE;

		samplerCreateInfo.maxAnisotropy = m_Device.properties.limits.maxSamplerAnisotropy;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;

		if (vkCreateSampler(m_Device.device(), &samplerCreateInfo, nullptr, &m_TextureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}


	void Model::CreateTextureFromFile(const std::string &filepath) {
		createTexture(filepath);
		createTextureImageView();
		createTextureSampler();
	}

	void Model::createDescriptorSets(int framesCount, std::unique_ptr<DescriptorPool>& descPool, std::unique_ptr<DescriptorSetLayout>& descLayout) {
		m_DescriptorSet.resize(framesCount);
		for (int i = 0; i < framesCount; i++) {
			VkDescriptorImageInfo descriptorImageInfo{};
			descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			descriptorImageInfo.imageView = m_TextureImageView;
			descriptorImageInfo.sampler = m_TextureSampler;
			DescriptorWriter descriptorWriter(*descLayout, *descPool);
			descriptorWriter.writeImage(0, &descriptorImageInfo).build(m_DescriptorSet[i]);
		}

	 }
}  // namespace 