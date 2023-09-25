#ifndef WAMI_UTILS_HPP
#define WAMI_UTILS_HPP

#include <systemc.h>

// CtoS-13.1's gcc-4.4 does not support (for some reason) the following
// headers. The IFNDEF should make the trick. At synthesis time, the macro
// __CTOS__ is defined; at compile time, it is not, thus the logging function
// will be regularly used.
#ifndef __CTOS__
    #include <initializer_list>
    #include <sstream>

template <typename T> void __to_str(std::stringstream &ss, T t) { ss << t; }

// recursive variadic function
template <typename T, typename... Ts> void __to_str(std::stringstream &ss, T t, Ts... ts)
{
    ss << t;
    __to_str(ss, ts...);
}

template <class T, typename... Ts> std::string __to_str(T t, Ts... ts)
{
    std::stringstream ss;
    __to_str(ss, t, ts...);
    return ss.str();
}

    #define STR(...) __to_str(__VA_ARGS__).c_str()

    // void SC_REPORT_TIME(std::string who, std::string msg)
    //{
    //  SC_REPORT_INFO(who.c_str(), STR("@", sc_time_stamp(), " ", msg));
    //}

#else
    #define STR(...) "Enable logging functions in wami_utils.hpp"
#endif

#endif
