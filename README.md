# peach 🍑

![CMake](https://github.com/Glebanister/peach/workflows/CMake/badge.svg)

A lightweight dependency-free scripting language.
All scripts can be evaluated in C++17 program runtime.

## Documentation

Wiki is avaiable on github: [syntax page](https://github.com/Glebanister/peach/wiki/Syntax)

## Current features

- Command line interpreter

- File interpreter

- Single variable type - `std::int32_t`

- `if/else` conditions

- `while` loops

- Division into blocks with indentation

- Variable declaration with `let` keyword

- Arithmetical, logical operators and corresponding assignation operators (`a += 1` is simillar to `a = a + 1`)

- Informative error messages

## Building

- Make sure following commands works fine on your machine:

```bash
git --version
cmake --version
clang-10 --version
```

- Build project by typing in your command line:

```bash
mkdir build && cd build
cmake ..
make
```

- Executable file can be found in `/build/peach` if build was successfull

## How to use

### Using with command line interface

- Simply run `/build/peach`, you will get:

```bash
gleb@ZenBook:~/Documents/projects/peach$ ./build/peach
Peach
>>> let even_numbers = 0
0
>>> let n = 11
11
>>> while n -= 1
...     even_numbers += n % 2 == 0
...
5
>>>
```

- And you're good to go

### Write script

- Simply run `/build/peach <filename>`, where `<filename>` contains program code, for example, if this is `hello.pch`

```javascript
let a = 123
let cnt_even = 0
let cnt_odd = 0
let some_other_variable
while (a != 0) & cnt_even < 10
    if a % 2 == 0
        cnt_even += 1
        if cnt_even % 3 == 2:
            some_other_variable += 2 ** cnt_even
    else
        cnt_odd += 1
    a -= 1
some_other_variable
```

Then you program evaluation will look like:

```bash
gleb@ZenBook:~/Documents/projects/peach$ ./build/peach hello.pch
292
```

### Evaluate in C++17 program runtime

```C++
#include <fstream>

#include "PeachCli.hpp"

int main()
{
    auto programFile = std::ifstream("<your-peach-program-relative-path>");
    auto interpreter = peach::cli::PeachCli();
    peach::cli::PeachCli().getScope().declare("var", 10);
    peach::cli::PeachCli().getScope().declare("bar", 20);
    // Now interpreter contains two variables: var = 10, bar = 20
    interpreter.executeProgram(programFile, std::cerr);
    std::cout << "var: " << interpreter.getScope()["var"] << std::endl;
    std::cout << "bar: " << interpreter.getScope()["bar"] << std::endl;
}
```
