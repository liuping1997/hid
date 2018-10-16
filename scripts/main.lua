local app = require("app")
local console = require("console")
local proto = {
{1000,0,"byte","vein1"},
{1001,0,"byte","vein2"},
{1002,0,"byte","vein3"},
{1003,0,"byte","vein4"},
{1004,0,"byte","vein5"},
{1005,0,"byte","vein6"},
{1006,0,"byte","vein7"}
}

function load()
	print("lua load")
end

function event_loop()
	--print("event_loop")
	test_console_print()
end

function test_open()
	app.open(0x051a,0x511b)
	print("test_open")
end

function test_close()
	app.close()
	print("test_close")
end

function test_read()
	local cmd = 0xb5
	local len = 0x26
	local buf = app.read(cmd,len)
	local b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,
		b11,b12,b13,b14,b15,b16,b17,b18, b19,b20,
		b21,b22,b23,b24,b25,b26,b27,b28,b29,b30,
		b31,b32,b33,b34,b35,b36,b37,b38 = string.byte(buf,1,len)
	print("test_read start")
	print(b1,b2,b3,b4,b5,b6,b7,b8,b9,b10) 
	print(b11,b12,b13,b14,b15,b16,b17,b18,b19,b20)
	print(b21,b22,b23,b24,b25,b26,b27,b28,b29,b30)
	print(b31,b32,b33,b34,b35,b36,b37,b38)
	print("test_read end")
end

function test_write()
	print("test_write")
end

function test_console_print()
	console.set_cursor_visible(false)
	--console.set_buffer_size(300,200)
	console.print(100,1,"xxxxxxxxxxxxx\n")
	console.print(100,2,"yyyyyyyyyyyyy\n")
	console.print(100,3,"zzzzzzzzzzzzz\n")
	console.print(100,4,"8888888\n")
end