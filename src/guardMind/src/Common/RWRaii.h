#pragma once
#include <type_traits>
#include <functional>
namespace gyd {
	/* Ԫģ�壬�����const������ȥ��const���η� */
	template<typename T>
	struct no_const {
		using type = typename std::conditional<std::is_const<T>::value, typename std::remove_const<T>::type, T>::type;
	};
	/*
	 * RAII��ʽ����������ͷ���Դ����
	 * ���󴴽�ʱ,ִ��acquire(������Դ)����(����Ϊ�պ���[]{})
	 * ��������ʱ,ִ��release(�ͷ���Դ)����
	 * ��ֹ���󿽱��͸�ֵ
	 */
	class raii {
	public:
		using fun_type = std::function<void()>;
		/* release: ����ʱִ�еĺ���
		 * acquire: ���캯��ִ�еĺ���
		 * default_com:_commit,Ĭ��ֵ,����ͨ��commit()������������
		 */
		explicit raii(fun_type release, fun_type acquire = [] {}, bool default_com = true) noexcept :
			_commit(default_com), _release(release) {
			acquire();
		}
		/* ��������ʱ����_commit��־ִ��_release���� */
		~raii() noexcept {
			if (_commit)
				_release();
		}
		/* �ƶ����캯�� ������ֵ��ֵ */
		raii(raii&& rv)noexcept :_commit(rv._commit), _release(rv._release) {
			rv._commit = false;
		};
		/* ���ÿ������캯�� */
		raii(const raii&) = delete;
		/* ���ø�ֵ������ */
		raii& operator=(const raii&) = delete;

		/* ����_commit��־ */
		raii& commit(bool c = true)noexcept { _commit = c; return *this; };
	private:
		/* Ϊtrueʱ��������ִ��_release */
		bool _commit;
	protected:
		/* ����ʱִ���к��� */
		std::function<void()> _release;
	}; /* raii */

	/* ����ʵ����Դ��raii������
	 * TΪ��Դ����
	 * acquireΪ������Դ������������ԴT
	 * releaseΪ�ͷ���Դ����,�ͷ���ԴT
	 */
	template<typename T>
	class raii_var {
	public:
		using    _Self = raii_var<T>;
		using   acq_type = std::function<T()>;
		using   rel_type = std::function<void(T &)>;
		explicit raii_var(acq_type acquire, rel_type release) noexcept :
			_resource(acquire()), _release(release) {
			//���캯����ִ��������Դ�Ķ���acquire()����ʼ��resource;
		}
		/* �ƶ����캯�� */
		raii_var(raii_var&& rv) :
			_resource(std::move(rv._resource)),
			_release(std::move(rv._release))
		{
			rv._commit = false;//������ֵ��������ʱ����ִ��_release
		}
		/* ��������ʱ����_commit��־ִ��_release���� */
		~raii_var() noexcept {
			if (_commit)
				_release(_resource);
		}
		/* ����_commit��־ */
		_Self& commit(bool c = true)noexcept { _commit = c; return *this; };
		/**�ص�����** ��ȡ��Դ���� */
			T& get() noexcept { return _resource; }
		T& operator*() noexcept
		{
			return get();
		}

		/* ���� T���Ͳ�ͬѡ��ͬ��->������ģ�� */
		template<typename _T = T>
		typename std::enable_if<std::is_pointer<_T>::value, _T>::type operator->() const noexcept
		{
			return _resource;
		}
		template<typename _T = T>
		typename std::enable_if<std::is_class<_T>::value, _T*>::type operator->() const noexcept
		{
			return std::addressof(_resource);
		}

	private:
		/* Ϊtrueʱ��������ִ��release */
		bool    _commit = true;
		T   _resource;
		rel_type _release;
	};
	/* ���� raii ����,
	 * ��std::bind��M_REL,M_ACQ��װ��std::function<void()>����raii����
	 * RES      ��Դ����
	 * M_REL    �ͷ���Դ�ĳ�Ա������ַ
	 * M_ACQ    ������Դ�ĳ�Ա������ַ
	 */
	template<typename RES, typename M_REL, typename M_ACQ>
	raii make_raii(RES & res, M_REL rel, M_ACQ acq, bool default_com = true)noexcept {
		// ����ʱ����������
		// ��̬�������õ���is_class,is_member_function_pointer�������ڱ����ڵļ��㡢��ѯ���жϡ�ת����type_traits��,
		// �е�������java�ķ���(reflect)�ṩ�Ĺ���,����ֻ�����ڱ����ڣ�������������ʱ��
		// ����type_traits����ϸ���ݲμ�:http://www.cplusplus.com/reference/type_traits/
		static_assert(std::is_class<RES>::value, "RES is not a class or struct type.");
		static_assert(std::is_member_function_pointer<M_REL>::value, "M_REL is not a member function.");
		static_assert(std::is_member_function_pointer<M_ACQ>::value, "M_ACQ is not a member function.");
		assert(nullptr != rel && nullptr != acq);
		auto p_res = std::addressof(const_cast<typename no_const<RES>::type&>(res));
		return raii(std::bind(rel, p_res), std::bind(acq, p_res), default_com);
	}
	/* ���� raii ���� ����M_ACQ�ļ򻯰汾 */
	template<typename RES, typename M_REL>
	raii make_raii(RES & res, M_REL rel, bool default_com = true)noexcept {
		static_assert(std::is_class<RES>::value, "RES is not a class or struct type.");
		static_assert(std::is_member_function_pointer<M_REL>::value, "M_REL is not a member function.");
		assert(nullptr != rel);
		auto p_res = std::addressof(const_cast<typename no_const<RES>::type&>(res));
		return raii(std::bind(rel, p_res), [] {}, default_com);
	}
} /* namespace gyd*/