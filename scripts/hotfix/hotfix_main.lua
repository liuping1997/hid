local M = {}
local elapsed = 0
local hotfix = require("hotfix")

function M.init()
    hotfix.init()
end

function M.event_loop(dt)
    elapsed = elapsed + dt
    if elapsed > 2 then
      hotfix.check()
      --print("elapsed:",elapsed)
      elapsed = 0
    end
end

return M