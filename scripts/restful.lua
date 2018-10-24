local http = require("http")

local M = {}
local hid = nil

function M.init(_hid)
	hid = _hid
	http.init("0.0.0.0", 8011)
	http.get(
		"/hid/open",
		function(params)
			return (hid.open() and "true") or "false"
		end
	)
	http.get(
		"/hid/close",
		function(params)
			hid.close()
			return "true"
		end
	)
	http.get(
		"/hid/read",
		function(params)
			local id = tonumber(params["id"])
			local bits = tonumber(params["bits"])
			local rbuf = hid.read_buffer()
			if bits == 1 then
				return string.format("0x%02x", rbuf[id + 1])
			elseif bits == 2 then
				return string.format("0x%02x%02x", rbuf[id + 1], rbuf[id + 2])
			elseif bits == 4 then
				return string.format("0x%02x%02x%02x%02x", rbuf[id + 1], rbuf[id + 2], rbuf[id + 3], rbuf[id + 4])
			else
				local str = "error bits"
				print(str)
				return str
			end
		end
	)
	http.get(
		"/hid/readall",
		function(params)
			local fmt = string.rep("%02x", 45, "")
			local rbuf = hid.read_buffer()
			local ret =
				string.format(
				fmt,
				rbuf[1],
				rbuf[2],
				rbuf[3],
				rbuf[4],
				rbuf[5],
				rbuf[6],
				rbuf[7],
				rbuf[8],
				rbuf[9],
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
				rbuf[41],
				rbuf[42],
				rbuf[43],
				rbuf[44],
				rbuf[45]
			)
			return ret
		end
	)
	http.get(
		"/hid/writeraw",
		function(params)
			local str = params["data"]
			local d = {}
			local len = math.floor(str:len(str) / 2)
			for i = 1, len do
				d[i] = math.floor(tonumber(str:sub(i * 2 - 1, i * 2), 16))
			end
			for i = 1, 16 do
				if d[i] == nil then
					d[i] = 0
				end
			end
			hid.write(string.char(d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15], d[16]))
			return "true"
		end
	)
	http.get(
		"/hid/write",
		function(params)
			local cmd = params["name"]
			local mask = tonumber(params["mask"], 16)
			local data1 = tonumber(params["data1"])
			local data2 = tonumber(params["data2"])
			local data3 = tonumber(params["data3"])
			hid.write(cmd, mask, data1, data2, data3)
			return "true"
		end
	)
end

return M
