local M = {}
local rbuf = {}
local hid = require("hid")
local utils = require("utils")
local console = require("console")

-- 设备状态
local status = {
  on = "开",
  off = "关",
  lay = "放置",
  leave = "取走",
  checked = "选中",
  unchecked = "未选"
}
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
  for i = 1, 64 do
    rbuf[i] = i
  end
end

function M.event_loop(dt)
  M.read()
  M.showinfo()
end

function M.read_buffer()
  return rbuf
end

local function parse_short(d)
  if d == nil then
    return 0
  end
  d = d & 0xffff
  return d >> 8, d & 0xff
end

function M.write(name, id, data1, data2, data3)
  if hid.success == false then
    print("write hid failure.hid not opened.")
    return
  end

  data1 = math.ceil(data1)
  data2 = math.ceil(data2)
  data3 = math.ceil(data3)

  local device = devices[name]
  local len = device.len
  local type = device.type
  local d = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  d[1] = 0
  d[2] = 0
  d[3] = 2
  d[4] = 0
  d[5] = len
  d[6] = 0
  d[7] = device.cmd
  d[8] = id
  if type == "byte" then
    if data1 ~= nil then
      d[9] = data1 & 0xf
    end
    if data2 ~= nil then
      d[10] = data2 & 0xf
    end
    if data3 ~= nil then
      d[11] = data3 & 0xf
    end
  else
    if data1 ~= nil then
      d[9], d[10] = parse_short(data1)
    end
    if data2 ~= nil then
      d[11], d[12] = parse_short(data2)
    end
    if data3 ~= nil then
      d[13], d[14] = parse_short(data3)
    end
  end
  print(name, type, data1, data2, data3, d[1], d[2], d[3], d[4])
  local crc = utils.crc16_kermit(string.char(d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]), len + 5)
  d[len + 5 + 1], d[len + 5 + 2] = string.byte(crc, 1, 2)
  print(string.format("crc:%x%x", d[len + 5 + 1], d[len + 5 + 2]))
  print(string.format("%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x", d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]))
  return hid.write(string.char(d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]))
end

function M.open()
  hid.open()
end

function M.close()
  hid.close()
end

function M.read()
  if hid.success == false then
    return
  end

  local len = 0x2d
  local buf = hid.read(len)
  -- shift array + 1
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
    rbuf[41],
    rbuf[42],
    rbuf[43],
    rbuf[44],
    rbuf[45] = string.byte(buf, 1, len)
end

local function is_on(d)
  if d == nil then
    return "-"
  end
  return (d > 0 and status.on) or status.off
end

-- 判断高位开关
local function is_h_on(d)
  if d == nil then
    return "-"
  end
  return ((d >> 4) > 0 and status.on) or status.off
end

-- 判断低位开关
local function is_l_on(d)
  if d == nil then
    return "-"
  end
  return ((d & 0xf) > 0 and status.on) or status.off
end

local function is_checked(d)
  if d == nil then
    return "-"
  end
  return ((d >> 4) > 0 and status.checked) or status.unchecked
end

local function is_leaved(d)
  if d == nil then
    return "-"
  end
  return ((d & 0xf) > 0 and status.leave) or status.lay
end

local function to_short(h8, l8)
  if h8 == nil or l8 == nil then
    return 0
  end
  return l8 << 8 + h8
end

local function to_byte(d)
  if d == nil then
    return 0
  end
  return d
end

function M.showinfo()
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

function M.showdata()
  read()
  print(string.format("head:%d %d %d %d %d %d %d", rbuf[1], rbuf[2], rbuf[3], rbuf[4], rbuf[5], rbuf[6], rbuf[7]))
  print(string.format("body:%d %d %d %d %d %d %d %d", rbuf[8], rbuf[9], rbuf[10], rbuf[11], rbuf[12], rbuf[13], rbuf[14], rbuf[15]))
  print(string.format("body:%d %d %d %d %d %d %d %d", rbuf[16], rbuf[17], rbuf[18], rbuf[19], rbuf[20], rbuf[21], rbuf[22], rbuf[23]))
  print(string.format("body:%d %d %d %d %d %d %d %d", rbuf[24], rbuf[25], rbuf[26], rbuf[27], rbuf[28], rbuf[29], rbuf[30], rbuf[31]))
  print(string.format("body:%d %d %d %d %d %d %d", rbuf[32], rbuf[33], rbuf[34], rbuf[35], rbuf[36], rbuf[37], 0))
end

function open_hid()
  hid.open()
end

function close_hid()
  hid.close()
end

function print_hid()
  showinfo()
end

return M
