package.path = "../../scripts/?.lua;./scripts/?.lua" .. package.path

local app = require("app")
local console = require("console")
local timer = require("timer")
-- 读缓冲区
local rbuf = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
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

-- 异常捕获
function try(block)
	local main = block.main
	local catch = block.catch
	local finally = block.finally

	assert(main)

	-- try to call it
	local ok, errors = xpcall(main, debug.traceback)
	if not ok then
		-- run the catch function
		if catch then
			catch(errors)
		end
	end

	-- run the finally function
	if finally then
		finally(ok, errors)
	end

	-- ok?
	if ok then
		return errors
	end
end

function init()
	print("lua init")
	console.set_cp(65001, 10)
	console.set_cursor_visible(false)
	console.set_window_size(120, 40)
	console.set_buffer_size(120, 40)
end

function event_loop(dt)
	try {
		main = function()
			read_hid()
			print_hid()
			timer.update(dt)
		end,
		catch = function(errors)
			print("catch : " .. errors)
		end,
		finally = function(ok, errors)
		end
	}
	--print("dt:" .. dt)
end

function open_hid()
	validhid = app.open(0x051a, 0x511b)
end

function close_hid()
	validhid = false
	app.close()
end

function is_on(d)
	if d == nil then
		return "-"
	end
	return (d > 0 and status.on) or status.off
end

-- 判断高位开关
function is_h_on(d)
	if d == nil then
		return "-"
	end
	return ((d >> 4) > 0 and status.on) or status.off
end

-- 判断低位开关
function is_l_on(d)
	if d == nil then
		return "-"
	end
	return ((d & 0xf) > 0 and status.on) or status.off
end

function is_checked(d)
	if d == nil then
		return "-"
	end
	return ((d >> 4) > 0 and status.checked) or status.unchecked
end

function is_leaved(d)
	if d == nil then
		return "-"
	end
	return ((d & 0xf) > 0 and status.leave) or status.lay
end

function to_short(h8, l8)
	if h8 == nil or l8 == nil then
		return 0
	end
	return l8 << 8 + h8
end

function to_byte(d)
	if d == nil then
		return 0
	end
	return d
end

function print_hid()
	local x = 70
	local b = rbuf
	console.print(x, 01, string.format("%s:%s %s", "洗手液:", is_on(b[8]), "", ""))
	console.print(x, 02, string.format("%s:%s %s", "输液瓶:", is_checked(b[9]), is_leaved(b[9]), is_on(b[10])))
	console.print(x, 03, string.format("%s:%s %s", "手  套:", is_checked(b[12]), is_leaved(b[12])))
	console.print(x, 04, string.format("%s:%s %s", "口  罩:", is_checked(b[14]), is_leaved(b[14])))
	console.print(x, 05, string.format("%s:%s %s", "压脉带:", is_checked(b[16]), is_leaved(b[16])))
	console.print(x, 06, string.format("%s:%s %s", "棉  棒:", is_checked(b[18]), is_leaved(b[18])))
	console.print(x, 07, string.format("%s:%s %s", "输液贴:", is_checked(b[20]), is_leaved(b[20])))
	console.print(x, 08, string.format("%s:%s %s %d", "采血针:", is_checked(b[22]), is_leaved(b[22]), to_short(b[23], b[24])))
	console.print(x, 09, string.format("%s:%s 5ml(%s) 2ml(%s)", "采血管:", is_checked(b[26]), is_h_on(b[27]), is_l_on(b[27])))
	console.print(x, 10, string.format("%s:%s %s %d %d", "药  瓶:", is_checked(b[16]), is_leaved(b[16]), to_short(b[33], b[34]), to_byte(b[35])))
	console.print(x, 11, string.format("%s:%s %s %d %d %d", "注射器:", is_checked(b[37]), is_leaved(b[37]), to_short(b[38], b[39]), to_byte(b[40]), to_byte(b[41])))
end

function parse_short(d)
	if d == nil then
		return 0
	end
	local high = d >> 4
	local low = d & 0xf
	return low, high
end

function write_cmd(name, id, data1, data2, data3)
	if validhid == false then
		return
	end

	data1 = math.ceil(data1)
	data2 = math.ceil(data2)
	data3 = math.ceil(data3)

	local device = devices[name]
	local len = device.len
	local type = device.type
	local d = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	d[1] = device.cmd
	d[2] = id
	if type == "byte" then
		d[3] = data1
		d[4] = data2
		d[5] = data3
	else
		d[3], d[4] = parse_short(data1)
		d[5], d[6] = parse_short(data2)
		d[7], d[8] = parse_short(data3)
	end
	device.data = string.char(d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10])
	return app.write(device.data, len)
