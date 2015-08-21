
NET_MODULES := HTTPClient IPaddr NetworkManager NTP Tcp Udp
NET_SOURCES := $(NET_MODULES:%=net/%.cpp)
NET_OBJECTS := $(NET_MODULES:%=${OBJDIR}/net_%.o)

net:: $(NET_OBJECTS)

${OBJDIR}/net_%.o: net/%.cpp
	$(CPP) $(CPPFLAGS) -c -o $@ $<
