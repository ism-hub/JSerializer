#pragma once

#include "../../helpers/helpers.hpp"
#include "../../helpers/NameValuePair.hpp"
#include "../serialization/JsonContex.hpp"
#include <ArduinoJson.hpp>
#include <algorithm>
#include <memory>
//#include <ObserverDesignPattern/Property.hpp>
#include <map>

namespace ser {

template<class MapF>
class JsonSaveArchive {
	DynamicJsonDocument& jsonDoc;
	std::vector<ser::JsonContex>& contexList;
	MapF& mappingFile;

	int idCounter = 1;
	std::map<void*, int> ptrSerData;// key: id, value: 0 means we didn't serialize the data yet.
public:

	JsonSaveArchive(DynamicJsonDocument& jsonDoc, std::vector<ser::JsonContex>& contexList, MapF& mappingFile)
	 : jsonDoc(jsonDoc), contexList(contexList), mappingFile(mappingFile) {
	}
	~JsonSaveArchive() {
		clear();
	}

	//don't forget to call clean after using this method!!
	template<typename ModelType>
	JsonObject Model2Json(const ModelType& model){//const reference can take rvalues
		// creating the json contex
		JsonObject root = jsonDoc.to<JsonObject>();
		contexList.push_back(root);

		//starting the serialization of model by serializing his properties
		mappingFile.Model2Json(model, *this);//checking to see if the user want to override our default implementation (by manually adding some keys)
		//calling to start to serialize the model properties (adding the keys our user didn't add)
		model.save( *this);

		return root;
	}

	void clear(){
		contexList.clear();
		jsonDoc.clear();
	}

	JsonContex& getCurrentContex(){
		return contexList.back();
	}

	//! Unwinds to process all data
	template <class T, class ... Other> inline
	void addProperties(const T& head, const Other& ... tail )
	{
		addProperty(head);
		addProperties(tail ... );
	}

	void addProperties(){}


	//catching all the NameValuePairs. we save its name as the nextName for the json context
	//if we handle a NameValuePair it means that we inside an object and we want to add to it a property named head.name
	//if a property names like that already exists we dont cont (the mapping file already custom added it)
	template<class T>
	void addProperty(const NameValuePair<T>& head){
		/*Serial.print(F("Inside addProperty(const NameValuePair... the name is:"));Serial.println(head.name);
		Serial.print(F("settings heap size: "));Serial.println(ESP.getFreeHeap());
		Serial.print(F("num of elements in the contexList: "));Serial.println(contexList.size());
		Serial.print(F("capacity of contexList: "));Serial.println(contexList.capacity());
		Serial.print(F("jsonDoc.size(): "));Serial.println(jsonDoc.size());*/
		auto& contex = contexList.back();
		contex.nextName = &head.name;//save the name

		const std::vector<String>& bannedKeys = contex.listOfNotAllowedKeys;
		if(contex.containsKey(head.name) || std::find(bannedKeys.begin(), bannedKeys.end(), head.name) != bannedKeys.end())//if we already have this key we dont handle this object
			return;

		addProperty(head.value);//continue
	}
protected:
	//! property with save method - the property is an object,
	template<class T, typename std::enable_if<has_save_method<T>::value ,  sfinae>::type = {}>
	void addProperty(const T& t) {
		//getting the correct contex
		auto& contex = contexList.back();

		//adding this property to the current contex
		//JsonContex propContex = contex.createNestedObject();//add here a destructor that pops the element from the list by itself?
		contexList.push_back(contex.createNestedObject());
		mappingFile.Model2Json(t, *this);
		t.save(*this);//adding this property properties (basically like calling 'addProperties' with all the properties the user want)
		contexList.pop_back();//must to remember to pot the context after we done with it (do some auto pop context thing on scope destruction..)
	}

	//int/string/double/bool properties;
	template<class T, typename std::enable_if<std::is_arithmetic<typename std::remove_reference<T>::type>::value || std::is_same<typename std::remove_reference<T>::type, String>::value   , sfinae>::type = {}>
	void addProperty(const T& t) {
		auto& contex = contexList.back();

		//delay(0); TODO: maybe uncomment it
		//adding this property to the current contex
		if(!contex.add(t)){
		/*	Serial.println(F("ERROR: failed to set a key in enable_if<std::is_arithmetic"));
			//Serial.println(__PRETTY_FUNCTION__);
			Serial.print(F("is the contex which we add the value is Object: "));Serial.println(contex.isJsonObject());

			if(contex.isNextKeyIsNull()){
				Serial.print(F("The next key is null "));
			}else{
				Serial.print(F("The next key is: "));Serial.println(*contex.nextName);
			}
			Serial.print(F("The value: "));Serial.println(t);
			contex.printAllKeys();*/ //TODO: Handle errors
		}
			//std::cout << "ERROR: failed to set a key in enable_if<std::is_arithmetic" << std::endl;
	}


	//NameValuePair - means that we are in objJson contex, vector mean that the property is an array
	template<class T, typename A>
	void addProperty(std::vector<T, A> const &  vec) {

		//getting the correct contex
		auto& contex = contexList.back();

		//creating new array contex
		JsonContex propJsonArrCntx = contex.createNestedArray();
		contexList.push_back(propJsonArrCntx);

		//adding all the array items
		for(const auto& t : vec){
			addProperty(t);
		}

		contexList.pop_back();
	}

	//handle property type (invisible)
//	template <typename T, typename MappingFileType>
//	void addProperty(Model::Property<T> const & prop, MappingFileType& mappingFile) {
//		addProperty(prop.get(), mappingFile);
//	}

	//handle shared pointer type (invisible)
	template <typename T>
	void addProperty(const std::shared_ptr<T>& sharedPtr) {
		if(sharedPtr != nullptr){
			if(ptrSerData[sharedPtr.get()])
				addProperty(make_nvp("id", ptrSerData[sharedPtr.get()]));
			else{
				ptrSerData[sharedPtr.get()] = idCounter++;
				// auto p = ptr<T>(sharedPtr);
				addProperty(make_nvp("ptr", ptr<T>{idCounter - 1, sharedPtr}));
			}
		}
		else{// if we null ptr we need to add null property
			if(!contexList.back().add((char*)0)){}
				//Serial.println(F("ERROR: failed to set null value")); //TODO: handle errors
		}
	}

};

} 
