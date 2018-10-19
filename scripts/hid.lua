local M = {}
local hidapi = require("hidapi")
local ins = nil
local id = 0
local adapter_command = {
  address_list = 0x01,
  data = 0x02
}

function M.init()
    hidapi.init()
end

function M.open()
    ins = hidapi.open(0x051A, 0x511B)
    if ins == nil then
      print("open hid failure")
    end
    return ins ~= nil
end

function M.close()
    if ins ~= nil then
      ins:close()
    end
    ins = nil
end

local function next_packet_id()
  id = id + 1
  id = id % 0xFFFF
  return id
end

function M.send_packet(data)
  --[[local id = next_packet_id()

  print('id', id)

  local packet = {
    id % 0xFF, -- id1
    id // 0xFF, -- id2
    0x00, -- message number
    0x01, ---message count
    #data
  }
  for _, byte in ipairs(data) do
    table.insert(packet, byte)
  end
  ]]
  print(table.unpack(data))
  hidapi:write(string.char(table.unpack(data)))
end

function M.write(data)
  hidapi:write(data)
end

function M.read(len)
  return hidapi:read(len)
end

function M.send_message(message)
  local id = next_packet_id()
  local message_bytes = {
    adapter_command.data,
    id % 0xFF, -- id1
    id // 0xFF, -- id2
    0x00, -- id3
    0x00, -- id4
    #(message.data) + 4,
    message.destination,
    #(message.data) + 8,
    message.source
  }

  for _, byte in ipairs(message.data) do
    table.insert(message_bytes, byte)
  end

  send_packet(message_bytes)
end

--[[
send_message({
  source = 0xE4,
  destination = 0xFF,
  data = {
    0x01
  }
})
]]
    -- function onPacketReceived(packet) {
    --     var reader = new stream.Reader(packet);
    --     var type = reader.readUInt8();
    --
    --     if (type == COMMAND_DATA) {
    --         var status = reader.readUInt8();
    --
    --         if (status == STATUS_VALID) {
    --             var length = reader.readUInt8();
    --
    --             if (length >= HEADER_LENGTH) {
    --                 var destination = reader.readUInt8();
    --                 var ignored = reader.readUInt8();
    --                 var source = reader.readUInt8();
    --                 var command = reader.readUInt8();
    --                 var data = reader.readBytes(packet.length - 7);
    --
    --                 self.emit("message", {
    --                     command: command,
    --                     source: source,
    --                     destination: destination,
    --                     data: data
    --                 });
    --             }
    --         }
    --     }
    --
    --     delete reader;
    -- }

local function handle_received_message_or_something(message)

end

function test_read_hid()
    if inside == nil then
        print("hit not opend")
      return
    end
    print("test read hid")
    ins:write(string.char(00,00,02,00,02,00,0xb5,0x41,0x16))
    local rx = ins:read(65)
    if rx and #rx > 1 then
      handle_received_message_or_something(rx)
      for i = 1, #rx do
        io.write(string.byte(rx, i) .. ',')
      end
      io.write('\n')
    end
end

return M