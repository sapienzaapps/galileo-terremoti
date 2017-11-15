
NET_MODULES := HTTPClient IPaddr DataStream Tcp Udp TraceAccumulator MQTT SCSAPI_MQTT SCSAPI_HTTP SCSAPI_LocalTest MQTT_Subscribe WebSocket
NET_SOURCES := $(NET_MODULES:%=net/%.cpp)
NET_OBJECTS := $(NET_MODULES:%=${OBJDIR}/net_%.o)

net:: $(NET_OBJECTS)

${OBJDIR}/net_%.o: net/%.cpp net/%.h
	$(CPP) $(CPPFLAGS) ${MAINFLAGS} -c -o $@ $<
