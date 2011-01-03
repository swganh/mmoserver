#ifndef _PYTHON_ENGINE_ENGINE_H_
#define _PYTHON_ENGINE_ENGINE_H_

#include <boost/python.hpp>
#include <map>
#include <vector>

namespace python{
namespace engine{
namespace bp = boost::python;

typedef std::map<std::string,bp::object> bp_object_map;
class object;
/*! Assists the user in loading and executing Python scripts from
*   within C++.
*
*/
class engine
{
public:
    engine(const std::string& base_path);
    /**
    * \brief load's the python script into memory
    *  and stores the Boost Python Object into a 
    *  private map for later use in @runLoadedFile
    *
    * \param filename The file to load into memory
    */
    void load(const std::string& filename);
     /**
    * \brief run's the python file
    *
    * \param filename The file to run from the loaded_files_ map
    *   if not found, a message will be given
    */
    void run(const std::string& filename);

    /**
    * \brief registers a module into the Python virtual machine
    *
    * \param init_func The _inittab struct that has the modules
    *  name and function call
    */
    void registerModule(_inittab init_func);

    /**
    * \brief outputs the currently loaded files available
    *   for immediate execution.
    *
    */
    bp_object_map printLoadedFiles() { return loaded_files_; }

    /**
    * \brief checks to see if the filename has been loaded
    * 
    * \returns true if the file has been loaded, false else.
    */
    bool isFileLoaded(const std::string& filename);

    /**
    * \brief sets the default file path to find scripts
    *  
    * \param filepath sets the path_ behind the scenes
    */
    bp::object getLoadedFile(const std::string& filename);
private:
    // hide default ctor
    engine();
    void setFullPath(const std::string& filename, const std::string& root_path);
    void setFullPath(const std::string& filename);
    // base path set by the default ctor
    std::string base_path_;
    /**
    * \brief helper function to take in base filename and return full path
    *
    * \param filename to add to base path
    * \returns char* of full path after adding filename
    */
    char* fullPath(const std::string& filename);

    /**
    * \brief full path including .py
    */
    std::string full_path_;

    /**
    * \brief a std::map containing the name and boost python object
    *   of all loaded files
    */
    bp_object_map loaded_files_; 
};
}// namespace engine
} // namespace python
#endif //_PYTHON_ENGINE_ENGINE_H_