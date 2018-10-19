package.path = "../../scripts/?.lua;./scripts/?.lua;./?.lua;../../scripts/hotfix/?.lua;./scripts/hotfix/?.lua;" .. package.path

local M = {}
local main = require("main")
local hotfix = require("hotfix_main")

function init()
    main.init()
    hotfix.init()
end

function event_loop(dt)
    main.event_loop(dt)
    hotfix.event_loop(dt)
end

function open_hid()
    main.open_hid()
end

function close_hid()
    main.close_hid()
end

function print_hid()
    main.print_hid()
end

return M