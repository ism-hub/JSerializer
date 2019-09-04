#pragma once

#include <functional>
#include <vector>
#include "../serialization/JsonContex.hpp"

namespace ser {

class DoNothingMappingFile {
public:
	template<typename ... Args>
	void Model2Json(Args& ...) {}

	template<typename ... Args>
	void Json2Model(Args& ...) {}

	template<typename T, typename A>
	std::function<T* (std::vector<T, A>&, ser::JsonContex&)> getVectorComperator(std::vector<T, A>&, ser::JsonContex&)
	{return std::function<T* (std::vector<T, A>&, ser::JsonContex&)>();}
};

}
