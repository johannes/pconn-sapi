pconn: $(SAPI_PCONN_PATH)

$(SAPI_PCONN_PATH): $(PHP_GLOBAL_OBJS) $(PHP_SAPI_OBJS) $(PHP_PCONN_OBJS)
	$(BUILD_PCONN)

