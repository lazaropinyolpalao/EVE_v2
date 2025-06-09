
#include <functional>

#include "boss.hpp"

Boss::Boss(): stop_{false}{

	//Get number of cores
	unsigned int worker_count = std::thread::hardware_concurrency();
	for (int i = 0; i != worker_count; i++) {

		std::packaged_task<void()> worker([this]() {
			std::function<void()> job;
			while (!stop_) {
				{
					// Lock mutex to avoid that more than one thread can access the jobs queue
					std::unique_lock<std::mutex> lk(queue_mutex_);
					job_condition_.wait(lk, [this] {return stop_ || !jobs_.empty(); });
					if (!jobs_.empty()) {
						//Get the first job available
						job = jobs_.front();
						//Remove the job from the list
						jobs_.pop();
					}
				}
				if (job) {
					job();
					job = nullptr;
				}
			}
		});

		//Add the worker to the worker list
		workers_.push_back(std::thread{ std::move(worker) });
	}

}

Boss::~Boss()	{
	{
		std::unique_lock<std::mutex> lk(queue_mutex_);
		stop_ = true;
	}
	job_condition_.notify_all();
	for (auto& worker : workers_) {
		worker.join();
	}
}



int Boss::get_job_count() {
	// Lock mutex to avoid that more than one thread can access the jobs queue
	std::lock_guard<std::mutex> lock{ queue_mutex_ };
	return (int) jobs_.size();
}

std::mutex* Boss::get_mutex() {
	return &queue_mutex_;
}

//template<typename Function, typename ...Args>
//auto Boss::add(Function&& task, Args && ...args) -> std::future<decltype(task(args...))>
//{
//	std::lock_guard<std::mutex> lock{ queue_mutex_ };
//	using ReturnType = decltype(task(args...));
//	auto t = std::make_shared<std::packaged_task<ReturnType()>>(task);
//	std::future<ReturnType> f = t->get_future();
//
//	jobs_.push([t]() {(*t)(); });
//
//
//	job_available_.notify_one();
//
//	return f;
//}