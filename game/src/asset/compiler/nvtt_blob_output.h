#pragma once

#include <nvtt/nvtt.h>

#include <core/types.h>

struct NvttBlob : public nvtt::OutputHandler
{
	// 148 is the size of dds dxt10 header
	NvttBlob()
	{
		write_location = 0;
		data = (std::byte*)malloc(148);
		size = 148;
	}

	~NvttBlob()
	{
		delete[] data;
	}

	bool writeData(const void* data, int size) override
	{
		memcpy(this->data + write_location, data, size);
		write_location += size;
		return true;
	}

	void endImage() override
	{
	}

	int write_location;
	std::byte* data;
	int size;

	virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) override
	{
		this->size += size;
		data = (std::byte*)realloc(this->data, this->size);
	}

};