# Half & Single Precision Floating-Point Arithmetic Library

A C++ implementation of IEEE 754-style __half-precision (binary16)__ and __single-precision (binary32)__ floating-point arithmetic, built from scratch with manual bit-level handling. The library supports parsing hexadecimal operands, arithmetic operations, configurable rounding modes, denormal numbers, infinities, NaNs, signed zero, overflow handling, and both fused and separate multiply-add variants.

## Overview

The project provides two numeric classes:

- __`HalfPrecision`__ — a 16-bit floating-point type with 1 sign bit, 5 exponent bits, and 10 mantissa bits.
- __`SinglePrecision`__ — a 32-bit floating-point type with 1 sign bit, 8 exponent bits, and 23 mantissa bits.

Both types store their raw representation in a union with bitfields, allowing direct access to sign, exponent, and mantissa. Arithmetic is implemented manually rather than delegated to hardware floating-point operations, which makes the project a practical exercise in binary representation, normalization, rounding, and IEEE 754 edge cases.

## Key Features

### Construction and conversion
- Construct from raw bit patterns.
- Construct from separate mantissa, exponent, and sign fields.
- Parse hexadecimal strings such as `0x3c00` and `0x3f800000`.
- Convert back to a hexadecimal string representation.
- Produce a canonical hexadecimal floating-point view such as `0x1.000000p+0`, `nan`, `inf`, or `-inf`.

### Classification
- Detect zero and signed zero.
- Detect infinity.
- Detect denormalized values.
- Detect NaN.

### Arithmetic operations
- Addition.
- Subtraction.
- Multiplication.
- Division.
- Generic operation dispatch through a single `operation` method.
- Fused multiply-add (`fma`).
- Separate multiply-add (`mad`) with rounding applied to the multiplication before the addition.

### Rounding modes
All arithmetic operations accept a rounding mode:
- `0` — toward zero.
- `1` — toward nearest, ties to even.
- `2` — toward positive infinity.
- `3` — toward negative infinity.

Rounding is applied to the result mantissa and supports correct handling of overflow, including transitions to maximum finite values or infinity depending on the rounding mode and sign.

### Edge-case handling
- Signed zero.
- NaN propagation.
- Infinity arithmetic.
- Denormalized operands and results.
- Overflow and underflow behavior.
- Division by zero.
- Invalid operations such as `0 / 0`, `inf / inf`, `0 * inf`, and `inf - inf`.

### Command-line interface
The project includes a CLI for evaluating expressions:

```bash
./fp_calc f 0 0xB9CD542
./fp_calc f 1 + 0x3f800000 0x40000000
./fp_calc h 1 * 0x3c00 0x4000
```

Where:
- `f` selects single precision, `h` selects half precision.
- The second argument is the rounding mode.
- The remaining arguments define either a single value or an operation with two operands.

### Automated test harness
The project includes a test runner that reads floating-point test vectors and expected answers from files, executes the corresponding conversions or arithmetic operations, and reports pass/fail statistics with detailed failure logs.

## Design Notes

The implementation separates the numeric representation from the arithmetic logic:

- Each precision has its own class with the same public interface.
- Shared operations follow the same structure: classification, special-case handling, exponent alignment, mantissa arithmetic, normalization, and rounding.
- Addition and subtraction align exponents before operating on mantissas.
- Multiplication works on extended mantissas and then normalizes and rounds the result.
- Division uses binary long division with remainder tracking to produce the quotient bits.
- Rounding uses guard, round, and sticky-style information derived from discarded bits and the chosen rounding mode.
- Overflow handling maps results to the largest finite value or infinity depending on the rounding direction.

The code avoids relying on the platform’s native floating-point arithmetic for the core operations, making the representation and behavior explicit and testable.

## What This Project Demonstrates

- Practical understanding of IEEE 754 floating-point representation.
- Manual implementation of binary16 and binary32 arithmetic.
- Bit-level manipulation of sign, exponent, and mantissa fields.
- Implementation of multiple rounding modes and overflow policies.
- Handling of denormal numbers, NaNs, infinities, and signed zero.
- Clean C++ class design with a consistent interface across two numeric types.
- Command-line tooling and file-driven automated regression testing.
