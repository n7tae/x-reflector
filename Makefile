# Copyright (c) 2018 by Thomas A. Early N7TAE

# if you change these locations, make sure the sgs.service file is updated!
BINDIR=/usr/local/bin
CFGDIR=/usr/local/etc

# choose this if you want debugging help
#CPPFLAGS=-g -ggdb -W -Wall -std=c++11 -DCFG_DIR=\"$(CFGDIR)\"
# or, you can choose this for a much smaller executable without debugging help
CPPFLAGS=-W -Wall -std=c++11 -DCFG_DIR=\"$(CFGDIR)\"

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

xrfd :  $(OBJS)
	g++ $(CPPFLAGS) -o sgs $(OBJS) -lconfig++ -pthread

%.o : %.cpp
	g++ $(CPPFLAGS) -MMD -MD -c $< -o $@

.PHONY: clean

clean:
	$(RM) $(OBJS) $(DEPS) xrfd

-include $(DEPS)

# install, uninstall need root priviledges
install : xrfd
	/bin/cp -f xrf $(BINDIR)
	/bin/ln -s $(shell pwd)/xrfd.cfg $(CFGDIR)
	/bin/cp -f xrfd.service /lib/systemd/system
	systemctl enable xrfd.service
	systemctl daemon-reload
	systemctl start xrfd.service

uninstall :
	systemctl stop xrfd.service
	systemctl disable xrfd.service
	/bin/rm -f /lib/systemd/system/xrfd.service
	systemctl daemon-reload
	/bin/rm -f $(BINDIR)/xrfd
	/bin/rm -f $(CFGDIR)/xrfd.cfg
