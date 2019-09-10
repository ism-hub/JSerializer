# JSerializer
Json serializer, the main feature is the possibility to change the default de/serialization by using an external 'mapping file', for example, to create a REST API for your model you can define the small changes needed, in a concise manner, to the default de/serialization in a mapping file (and you will be able to update you model according to the REST specs). 
Using [ArduinoJson](https://github.com/bblanchon/ArduinoJson) and inspired by [Cereal](https://github.com/USCiLab/cereal).

# Code Sample
See the example in the examples folder for a much more detailed example (with the mapping-file feature), because the API will be greatly changed I won't document it all here, this will get updated after the rework.
```C++
class Student {
    string _name;
    int _id;
public:
    Student(string name, int id) : _name(name), _id(id) {}
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

int main(){
    Student bob("Bob", 1);
    //creating serialization service
    auto serializationServ = std::make_shared<ser::SerializationService>();
    ser::DefaultSerializationService defaultSerializer(serializationServ);// wrapping the SerializationService for ease of use.

    //default serialization
    std::string serializedDefault = "";
    defaultSerializer.Model2Json(bob, serializedDefault);
    std::cout << serializedDefault << std::endl;//{"name":"Bob","id":1}

    return 0;    
}
```


