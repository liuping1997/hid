local hid = require 'luahidapi'

local gb = hid.open(0x04d8, 0xfcf0)

local adapter_command = {
  address_list = 0x01,
  data = 0x02
}

local id = 0
local function next_packet_id()
  id = id + 1
  id = id % 0xFFFF
  return id
end

local function send_packet(data)
  local id = next_packet_id()

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

  print(table.unpack(packet))

  gb:write(string.char(table.unpack(packet)))
end

local function send_message(message)
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

send_message({
  source = 0xE4,
  destination = 0xFF,
  data = {
    0x01
  }
})

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

while true do
  local rx = gb:read(61)
  if rx and #rx > 1 then
    handle_received_message_or_something(rx)

    for i = 1, #rx do
      io.write(string.byte(rx, i) .. ',')
    end
    io.write('\n')
  end
end
