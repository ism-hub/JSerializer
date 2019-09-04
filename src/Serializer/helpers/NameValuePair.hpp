#pragma once

#include <memory>
#include <WString.h>

namespace ser {

struct NameValuePairCore {};

template <class T>
class NameValuePair : NameValuePairCore
 {
   private:
     // If we get passed an array, keep the type as is, otherwise store
     // a reference if we were passed an l value reference, else copy the value
     //using Type = typename std::conditional<std::is_array<typename std::remove_reference<T>::type>::value,
     //                                       typename std::remove_cv<T>::type,
     //                                       typename std::conditional<std::is_lvalue_reference<T>::value,
     //                                                                 T,
     //                                                                 typename std::decay<T>::type>::type>::type;

     // prevent nested nvps
     static_assert( !std::is_base_of<NameValuePairCore, T>::value,
                    "Cannot pair a name to a NameValuePair" );

     NameValuePair & operator=( NameValuePair const & ) = delete;

   public:
     //! Constructs a new NameValuePair
     /*! @param n The name of the pair
         @param v The value to pair.  Ideally this should be an l-value reference so that
                  the value can be both loaded and saved to.  If you pass an r-value reference,
                  the NameValuePair will store a copy of it instead of a reference.  Thus you should
                  only pass r-values in cases where this makes sense, such as the result of some
                  size() call.
         @internal */
     NameValuePair(String n, T&& v ) : name(n), value(std::forward<T>(v)) {}

     String name;
     T value;
 };

template<class T> inline
NameValuePair<T> make_nvp(String name, T&& value){
	return {name, std::forward<T>(value)};
}

} /* namespace mycereal */

#define MACRO_NVP(T) ::ser::make_nvp(#T, T)
