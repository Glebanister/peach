# peach 🍑

## What is it

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

## How to use

### Building

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

## Where can you use it

### Binding with your C++ code

First of all, this is my pet project, so i am not sure if someone
ever will use it, but i know where it can be used. For example,
you have graphical application, that every tick reads some integer array
from text file, then draws graph based on this numbers. But numbers need to be processed, for example, you don't want picture anomalies.
So you write peach script for processing each number, for example:

```javascript
let data_unit
data_unit < 10 | data_unit > 100
```

Now, when your application is already running, you are able to
change ***logic*** of number cut, in other words, in runtime you simply
can change code to:

```javascript
let data_unit
let good = 0
let i = 2
while i < 10
    good += data_unit % i == 0
    i += 1
good > 2
```

This is just an example, it makes no sense.

### Writing actual scripts with it

Use peach interpreter to evaluate your scripts from console:

```javascript
# main.pch

let a = 123
let cnt_even = 0
let cnt_odd = 0
while (a != 0) & cnt_even > 10
    if a % 2 == 0
        cnt_even += 1
    else
        cnt_odd += 1
    a -= 1
cnt_odd
```

```bash
$ peach main.pch
10
```

### Using interpreter

Peach interpretates line by line, it means you can use peach console
as calculator, for example.
