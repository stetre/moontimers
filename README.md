## MoonTimers: Concurrent timers for Lua

MoonTimers is a Lua module that provides concurrent timers.

It runs on GNU/Linux and on Windows (MSYS2/MinGW) and requires 
[Lua](http://www.lua.org/) (>=5.3).

_Author:_ _[Stefano Trettel](https://www.linkedin.com/in/stetre)_

[![Lua logo](./doc/powered-by-lua.gif)](http://www.lua.org/)

#### License

MIT/X11 license (same as Lua). See [LICENSE](./LICENSE).

#### Documentation

See the [Reference Manual](https://stetre.github.io/moontimers/doc/index.html).

#### Getting and installing

Setup the build environment as described [here](https://github.com/stetre/moonlibs), then:

```sh
$ git clone https://github.com/stetre/moontimers
$ cd moontimers
moontimers$ make
moontimers$ sudo make install
```

#### Examples

The example below creates a timer, starts it so to expire after 2 seconds, and restarts it
each time it expires.

Other examples can be found in the **examples/** directory of this repo.

```lua
-- MoonTimers example: hello.lua
local timers = require("moontimers")

local duration = 2 -- seconds
local function callback(timer, exptime)
   print(string.format("timer expired, system time = %f", timers.now()))
   timer:start(exptime+duration)
end

-- Create a timer:
local t1 = timers.new(duration, callback)

-- Start it:
t1:start()

-- Event loop:
while true do
   timers.trigger()
end
```


The script can be executed at the shell prompt with the standard Lua interpreter:

```shell
$ lua hello.lua
```

See the `examples/` directory.

#### See also

* [MoonLibs - Graphics and Audio Lua Libraries](https://github.com/stetre/moonlibs).
