#pragma once

#include <functional>
#include <deque>

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function);

	void execute();
};