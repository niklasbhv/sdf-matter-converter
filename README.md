# sdf-matter-converter

Tool to convert between the Matter data model and the Semantic Definition Format.
The project is usable as a library or as a command line tool.

## Prerequisites

As this software uses the `std::unordered_map` data type in combination with incomplete types, it has certain requirements for the compiler version.

- GCC >= 12.1
- Clang >= 15
- ESP-IDF >= 5.1

If you want to use the validation functionality, the library `libxml2` has to be installed. For Ubuntu this can be done with:

```
sudo apt-get install libxml2-dev
```

## Library installation and building

Using CMake:

```
cmake .
```

Before building, check that all prerequisites are fulfilled.

## Documentation

The documentation for this software can be generated by using doxygen.
For Linux this can be done with:

```
doxygen
```

On Windows this can be done by using a tool like [Doxywizard](https://www.doxygen.nl/manual/doxywizard_usage.html)

## Using the command line tool

Run the converter with:

```
./sdf-matter-converter [flags]
```

Possible flags are:

| Parameter         | Arguments                          | Default |
|-------------------|------------------------------------|---------|
| `--matter-to-sdf` | -                                  | False   |
| `--sdf-to-matter` | -                                  | False   |
| `--roundtrip`     | -                                  | False   |
| `-sdf-model`      | Path to the sdf-model              | -       |
| `-sdf-mapping`    | Path to the sdf-mapping            | -       |
| `-device-xml`     | Path to the device type definition | -       |
| `-cluster-xml`    | Path to the cluster definition     | -       |
| `-validate`       | Path to the schema (JSON or XSD)   | -       |
| `-o, -output`     | Path for the output files          | -       |
| `-h, --help`      |                                    | -       |

## Using the library

The core library exposes two functions. One for converting SDF to the Matter data model and one for converting the Matter data model to SDF.
It is located in the `lib\converter` subfolder.
The documentation for the library can be generated by using the before mentioned generation process.

## Mappings Overview

### Matter &rarr; SDF

| Matter             | SDF         |
|--------------------|-------------|
| Node               | SDF-Model   |
| Device             | sdfThing    |
| Endpoint           | sdfThing    |
| Cluster            | sdfObject   |
| Attribute          | sdfProperty |
| Command            | sdfAction   |
| Event              | sdfEvent    |
| Global Matter type | sdfData     |

### SDF &rarr; Matter

| SDF              | Matter                     |
|------------------|----------------------------|
| sdfThing         | Endpoint                   |
| sdfObject        | Cluster                    |
| sdfProperty      | Attribute                  |
| sdfAction        | Command                    |
| `sdfInputData`   | Client command data fields |
| `sdfOutputData`  | Server command data fields |
| sdfEvent         | Event                      |
| `sdfOutputData`  | Event data field           |
| sdfData          | Matter data type           |

## License

This project is licensed under the [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0).