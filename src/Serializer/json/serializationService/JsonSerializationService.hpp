#pragma once

#include <vector>
#include <ArduinoJson.h>
#include <WString.h>
#include <algorithm>
#include "../../helpers/NameValuePair.hpp"
#include "../archive/JsonSaveArchive.hpp"
#include "../archive/JsonLoadArchive.hpp"

namespace ser {

//-------------------------------------------- detail input archive

class SerializationService{
	std::vector<ser::JsonContex> contexList;
	DynamicJsonDocument jsonDoc;//TODO: need version 6.6 to not specify that size
	std::vector<std::function<void()>> callbacks;

public:
	SerializationService() : jsonDoc(DynamicJsonDocument(4096)) {//TODO: need version 6.6 to not specify that size
	}
	~SerializationService(){
	}

	template<typename ModelType, typename instream, typename MappingFile>
	void Model2Json(const ModelType& model, instream& stream, MappingFile& mappingFile){//const reference can take rvalues
		ser::JsonSaveArchive<MappingFile> saveArchive(jsonDoc, contexList, mappingFile);

		//JsonObject root = saveArchive.Model2Json(model);
	//	delay(0);
		saveArchive.Model2Json(model);
		serializeJson(jsonDoc, stream);
		//root.printTo(stream);
	//	_saveArchive.clear();
	}

	//loading only the keys that are in the json if no key is present we just ignore the property, even if it has nameValuePair
	template<typename ModelType, typename outstream, typename MappingFile>
	void Json2Model(ModelType& model,  outstream& json, MappingFile& mappingFile){//we are taking references and fill them
		ser::JsonLoadArchive<MappingFile> loadArchive(jsonDoc, contexList, callbacks, mappingFile);
	//	delay(0);
		loadArchive.Json2Model(model, json);
		//_loadArchive.clear();
	}

	template<typename ModelType, typename instream, typename MappingFile>
	void serialize(const ModelType& model, instream& stream, MappingFile& mappingFile){
		Model2Json(model, stream, mappingFile);
	}

};

} 
