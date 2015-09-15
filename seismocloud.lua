-- Script Wireshark per decodifica del protocollo di comunicazione tra App e dispositivo
-- Aggiungere dentro il file init.lua (file principale di scripting Wireshark) la riga:
-- dofile("seismocloud.lua")
-- Posizionando questo file nella stessa directory (o alternativamente inserendo il path
-- completo dove si trova il file.

seismoproto = Proto("seismoproto", "SeismoCloud Protocol")

function mac2string(buffer, i)
	return string.format('%02X', buffer(i, 1):uint())
		.. string.format('%02X', buffer(i+1, 1):uint())
		.. string.format('%02X', buffer(i+2, 1):uint())
		.. string.format('%02X', buffer(i+3, 1):uint())
		.. string.format('%02X', buffer(i+4, 1):uint())
		.. string.format('%02X', buffer(i+5, 1):uint());
end

function hex2float (c)
	if c == 0 then return 0.0 end
	local c = string.gsub(string.format("%08X", c),"(..)",function (x) return string.char(tonumber(x, 16)) end)
	local b1,b2,b3,b4 = string.byte(c, 1, 4)
	local sign = b1 > 0x7F
	local expo = (b1 % 0x80) * 0x2 + math.floor(b2 / 0x80)
	local mant = ((b2 % 0x80) * 0x100 + b3) * 0x100 + b4

	if sign then
		sign = -1
	else
		sign = 1
	end

	local n

	if mant == 0 and expo == 0 then
		n = sign * 0.0
	elseif expo == 0xFF then
		if mant == 0 then
			n = sign * math.huge
		else
			n = 0.0/0.0
		end
	else
		n = sign * math.ldexp(1.0 + mant / 0x800000, expo - 0x7F)
	end

	return n
end


function seismoproto.dissector(buffer, pinfo, tree)
	pinfo.cols.protocol = "SEISMOPROTO"
	if buffer(0, 4):uint() == 0x494e4756 then -- INGV in hex
		local i = 0
		local subtree = tree:add(seismoproto, buffer(), "SeismoCloud Protocol Data")
		subtree:add(buffer(i, 5), i .. ": Magic bytes")
		i = i + 5

		local command = "UNKNOWN"
		local cmdbyte = buffer(i, 1):uint()
		if cmdbyte == 1 then
			command = "DISCOVERY"
		elseif cmdbyte == 2 then
			command = "DISCOVERY_REPLY"
		elseif cmdbyte == 3 then
			command = "PING"
		elseif cmdbyte == 4 then
			command = "PONG"
		elseif cmdbyte == 5 then
			command = "STREAM_START"
		elseif cmdbyte == 6 then
			command = "STREAM_STOP"
		elseif cmdbyte == 7 then
			command = "SENDGPS"
		elseif cmdbyte == 8 then
            command = "OK"
        elseif cmdbyte == 9 then
            command = "SETSYSLOG"
        elseif cmdbyte == 10 then
            command = "REBOOT"
        elseif cmdbyte == 11 then
            command = "GETINFO"
        elseif cmdbyte == 12 then
			command = "GETINFO_REPLY"
		end
		subtree:add(buffer(i, 1), i .. ": " .. command .. " command")
		i = i + 1

		if cmdbyte == 2 then -- DISCOVERY_REPLY
			subtree:add(buffer(i, 6), i .. ": MAC Address: " .. mac2string(buffer, i));
			i = i + 6

			subtree:add(buffer(i, 4), i .. ": Version")
			i = i + 4

			subtree:add(buffer(i, 8), i .. ": Model")
			i = i + 8
		elseif cmdbyte == 7 then
			subtree:add(buffer(i, 6), i .. ": Dest. MAC Address: " .. mac2string(buffer, i));
			i = i + 6

			local lat = buffer(i, 1):uint() * 16777216
				+ buffer(i+1, 1):uint() * 65536
				+ buffer(i+2, 1):uint() * 256
				+ buffer(i+3, 1):uint();
			subtree:add(buffer(i, 4), i .. ": GPS Latitude: " .. hex2float(lat))
			i = i + 4

			local lon = buffer(i, 1):uint() * 16777216
				+ buffer(i+1, 1):uint() * 65536
				+ buffer(i+2, 1):uint() * 256
				+ buffer(i+3, 1):uint();
			subtree:add(buffer(i, 4), i .. ": GPS Longitude: " .. hex2float(lon))
			i = i + 4
        elseif cmdbyte == 12 then
            subtree:add(buffer(i, 6), i .. ": Dest. MAC Address: " .. mac2string(buffer, i));
            i = i + 6

            subtree:add(buffer(i, 4), i .. ": SYSLOG server");
            i = i + 4

            subtree:add(buffer(i, 4), i .. ": X threshold");
            i = i + 4

            subtree:add(buffer(i, 4), i .. ": Y threshold");
            i = i + 4

            subtree:add(buffer(i, 4), i .. ": Z threshold");
            i = i + 4

            subtree:add(buffer(i, 4), i .. ": Uptime: " .. buffer(i, 4):uint());
            i = i + 4

            subtree:add(buffer(i, 4), i .. ": UNIX time: " .. buffer(i, 4):uint());
            i = i + 4

            subtree:add(buffer(i, 4), i .. ": Software version");
            i = i + 4

            subtree:add(buffer(i, 4), i .. ": Free RAM");
            i = i + 4

            subtree:add(buffer(i, 4), i .. ": Latency");
            i = i + 4

            subtree:add(buffer(i, 4), i .. ": NTP server");
            i = i + 4

            local b = i
            local strsize = 0
            while buffer(b, 1):uint() > 0 do
                strsize = strsize + 1
                b = b + 1
            end

            subtree:add(buffer(i, strsize), i .. ": HTTP Base address")
            i = i + strsize

            b = i
            strsize = 0
            while buffer(b, 1):uint() > 0 do
                strsize = strsize + 1
                b = b + 1
            end

            subtree:add(buffer(i, strsize), i .. ": Platform name")
            i = i + strsize

            b = i
            strsize = 0
            while buffer(b, 1):uint() > 0 do
                strsize = strsize + 1
                b = b + 1
            end

            subtree:add(buffer(i, strsize), i .. ": Accelerometer name")
            i = i + strsize
		end
	end
end

udp_table = DissectorTable.get("udp.port")
udp_table:add(62001, seismoproto)
