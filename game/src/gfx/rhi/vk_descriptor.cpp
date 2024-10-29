#include "pch.h"

#include "vk_descriptor.h"
#include "vk_helper.h"

void DescriptorLayoutBuilder::add_binding(uint32_t binding, VkDescriptorType type)
{
	VkDescriptorSetLayoutBinding b{};
	b.binding = binding;
	b.descriptorType = type;
	b.descriptorCount = 1;

	bindings.push_back(b);
}

void DescriptorLayoutBuilder::clear()
{
	bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags shader_stages, void* pNext /*= nullptr*/, VkDescriptorSetLayoutCreateFlags flags /*= 0*/)
{
	for (auto& b : bindings)
	{
		b.stageFlags = shader_stages;
	}

	VkDescriptorSetLayoutCreateInfo layout_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layout_info.pNext = pNext;

	layout_info.bindingCount = (uint32_t)bindings.size();
	layout_info.pBindings = bindings.data();
	layout_info.flags = flags;

	VkDescriptorSetLayout layout;
	VULKAN_CHECK(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &layout), "Failed to create Descriptor Set Layout");

	return layout;
}

void DescriptorAllocator::init_pool(VkDevice device, uint32_t max_sets, std::vector<PoolSizeRatio> pool_sizes)
{
	std::vector<VkDescriptorPoolSize> sizes;
	for (auto& p : pool_sizes)
	{
		VkDescriptorPoolSize s{};
		s.type = p.type;
		s.descriptorCount = (uint32_t)(max_sets * p.ratio);
		sizes.push_back(s);
	}

	VkDescriptorPoolCreateInfo pool_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };

	pool_info.poolSizeCount = (uint32_t)sizes.size();
	pool_info.pPoolSizes = sizes.data();
	pool_info.maxSets = max_sets;

	VULKAN_CHECK(vkCreateDescriptorPool(device, &pool_info, nullptr, &pool), "Failed to create Descriptor Pool");
}

void DescriptorAllocator::clear_descriptor_pool(VkDevice device)
{
	vkResetDescriptorPool(device, pool, 0);
}

void DescriptorAllocator::destroy_pool(VkDevice device)
{
	vkDestroyDescriptorPool(device, pool, nullptr);
}

VkDescriptorSet DescriptorAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout)
{
	VkDescriptorSetAllocateInfo alloc_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };

	alloc_info.descriptorPool = pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &layout;

	VkDescriptorSet set;
	VULKAN_CHECK(vkAllocateDescriptorSets(device, &alloc_info, &set), "Failed to allocate Descriptor Set");

	return set;
}


// writer

void DescriptorWriter::clear()
{
	image_infos.clear();
	buffer_infos.clear();
	writes.clear();
}

void DescriptorWriter::update_set(VkDevice device, VkDescriptorSet set)
{
	for (auto& write : writes)
	{
		write.dstSet = set;
	}

	vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}

void DescriptorWriter::write_image(int binding, VkImageView iv, VkSampler sampler, VkImageLayout layout, VkDescriptorType type)
{
	VkWriteDescriptorSet w { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

	w.dstBinding = binding;
	w.dstSet = VK_NULL_HANDLE;

	w.descriptorCount = 1;
	w.descriptorType = type;

	w.pImageInfo = &image_infos.emplace_back(VkDescriptorImageInfo{ sampler, iv, layout });

	writes.push_back(w);
}

void DescriptorWriter::write_buffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type)
{
	VkWriteDescriptorSet w{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

	w.dstBinding = binding;
	w.dstSet = VK_NULL_HANDLE;

	w.descriptorCount = 1;
	w.descriptorType = type;

	w.pBufferInfo = &buffer_infos.emplace_back(VkDescriptorBufferInfo{ buffer, offset, size });

	writes.push_back(w);
}
