# uint239_t — A Custom 239-bit Unsigned Integer with ITMO Endian Representation

A from-scratch implementation of a 239-bit unsigned integer type (`uint239_t`) stored in a non-standard byte order called __ITMO Endian__. The project explores low-level bit manipulation, custom numeric type design, and arithmetic implementation without relying on standard containers or `std::bitset`.

## Overview

`uint239_t` is a 35-byte structure that encodes a 239-bit unsigned integer alongside a 35-bit shift value. The layout is deliberately non-standard:

- __239 significant bits__ — the actual numeric value.
- __35 service bits__ — one per byte, storing the shift amount in direct binary code.
- __6 padding bits__ — always zero, but they participate in circular shifting.

The shift defines how many positions the significant bits have been rotated left in a circular manner. This means the same numeric value can have multiple valid byte representations, all equivalent under the shift semantics.

The library provides a complete set of operations on this type, including conversions, arithmetic, comparisons, and stream output — all implemented with manual bitwise logic and raw arrays.

## Key Features

- __Conversions__
  - `FromInt(uint32_t value, uint32_t shift)` — constructs a `uint239_t` from a 32-bit unsigned integer with a given shift.
  - `FromString(const char* str, uint32_t shift)` — parses a decimal string into `uint239_t` using repeated division by 2.

- __Arithmetic operations__
  - Addition (`operator+`) — bitwise addition with carry propagation, shift = sum of operand shifts.
  - Subtraction (`operator-`) — bitwise subtraction with borrow, shift = difference of operand shifts.
  - Multiplication (`operator*`) — shift-and-add multiplication over binary strings, shift = sum of operand shifts.
  - Division (`operator/`) — binary long division via repeated subtraction, shift = difference of operand shifts.

- __Comparison and equality__
  - `operator==` and `operator!=` — compare values accounting for different shifts.
  - `operator>` — lexicographic comparison of significant bits.

- __Shift handling__
  - `GetShift(const uint239_t&)` — extracts the 35-bit shift value from the service bits.
  - `Shift(const uint239_t&, uint64_t)` — applies an additional circular shift to the value.
  - Internal helper `WithoutShift` — normalizes a value to zero shift for output and comparison.

- __Stream output__
  - `operator<<` — prints the normalized byte representation and the current shift.

- __Constraints respected__
  - No standard containers (`std::vector`, `std::list`, etc.).
  - No `std::bitset`.
  - Pure C++ with manual memory management and bitwise operations.

## Design Notes

The implementation treats the 35-byte array as a bit field with a non-trivial access pattern. Each byte holds 7 significant bits and 1 service bit, so reading or writing a logical bit requires computing both the byte index and the bit offset within that byte, while also accounting for the circular shift.

Arithmetic operations are implemented at the bit level:

- Addition and subtraction iterate over 239 bit positions, extracting the corresponding bits from each operand (adjusted for their shifts), performing the operation with carry/borrow, and writing the result back.
- Multiplication converts the operands to binary strings, performs shift-and-add multiplication, then packs the result back into the 35-byte format.
- Division uses binary long division: it builds the quotient bit by bit, subtracting the divisor from the remainder when possible.

The shift value itself is stored in the most significant bit of each byte, forming a 35-bit integer. All shift arithmetic is performed modulo \(2^{35}\) to match the storage width.

## Testing

The project includes a GoogleTest suite covering:

- Conversion from `uint32_t` and decimal strings.
- Bit-level representation checks against expected binary strings.
- Addition, subtraction, multiplication, and division across various shifts.
- Equality and inequality with different shift values.
- Edge cases such as zero, maximum values, and shift overflow.

Tests are built with CMake and run via CTest:

```bash
cmake --build . --target number_tests && ctest -V
```

### What This Project Demonstrates

- Deep understanding of binary representation and bitwise operations.
- Ability to design and implement a custom numeric type from scratch.
- Manual memory management and low-level data packing.
- Implementation of arithmetic algorithms (addition, subtraction, multiplication, division) without standard library support.
- Working within strict constraints (no STL containers, no `std::bitset`).
- Test-driven development with GoogleTest and CMake.