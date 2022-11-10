# Data

C header-only libraries of various data structures and stuff like that.

## flex_buf

A `flex_buf` is simply a `char` array that can expand when neccessary. See the
[header](flex_buf.h) itself for information.

## slice

Implements a simple, untyped slice. It grows, it shrinks, you have to perform
the type-checking yourself. See the [header](slice.h) itself for information.
