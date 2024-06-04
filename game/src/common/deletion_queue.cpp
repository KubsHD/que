#include "pch.h"

#include "deletion_queue.h"

void DeletionQueue::push_function(std::function<void()>&& function)
{
	deletors.push_back(function);
}

void DeletionQueue::execute()
{
	for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
		(*it)();
	}



	deletors.clear();
}
