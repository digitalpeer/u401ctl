#
# u401ctl Makefile
#
# Copyright (C) 2010 by Josh Henderson <digitalpeer@digitalpeer.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

#
# This downloads and compiles a specific version of libusb so it doesn't depend
# on a system install of libusb.
#

LIBUSB_VERSION=0.1.12
LIBUSB_URL=http://downloads.sourceforge.net/project/libusb/libusb-0.1%20%28LEGACY%29/$(LIBUSB_VERSION)/libusb-$(LIBUSB_VERSION).tar.gz

CFLAGS=-Wall -g -O2 -I./libusb/include/ -L./libusb/lib

all: u401ctl

libusb-$(LIBUSB_VERSION).tar.gz:
	wget $(LIBUSB_URL) -O libusb-$(LIBUSB_VERSION).tar.gz

libusb-$(LIBUSB_VERSION)/.extracted: libusb-$(LIBUSB_VERSION).tar.gz
	tar xf libusb-$(LIBUSB_VERSION).tar.gz
	touch $@

libusb/.installed: libusb-$(LIBUSB_VERSION)/.extracted 
	( cd libusb-$(LIBUSB_VERSION) && \
		./configure --prefix=$(PWD)/libusb && \
		make && \
		make install; \
	)
	touch $@

u401ctl: u401ctl.c libusb/.installed
	$(CC) $(CFLAGS) $< -o $@ -static -lusb

clean:
	rm -rf libusb-$(LIBUSB_VERSION) libusb u401ctl 