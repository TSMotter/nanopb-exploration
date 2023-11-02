# nanopb exploration

- This repository is a sandbox for exploring nanopb's functionalities and getting a better grasp on how it works.
- The exploration happens through the implementation of a handful of very verbose examples
- Each Example is a slightly more complex usecase then the previous one.
    - Even if your use case is simple, it's recommended to check the later examples for they might use more elegant and better fitting approaches for your usecase.
- All examples, when executed, generate binary files whose content represent the protobuf payload.
    - These files can be used with `protoc --decode` option to debug as is explained in section [Usage and troubleshooting](#Usage-and-troubleshooting)

## Examples:
- **ExampleA:** Simplest example;
- **ExampleB:** Streams custom callbacks;
- **ExampleC:** Fields callbacks for repeated fields;
- **ExampleD:** Fields callbacks for repeated fields + repeated sub messages;
- **ExampleE:** Protocol.options file + usage of fixed size char field;
- **ExampleF:** Use HDLC protocol to control message framing where the message payload is protobuf encoded;


## This project uses:
- **cmake** & **make** are used as build system
    - cmake version 3.22.1
    - GNU Make 4.3
- **clang-format** is used as formatter/code beautifier
    - Ubuntu clang-format version 14.0.0-1ubuntu1
- **yahdlc** is used as HDLC protocol library (already contained in the 'external' folder)
    - 1.1
- **nanopb** is used as a C implementation of Google's protobuf (already contained in the 'external' folder)
    - nanopb-0.4.5
- An NXP ring_buffer implementation is also imported into the workspace (already contained in the 'external' folder)


## How to compile and run the examples
- If you wish to use docker, build the image and launch it using the helper scripts inside of the `docker` folder
- The repository can be operated outside of the docker container if all the dependencies are met 
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


## Dealing with nanopb and protobuf
### Installation
- Installs nanopb generator script and protoc
```bash
sudo apt-get update
sudo apt-get install nanopb
which protoc
which nanopb_generator.py 
```

### Usage and troubleshooting
- protoc usage:
```bash
protoc --python_out=. protocol.proto
protoc --decode Sample exampleC/protocol.proto < protobuf_payload.bin
```
- nanopb usage:
```bash
nanopb_generator.py protocol.proto
```
- Helper tool to debug protobufs:
    - https://protobuf-decoder.netlify.app/