end

function read_hid()
	if validhid == false then
		return
	end
	local cmd = 0xb5
	local len = 0x26
	local buf = app.read(cmd, len)
	-- shift array + 1
	rbuf[01],
		rbuf[01],
		rbuf[02],
		rbuf[03],
		rbuf[04],
		rbuf[05],
		rbuf[06],
		rbuf[07],
		rbuf[08],
		rbuf[09],
		rbuf[10],
		rbuf[11],
		rbuf[12],
		rbuf[13],
		rbuf[14],
		rbuf[15],
		rbuf[16],
		rbuf[17],
		rbuf[18],
		rbuf[19],
		rbuf[20],
		rbuf[21],
		rbuf[22],
		rbuf[23],
		rbuf[24],
		rbuf[25],
		rbuf[26],
		rbuf[27],
		rbuf[28],
		rbuf[29],
		rbuf[30],
		rbuf[31],
		rbuf[32],
		rbuf[33],
		rbuf[34],
		rbuf[35],
		rbuf[36],
		rbuf[37],
		rbuf[38],
		rbuf[39],
		rbuf[40],
		rbuf[41] = string.byte(buf, 1, len)
end

function test_read_hid()
	read_hid()
	print(string.format("head:%d %d %d %d %d %d %d", rbuf[1], rbuf[2], rbuf[3], rbuf[4], rbuf[5], rbuf[6], rbuf[7]))
	print(string.format("body:%d %d %d %d %d %d %d %d", rbuf[8], rbuf[9], rbuf[10], rbuf[11], rbuf[12], rbuf[13], rbuf[14], rbuf[15]))
	print(string.format("body:%d %d %d %d %d %d %d %d", rbuf[16], rbuf[17], rbuf[18], rbuf[19], rbuf[20], rbuf[21], rbuf[22], rbuf[23]))
	print(string.format("body:%d %d %d %d %d %d %d %d", rbuf[24], rbuf[25], rbuf[26], rbuf[27], rbuf[28], rbuf[29], rbuf[30], rbuf[31]))
	print(string.format("body:%d %d %d %d %d %d %d", rbuf[32], rbuf[33], rbuf[34], rbuf[35], rbuf[36], rbuf[37], 0))
end

function test_write_hid()
	-- 电机控制
	motor_power = {a = 0, b = 0, c = 0}
	timer.every(
		0.1,
		function()
			write_cmd("motor", 0x7, motor_power.a, motor_power.b, motor_power.c)
			print("motor power:", motor_power.a, motor_power.b, motor_power.c)
		end,
		50
	)

	local motor_move1, motor_move2
	motor_move1 = function()
		timer.tween(4, motor_power, {a = 2048, b = 2048, c = 2048}, "linear", motor_move2)
	end
	motor_move2 = function()
		timer.tween(4, motor_power, {a = 0, b = 0, c = 0}, "linear", motor_move1)
	end
	motor_move1()
	motor_move2()

	-- Led控制
	led_power = {a = 0, b = 0}
	timer.every(
		0.1,
		function()
			local a = math.ceil(led_power.a)
			local data = (a << 1) + led_power.b
			write_cmd("led", 0x7, data, 0, 0)
			print("led power:", led_power.a, led_power.b, data)
		end,
		50
	)

	local led_move1, led_move2
	led_move1 = function()
		timer.tween(4, led_power, {a = 0xf, b = 0xf}, "linear", led_move2)
	end
	led_move2 = function()
		timer.tween(4, led_power, {a = 0, b = 0}, "linear", led_move1)
	end
	led_move1()
	led_move2()

	-- 跑马灯控制
	marquee_power = {a = 0, b = 0}
	timer.every(
		0.1,
		function()
			write_cmd("marquee", 0x3, marquee_power.a, marquee_power.b, 0)
			print("marquee power:", marquee_power.a, marquee_power.b, 0)
		end,
		50
	)

	local marquee_move1, marquee_move2
	marquee_move1 = function()
		timer.tween(4, marquee_power, {a = 0xff, b = 0xff}, "linear", marquee_move2)
	end
	marquee_move2 = function()
		timer.tween(4, marquee_power, {a = 0, b = 0}, "linear", marquee_move1)
	end
	marquee_move1()
	marquee_move2()
end
