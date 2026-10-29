# ArgParser — Custom Command-Line Argument Parser in C++

A lightweight C++ library for parsing command-line arguments, built from scratch with a fluent, chainable API. The project provides a clean way to define string, integer, and boolean arguments, configure defaults, collect repeated values, handle positional arguments, and generate help output — all without external dependencies beyond the standard library and GoogleTest for testing.

## Overview

`ArgParser` is designed around two cooperating classes:

- __`ArgParser`__ — the public entry point. It registers arguments, parses `argv` / `vector<string>`, retrieves parsed values, and builds help descriptions.
- __`ComArg`__ — an argument descriptor that stores metadata (name, short name, description, type, multiplicity, default value) and the parsed values themselves.

The parser supports both short (`-p`) and long (`--param`) forms, `--key=value` syntax, clustered short flags (`-ac`), and repeated parsing with dynamically added arguments. It was built as a reusable library and can be embedded into other C++ command-line tools.

## Key Features

- __String, integer, and flag arguments__
  - `AddStringArgument`, `AddIntArgument`, `AddFlag`, and `AddHelp`
  - Short-name and long-name overloads for each argument type

- __Defaults and value storage__
  - `.Default(...)` for strings, integers, and booleans
  - `.StoreValue(...)` / `.StoreValues(...)` to write parsed results directly into external variables

- __Multi-value arguments__
  - `.MultiValue(min_count)` collects repeated occurrences of an argument
  - Minimum-count validation is enforced during parsing

- __Positional arguments__
  - `.Positional()` marks an argument to consume unnamed values from the command line
  - Works alongside named arguments and flags

- __Help generation__
  - `Help()` prints a formatted help message
  - `HelpDescription()` returns the help text as a string
  - Automatically includes argument names, types, descriptions, defaults, and multiplicity

- __Flexible parsing__
  - `Parse(const std::vector<std::string>&)` and `Parse(int argc, char** argv)`
  - Supports `--key=value`, `--key value`, `-k value`, and clustered short flags
  - Repeated parsing allows arguments to be added between parse calls

- __Tested with GoogleTest__
  - Covers strings, integers, flags, defaults, multi-values, positional arguments, help output, and repeated parsing

## Design Notes

The parser keeps argument definitions and parsed state separate: `ArgParser` owns a collection of `ComArg` descriptors, while `ComArg` encapsulates the type-specific logic and storage. This makes the public API expressive and chainable:

```cpp
ArgParser parser("My Parser");
std::vector<int> values;

parser.AddIntArgument('n', "number", "Some number");
parser.AddFlag('v', "verbose", "Enable verbose output");
parser.AddIntArgument("input")
      .MultiValue(1)
      .Positional()
      .StoreValues(values);

parser.Parse(argc, argv);
```

The implementation handles type checking, default values, minimum argument counts, and value propagation into user-provided variables. Help output is generated from the same metadata used during parsing, so documentation stays consistent with the actual argument definitions.

## Testing & Build

The library is built with __CMake__ and tested with __GoogleTest__ via __CTest__. The test suite exercises the public interface and verifies parsing behavior, error cases, default handling, multi-value collection, positional arguments, and help generation.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## What This Project Demonstrates

- Practical C++ class design with clear separation of concerns
- Fluent interface and method chaining
- Type-aware argument handling and validation
- Manual parsing of command-line syntax, including edge cases
- Test-driven development with GoogleTest
- CMake-based library structure and CTest integration