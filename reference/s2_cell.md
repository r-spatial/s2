# Create S2 Cell vectors

The S2 cell indexing system forms the basis for spatial indexing in the
S2 library. On their own, S2 cells can represent points or areas. As a
union, a vector of S2 cells can approximate a line or polygon. These
functions allow direct access to the S2 cell indexing system and are
designed to have minimal overhead such that looping and recursion have
acceptable performance when used within R code.

## Usage

``` r
s2_cell(x = character())

s2_cell_sentinel()

s2_cell_invalid()

as_s2_cell(x, ...)

# S3 method for class 's2_cell'
as_s2_cell(x, ...)

# S3 method for class 'character'
as_s2_cell(x, ...)

# S3 method for class 's2_geography'
as_s2_cell(x, ...)

# S3 method for class 'wk_xy'
as_s2_cell(x, ...)

# S3 method for class 'integer64'
as_s2_cell(x, ...)

new_s2_cell(x)
```

## Arguments

- x:

  The canonical S2 cell identifier as a character vector.

- ...:

  Passed to methods

## Value

An object of class s2_cell

## Details

Under the hood, S2 cell vectors are represented in R as vectors of type
[`double()`](https://rdrr.io/r/base/double.html). This works because S2
cell identifiers are 64 bits wide, as are `double`s on all systems where
R runs (The same trick is used by the bit64 package to represent signed
64-bit integers). As a happy accident, `NA_real_` is not a valid or
meaningful cell identifier, so missing value support in the way R users
might expect is preserved. It is worth noting that the underlying value
of `s2_cell_sentinel()` would normally be considered `NA`; however, as
it is meaningful and useful when programming with S2 cells, custom
[`is.na()`](https://rdrr.io/r/base/NA.html) and comparison methods are
implemented such that `s2_cell_sentinel()` is greater than all valid S2
cells and not considered missing. Users can and should implement
compiled code that uses the underlying bytes of the vector, ensuring
that the class of any returned object that should be interpreted in this
way is constructed with `new_s2_cell()`.

## Examples

``` r
s2_cell("4b59a0cd83b5de49")
#> <s2_cell[1]>
#> [1] 4b59a0cd83b5de49
as_s2_cell(s2_lnglat(-64, 45))
#> <s2_cell[1]>
#> [1] 4b59a0cd83b5de49
as_s2_cell(s2_data_cities("Ottawa"))
#> <s2_cell[1]>
#> [1] 4cce045470cbd267
```
