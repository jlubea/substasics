#ifndef SUBSTASICS_DEPENDENCY_INJECTION_TRACE_H
#define SUBSTASICS_DEPENDENCY_INJECTION_TRACE_H

#include "substasics/platform/exports.h"
#include "substasics/platform/mutex.h"
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <vector>

namespace substasics { namespace dependency_injection {

	template<class _GeneratedType>
	class iobject_factory
	{
	public:
		virtual boost::shared_ptr<_GeneratedType> get() = 0;

		//template<typename _T1>
		//virtual boost::shared_ptr<_GeneratedType> get(_T1 &t1) = 0;

		//template<typename _T1, typename _T2>
		//virtual boost::shared_ptr<_GeneratedType> get(_T1 &t1, _T2 &t2) = 0;

		//template<typename _T1, typename _T2, typename _T3>
		//virtual boost::shared_ptr<_GeneratedType> get(_T1 &t1, _T2 &t2, _T3 &t3) = 0;

		// if you want more constructor arguments, then you'll need to create a custom implementation of iobject_factory
	};

	template<class _GeneratedType>
	class object_factory : public iobject_factory<_GeneratedType>
	{
	public:
		virtual boost::shared_ptr<_GeneratedType> get()
		{
			return boost::shared_ptr<_GeneratedType>(new _GeneratedType());
		}
	};

	template<class _GeneratedType>
	class singleton_factory : public iobject_factory<_GeneratedType>
	{
	public:
		virtual boost::shared_ptr<_GeneratedType> get()
		{
			if (_singleton.get() == NULL)
			{
				substasics::platform::scoped_lock lock (_initializationMutex);
				if (_singleton.get() == NULL)
				{
					_singleton.reset(new _GeneratedType());
				}
			}
			return _singleton;
		}

	private:
		boost::shared_ptr<_GeneratedType> _singleton;
		substasics::platform::mutex _initializationMutex;
	};


	namespace detail {

		struct type_and_factory
		{
			type_and_factory(const type_info &type_, boost::any factory_) :
				type(type_),
				factory(factory_)
			{
			}

			const type_info &type;
			boost::any factory;
		};

	}

	//
	// Use this class to facilitate dependency injection
	//
	class kernel
	{
	public:
		template <class _InterfaceType, class _GeneratedType>
		void bind(iobject_factory<_GeneratedType> *factory)
		{
			bind<_InterfaceType, _GeneratedType>(boost::shared_ptr< iobject_factory<_GeneratedType> >(factory));
		}

		template <class _InterfaceType, class _GeneratedType>
		void bind(boost::shared_ptr< iobject_factory<_GeneratedType> > factory)
		{
			for (std::vector<detail::type_and_factory>::iterator it = _factories.begin(); it != _factories.end(); it++)
			{
				if (it->type == typeid(_InterfaceType))
				{
					it->factory = factory;
					return;
				}
			}

			detail::type_and_factory type_and_factory(typeid(_InterfaceType), factory);
			_factories.push_back(type_and_factory);
		}

		template <class _InterfaceType>
		boost::shared_ptr<_InterfaceType> get()
		{
			for (std::vector<detail::type_and_factory>::iterator it = _factories.begin(); it != _factories.end(); it++)
			{
				if (it->type == typeid(_InterfaceType))
				{
					boost::shared_ptr< iobject_factory<_InterfaceType> > factory = *boost::unsafe_any_cast< boost::shared_ptr< iobject_factory<_InterfaceType> > >(&it->factory);
					return factory->get();
				}
			}

			return boost::shared_ptr<_InterfaceType>();
		}

	private:
		std::vector<detail::type_and_factory> _factories;
	};

	//
	// Use this class wherever a global Singleton-style kernel is preferred
	//
	class SUBSTASICS_API global_kernel : private boost::noncopyable
	{
	private:
		global_kernel()
		{
		}

	public:
		static global_kernel &Instance()
		{
			if (__instance == NULL)
			{
				substasics::platform::scoped_lock lock(__initializationMutex);
				if (__instance == NULL)
				{
					__instance = new global_kernel();
				}
			}
			return *__instance;
		}

		template <class _InterfaceType, class _GeneratedType>
		void bind(iobject_factory<_GeneratedType> *factory)
		{
			_kernel.bind<_InterfaceType, _GeneratedType>(factory);
		}

		template <class _InterfaceType, class _GeneratedType>
		void bind(boost::shared_ptr< iobject_factory<_GeneratedType> > factory)
		{
			_kernel.bind<_InterfaceType, _GeneratedType>(factory);
		}

		template <class _InterfaceType>
		boost::shared_ptr<_InterfaceType> get()
		{
			return _kernel.get<_InterfaceType>();
		}

	private:
		kernel _kernel;
		static global_kernel *__instance;
		static substasics::platform::mutex __initializationMutex;
	};

}
}

#endif