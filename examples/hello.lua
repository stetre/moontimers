#!/usr/bin/env lua
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

