# CyndaQuil Programming Language

## Introduction

CyndaQuil is a statically-typed, compiled programming language designed for high performance and safety. It aims to provide developers with a robust toolset for system-level programming, ensuring both efficiency and reliability.

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)

## Features

- **Static Typing**: Ensures type safety at compile time, reducing runtime errors.
- **High Performance**: Compiled directly to machine code for optimized execution.
- **Memory Safety**: Incorporates features to prevent common memory errors.
- **Modern Syntax**: Clean and expressive syntax facilitating ease of use and readability.

## Installation

To build and install CyndaQuil from source:

1. **Clone the Repository**:

   ```bash
   git clone https://github.com/Quil180/CyndaQuilLanguage.git
   cd CyndaQuilLanguage
   ```

2. **Build the Compiler**:

   Ensure you have `cmake` and a C++ compiler installed. Then, execute:

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

3. **Installing onto System (Not Tested)**:

   After building, you can install the compiler (may require superuser privileges):

   ```bash
   sudo make install
   ```

## Usage

Once installed, you can compile CyndaQuil source files using the `ember` compiler:

```bash
ember path/to/your_program.cq
```

This will produce an executable binary in the same directory as your source file.

## Documentation

Comprehensive documentation is available in the `documents` folder of this repository. Key documents include:

- **Language Specification**: Detailed description of the language's syntax and semantics.
- **Compiler Design**: Insights into the architecture and implementation of the CyndaQuil compiler.
- **Standard Library Reference**: Information on the available modules and functions in the standard library.

To access these documents:

1. Navigate to the `documents` folder:

   ```bash
   cd documents
   ```

2. Open the desired PDF files using your preferred PDF viewer.

## Contributing

Contributions to CyndaQuil are welcome! To contribute:

1. Fork the repository.
2. Create a new branch for your feature or bugfix.
3. Commit your changes with clear and concise messages.
4. Submit a pull request detailing your changes.

Please ensure that your code adheres to the project's coding standards and includes appropriate tests.

## License

This project is licensed under the MIT License. For more details, refer to the `LICENSE` file in the repository. 
