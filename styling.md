# Style guide and conventions
This markdown file contains some styling conventions to (hopefully) maintain some consistency throughout the codebase.

## C++

### Pointer/Reference declarations
Pointers and references are declared with the respective decorator added to the variable name, not the type:  
`bool *booleanPointer`  
`int &integerReference`

### Classes
Define class names in PascalCase
Define class methods in PascalCase
Define class variable in camelCase

### Enums
Define enum type names in PascalCase
Define enum values in PascalCase

### Fixed-width types
prefer standard types whenever possible.  
e.g: `uint32_t` over `Uint32`

### Preprocessor directives
Define preprocessor constants in all-caps snake_case

## Map conventions
Each map should have a single border of walls `#`

## Value conversion
Use `static_cast`?