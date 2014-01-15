// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <boost/python.hpp>
#include <boost/python/detail/wrap_python.hpp>

namespace swganh {
namespace scripting {
    
    /**
     * User defined mutex type designed to hold an instance of the GIL
     * for python.
     */
    class GilMutex
    {
    public:
        void lock()
        {
            state_ = PyGILState_Ensure();
        }
    
        void unlock()
        {
            PyGILState_Release(state_);
        }
    
    private:
        PyGILState_STATE state_;
    };

    /**
     * A simple lock that manages its own internal GilMutex instance.
     */
    class ScopedGilLock
    {
    public:
        ScopedGilLock()
        {
            mutex_.lock();
        }
    
        ~ScopedGilLock()
        {
            mutex_.unlock();
        }
    
    private:
        GilMutex mutex_;
    };

    class ScopedGilRelease
    {
    public:
        ScopedGilRelease()
        {
    		state_ = PyEval_SaveThread();
        }
    
        ~ScopedGilRelease()
        {
    		PyEval_RestoreThread(state_);
        }
    private:
        PyThreadState* state_;    
    };

	template<typename T>
	void GetValue(boost::python::object p_object, std::string key, T& extract_value)
	{
			if (p_object.contains(key))
			{
				boost::python::extract<T> tmp_x(p_object[key]);
				if (tmp_x.check())
					extract_value = tmp_x();
			}
	}

	void logPythonException();
	
}}  // namespace swganh::scripting
