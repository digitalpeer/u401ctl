/**
 * Simple utility that allows you to control a U401 USB device.
 *
 * Copyright (C) 2010 by Josh Henderson <digitalpeer@digitalpeer.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "usb.h"

#include <stdio.h>
#include <string.h>

#ifdef DEBUG
#define DBG(format, args...) fprintf(stdout,format, ## args);
#else
#define DBG(format, args...) /* */
#endif

#define ERR(format, args...) fprintf(stderr,format, ## args);
#define WARN(format, args...) fprintf(stdout,format, ## args);
#define INFO(format, args...) fprintf(stdout,format, ## args);

#define VENDOR_ID  0x0DE7
#define PRODUCT_ID 0x0191

/*
 * Try to find a U401 device on the provided bus.
 */
static struct usb_device* find_U401(struct usb_bus* bus)
{
   struct usb_device* dev;
   
   for (; bus; bus = bus->next)
   {
      for (dev = bus->devices; dev; dev = dev->next)
      {
	 if (dev->descriptor.idVendor == VENDOR_ID && 
	     dev->descriptor.idProduct == PRODUCT_ID)
	 {
	    return dev;
	 }
      }
   }

   return NULL;
}

static inline void buffer_set(char *buf, int a, int b, int c, int d, int e, int f, int g, int h)
{
   buf[0] = a;
   buf[1] = b;
   buf[2] = c;
   buf[3] = d;
   buf[4] = e;
   buf[5] = f;
   buf[6] = g;
   buf[7] = h;
}

static int send_command(struct usb_dev_handle *handle, char *command, int comLen, int resLen)
{
   int ret = usb_control_msg( handle, 0x21, 9, 0x0200, 0, command, comLen, 5000 );

   // check that send was successful
   if (ret != comLen)
      return -1;

   // does the command expect a result?
   if ( resLen > 0 )
   {
      ret = usb_bulk_read( handle, 0x81, command, resLen, 5000 );
      if ( ret != resLen )
	 return -2;
   }
   
   return ret;
}

static int parse_command(const char* command, const char** key, const char** value)
{
   static char buffer[1024];
   if (strlen(command) > sizeof(buffer)-1)
      return -1;
   strcpy(buffer,command);
   *key = strtok(buffer, "= ");
   *value = strtok(NULL, "= ");
   return *key && *value ? 0 : -1;
}

static inline int get_bit(int bit)
{
   int res = 0;
   return res | (1<<bit);
}

static void usage(const char* base)
{
   printf("Usage: %s [OUTPUT]=[on|off] ...\n"
	  "Where OUTPUT is 0-7,all and set equal to on or off.\n"
	  ,base);
}

int main(int argc, const char** argv)
{
   int x;
   int result = 0;
   int busses;
   int devices;
   int ret;
   struct usb_bus *bus_list;
   struct usb_device *dev = NULL;
   struct usb_dev_handle *handle;
   char buffer[8];

   if (argc < 2 || 
       strcmp(argv[1],"-h") == 0 || 
       strcmp(argv[1],"--help") == 0)
   {
      usage(argv[0]);
      return 1;
   }

   usb_set_debug(0);

   // initialize the usb system
   usb_init();
   
   // update info on busses
   busses = usb_find_busses(); 

   // update info on devices
   devices = usb_find_devices();

   // get actual bus objects 
   bus_list = usb_get_busses(); 

   if ((dev = find_U401(bus_list)) == NULL)
   {
      ERR("Can't find a U401 device plugged in.\n");
      return -1;
   }

   if ((handle = usb_open(dev) ) == NULL)
   {
      ERR("A U401 was found, but cannot open it.\n");
      return -1;
   }

   usb_detach_kernel_driver_np(handle, 0);

   int r;
   if ((r = usb_claim_interface( handle, 0 )) )
   {
      ERR("A U401 was found, but it couldn't be claimed.\n");
      result = -1;
      goto cleanup;
   }

   usb_set_altinterface(handle,0);

   if (usb_set_configuration(handle, 1))
   {
     DBG("Unable to configure U401.\n");
   }

   // set port A as output
   buffer_set( buffer, 0x09, 0xFF, 0xff, 0, 0, 0, 0, 0 );
   ret = send_command( handle, buffer, 8, 0 );

   for (x = 1; x < argc;x++)
   {
      const char* key = 0;
      const char* value = 0;

      if (parse_command(argv[x],&key,&value))
      {
      	 ERR("Unable to parse %s\n",argv[x]);
	 result = -1;
	 goto cleanup;
      }

      int mask = 0;
      if (!strcasecmp(key,"all"))
      {
	 mask = 0xFF;
      }
      else
      {
	 mask = get_bit(atoi(key));	 	 
      }

      DBG("%d=%s\n",mask,value);
	 
      if (!strcasecmp(value,"on"))
      {
	 buffer_set( buffer, 0x03, 0xFF, mask, 0, 0, 0, 0, 0 );
	 ret = send_command( handle, buffer, 8, 0 );

	 if (ret != 8)
	 {
	    ERR("Failed to set output\n");
	    result = -1;
	    goto cleanup;
	 }
      }
      else if (!strcasecmp(value,"off"))
      {
	 buffer_set( buffer, 0x03, ~mask, 0, 0, 0, 0, 0, 0 );
	 ret = send_command( handle, buffer, 8, 0 );

	 if (ret != 8)
	 {
	    ERR("Failed to set output\n");
	    result = -1;
	    goto cleanup;
	 }
      }
      else
      {
	 ERR("Expecting 'on' or 'off' status for value in %s\n",argv[x]);
      }
   }

 cleanup:

   if (usb_release_interface(handle, 0) || usb_close(handle))
   {
      ERR("Failed to release interface\n");
      result = -1;
   }

   return result;
}
