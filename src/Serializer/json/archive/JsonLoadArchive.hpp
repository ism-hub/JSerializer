#pragma once

#include "../../helpers/helpers.hpp"
#include "../../helpers/NameValuePair.hpp"
#include "../serialization/JsonContex.hpp"
#include <ArduinoJson.hpp>
#include <algorithm>
//#include <iostream>
#include <memory>
//#include <ObserverDesignPattern/Property.hpp>
#include <functional>
#include <map>


namespace ser {

template<class MapF>
class JsonLoadArchive {
	DynamicJsonDocument& jsonDoc;
	std::vector<JsonContex>& contexList;
	std::vector<std::function<void()>>& callbacks;//at the end of the serialization we call them
	std::vector<std::function<void*()>> lastCallbacks;//TODO: rework this whole thing
	MapF& mappingFile;
public:
	//JsonLoadArchive(MappingFileType mappingFile):_mappingFile(mappingFile){
//		Serial.printf("settings heap size: %u\n", ESP.getFreeHeap());
//		Serial.println("--------------------------- JsonLoadArchive CTOR");
//		Serial.println(__PRETTY_FUNCTION__);
	//}
	JsonLoadArchive(DynamicJsonDocument& jsonDoc, std::vector<JsonContex>& contexList, std::vector<std::function<void()>>& callbacks, MapF& mappingFile)
	 : jsonDoc(jsonDoc), contexList(contexList), callbacks(callbacks), mappingFile(mappingFile){}
	~JsonLoadArchive() {
		clear();
	}

	//loading only the keys that are in the json if no key is present we just ignore the property, even if it has nameValuePair
	template<typename ModelType, typename outstream>
	void Json2Model(ModelType& model, outstream& json){//we are taking references and fill them
		//JsonObject root = jsonDoc.parseObject(json);
		deserializeJson(jsonDoc, json);
		contexList.push_back(jsonDoc.as<JsonObject>());

		//starting the serialization of model by serializing his properties
		mappingFile.Json2Model(model, *this);//checking to see if the user want to override our default implementation (he can add keys to the banned keys list of the contex and we will ignore them when loading)
		//calling to start to serialize the model properties (adding the keys our user didn't add)
		model.load(*this);

		for(auto& fnc : callbacks)
			fnc();
		for(auto& fnc : lastCallbacks)
			fnc();
	}

	void clear(){
		contexList.clear();
		jsonDoc.clear();
		callbacks.clear();
	}

	void addCallBack(std::function<void()> fnc){
		callbacks.push_back(fnc);
	}

	JsonContex& getCurrentContex(){
		return contexList.back();
	}

	//! Unwinds to process all data
	template <class T, class ... Other> inline
	void loadProperties(T&& head, Other&& ... tail )
	{
		loadProperty(std::forward<T>(head));
		loadProperties(std::forward<Other>(tail) ... );
	}

	void loadProperties(){}

	template<class T>
	void loadProperty(NameValuePair<T>&& head){
		auto& contex = contexList.back();
		contex.nextName = &head.name;//save the name

		//contex.printAllKeys();

		const std::vector<String>& bannedKeys = contex.listOfNotAllowedKeys;
		if(std::find(bannedKeys.begin(), bannedKeys.end(), head.name) != bannedKeys.end())//if we already have this key we dont handle this object
			return;
		if(!contex.containsKey(head.name))
			return;

		loadProperty(head.value);//continue
	}

protected:

	//int/string/double/bool properties; we not nameValuePair so we are inside an array
	template<class T, typename std::enable_if<std::is_arithmetic<typename std::remove_reference<T>::type>::value || std::is_same<typename std::remove_reference<T>::type, String>::value   , sfinae>::type = {}>
	void loadProperty(T& t) {
		auto& contex = contexList.back();
		t = contex.getKeyValue().operator T();
	}


	//! property with save method - the property is an object, we didn't get NameValuePair so we are inside array
	template<class T, typename std::enable_if<has_load_method<T>::value ,  sfinae>::type = {}>
	void loadProperty(T& t) {
		//the contex in which t is defined
		auto& contex = contexList.back();


		JsonObject tJson = contex.getKeyValue().operator JsonObject();
		contexList.push_back(tJson);//getting t contex (t in json format)
		mappingFile.Json2Model(t, *this);//if we want to manually handle part of the deserialization (twik the default behaviour hook)
		t.load(*this);//basically calling loadProperties function with the nameValuePairs of the properties we wants from the json and where to put them (refernce)
		contexList.pop_back();//must to remember to pot the context after we done with it (after we done loading that obj)
	}

