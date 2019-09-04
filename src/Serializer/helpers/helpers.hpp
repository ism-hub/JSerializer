#pragma once

#include <vector>

namespace ser {

enum class sfinae {};

template <typename T>
struct has_load_method
{
    struct dummy { /* something */ };

    template <typename C, typename P>
    static auto test(P * p) -> decltype(std::declval<C>().load(*p), std::true_type());

    template <typename, typename>
    static std::false_type test(...);

    typedef decltype(test<T, dummy>(nullptr)) type;
    static const bool value = std::is_same<std::true_type, decltype(test<T, dummy>(nullptr))>::value;
};
//--------------------------------------------

//-------------------------------------------- detail output archive
template <typename T>
struct has_save_method
{
    struct dummy { /* something */ };

    template <typename C, typename P>
    static auto test(P * p) -> decltype(std::declval<C>().save(*p), std::true_type());

    template <typename, typename>
    static std::false_type test(...);

    typedef decltype(test<T, dummy>(nullptr)) type;
    static const bool value = std::is_same<std::true_type, decltype(test<T, dummy>(nullptr))>::value;
};
//--------------------------------------------

//--------------------------------------------

//--------------------------------------------

	template<class T>
	struct ptr{
		int id;
		std::shared_ptr<T> t;

		template<class Archive>
		void save(Archive& ar) const {
			ar.addProperties(make_nvp("id", id), make_nvp("data", *t));
		}

		template<class Archive>
		void save(Archive& ar) {
			ar.loadProperties(make_nvp("id", id), make_nvp("data", *t));
		}
	};


} 
