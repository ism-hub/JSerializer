#pragma once

#include <ArduinoJson.h>
#include <vector>

namespace ser {

class JsonContex {
public:
	JsonObject _jsonObj;
	JsonArray _jsonArr;
	const String* nextName ;//next Key to read/write to
	std::vector<String> listOfNotAllowedKeys;// list of names that we not aloowed as keys in this context,
	int nextIndx=0;// like nextName only for arrays
	JsonContex(JsonObject jsonObj): _jsonObj(jsonObj), nextName(NULL) {}
	JsonContex(JsonArray jsonArr):_jsonArr(jsonArr), nextName(NULL) {}

	inline bool isJsonObject(){
		return _jsonArr.isNull();
	}

	/*void printAllKeys(){
		if(!isJsonObject()){
			//Serial.println(F("we not and object"));
			return;
		}
		for (auto kv : _jsonObj) {
			std::cout << kv.key().c_str() << std::endl;
		}
	}*/

	JsonContex createNestedObject() {
		if(isJsonObject())
			return JsonContex(_jsonObj.createNestedObject(*nextName));
		return JsonContex(_jsonArr.createNestedObject());
	}

	JsonContex createNestedArray() {
		if(isJsonObject())
			return _jsonObj.createNestedArray(*nextName);
		return _jsonArr.createNestedArray();
	}

	bool isNextKeyIsNull(){
		auto chp = getKeyValue();
		if(isJsonObject()){
			auto multiVal = (_jsonObj)[*(nextName)];
			return (multiVal.is<char*>() && multiVal.as<char*>() == nullptr);
		}
		else{
			auto multiVal = (_jsonArr)[nextIndx];
			return (multiVal.is<char*>() && multiVal.as<char*>() == nullptr);
		}
	}

	//revisit the reference stuff
	template<typename T>
	bool add(const T& t){
		if(isJsonObject()){
			return  _jsonObj[*nextName].set(t);
		}
		return _jsonArr.add(t);
	}

	template<typename TString>
	bool containsKey(const TString& key){
		if(!isJsonObject())
			return false;
		bool b = _jsonObj.containsKey(key);
		return b;
	}

	class opTgetKet{
	public:
		JsonContex& jCntx;
		String name;
		opTgetKet(JsonContex& jsonCntx, String name = "") : jCntx(jsonCntx), name(name) {}

		template<typename T>
		operator T(){
			if(jCntx.isJsonObject())
				return (jCntx._jsonObj)[*(name == "" ? jCntx.nextName : &name)];
			else
				return (jCntx._jsonArr)[jCntx.nextIndx];
		}
	};

	opTgetKet getKeyValue() {//TODO: implement properly
		return opTgetKet(*this);
	}

	opTgetKet getKeyValue(String name) {//TODO: implement properly
		return opTgetKet(*this, name);
	}

};

} 