	template<class T, class M>
	struct fml{
		T t;
		M& vec;
		bool firstRun = true;

		fml(M& vec) : vec(vec){}

		T* operator()(){
			if(!firstRun){
				vec.push_back(std::move(t));
			}
			firstRun = false;
			return &t;
		}
	};

	//NameValuePair - means that we are in objJson contex, vector mean that the property is an array
	template<class T, typename A>
	void loadProperty(std::vector<T, A>&  vec) {

		//the context where the vector is a property
		auto& contex = contexList.back();

		//the contx which is the Json of the vector
		JsonArray jArr = contex.getKeyValue().operator JsonArray();
		contexList.push_back(jArr);

		//comperator - can check if an json entry of the array have a corresponding element in the list (if the json have instace of it in the list and  we need only to update that instance)
		std::function<T* (std::vector<T, A>&, JsonContex&)> comperator;
		comperator = mappingFile.getVectorComperator(vec, contexList.back());//this func can also change vec (dlete/add items)

		//add what is the json, (not touching what is already inside) we can't only update those who are there or delete those who aren't etc.. cause we don't know the keys of the items, the mappingFiles prob deal with it, maybe after i will do the i will return to this TODO: revisit after the Mapping files for the load section is over
		for(int i = 0; i < jArr.size(); i++){
			contexList.back().nextIndx = i;
			auto jsonVar = jArr[i];

			fml<T, std::vector<T, A>> f(vec);
			
			lastCallbacks.push_back(f);
			T& val = *(T*)lastCallbacks.back()();
			//T& val = savedValueLmb();
			

			if(jsonVar.is<JsonObject>()){
				contexList.push_back(jsonVar.as<JsonObject>());
			//	lastCallbacks.push_back(std::move(savedValueLmb));
				loadProperty(val);
				contexList.pop_back();
			}else if(jsonVar.is<JsonArray>()){
				contexList.push_back(jsonVar.as<JsonArray>());
			//	lastCallbacks.push_back(std::move(savedValueLmb));
				loadProperty(val);
				contexList.pop_back();
			}else{
				//vec.push_back(std::make_shared<T>("sss", 4));
			//	lastCallbacks.push_back(std::move(savedValueLmb));
				loadProperty(val);
			}

			
			/*T* pt = NULL;
			if(comperator != nullptr){ //not null -> contexList.back().getKeyValue() is a JsonObject
				JsonObject tJson = contexList.back().getKeyValue().operator JsonObject();
				JsonContex jContex = tJson;
				pt = comperator(vec, jContex);
			}
			if(pt != NULL){//TODO: add something in the lines of contexList.back()[i]
				loadProperty(*pt);
			}else{
				vec.push_back(T());
				loadProperty(vec.back());
			}*/
		}
		contexList.pop_back();
	}

	//handle property type (invisible)
//	template <typename T, typename MappingFile>
//	void loadProperty(Model::Property<T>& prop, MappingFile& mappingFile) {
//		T t(prop.get());//copy ctor of that obj. maybe change the whole property notification concept
//		loadProperty(t, mappingFile);
//		prop = t;
//	}

	std::map<int, std::shared_ptr<void>> ptrMap;//mapping ptr id to its 
	
	//handle shared pointer type (invisible)
	template <typename T>
	void loadProperty(std::shared_ptr<T>& sharedPtr) {
		auto& contex = contexList.back();

		//contex.printAllKeys();

		if(!contex.isJsonObject() && contex.isNextKeyIsNull()){//maybe we pointing to some value, because we 'merge load' we need to update that to null
			sharedPtr = nullptr;
			return;
		}
		int id;
		if(contex.isJsonObject())
			id = contex.getKeyValue("id").operator int();
		else
			id = contex.getKeyValue().operator int();
		if(contex.containsKey("data")){
			if(sharedPtr == nullptr)//we assume it's ok to create T (call its CTOR) if the user didn't create one
				sharedPtr = std::make_shared<T>();
	//		sharedPtr = std::make_shared<T>();
			//contexList.push_back(contex.getKeyValue("data").operator JsonObject());
			String data =  "data";
			contex.nextName = &data;//contexList.back().nextName = &data;
			loadProperty(*sharedPtr);
			//contexList.pop_back();
			ptrMap[id] = sharedPtr;
		}else{
			//sharedPtr = std::make_shared<T>("aaa", 32);
			auto lmb = [=, &sharedPtr](){sharedPtr = std::static_pointer_cast<T>(this->ptrMap[id]);};//someone else will fill it to the map and we will use it.
			callbacks.push_back(lmb);
		}

		
	}
};

} 
