// test mix of log4cpp and OCL::logging implementations

#include <iostream>
#include <log4cpp/HierarchyMaintainer.hh>
#include "logging/Category.hpp"

using namespace RTT;

int main(int argc, char** argv)
{
    // use only OCL::logging Category's
    log4cpp::HierarchyMaintainer::set_category_factory(
        OCL::logging::Category::createOCLCategory);

    std::string name = "org.test.c1";

    // what types do we get from the category itself?
    std::cout << "\nFrom category???\n";
    log4cpp::Category& category2 = log4cpp::Category::getInstance(name);
    std::cout << "category2 type " << typeid(category2).name() << std::endl;
    std::cout << "category2 ptype " << typeid(&category2).name() << std::endl;

    OCL::logging::Category* category = 
        dynamic_cast<OCL::logging::Category*>(&category2);
    if (0 != category)
    {
        std::cout << "category type " << typeid(*category).name() << std::endl;
    }
    else
    {
        std::cout << "Unable cast" << std::endl;
    }

    // and directly?
    std::cout << "\nDirectly ...\n";
    category = dynamic_cast<OCL::logging::Category*>(
        &log4cpp::Category::getInstance(name));
    if (0 != category)
    {
        std::cout << "category type " << typeid(*category).name() << std::endl;
    }
    else
    {
        std::cout << "Unable cast" << std::endl;
    }

    // and through hierarchy maintainer?
    std::cout << "\nThrough hierarchy maintainer ...\n";
    log4cpp::Category* p = log4cpp::HierarchyMaintainer::getDefaultMaintainer().getExistingInstance(name);
    std::cout << "category ptype " << typeid(p).name() << std::endl;
    std::cout << "category type  " << typeid(*p).name() << std::endl;
    category = dynamic_cast<OCL::logging::Category*>(p);
    if (0 != category)
    {
        std::cout << "category type " << typeid(*category).name() << std::endl;
    }
    else
    {
        std::cout << "Unable cast" << std::endl;
    }
        
    category = dynamic_cast<OCL::logging::Category*>(
        log4cpp::HierarchyMaintainer::getDefaultMaintainer().getExistingInstance(name));
    if (0 != category)
    {
        std::cout << "category type " << typeid(*category).name() << std::endl;
    }
    else
    {
        std::cout << "Unable cast" << std::endl;
    }
    
    return 0;
}
