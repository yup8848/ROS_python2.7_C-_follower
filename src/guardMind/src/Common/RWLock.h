#pragma once
#include <cstdlib>
#include <cassert>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include "RWRaii.h"
#include <iostream>

using namespace std;
using namespace std::chrono;
using namespace gyd;

#if 0
/*
 * atomic实现读写资源锁,独占写,共享读,禁止复制构造函数和'='赋值操作符
 * WRITE_FIRST为true时为写优先模式,如果有线程等待读取(m_writeWaitCount>0)则等待,优先让写线程先获取锁
 * 允许嵌套加锁
 * readLock/Unlock 实现共享的读取加/解锁，线程数不限
 * writeLock/Unlock 实现独占的写入加/解锁,同时只允许一个线程写入，
 * 当有线程在读取时，写入线程阻塞，当写入线程执行时，所有的读取线程都被阻塞。
 */
class RWLock {
#define WRITE_LOCK_STATUS -1
#define FREE_STATUS 0
private:
	/* 初始为0的线程id */
	static const  std::thread::id NULL_THEAD;
	const bool WRITE_FIRST;
	/* 用于判断当前是否是写线程 */
	thread::id m_write_thread_id;
	/* 资源锁计数器,类型为int的原子成员变量,-1为写状态，0为自由状态,>0为共享读取状态 */
	atomic_int m_lockCount;
	/* 等待写线程计数器,类型为unsigned int的原子成员变量*/
	atomic_uint m_writeWaitCount;
public:
	// 禁止复制构造函数
	RWLock(const RWLock&) = delete;
	// 禁止对象赋值操作符
	RWLock& operator=(const RWLock&) = delete;
	//RWLock& operator=(const RWLock&) volatile = delete;
	RWLock(bool writeFirst = false);;//默认为读优先模式
	virtual ~RWLock() = default;
	int readLock();
	int readUnlock();
	int writeLock();
	int writeUnlock();
	// 将读取锁的申请和释放动作封装为raii对象，自动完成加锁和解锁管理
	raii read_guard()const noexcept {
		return make_raii(*this, &RWLock::readUnlock, &RWLock::readLock);
	}
	// 将写入锁的申请和释放动作封装为raii对象，自动完成加锁和解锁管理
	raii write_guard()noexcept {
		return make_raii(*this, &RWLock::writeUnlock, &RWLock::writeLock);
	}
	//other class defineition code....
	static auto make_guard(RWLock &lock)->decltype(make_raii(lock, &RWLock::readUnlock, &RWLock::readLock, true)) {
		return make_raii(lock, &RWLock::readUnlock, &RWLock::readLock, true);
		
	//这里auto xxx -> xxx 的句法使用用了C++11的"追踪返回类型"特性，将返回类型后置，
	//使用decltype关键字推导出返回类型
	}
};
#endif


#include <iostream>

#ifdef _WIN32
# include <Windows.h>
#else
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#endif
using namespace std;

/*
 读写锁允许当前的多个读用户访问保护资源，但只允许一个写读者访问保护资源
*/

//-----------------------------------------------------------------
class CRWLockImpl
{
protected:
	CRWLockImpl();
	~CRWLockImpl();
	void ReadLockImpl();
	bool TryReadLockImpl();
	void WriteLockImpl();
	bool TryWriteLockImpl();
	void ReadUnlockImpl();
	void WriteUnlockImpl();

private:
#ifdef _WIN32
	SRWLOCK srwLock;
#else
	pthread_rwlock_t m_rwl;
#endif
};

//-----------------------------------------------------------------

class CRWLock : private CRWLockImpl
{
public:

	//创建读/写锁
	CRWLock() {};

	//销毁读/写锁
	~CRWLock() {};

	//获取读锁
	//如果其它一个线程占有写锁，则当前线程必须等待写锁被释放，才能对保护资源进行访问
	void ReadLock();

	//尝试获取一个读锁
	//如果获取成功，则立即返回true，否则当另一个线程占有写锁，则返回false
	bool TryReadLock();

	//获取写锁
	//如果一个或更多线程占有读锁，则必须等待所有锁被释放
	//如果相同的一个线程已经占有一个读锁或写锁，则返回结果不确定
	void WriteLock();

	//尝试获取一个写锁
	//如果获取成功，则立即返回true，否则当一个或更多其它线程占有读锁，返回false
	//如果相同的一个线程已经占有一个读锁或写锁，则返回结果不确定
	bool TryWriteLock();

	//释放一个读锁或写锁
	void ReadUnlock();
	void WriteUnlock();

	//// 将读取锁的申请和释放动作封装为raii对象，自动完成加锁和解锁管理
	//raii read_guard()const noexcept {
	//	return make_raii(*this, &CRWLock::ReadLock, &CRWLock::Unlock);
	//}
	//// 将写入锁的申请和释放动作封装为raii对象，自动完成加锁和解锁管理
	//raii write_guard()noexcept {
	//	return make_raii(*this, &CRWLock::WriteLock, &CRWLock::Unlock);
	//}

private:
	CRWLock(const CRWLock&);
	CRWLock& operator = (const CRWLock&);
};

inline void CRWLock::ReadLock()
{
	ReadLockImpl();
}

inline bool CRWLock::TryReadLock()
{
	return TryReadLockImpl();
}

inline void CRWLock::WriteLock()
{
	WriteLockImpl();
}

inline bool CRWLock::TryWriteLock()
{
	return TryWriteLockImpl();
}

inline void CRWLock::ReadUnlock()
{
	ReadUnlockImpl();
}
inline void CRWLock::WriteUnlock()
{
	WriteUnlockImpl();
}

class CMyRLockManager
{
public:
	CMyRLockManager(CRWLock& rwLock);
	//inline void read_guard()
	//{
	//	m_rwLock.ReadLock();
	//}
	//inline void write_guard()
	//{
	//	m_rwLock.WriteLock();
	//}
	~CMyRLockManager();
private:
	CRWLock &m_rwLock;
};

class CMyWLockManager
{
public:
	CMyWLockManager(CRWLock& rwLock);
	//inline void read_guard()
	//{
	//	m_rwLock.ReadLock();
	//}
	//inline void write_guard()
	//{
	//	m_rwLock.WriteLock();
	//}
	~CMyWLockManager();
private:
	CRWLock &m_rwLock;
};