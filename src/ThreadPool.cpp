#include"ThreadPool.h"

ThreadPoll::ThreadPoll(unsigned int size) {
	for (int i = 0; i < size; i++) {
		workers_.emplace_back(std::thread([this]() {
			while (true) {
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(queue_mutex_);
					condition_variable_.wait(lock, [this]() {	//lock�������������ڵȴ�ʱ����������ռ��
						return !tasks_.empty() || stop_;
						});
					if (stop_ && tasks_.empty()) return;
					task = tasks_.front();
					tasks_.pop();
				}
				task();
			}
			}
		));
	}
}

ThreadPoll::~ThreadPoll() {
	{
		std::unique_lock<std::mutex> lock(queue_mutex_);
		stop_ = true;
	}
	condition_variable_.notify_all();
	for (auto& th : workers_) {
		if (th.joinable()) {
			th.join();
		}
	}
}