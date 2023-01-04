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
 * atomicʵ�ֶ�д��Դ��,��ռд,�����,��ֹ���ƹ��캯����'='��ֵ������
 * WRITE_FIRSTΪtrueʱΪд����ģʽ,������̵߳ȴ���ȡ(m_writeWaitCount>0)��ȴ�,������д�߳��Ȼ�ȡ��
 * ����Ƕ�׼���
 * readLock/Unlock ʵ�ֹ���Ķ�ȡ��/�������߳�������
 * writeLock/Unlock ʵ�ֶ�ռ��д���/����,ͬʱֻ����һ���߳�д�룬
 * �����߳��ڶ�ȡʱ��д���߳���������д���߳�ִ��ʱ�����еĶ�ȡ�̶߳���������
 */
class RWLock {
#define WRITE_LOCK_STATUS -1
#define FREE_STATUS 0
private:
	/* ��ʼΪ0���߳�id */
	static const  std::thread::id NULL_THEAD;
	const bool WRITE_FIRST;
	/* �����жϵ�ǰ�Ƿ���д�߳� */
	thread::id m_write_thread_id;
	/* ��Դ��������,����Ϊint��ԭ�ӳ�Ա����,-1Ϊд״̬��0Ϊ����״̬,>0Ϊ�����ȡ״̬ */
	atomic_int m_lockCount;
	/* �ȴ�д�̼߳�����,����Ϊunsigned int��ԭ�ӳ�Ա����*/
	atomic_uint m_writeWaitCount;
public:
	// ��ֹ���ƹ��캯��
	RWLock(const RWLock&) = delete;
	// ��ֹ����ֵ������
	RWLock& operator=(const RWLock&) = delete;
	//RWLock& operator=(const RWLock&) volatile = delete;
	RWLock(bool writeFirst = false);;//Ĭ��Ϊ������ģʽ
	virtual ~RWLock() = default;
	int readLock();
	int readUnlock();
	int writeLock();
	int writeUnlock();
	// ����ȡ����������ͷŶ�����װΪraii�����Զ���ɼ����ͽ�������
	raii read_guard()const noexcept {
		return make_raii(*this, &RWLock::readUnlock, &RWLock::readLock);
	}
	// ��д������������ͷŶ�����װΪraii�����Զ���ɼ����ͽ�������
	raii write_guard()noexcept {
		return make_raii(*this, &RWLock::writeUnlock, &RWLock::writeLock);
	}
	//other class defineition code....
	static auto make_guard(RWLock &lock)->decltype(make_raii(lock, &RWLock::readUnlock, &RWLock::readLock, true)) {
		return make_raii(lock, &RWLock::readUnlock, &RWLock::readLock, true);
		
	//����auto xxx -> xxx �ľ䷨ʹ������C++11��"׷�ٷ�������"���ԣ����������ͺ��ã�
	//ʹ��decltype�ؼ����Ƶ�����������
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
 ��д������ǰ�Ķ�����û����ʱ�����Դ����ֻ����һ��д���߷��ʱ�����Դ
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

	//������/д��
	CRWLock() {};

	//���ٶ�/д��
	~CRWLock() {};

	//��ȡ����
	//�������һ���߳�ռ��д������ǰ�̱߳���ȴ�д�����ͷţ����ܶԱ�����Դ���з���
	void ReadLock();

	//���Ի�ȡһ������
	//�����ȡ�ɹ�������������true��������һ���߳�ռ��д�����򷵻�false
	bool TryReadLock();

	//��ȡд��
	//���һ��������߳�ռ�ж����������ȴ����������ͷ�
	//�����ͬ��һ���߳��Ѿ�ռ��һ��������д�����򷵻ؽ����ȷ��
	void WriteLock();

	//���Ի�ȡһ��д��
	//�����ȡ�ɹ�������������true������һ������������߳�ռ�ж���������false
	//�����ͬ��һ���߳��Ѿ�ռ��һ��������д�����򷵻ؽ����ȷ��
	bool TryWriteLock();

	//�ͷ�һ��������д��
	void ReadUnlock();
	void WriteUnlock();

	//// ����ȡ����������ͷŶ�����װΪraii�����Զ���ɼ����ͽ�������
	//raii read_guard()const noexcept {
	//	return make_raii(*this, &CRWLock::ReadLock, &CRWLock::Unlock);
	//}
	//// ��д������������ͷŶ�����װΪraii�����Զ���ɼ����ͽ�������
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