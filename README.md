# Heatline - source-annotating profiler

**WORK IN PROGRESS, EXPERIMENTAL**

Everyone loves profiling code. Usual bottom-up logs like the one below are
really fancy:

```
 [Bottom up (heavy) profile]:
  Note: percentage shows a share of a particular caller in the total
  amount of its parent calls.
  Callers occupying less than 2.0% are not shown.

   ticks parent  name
     17   18.9%  node::ContextifyScript::New(v8::FunctionCallbackInfo<v8::Value> const&)
     17  100.0%    v8::internal::Builtin_HandleApiCallConstruct(int, v8::internal::Object**, v8::internal::Isolate*)
     17  100.0%      LazyCompile: ~runInThisContext bootstrap_node.js:403:28
     17  100.0%        LazyCompile: NativeModule.compile bootstrap_node.js:485:44
     17  100.0%          LazyCompile: ~NativeModule.require bootstrap_node.js:419:34
      5   29.4%            Function: <anonymous> repl.js:1:11
      4   23.5%            Function: ~<anonymous> internal/repl.js:1:11
      2   11.8%            LazyCompile: ~startup bootstrap_node.js:12:19
      2   11.8%            Function: ~<anonymous> stream.js:1:11
      1    5.9%            LazyCompile: ~setupGlobalVariables bootstrap_node.js:202:32
      1    5.9%            Function: ~<anonymous> util.js:1:11
      1    5.9%            Function: ~<anonymous> tty.js:1:11
      1    5.9%            Function: ~<anonymous> readline.js:1:11
```

However, in some JavaScript instensive apps it may desirable to find which
lines of the functions were executed most.

## Heatline

This can be done with `heatline` pretty easly. `heatline` is built from two
parts: cli tool and a module to be included in user application.

## User module

```js
require('heatline')();  // will start a server on port number 11337
```

## CLI tool

One demo is better than a thousand of words:

[![asciicast](https://asciinema.org/a/4qlgmk89prjiehzn97xg3ye6b.png)](https://asciinema.org/a/4qlgmk89prjiehzn97xg3ye6b)

## How it works?

There is a C++ API in V8 for collecting line hits information, this module has a
C++ addon that consumes this API. See `deps/v8/include/v8-profiler.h` in
[node.js repo][0].

#### LICENSE

This software is licensed under the MIT License.

Copyright Fedor Indutny, 2016.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

[0]: https://github.com/nodejs/node/blob/master/deps/v8/include/v8-profiler.h
