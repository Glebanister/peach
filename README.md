# peach 🍑

## Status

In developing.
Simple scripts work fine, but a lot of bugs still undiscovered

## What is it

Lightweight scripting language with C++17, that can be
extended by user. All scripts can be evaluated in your program runtime.

## Where can you use it

### Binding with your C++ code

First of all, this is my pet project, so i am not sure if someone
ever will use it, but i know where it can be used. For example,
you have graphical application, that every tick reads some integer array
from text file, then draws graph based on this numbers. But numbers need to be processed, for example, you don't want picture anomalies.
So you write peach script for processing each number, for example:

```c++
data_unit < 10 | data_unit > 100
```

Now, when your application is already running, you are able to
change ***logic*** of number cut, in other words, in runtime you simply
can change code to:

```c++
good = 0
for (i = 2; i < 10; ++i):
    good += data_unit % i == 0
good > 2
```

This is just an example, it makes no sense.

### Writing actual scripts with it

Use peach interpreter to evaluate your scripts from console:

```python
# main.pch

a := 4
b := 11
while a + b != 26
    a := a + b
a + b
```

```bash
$ peach main.pch
26
```

### Using interpreter

Peach interpretates line by line, it means you can use peach console
as calculator, for example.

## Syntax example

***Warning***: work is not done yet, so isn't final version.

```python
res = 4
i = 0
while res != 0
    i = i + 1
    res = res - 1
    if i % 2 == 0
        res = res + a + b + i
    elif i % 2 == 1
        res = 0
    else
        res = res + c - i
res = res + c
```
