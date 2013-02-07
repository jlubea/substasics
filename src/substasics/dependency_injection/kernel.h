#ifndef SUBSTASICS_DEPENDENCY_INJECTION_TRACE_H
#define SUBSTASICS_DEPENDENCY_INJECTION_TRACE_H

#include "substasics/platform/exports.h"
#include "substasics/platform/mutex.h"
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <map>

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


	class kernel
	{
	public:
		template <class _GeneratedType>
		void bind(const std::string &key, boost::shared_ptr< iobject_factory<_GeneratedType> > factory)
		{
			_factories[key] = factory;
		}

		template <class _InterfaceType>
		boost::shared_ptr<_InterfaceType> get(const std::string &key)
		{
			const std::map<std::string, boost::any>::iterator it = _factories.find(key);
			if (it == _factories.end())
			{
				return boost::shared_ptr<_InterfaceType>();
			}

			boost::shared_ptr< iobject_factory<_InterfaceType> > factory = *boost::unsafe_any_cast< boost::shared_ptr< iobject_factory<_InterfaceType> > >(&it->second);
			return factory->get();
		}

	private:
		std::map<std::string, boost::any> _factories;
	};

}
}

#endif