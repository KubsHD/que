#pragma once

#include <core/types.h>

#include <vulkan/vulkan.h>
#include <span>

struct DescriptorLayoutBuilder {
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	void add_binding(uint32_t binding, VkDescriptorType type);
	void clear();

	VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shader_stages, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
};

struct DescriptorAllocator {
	struct PoolSizeRatio {
		VkDescriptorType type;
		float ratio;
	};

	VkDescriptorPool pool;

	void init_pool(VkDevice device, uint32_t max_sets, std::vector<PoolSizeRatio> pool_sizes);
	void clear_descriptor_pool(VkDevice device);
	void destroy_pool(VkDevice device);

	VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
	void free(VkDevice device, VkDescriptorSet set);
};

struct DescriptorWriter {
	std::deque<VkDescriptorImageInfo> image_infos;
	std::deque<VkDescriptorBufferInfo> buffer_infos;
	std::vector<VkWriteDescriptorSet> writes;

	void write_image(int binding, VkImageView iv, VkSampler sampler, VkImageLayout layout, VkDescriptorType);
	void write_buffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);


	void clear();
	void update_set(VkDevice device, VkDescriptorSet set);
};