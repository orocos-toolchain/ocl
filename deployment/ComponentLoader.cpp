/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:34:55 CEST 2008  ComponentLoader.cpp

                        ComponentLoader.cpp -  description
                           -------------------
    begin                : Thu July 03 2008
    copyright            : (C) 2008 Peter Soetens
    email                : peter.soetens@fmtc.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

// suppress warning in ComponentLoader.hpp
#define OCL_STATIC
#include "ocl/ComponentLoader.hpp"
#include "ComponentLoader.hpp"
#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <boost/filesystem.hpp>

#ifdef _WIN32
#include <rtt/os/win32/dlfcn.h>
#else
#include <dlfcn.h>
#endif

using namespace RTT;
using namespace std;
using namespace boost::filesystem;

namespace OCL
{
    FactoryMap* ComponentFactories::Factories = 0;
}

using namespace OCL;

// chose the file extension applicable to the O/S
#ifdef  __APPLE__
static const std::string SO_EXT(".dylib");
#else
# ifdef _WIN32
static const std::string SO_EXT(".dll");
# else
static const std::string SO_EXT(".so");
# endif
#endif

// choose how the PATH looks like
# ifdef _WIN32
static const std::string delimiters(";");
static const std::string default_delimiter(";");
# else
static const std::string delimiters(":;");
static const std::string default_delimiter(":");
# endif

namespace OCL {
namespace deployment {

boost::shared_ptr<ComponentLoader> ComponentLoader::minstance;

