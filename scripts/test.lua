local app = require("app")
local console = require("console")
local timer = require("timer")
local hid = require("hid")
local utils = require("utils")
local http = require("http")

local M = {}
-- 读缓冲区
local rbuf = {}
local validhid = false
-- 设备状态
local status = {on = "开", off = "关", lay = "放置", leave = "取走", checked = "选中", unchecked = "未选"}
local devices = {
	motor = {
		cmd = 0xAA,
		len = 0x09,
		type = "short",
		data = "0",
		tween = {}
	},
	led = {
		cmd = 0xAB,
		len = 0x03,
		type = "byte",
		data = "0",
		tween = {}
	},
	marquee = {
		cmd = 0xAC,
		len = 0x05,
		type = "byte",
		data = "0",
		tween = {}
	}
}

function M.init()
	print("lua init")
end

function M.event_loop(dt)
	timer.update(dt)
end

local function to_byte(d)
	if d == nil then
		return 0
	end
	return d
end

local function parse_short(d)
	if d == nil then
		return 0
	end
	d = d & 0xffff
	return d >> 8, d & 0xff
end

function M.hid_read_by_id(id, mask)
	return rbuf[id] & mask
end

local function test_read_hid()
	M.read_hid()
	print(string.format("head:%d %d %d %d %d %d %d", rbuf[1], rbuf[2], rbuf[3], rbuf[4], rbuf[5], rbuf[6], rbuf[7]))
	print(string.format("body:%d %d %d %d %d %d %d %d", rbuf[8], rbuf[9], rbuf[10], rbuf[11], rbuf[12], rbuf[13], rbuf[14], rbuf[15]))
	print(string.format("body:%d %d %d %d %d %d %d %d", rbuf[16], rbuf[17], rbuf[18], rbuf[19], rbuf[20], rbuf[21], rbuf[22], rbuf[23]))
	print(string.format("body:%d %d %d %d %d %d %d %d", rbuf[24], rbuf[25], rbuf[26], rbuf[27], rbuf[28], rbuf[29], rbuf[30], rbuf[31]))
	print(string.format("body:%d %d %d %d %d %d %d", rbuf[32], rbuf[33], rbuf[34], rbuf[35], rbuf[36], rbuf[37], 0))
end

local motor_power = {state = false}
local led_power = {state = false}
local marquee_power = {state = false}

function test_write_hid()
	-- 防止重入
	if motor_power.state == true or led_power.state == true or marquee_power.state == true then
		print("正在测试中,请稍等...")
		return
	end

	-- 电机控制
	motor_power = {a = 0, b = 0, c = 0, state = true}
	local motor_timer =
		timer.every(
		0.1,
		function()
			M.write_hid("motor", 0x7, motor_power.a, motor_power.b, motor_power.c)
			print("motor power:", motor_power.a, motor_power.b, motor_power.c)
		end,
		50
	)

	local motor_move1, motor_move2
	motor_move1 = function()
		if timer.valid(motor_timer) == true then
			timer.tween(4, motor_power, {a = 2048, b = 2048, c = 2048}, "linear", motor_move2)
		else
			motor_power.state = false
		end
	end
	motor_move2 = function()
		if motor_power.state == false then
			return
		end
		timer.tween(4, motor_power, {a = 0, b = 0, c = 0}, "linear", motor_move1)
	end
	motor_move1()
	motor_move2()

	-- Led控制
	led_power = {a = 0, b = 0, state = true}
	local led_timer =
		timer.every(
		0.1,
		function()
			local a = math.ceil(led_power.a)
			local data = (a << 1) + led_power.b
			M.write_hid("led", 0x7, data, 0, 0)
			print("led power:", led_power.a, led_power.b, data)
		end,
		50
	)

	local led_move1, led_move2
	led_move1 = function()
		if timer.valid(led_timer) == true then
			timer.tween(4, led_power, {a = 0xf, b = 0xf}, "linear", led_move2)
		else
			led_power.state = false
		end
	end
	led_move2 = function()
		if led_power.state == false then
			return
		end
		timer.tween(4, led_power, {a = 0, b = 0}, "linear", led_move1)
	end
	led_move1()
	led_move2()

	-- 跑马灯控制
	marquee_power = {a = 0, b = 0, state}
	local marquee_timer =
		timer.every(
		0.1,
		function()
			M.write_hid("marquee", 0x3, marquee_power.a, marquee_power.b, 0)
			--print("marquee power:", marquee_power.a, marquee_power.b, 0)
		end,
		50
	)

	local marquee_move1, marquee_move2
	marquee_move1 = function()
		if timer.valid(marquee_timer) == true then
			timer.tween(4, marquee_power, {a = 0xff, b = 0xff}, "linear", marquee_move2)
		else
			marquee_power.state = false
		end
	end
	marquee_move2 = function()
		if marquee_power.state == false then
			return
		end
		timer.tween(4, marquee_power, {a = 0, b = 0}, "linear", marquee_move1)
	end
	marquee_move1()
	marquee_move2()
end

function test_func()
	--[[
	local data = utils.crc16_kermit(string.char(0,0,2,0,2,0,0xb5))
	local h,l = string.byte(data,1,2)
	print(string.format("%x%x",h,l))
	]]
end

return M
