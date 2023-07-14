# nanopb exploration
___

## This project uses:
- **cmake** & **make** are used as build system
- **clang-format** is used as formatter/code beautifier
- **doxygen** is used to generate documentation
- **gtest** is downloaded in build-time into the `build/_deps` folder and provides infrastructure for unit tests
    - Link: [github.com/google/googletest](https://github.com/google/googletest)

### Versions present in development machine:
- **cmake:** cmake version 3.22.1
- **make:** GNU Make 4.3
- **clang-format:** Ubuntu clang-format version 14.0.0-1ubuntu1
- **doxygen:** 1.9.1
- **gtest:** 1.13.0

___

## How to operate the repository
- To format the complete code base with clang-format:
```bash
./bbuild.sh -f
```

- To build the exampleA:
```bash
./bbuild.sh -b exampleA
```

- To rebuild the exampleA:
```bash
./bbuild.sh -r exampleA
```

- To execute the built binary of the exampleA:
```bash
./bbuild.sh -e exampleA
```

- To format, build and execute the exampleA:
```bash
./bbuild.sh -f -b -e exampleA
```

- Examples: 
```bash
./bbuild.sh -f -b -e exampleA # To format, build and execute the exampleA
./bbuild.sh -f -r -e test # To format, build and execute the unit tests
```

- To generate doxygen documentation (generated docs will be available at `build/documentation/html/index.html`):
```bash
./bbuild.sh -r documentation
```

- To check all options available:
```bash
./bbuild.sh --help
```

___

## Installation
- Installs nanopb generator script and protoc
```bash
sudo apt-get update
sudo apt-get install nanopb
which protoc
which nanopb_generator.py 
```
- Installs nanopb libraries in the system
```bash
git clone git@github.com:nanopb/nanopb.git
cd nanopb/
git checkout <version>
cmake -S . -B build
cmake --build build --target help
cmake --build build
cmake --install build --prefix ~/usr/
sudo mv build/CMakeFiles/protobuf-nanopb-static.dir/pb*.o /usr/lib/x86_64-linux-gnu/
```
- Despite this procedure, I'm getting a link error
- The easiest approach for now was to incorporate nanopb's source into the "external" folder
- In a future version could even make cmake create pb files on every build, which is more elegant

## Usage
- For python code:
```bash
protoc --python_out=. protocol.proto
```
- For C code:
```bash
nanopb_generator.py protocol.proto
```

# Examples:
- ExampleA: Simplest example
- ExampleB: Basic functionality of streams custom callbacks