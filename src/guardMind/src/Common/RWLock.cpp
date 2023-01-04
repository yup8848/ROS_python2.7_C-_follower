#include "RWLock.h"
#include <stdexcept>

#if 0
RWLock::RWLock(bool writeFirst) :
	WRITE_FIRST(writeFirst),
	m_write_thread_id(),
	m_lockCount(0),
	m_writeWaitCount(0) {
}
int RWLock::readLock() {
	// ==时为独占写状态,不需要加锁
	if (this_thread::get_id() != this->m_write_thread_id) {
		int count;
		if (WRITE_FIRST)//写优先模式下,要检测等待写的线程数为0(m_writeWaitCount==0)
			do {
				while ((count = m_lockCount) == WRITE_LOCK_STATUS || m_writeWaitCount > 0);//写锁定时等待
			} while (!m_lockCount.compare_exchange_weak(count, count + 1));
		else
			do {
				while ((count = m_lockCount) == WRITE_LOCK_STATUS); //写锁定时等待
			} while (!m_lockCount.compare_exchange_weak(count, count + 1));
	}
	return m_lockCount;
}
int RWLock::readUnlock() {
	// ==时为独占写状态,不需要加锁
	if (this_thread::get_id() != this->m_write_thread_id)
		--m_lockCount;
	return m_lockCount;
}
int RWLock::writeLock() {
	// ==时为独占写状态,避免重复加锁
	if (this_thread::get_id() != this->m_write_thread_id) {
		++m_writeWaitCount;//写等待计数器加1
		// 没有线程读取时(加锁计数器为0)，置为-1加写入锁，否则等待
		for (int zero = FREE_STATUS; !this->m_lockCount.compare_exchange_weak(zero, WRITE_LOCK_STATUS); zero = FREE_STATUS);
		--m_writeWaitCount;//获取锁后,计数器减1
		m_write_thread_id = this_thread::get_id();
	}
	return m_lockCount;
}
int RWLock::writeUnlock() {
	if (this_thread::get_id() != this->m_write_thread_id) {
		throw runtime_error("writeLock/Unlock mismatch");
	}
	assert(WRITE_LOCK_STATUS == m_lockCount);
	m_write_thread_id = NULL_THEAD;
	m_lockCount.store(FREE_STATUS);
	return m_lockCount;
}
const std::thread::id RWLock::NULL_THEAD;
#endif


CRWLockImpl::CRWLockImpl()
{
#if defined(_WIN32)

	InitializeSRWLock(&srwLock);

#else
	if (pthread_rwlock_init(&m_rwl, NULL))
		cout << "cannot create reader/writer lock" << endl;
#endif
}

CRWLockImpl::~CRWLockImpl()
{
#if defined(_WIN32)

#else
	pthread_rwlock_destroy(&m_rwl);
#endif	
}

void CRWLockImpl::ReadLockImpl()
{
#ifdef _WIN32
	AcquireSRWLockShared(&srwLock);
#else
	if (pthread_rwlock_rdlock(&m_rwl))
		cout << "cannot lock reader/writer lock" << endl;
#endif
}

bool CRWLockImpl::TryReadLockImpl()
{
#ifdef _WIN32
	if (TryAcquireSRWLockShared(&srwLock))
		return true;
#else
	int rc = pthread_rwlock_tryrdlock(&m_rwl);
	if (rc == 0)
		return true;
	else if (rc == EBUSY)
		return false;
	else
		cout << "cannot lock reader/writer lock" << endl;
#endif
	return false;
}

void CRWLockImpl::WriteLockImpl()
{
#ifdef _WIN32
	AcquireSRWLockExclusive(&srwLock);
#else
	if (pthread_rwlock_wrlock(&m_rwl))
		cout << "cannot lock reader/writer lock" << endl;
#endif
}

bool CRWLockImpl::TryWriteLockImpl()
{
#ifdef _WIN32
	if (TryAcquireSRWLockExclusive(&srwLock))
		return true;
#else
	int rc = pthread_rwlock_trywrlock(&m_rwl);
	if (rc == 0)
		return true;
	else if (rc == EBUSY)
		return false;
	else
		cout << "cannot lock reader/writer lock" << endl;
#endif
	return false;
}

void CRWLockImpl::ReadUnlockImpl()
{
#ifdef _WIN32
	ReleaseSRWLockShared(&srwLock);
#else
	if (pthread_rwlock_unlock(&m_rwl))
		cout << "cannot unlock reader/writer lock" << endl;
#endif
}

void CRWLockImpl::WriteUnlockImpl()
{
#ifdef _WIN32
	ReleaseSRWLockExclusive(&srwLock);
#else
	if (pthread_rwlock_unlock(&m_rwl))
		cout << "cannot unlock reader/writer lock" << endl;
#endif
}
CMyRLockManager::CMyRLockManager(CRWLock& rwLock)
	:m_rwLock(rwLock)
{
	m_rwLock.ReadLock();
}

CMyRLockManager::~CMyRLockManager()
{
	m_rwLock.ReadUnlock();
}

CMyWLockManager::CMyWLockManager(CRWLock& rwLock)
	:m_rwLock(rwLock)
{
	m_rwLock.WriteLock();
}

CMyWLockManager::~CMyWLockManager()
{
	m_rwLock.WriteUnlock();
}