#include "utilities.h"

#include "anh/logger.h"
#include <boost/python.hpp>

using namespace swganh::scripting;

void swganh::scripting::logPythonException()
{
    PyObject *exc, *val, *tb;
    PyErr_Fetch(&exc, &val, &tb);
    PyErr_NormalizeException(&exc, &val, &tb);
    boost::python::handle<> hexc(exc), hval(boost::python::allow_null(val)), htb(boost::python::allow_null(tb));
    if(!hval)
    {
        LOG(error) << std::string(boost::python::extract<std::string>(boost::python::str(hexc)));
    }
    else
    {
        boost::python::object traceback(boost::python::import("traceback"));
        boost::python::object format_exception(traceback.attr("format_exception"));
        boost::python::list formatted_list(format_exception(hexc,hval,htb));
        for(int count = 0; count < len(formatted_list); ++count)
        {
            LOG(error) << std::string(boost::python::extract<std::string>(formatted_list[count].slice(0,-1)));
        }
    }
}