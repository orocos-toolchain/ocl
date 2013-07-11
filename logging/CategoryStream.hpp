#ifndef CATEGORY_STREAM_HPP
#define CATEGORY_STREAM_HPP 1

#include <ocl/OCL.hpp>
#include <log4cpp/Priority.hh>
#include <rtt/rt_string.hpp>

namespace OCL {
namespace logging {

class OCL_API Category;

/**
 * This is a utility class which you can use to stream messages into a 
 * category object.
 * It provides an std::iostream like syntax using the << operator, but you
 * need to call flush() in order to do the actual write of your message.
 */
class OCL_API CategoryStream
{
public:

    /**
     * Construct a CategoryStream for given Category with given priority.
     * @param category The category this stream will send log messages to.
     * @param priority The priority the log messages will get or 
     * Priority::NOTSET to silently discard any streamed in messages.
     **/
    CategoryStream(Category* rt_category, log4cpp::Priority::Value priority);

    /**
     * Copy-constructor needed because the output string stream can't be
     * copied. We rater have to copy the underlying (real-time) string.
     * @param rhs The CategoryStream to copy from
     */ 
    CategoryStream(const CategoryStream & rhs);

    /**
     * Destructor for CategoryStream which also flushes any remaining data
     * to the Category object.
     **/
    virtual ~CategoryStream();

    /**
     * Flush the contents of the stream buffer to the Category and
     * empties the buffer.
     **/
    void flush();

    /**
     * Stream in arbitrary types and objects.  
     * @param t The value or object to stream in.
     * @returns A reference to itself.
     **/
    template<typename T> CategoryStream& operator<<(const T& t) 
    {
         if (_priority != log4cpp::Priority::NOTSET) 
         {
             (oss) << t;
         }
         return *this;
     }

private:

    Category* _category;
    log4cpp::Priority::Value _priority;
    RTT::rt_ostringstream oss;

};


} // namespace logging
} // namespace OCL

#endif // CATEGORY_STREAM_HPP
