#ifndef __BOSS_HPP__
#define __BOSS_HPP__	1

#include <queue>
#include <functional>
#include <future>
#include <iostream>
#include <condition_variable>

/**
 * @brief	Manages the multithread management of tasks
 */
class Boss {

public:

	Boss();

	~Boss();

	/**
	* @brief Adds a task to the job list
	*
	* @param task Task to execute
	* 
	* @return std::future<T> Future result of the task
	*/
	template<typename T>
	auto add(std::function<T()> task) -> std::future<T>;

	/**
	 * @brief Returns the number of jobs remaining on the list
	 * 
	 * @return int Number of jobs remaining
	 */
	int get_job_count();

	/**
	 * @brief Get the mutex used to lock thread management
	 * 
	 * @return std::mutex* Pointer to the mutex object
	 */
	std::mutex* get_mutex();	

private:

	/**
	 * @brief Condition used to lock threads until a condition is met
	 */
	std::condition_variable job_condition_;

	/**
	 * @brief Mutex used alongside the condition to prevent data races on multithred management
	 */
	std::mutex queue_mutex_;

	/**
	 * @brief Vector of the threads available
	 */
	std::vector<std::thread> workers_;

	/**
	 * @brief Queue of jobs remaining
	 */
	std::queue<std::function<void()>> jobs_;

	bool stop_;

};

template<typename T>
auto Boss::add(std::function<T()> task) -> std::future<T> {
	// Lock mutex to avoid that more than one thread can access the jobs queue
	//std::lock_guard<std::mutex> lock{ queue_mutex_ };

	auto t = std::make_shared<std::packaged_task<T()>>(task);
	std::future<T> f = t->get_future();

	std::unique_lock<std::mutex> lk(queue_mutex_);

	// As we receive a shared pointer to a function pointer, we take it out of the shared
	// pointer in order to call the function in the packaged task
	jobs_.push([t]() {
		(*t)();
		});

	lk.unlock();
	job_condition_.notify_one();

	return f;
}

#endif //__BOSS_HPP__