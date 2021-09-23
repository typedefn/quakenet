/*
 * TsQueue.hpp
 *
 *  Created on: Sep 21, 2021
 *      Author: mastakillah
 */

#ifndef SRC_TSQUEUE_HPP_
#define SRC_TSQUEUE_HPP_

#include <deque>
#include <mutex>

template<typename T>
class TsQueue {
public:
	TsQueue() = default;
	TsQueue(const TsQueue<T>&) = delete;
	~TsQueue() {
		clear();
	}
	const T front() {
		std::lock_guard<std::mutex> lock(muxQueue);
		T o = deqQueue.front();
		return o;
	}

	void push_back(const T &item) {
		std::lock_guard<std::mutex> lock(muxQueue);
		deqQueue.emplace_back(std::move(item));
	}

	void push(const T &item) {
		std::lock_guard<std::mutex> lock(muxQueue);
		deqQueue.emplace_back(item);
	}

	void push_front(const T &item) {
		std::lock_guard<std::mutex> lock(muxQueue);
		deqQueue.emplace_front(item);
	}

	bool empty() {
		std::lock_guard<std::mutex> lock(muxQueue);
		bool b = deqQueue.empty();
		return b;
	}


	size_t count() {
		std::lock_guard<std::mutex> lock(muxQueue);
		size_t s = deqQueue.size();
		return s;
	}

	void clear() {
		std::lock_guard<std::mutex> lock(muxQueue);
		deqQueue.clear();
	}

	T pop() {
		std::lock_guard<std::mutex> lock(muxQueue);
		auto t = std::move(deqQueue.front());
		deqQueue.pop_front();
		return t;
	}



protected:
	std::mutex muxQueue;
	std::deque<T> deqQueue;
};
#endif /* SRC_TSQUEUE_HPP_ */
