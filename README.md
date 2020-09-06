# peach 🍑

Lightweight dependency-free scripting language with C++17, that can be
extended by user. All scripts can be evaluated in your program runtime.

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
```

- Executable file can be found in `/build/peach` if build was successfull

## How to use

### Using with command line interface

- Simply run `/build/peach`, you will get:

```bash
gleb@ZenBook:~/Documents/projects/peach$ ./build/peach 
Peach
>>>
```

- And you're good to go

### Write script

- Simply run `/build/peach <filename>`, where `<filename>` contains program code, for example, if this is `hello.pch`

```javascript
let a = 10
a += 1
```

Then you program evaluation will look like:

```bash
gleb@ZenBook:~/Documents/projects/peach$ ./build/peach hello.pch
11
```

## Documentation

Wiki is avaiable on github: [github wiki](https://github.com/Glebanister/peach/wiki)
