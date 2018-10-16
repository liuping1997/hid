local app = require("app")
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

function test_open()
	app.open(0x051a,0x511b)
	print("test_open")
end

function test_close()
	app.close()
	print("test_close")
end

function test_read()
	app.read(string.char(0xb5),0x26)
	print("test_read")
end

function test_write()
	print("test_write")
end