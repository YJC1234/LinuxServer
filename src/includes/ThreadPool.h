#pragma once
#include <condition_variable>  // NOLINT
#include <functional>
#include <future>  // NOLINT
#include <memory>
#include <mutex>  // NOLINT
#include <queue>
#include <thread>  // NOLINT
#include <utility>
#include <vector>
#include<type_traits>
#include "common.h"

class ThreadPool {
public:
	DISALLOW_COPY_AND_MOVE(ThreadPool);
	ThreadPool(unsigned int size = 10);
	~ThreadPool();

	//添加任务到队列中参数：F(Args...),返回:future<typename invoke_result<F,Args...>::type>
	template<class F, class ...Args>
	auto Add(F&& f, Args&& ...args);

private:
	std::vector<std::thread> workers_;
	std::queue<std::function<void()>> tasks_;
	std::mutex queue_mutex_;
	std::condition_variable condition_variable_;
	std::atomic<bool> stop_{false};
};

//不能放在cpp中，c++模板不能支持分离编译
template<class F, class ...Args>
auto ThreadPool::Add(F&& f, Args&& ...args) {
	using return_type = typename std::invoke_result_t<F, Args...>;

	auto task =
		std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

	std::future<return_type> res = task->get_future();
	{//上锁
		std::unique_lock<mutex> lock(queue_mutex_);
		if (stop_) {
			throw std::runtime_error("enqueue on stopped ThreadPool");
		}
		tasks_.emplace_back([task]() {(*task)(); });
	}
	condition_variable_.notify_one();
	return res;
}