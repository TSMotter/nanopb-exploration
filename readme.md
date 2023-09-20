# nanopb exploration
___

# Examples:
- **ExampleA:** Simplest example;
- **ExampleB:** Streams custom callbacks;
- **ExampleC:** Fields callbacks for repeated fields;
- **ExampleD:** Fields callbacks for repeated fields + repeated sub messages;
- **ExampleE:** Protocol.options file + usage of fixed size char field;
- **ExampleF:** Use HDLC protocol to control message framing where the message peayload is protobuf encoded;

___

## This project uses:
- **cmake** & **make** are used as build system
- **clang-format** is used as formatter/code beautifier
- **yahdlc** is used as HDLC protocol library.
    - Link: [yahdlc - Yet Another HDLC](https://github.com/bang-olufsen/yahdlc/tree/master)
- **nanopb** is used as a C implementation of Google's protobuf

### Versions present in development machine:
- **cmake:** cmake version 3.22.1
- **make:** GNU Make 4.3
- **clang-format:** Ubuntu clang-format version 14.0.0-1ubuntu1
- **yahdlc** 1.1
- **nanopb** nanopb-0.4.5

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
./bbuild.sh -v -f -b -e exampleA # To format, build and execute the exampleA with verbose
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
git checkout nanopb-0.4.5
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
- protoc usage:
```bash
protoc --python_out=. protocol.proto
protoc --decode Sample exampleC/protocol.proto < binary
```
- nanopb usage:
```bash
nanopb_generator.py protocol.proto
```
- Helper tool to debug protobufs:
    - https://protobuf-decoder.netlify.app/