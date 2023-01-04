#pragma once
#include <iostream>
#include <queue>
#include <mutex>
#include <map>
using namespace std;

#ifdef SAFEQUEUE
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

template<typename T>
class SafeQueue {
public:
	SafeQueue() {}

	~SafeQueue() {}

	void InitQueue()
	{
		work = 1;
	}


	void EnQueue(T value)
	{
		lock_guard<mutex> lock(mt);
		if (work) {
			q.push(value);
			cv.notify_one();
		}
	}

	void Print()
	{
		lock_guard<mutex> lock(mt);
		queue<T> copy_queue = q;

		while (!q.empty())
		{
			copy_queue.push(q.front());
			q.pop();
		}
		while (!copy_queue.empty())
		{
			std::cout << copy_queue.front() << std::endl;
			copy_queue.pop();
		}
	}

	int DeQueue(T& value)
	{
		int ret = 0;
		//ռ�ÿռ����lock_guard ����һ������Ը���һ�㣬���������������ʹ�����������
		unique_lock<mutex> lock(mt);
		//�ڶ������� lambda���ʽ��false������ ������
		cv.wait(lock, [this] {return !work || !q.empty(); });
		if (!q.empty()) {
			value = q.front();
			q.pop();
			ret = 1;
		}

		return ret;
	}

	void setWork(int work)
	{
		lock_guard<mutex> lock(mt);
		this->work = work;
	}

	void Clear()
	{
		lock_guard<mutex> lock(mt);
		while (!q.empty())
		{
			q.clear();
		}
	}

	int Find(T value)
	{
		return nullptr;
	}

private:
	mutex mt;
	condition_variable cv;

	queue<T> q;
	//�Ƿ����ı�� 1 ������ 0������������ ������
	int work;
};

#endif //SAFEQUEUE

template<typename T>
class CQueueManager
{
public:
	CQueueManager(size_t max = 1000)
	{
		_count = max;
	}
	~CQueueManager() {}    
	
	void EnQueue(const T& value)
	{
		lock_guard<mutex> lock(mt);
		if (q.size() > _count)
		{
			q.pop();
		}
		q.push(value);
	}

	int DeQueue(T& value)
	{
		int ret = 0;
		//ռ�ÿռ����lock_guard ����һ������Ը���һ�㣬���������������ʹ�����������
		unique_lock<mutex> lock(mt);
		//�ڶ������� lambda���ʽ��false������ ������
		if (!q.empty()) {
			value = q.front();
			q.pop();
			ret = 1;
		}

		return ret;
	}
	int Count()
	{
		lock_guard<mutex> lock(mt);
		return q.size();
	}
	void Clear()
	{
		lock_guard<mutex> lock(mt);
		while (!q.empty())
		{
			queue<T> empty;
			swap(empty, q);
		}
	}
private:
	queue<T> q;
	//�����int����
	/*
	1>e:\project\irvms\irvms-dag\irvms-dag\datamanager.h(117): note: ���� �� ģ�� ��Ա���� "void CQueueManager<CSocketData>::EnQueue(const T &)" ʱ
1>        with
1>        [
1>            T=CSocketData
1>        ]
1>e:\project\irvms\irvms-dag\irvms-dag\socket.cpp(621): note: �μ������ڱ���ĺ��� ģ�� ʵ������void CQueueManager<CSocketData>::EnQueue(const T &)��������
1>        with
1>        [
1>            T=CSocketData
1>        ]
1>e:\project\irvms\irvms-dag\irvms-dag\socket.h(142): note: �μ������ڱ���� �� ģ�� ʵ���� "CQueueManager<CSocketData>" ������
	*/
	size_t _count;
	mutex mt;
};

template<typename K, typename V>
class CMapManager
{
public:
	//update,if no exist, ignore
	void Update(const K& key, const V& value)
	{
		lock_guard<mutex> lock(mt);
		typename map<K, V>::iterator it = q.find(key);
		if (it != q.end())
		{
			it->second = value;
		}
	}
	//add+update
	void Set(const K& key, const V& value)
	{
		lock_guard<mutex> lock(mt);
		typename map<K, V>::iterator it = q.find(key);
		if (it == q.end())
		{
			q.insert(make_pair(key, value));
		}
		else
		{
			it->second = value;
		}
	}

	int Get(const K& key, V& value)
	{
		lock_guard<mutex> lock(mt);
		typename map<K, V>::iterator it = q.find(key);
		if (it != q.end())
		{
			value = it->second;
			return 0;
		}
		return -1;
	}
	map<K, V> Get()
	{
		lock_guard<mutex> lock(mt);
		return q;
	}

	void Erase(const K& key)
	{
		lock_guard<mutex> lock(mt);
		typename map<K, V>::iterator it = q.find(key);
		if (it != q.end())
		{
			q.erase(it);
		}
	}
	int Count()
	{
		lock_guard<mutex> lock(mt);
		return q.size();
	}
	void Clear()
	{
		lock_guard<mutex> lock(mt);
		while (q.size() > 0)
		{
			q.clear();
		}
	}
private:
	map<K, V> q;
	mutex mt;
};
