https://github.com/mikeloomisgg/cppack
# cppack
A modern (c++17 required) implementation of the [msgpack spec](https://github.com/msgpack/msgpack/blob/master/spec.md).

Msgpack is a binary serialization specification. It allows you to save and load application objects like classes and structs over networks, to files, and between programs and even different languages.

Check out [this blog](https://mikeloomisgg.github.io/2019-07-02-making-a-serialization-library/) for my rational creating this library.

## Features
- Fast and compact
- Full test coverage
- Easy to use
- Automatic type handling
- Open source MIT license
- Easy error handling

### Single Header only template library
Want to use this library? Just #include the header and you're good to go. Its less than 1000 lines of code.


### Cereal style packaging
Easily pack objects into byte arrays using a pack free function:

```c++
struct Person {
  std::string name;
  uint16_t age;
  std::vector<std::string> aliases;

  template<class T>
  void msgpack(T &pack) {
    pack(name, age, aliases);
  }
};

int main() {
    auto person = Person{"John", 22, {"Ripper", "Silverhand"}};

    auto data = msgpack::pack(person); // Pack your object
    auto john = msgpack::unpack<Person>(data.data()); // Unpack it
}
```

[More Examples](msgpack/tests/examples.cpp)


### Roadmap
- Support for extension types
  - The msgpack spec allows for additional types to be enumerated as Extensions. If reasonable use cases come about for this feature then it may be added.
- Name/value pairs
  - The msgpack spec uses the 'map' type differently than this library. This library implements maps in which key/value pairs must all have the same value types.
- Endian conversion shortcuts
  - On platforms that already hold types in big endian, the serialization could be optimized using type traits.
