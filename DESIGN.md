# cxxopts++ Enhanced Design Document

## Overview
This document describes the enhanced design for cxxopts++ that adds support for:
1. Multi-level subcommands with dynamic option dependencies
2. Type system for automatic validation and conversion
3. Reversible parsing and state serialization

## 1. Multi-level Subcommands with Dynamic Option Dependencies

### Core Design
- **Command Hierarchy**: Each command can have child subcommands, forming a tree structure
- **Context Isolation**: Each command level has its own option set and parsing context
- **Option Inheritance**: Child commands inherit options from parent commands by default
- **Dependency Engine**: Evaluates boolean expressions to validate option dependencies

### Key Components

#### `Command` Class
- Represents a single command or subcommand
- Contains:
  - Command name and description
  - Parent command reference
  - Child commands map
  - Option set for this command level
  - Dependency rules
  - Help generation logic

#### `CommandParser` Class
- Recursive parser that traverses the command tree
- Maintains parsing context (current command, accumulated options)
- Handles option inheritance
- Validates dependencies after parsing

#### Dependency Expression Language
- Supports boolean operators: `&&`, `||`, `!`
- Parentheses for grouping: `( --a || --b ) && !--c`
- Operands are option names (with or without dashes)
- Evaluated using a recursive descent parser

### Parsing Flow
1. Start at root command
2. For each argument:
   a. If it's a subcommand, switch context and continue parsing
   b. If it's an option, parse it in current context
3. After all arguments parsed:
   a. Merge inherited options
   b. Evaluate all dependency rules
   c. Generate errors for violated rules

## 2. Type System for Automatic Validation & Conversion

### Core Design
- **Type Erasure**: Uses `std::any` or custom type erasure to store values
- **Converter Concept**: Classes that implement `from_string()` and `to_string()` methods
- **Validator Concept**: Classes that implement `validate()` method
- **Composition**: Support for nested types (containers of custom types)

### Key Components

#### `Type` Base Class
- Abstract base for all custom types
- Methods:
  - `virtual std::any from_string(const std::string&) const = 0;
  - `virtual std::string to_string(const std::any&) const = 0;
  - `virtual bool validate(const std::string&) const = 0;

#### `BasicType<T>` Template
- Implements `Type` for fundamental types
- Uses `std::istringstream`/`std::ostringstream` for conversion

#### `ContainerType<T>` Template
- Implements `Type` for container types
- Supports custom delimiters
- Validates each element individually

#### Example Custom Type
```cpp
class IPAddress : public Type {
public:
  std::any from_string(const std::string& s) const override;
  std::string to_string(const std::any& value) const override;
  bool validate(const std::string& s) const override;
};
```

### Conversion Flow
1. String input -> split (for containers)
2. Validate each segment
3. Convert to target type
4. Store in type-erased container

## 3. Reversible Parsing & State Serialization

### Core Design
- **Parse Tree**: Records the exact sequence of parsed elements
- **Metadata Storage**: Stores original input strings and parsing context
- **Serialization Format**: JSON with structured metadata
- **Reconstruction**: Rebuilds command line from JSON representation

### Key Components

#### `ParseNode` Class
- Represents a single parsed element (option, value, subcommand)
- Contains:
  - Element type (option, value, subcommand)
  - Original string representation
  - Position in command line
  - Associated metadata

#### `ParseTree` Class
- Root node representing the full command line
- Tree structure matching command hierarchy
- Methods for serialization/deserialization

#### `Serializer` Class
- Converts `ParseTree` to JSON
- Handles all type conversions
- Preserves original input strings

#### `Deserializer` Class
- Converts JSON back to `ParseTree`
- Reconstructs command line arguments

### Serialization Format Example
```json
{
  "command": "tool",
  "subcommands": [
    {
      "command": "module",
      "subcommands": [
        {
          "command": "subcmd",
          "options": [
            {
              "name": "--opt",
              "value": "123",
              "original": "--opt=123",
              "type": "int"
            }
          ]
        }
      ]
    }
  ]
}
```

## Implementation Plan

1. **Base Command Structure**: Implement `Command` and `CommandParser` classes
2. **Dependency Engine**: Implement expression parser and evaluator
3. **Type System**: Implement type erasure and custom type support
4. **Container Types**: Add support for vectors, maps, etc.
5. **Serialization**: Implement `ParseTree` and JSON serialization
6. **Integration**: Merge with existing cxxopts infrastructure
7. **Testing**: Create comprehensive test cases for all features

## Compatibility
- The enhanced library will be backward compatible with existing cxxopts code
- New features will be opt-in by default
- All existing APIs will continue to work unchanged