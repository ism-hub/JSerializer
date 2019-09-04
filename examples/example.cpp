/**
 * We will use the serialization to serialize/deserialize a simple model to two different Json formats - 
 * 1. Format for flash storage.
 * 2. Format for in REST style api (the partial deserialization feature shown here).
 * 
 * --------------------------
 * 
 * example - 
 * for flash the school object can be serialized to -
 *      {"name":"A++","location":"Canada","students":[{"id":1,"data":{"name":"Alice","id":90}},{"id":2,"data":{"name":"Dave","id":91}}],"teachers":[{"id":3,"data":{"name":"Bob","students":[1,2]}}]
 * 
 * for REST api it can be serialized to -
 *     {"links":[{"rel":"students","href":"/students"},{"rel":"teachers","href":"/teachers"}],"name":"A++","location":"Canada"}
 *                                         
 * 
 * partial update for the REST api can be sending an http-update with {"name":"Get A++ in C++"} will only update the name 
 * (deserialize into an existing object and will only deserialize the name)
 * 
 * --------------------------
 * 
 * We achieve this functionality by letting the user define the serialization for the object tree, for example -
 * The user can manually insert some keys for an object (in the load proccess) (and then ask the serializer to ignore these keys (because the user already inserted them))
 * The user can manually deserialize the things he wants in the object tree.
 * 
 * --------------------------
 * 
 * we can look at the Json serialization service as having default serialization while we can modify the default serialization with mapping-files.
 * 
 */
#include <iostream>
#include <Serializer.hpp>
#include <memory>
#include <vector>
#include <assert.h>

//Model
class Student {
    std::string _name;
    int _id;
public:
    template<class S>
    Student(S&& name, int id) : _name(std::forward<S>(name)), _id(id) {}
    Student(){}

    template<class Archive>
    void save(Archive& ar) const {
        ar.addProperties(ser::make_nvp("name", _name), ser::make_nvp("id", _id));
    }

    template<class Archive>
    void load(Archive& ar) {
         ar.loadProperties(ser::make_nvp("name", _name), ser::make_nvp("id", _id));
    }
};

class Teacher{
    std::string _name;
    std::vector<std::shared_ptr<Student>> _students;
public:
    template<class S, class VEC>
    Teacher(S&& name, VEC&& students) : _name(std::forward<S>(name)), _students(std::forward<VEC>(students)) {}
    Teacher(){}

    template<class Archive>
    void save(Archive& ar) const {
        ar.addProperties(ser::make_nvp("name", _name), ser::make_nvp("students", _students));
    }

    template<class Archive>
    void load(Archive& ar) {
        ar.loadProperties(ser::make_nvp("name", _name));
        ar.loadProperties(ser::make_nvp("students", _students));
    }
};

class School {
    std::string _name;
    std::string _location ;
    std::vector<std::shared_ptr<Student>> _students;
    std::vector<std::shared_ptr<Teacher>> _teachers;
public:
    template<class S, class S2, class VEC, class VEC2>
    School(S&& name, S2&& location, VEC&& teachers, VEC2&& students)
     : _name(std::forward<S>(name)), _location(std::forward<S2>(location)), _teachers(std::forward<VEC>(teachers)),  _students(std::forward<VEC2>(students)) {}
     School(){}

    template<class Archive>
    void save(Archive& ar) const {
        ar.addProperties(ser::make_nvp("name", _name), ser::make_nvp("location", _location),  ser::make_nvp("students", _students), ser::make_nvp("teachers", _teachers));
    }

    template<class Archive>
    void load(Archive& ar) {
         ar.loadProperties(ser::make_nvp("name", _name), ser::make_nvp("location", _location), ser::make_nvp("students", _students), ser::make_nvp("teachers", _teachers));
    }
};

//Changing the default serialization (using a mapping file) so it will serialize as a REST API
class SchoolRestMappingFile{
public:
    template<typename ... Args>
	void Model2Json(Args& ...) {}

    template<class Archive>
	void Model2Json(const School& school, Archive& ar) {
        std::vector<ser::Link> links;
        ser::JsonContex& context = ar.getCurrentContex();
        links.push_back(ser::Link("students", "/students"));
        links.push_back(ser::Link("teachers", "/teachers"));
        context.listOfNotAllowedKeys.push_back("students");
        context.listOfNotAllowedKeys.push_back("teachers");
        ar.addProperties(MACRO_NVP(links));
    }

	template<typename ... Args>
	void Json2Model(Args& ...) {}

	template<typename T, typename A>
	std::function<T* (std::vector<T, A>&, ser::JsonContex&)> getVectorComperator(std::vector<T, A>&, ser::JsonContex&)
	{return std::function<T* (std::vector<T, A>&, ser::JsonContex&)>();}
};

int main(){
    using Students = std::vector<std::shared_ptr<Student>>;
    using Teachers = std::vector<std::shared_ptr<Teacher>>;

    //Creating the model
    auto alice = std::make_shared<Student>("Alice", 90);
    auto dave = std::make_shared<Student>("Dave", 91);
    auto bob = std::make_shared<Teacher>("Bob", Students{alice, dave});
    School school("A++", "Canada", Teachers{bob}, Students{alice, dave});

    //creating serialization service
    auto serializationServ = std::make_shared<ser::SerializationService>();
    ser::DefaultSerializationService defaultSerializer(serializationServ);// wrapping SerializationService for ease of use.

    //default serialization
    std::string serializedDefault = "";
    defaultSerializer.Model2Json(school, serializedDefault);
    std::cout << serializedDefault << std::endl;//{"name":"A++","location":"Canada","students":[{"id":1,"data":{"name":"Alice","id":90}},{"id":2,"data":{"name":"Dave","id":91}}],"teachers":[{"id":3,"data":{"name":"Bob","students":[1,2]}}]}

    //default deserialization
    School school2;
    defaultSerializer.Json2Model(school2, "{\"name\":\"A++\",\"location\":\"Canada\",\"students\":[{\"id\":1,\"data\":{\"name\":\"Alice\",\"id\":90}},{\"id\":2,\"data\":{\"name\":\"Dave\",\"id\":91}}],\"teachers\":[{\"id\":3,\"data\":{\"name\":\"Bob\",\"students\":[1,2]}}]}");
    
    //checking to see if the same model after deserialization
    std::string serializedDefault2 = "";
    defaultSerializer.Model2Json(school2, serializedDefault2);
    assert(serializedDefault2 == serializedDefault);

    //Using mapping file for REST API (modifying the default serialization)
    SchoolRestMappingFile restMappingFile;
    std::string restSerialized = "";
    serializationServ->Model2Json(school, restSerialized, restMappingFile);
    std::cout << restSerialized << std::endl;//{"links":[{"rel":"students","href":"/students"},{"rel":"teachers","href":"/teachers"}],"name":"A++","location":"Canada"}

    //updating the model/part of the model 
    std::string afterUpdate = "";
    defaultSerializer.Json2Model(school, "{\"location\":\"Arctic\"}");//(we can use the REST mapping in this case aswell)
    defaultSerializer.Model2Json(school, afterUpdate);
    std::cout << afterUpdate << std::endl;//{"name":"A++","location":"Arctic","students":[{"id":1,"data":{"name":"Alice","id":90}},{"id":2,"data":{"name":"Dave","id":91}}],"teachers":[{"id":3,"data":{"name":"Bob","students":[1,2]}}]}
}