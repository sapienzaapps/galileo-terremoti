
NET_MODULES := HTTPClient IPaddr NetworkManager Tcp Udp TraceAccumulator MQTT SCSAPI_MQTT SCSAPI_HTTP MQTT_Subscribe MQTT_Publish
NET_SOURCES := $(NET_MODULES:%=net/%.cpp)
NET_OBJECTS := $(NET_MODULES:%=${OBJDIR}/net_%.o)

net:: $(NET_OBJECTS)

${OBJDIR}/net_%.o: net/%.cpp
	$(CPP) $(CPPFLAGS) ${MAINFLAGS} -c -o $@ $<
