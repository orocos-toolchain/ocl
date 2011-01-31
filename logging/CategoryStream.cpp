#include "CategoryStream.hpp"
#include "Category.hpp"

namespace OCL {
namespace logging {

CategoryStream::CategoryStream(Category* rt_category, log4cpp::Priority::Value priority) :
    _category(rt_category),
    _priority(priority)
{

}
 
CategoryStream::~CategoryStream()
{
    flush();
}   

void CategoryStream::flush()
{
    _category->log(_priority, oss.str());
    oss.flush();
}

CategoryStream& eol(CategoryStream& os)
{
    os.flush();

    return os;
}

CategoryStream::CategoryStream(const CategoryStream & rhs) :
    _category(rhs._category),
    _priority(rhs._priority)
{
    // Must copy the underlying buffer but not the output stream
    (*this).oss.str(rhs.oss.str());
}

} // namespace logging
} // namespace OCL