    // copied from RTT::PluginLoader
vector<string> splitPaths(string const& str)
{
    vector<string> paths;

    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        if ( !str.substr(lastPos, pos - lastPos).empty() )
            paths.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
    if ( paths.empty() )
        paths.push_back(".");
    return paths;
}

/**
 * Strips the 'lib' prefix and '.so'/'.dll'/... suffix (ie SO_EXT) from a filename.
 * Do not provide paths, only filenames, for example: "libcomponent.so"
 * @param str filename.
 * @return stripped filename.
 */
string makeShortFilename(string const& str) {
    string ret = str;
    if (str.substr(0,3) == "lib")
        ret = str.substr(3);
    if (ret.rfind(SO_EXT) != string::npos)
        ret = ret.substr(0, ret.rfind(SO_EXT) );
    return ret;
}

boost::shared_ptr<ComponentLoader> ComponentLoader::Instance() {
    if (!minstance)
        minstance.reset( new ComponentLoader() );
    return minstance;
}

void ComponentLoader::Release() {
    minstance.reset();
}

void ComponentLoader::import( std::string const& path_list )
{
    vector<string> paths = splitPaths(path_list + default_delimiter + component_path);

    for (vector<string>::iterator it = paths.begin(); it != paths.end(); ++it)
    {
        // Scan path/* (non recursive) for components
        path p = path(*it);
        if (is_directory(p))
        {
            log(Info) << "Importing component libraries from directory " << p.string() << " ..."<<endlog();
            for (directory_iterator itr(p); itr != directory_iterator(); ++itr)
            {
                log(Debug) << "Scanning file " << itr->path().string() << " ...";
                if (is_regular_file(itr->status()) && !is_symlink(itr->symlink_status()))
                    loadInProcess( itr->path().string(), makeShortFilename(itr->path().filename() ),  false);
                else {
                    if (is_symlink(itr->symlink_status()))
                        log(Debug) << "is symlink: ignored."<<endlog();
                    else
                        if (!is_regular_file(itr->status()))
                            log(Debug) << "not a regular file: ignored."<<endlog();
                }
            }
        }
        else
            log(Debug) << "No such directory: " << p << endlog();

        // Repeat for path/OROCOS_TARGET:
        p = path(*it) / OROCOS_TARGET_NAME;
        if (is_directory(p))
        {
            log(Info) << "Importing component libraries from directory " << p.string() << " ..."<<endlog();
            for (directory_iterator itr(p); itr != directory_iterator(); ++itr)
            {
                log(Debug) << "Scanning file " << itr->path().string() << " ...";
                if (is_regular_file(itr->status()) && !is_symlink(itr->symlink_status()))
                    loadInProcess( itr->path().string(), makeShortFilename(itr->path().filename() ),  false);
                else {
                    if (is_symlink(itr->symlink_status()))
                        log(Debug) << "is symlink: ignored."<<endlog();
                    else
                        if (!is_regular_file(itr->status()))
                            log(Debug) << "not a regular file: ignored."<<endlog();
                }
            }
        }
        else
            log(Debug) << "No such directory: " << p << endlog();
    }
}

bool ComponentLoader::import( std::string const& package, std::string const& path_list )
{
    vector<string> paths = splitPaths(path_list + default_delimiter + component_path);
    vector<string> tryouts( paths.size() * 4 );
    tryouts.clear();

    if ( isImported(package) ) {
        log(Info) <<"Component package '"<< package <<"' already imported." <<endlog();
        return true;
    } else {
        log(Info) << "Component package '"<< package <<"' not seen before." <<endlog();
    }

    path arg( package );
    path dir = arg.parent_path();
    string file = arg.filename();

    for (vector<string>::iterator it = paths.begin(); it != paths.end(); ++it)
    {
        path p = path(*it) / dir / (file + SO_EXT);
        tryouts.push_back( p.string() );
        if (is_regular_file( p ) && loadInProcess( p.string(), package, true ) )
            return true;
        p = path(*it) / dir / ("lib" + file + SO_EXT);
        tryouts.push_back( p.string() );
        if (is_regular_file( p ) && loadInProcess( p.string(), package, true ) )
            return true;
        p = path(*it) / dir / OROCOS_TARGET_NAME / (file + SO_EXT);
        tryouts.push_back( p.string() );
        if (is_regular_file( p ) && loadInProcess( p.string(), package, true ) )
            return true;
        p = path(*it) / dir / OROCOS_TARGET_NAME / ("lib" + file + SO_EXT);
        tryouts.push_back( p.string() );
        if (is_regular_file( p ) && loadInProcess( p.string(), package, true ) )
            return true;
    }
    log(Error) << "No such package found in path: " << package << ". Tried:"<< endlog();
    for(vector<string>::iterator it=tryouts.begin(); it != tryouts.end(); ++it)
        log(Error) << *it << " ";
    log(Error)<< endlog();
    return false;
}

bool ComponentLoader::isImported(string type_name)
{
    return ComponentFactories::Instance().find(type_name) != ComponentFactories::Instance().end();
}

// loads a single component in the current process.
bool ComponentLoader::loadInProcess(string file, string libname, bool log_error) {
    path p(file);
    char* error;
    void* handle;

    // check if the library is already loaded
    // NOTE if this library has been loaded, you can unload and reload it to apply changes (may be you have updated the dynamic library)
    // anyway it is safe to do this only if there isn't any isntance whom type was loaded from this library

    std::vector<LoadedLib>::iterator lib = loadedLibs.begin();
    while (lib != loadedLibs.end()) {
        // there is already a library with the same name
        if ( lib->shortname == libname) {
            log(Warning) <<"Library "<< lib->filename <<" already loaded... " ;

            bool can_unload = true;
            CompList::iterator cit;
            for( std::vector<std::string>::iterator ctype = lib->components_type.begin();  ctype != lib->components_type.end() && can_unload; ++ctype) {
                for ( cit = comps.begin(); cit != comps.end(); ++cit) {
                    if( (*ctype) == cit->second.type ) {
                        // the type of an allocated component was loaded from this library. it might be unsafe to reload the library
                        log(Warning) << "can NOT reload library because of the instance " << cit->second.type  <<"::"<<cit->second.instance->getName()  <<endlog();
                        can_unload = false;
                    }
                }
            }
            if( can_unload ) {
                log(Warning) << "try to RELOAD"<<endlog();
                dlclose(lib->handle);
                // remove the library info from the vector
                std::vector<LoadedLib>::iterator lib_un = lib;
                loadedLibs.erase(lib_un);
                lib = loadedLibs.end();
            }
            else
                return false;
        }
        else lib++;
    }

    handle = dlopen ( p.string().c_str(), RTLD_NOW | RTLD_GLOBAL );

    if (!handle) {
        if ( log_error ) {
            log(Error) << "Could not load library '"<< p.string() <<"':"<<endlog();
            log(Error) << dlerror() << endlog();
        }
        return false;
    }

    //------------- if you get here, the library has been loaded -------------
    log(Debug)<<"Succesfully loaded "<<libname<<endlog();
    LoadedLib loading_lib(file, libname, handle);
    dlerror();    /* Clear any existing error */

    // Lookup Component factories (multi component case):
    FactoryMap* (*getfactory)(void) = 0;
    vector<string> (*getcomponenttypes)(void) = 0;
    FactoryMap* fmap = 0;
    getfactory = (FactoryMap*(*)(void))( dlsym(handle, "getComponentFactoryMap") );
    if ((error = dlerror()) == NULL) {
        // symbol found, register factories...
        fmap = (*getfactory)();
        ComponentFactories::Instance().insert( fmap->begin(), fmap->end() );
        log(Info) << "Loaded multi component library '"<< file <<"'"<<endlog();
        getcomponenttypes = (vector<string>(*)(void))(dlsym(handle, "getComponentTypeNames"));
        if ((error = dlerror()) == NULL) {
            log(Debug) << "Components:";
            vector<string> ctypes = getcomponenttypes();
            for (vector<string>::iterator it = ctypes.begin(); it != ctypes.end(); ++it)
                log(Debug) <<" "<< *it;
            log(Debug) << endlog();
        }
        loadedLibs.push_back(loading_lib);
        return true;
    }

    // Lookup createComponent (single component case):
    dlerror();    /* Clear any existing error */

    RTT::TaskContext* (*factory)(std::string) = 0;
    std::string(*tname)(void) = 0;
    factory = (RTT::TaskContext*(*)(std::string))(dlsym(handle, "createComponent") );
    string create_error;
    error = dlerror();
    if (error) create_error = error;
    tname = (std::string(*)(void))(dlsym(handle, "getComponentType") );
    string gettype_error;
    error = dlerror();
    if (error) gettype_error = error;
    if ( factory && tname ) {
        std::string cname = (*tname)();
        if ( ComponentFactories::Instance().count(cname) == 1 ) {
            log(Warning) << "Component type name "<<cname<<" already used: overriding."<<endlog();
        }
        ComponentFactories::Instance()[cname] = factory;
        log(Info) << "Loaded component type '"<< cname <<"'"<<endlog();
        loading_lib.components_type.push_back( cname );
        loadedLibs.push_back(loading_lib);
        return true;
    }

    log(Error) <<"Unloading "<< loading_lib.filename  <<": not a valid component library:" <<endlog();
    if (!create_error.empty())
        log(Error) << "   " << create_error << endlog();
    if (!gettype_error.empty())
        log(Error) << "   " << gettype_error << endlog();
    dlclose(handle);
    return false;
}

std::vector<std::string> ComponentLoader::listComponentTypes() const {
    vector<string> names;
    OCL::FactoryMap::iterator it;
    for( it = OCL::ComponentFactories::Instance().begin(); it != OCL::ComponentFactories::Instance().end(); ++it) {
        names.push_back( it->first );
    }
    return names;
}

std::string ComponentLoader::getComponentPath() const {
    return component_path;
}

void ComponentLoader::setComponentPath( std::string const& newpath ) {
    component_path = newpath;
}


RTT::TaskContext *ComponentLoader::loadComponent(const std::string & name, const std::string & type)
{
    TaskContext* instance = 0;
    RTT::TaskContext* (*factory)(std::string name) = 0;
    log(Debug) << "Trying to create component "<< name <<" of type "<< type << endlog();

    // First: try loading from imported libraries. (see: import).
    if ( ComponentFactories::Instance().count(type) == 1 ) {
        factory = ComponentFactories::Instance()[ type ];
        if (factory == 0 ) {
            log(Error) <<"Found empty factory for Component type "<<type<<endlog();
            return 0;
        }
    }

    if ( factory ) {
        log(Debug) <<"Found factory for Component type "<<type<<endlog();
    } else {
        log(Error) << "Unable to create Orocos Component '"<<type<<"': unknown component type." <<endlog();
        return 0;
    }

    comps[name].type = type;

    try {
        comps[name].instance = instance = (*factory)(name);
    } catch(...) {
        log(Error) <<"The constructor of component type "<<type<<" threw an exception!"<<endlog();
    }

    if ( instance == 0 ) {
        log(Error) <<"Failed to load component with name "<<name<<": refused to be created."<<endlog();
    }
    return instance;
}

bool ComponentLoader::unloadComponent( RTT::TaskContext* tc ) {
    if (!tc)
        return false;
    CompList::iterator it;
    it = comps.find( tc->getName() );

    if ( it != comps.end() ) {
        delete tc;
        comps.erase(it);
        return true;
    }
    log(Error) <<"Refusing to unload a component I didn't load myself."<<endlog();
    return false;
}

std::vector<std::string> ComponentLoader::listComponents() const
{
    vector<string> names( comps.size() );
    for(map<string,ComponentData>::const_iterator it = comps.begin(); it != comps.end(); ++it)
        names.push_back( it->first );
    return names;
}

} // namespace deployment
} // namespace OCL
