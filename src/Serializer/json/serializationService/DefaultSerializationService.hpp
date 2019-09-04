#pragma once

#include "JsonSerializationService.hpp"
#include "../mappingFile/DoNothingMappingFile.hpp"

namespace ser {

class DefaultSerializationService {
	std::shared_ptr<ser::SerializationService> serializationService = nullptr;
	ser::DoNothingMappingFile mappingFile;
public:
	DefaultSerializationService(std::shared_ptr<ser::SerializationService> serializationService): serializationService(serializationService) {}
	~DefaultSerializationService() {}

	template<typename ModelType, typename instream>
	void Model2Json(const ModelType& model, instream& stream){//const reference can take rvalues
		serializationService->Model2Json(model, stream, mappingFile);
	}

	//loading only the keys that are in the json if no key is present we just ignore the property, even if it has nameValuePair
	template<typename ModelType, typename outstream>
	void Json2Model(ModelType& model, outstream& json){//we are taking references and fill them
		serializationService->Json2Model(model, json, mappingFile);
	}

	template<typename ModelType, typename instream>
	void serialize(const ModelType& model, instream& stream){
		Model2Json(model, stream);
	}
};

} /* namespace DALModule */
