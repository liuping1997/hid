local console = require("console")
local test = require("test")
local hid = require("hid001")
local restful = require("restful")

local M = {}
M.hid = nil
-- 读缓冲区
local rbuf = {}
local validhid = false

function M.init()
	print("lua init")
	console.set_cp(65001, 12)
	console.set_cursor_visible(false)
	console.set_window_size(120, 50)
	console.set_buffer_size(120, 30)
	M.hid = hid
	hid.init()
	restful.init(hid)
	test.init(hid)
	--hid.init()
end

function M.event_loop(dt)
	hid.event_loop(dt)
	test.event_loop(dt)
end

function test_func()
	--[[
	local data = utils.crc16_kermit(string.char(0,0,2,0,2,0,0xb5),5)
	local h,l = string.byte(data,1,2)
	print(string.format("%x%x",h,l))
	]]
end

return M
