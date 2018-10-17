package.path = "../../scripts/?.lua;./scripts/?.lua"..package.path

local app = require("app")
local console = require("console")
local tween = require("tween")
-- 读缓冲区
local rbuf = {}
local validhid = false
-- 设备状态
local status = {on = "开", off = "关", lay = "放置", leave = "取走", checked = "选中", unchecked = "未选"}
local devices = {
	motor = {
		cmd = 0xAA,
		len = 0x09,
		type = "byte",
		data = "0"
	},
	led = {
		cmd = 0xAB,
		len = 0x03,
		data = "0"
	},
	marquee = {
		cmd = 0xAC,
		len = 0x05,
		data = "0"
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
	try{
		main = function ()
			read_hid()
			print_hid()
		end,
		catch = function (errors)
			print("catch : " .. errors)
		end,
		finally = function (ok, errors)
		end,
	}
	--print("dt:" .. dt)
end

function open_hid()
	validhid = app.open(0x051a, 0x511b)
end

function close_hid()
	print("test_close")
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

function write_hid()
	-- 电机控制
	write_cmd("motor", 0, 500, 1000, 200)
	-- Led控制
	write_cmd("led", 0, 50, 0, 0)
	-- 跑马灯控制
	write_cmd("marquee", 0, 50, 10, 0)
end

function parse_short(d)
	local high = d >> 4
	local low = d & 0xf
	return low, high
end

function write_cmd(name, id, data1, data2, data3)
	if validhid == false then
		return
	end
	local device = devices[name]
	local len = device.len
	local type = device.type
	local d = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	d[1] = device.cmd
	d[2] = id
	if type == "byte" then
		d[3] = data1 & 0xf
		d[4] = data2 & 0xf
		d[5] = data3 & 0xf
	else
		d[3], d[4] = parse_short(data1)
		d[5], d[6] = parse_short(data2)
		d[7], d[8] = parse_short(data3)
	end
	local bytes = string.char(d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10])
	return app.write(bytes, len)
end

function read_hid()
	if validhid == false then
		return
	end
	local cmd = 0xb5
	local len = 0x26
	local buf = app.read(cmd, len)
	local b = rbuf
	-- shift array + 1
	b[01],
		b[01],
		b[02],
		b[03],
		b[04],
		b[05],
		b[06],
		b[07],
		b[08],
		b[09],
		b[10],
		b[11],
		b[12],
		b[13],
		b[14],
		b[15],
		b[16],
		b[17],
		b[18],
		b[19],
		b[20],
		b[21],
		b[22],
		b[23],
		b[24],
		b[25],
		b[26],
		b[27],
		b[28],
		b[29],
		b[30],
		b[31],
		b[32],
		b[33],
		b[34],
		b[35],
		b[36],
		b[37],
		b[38] = string.byte(buf, 1, len)
end
