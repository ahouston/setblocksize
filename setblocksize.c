/*
********************************************************************************
Header

Project:        setblocksize

This file:      setblocksize.c
Version:        V0.2

Description:    Reformat SCSI disk with specified block size
                Thanks to Seagate for providing protocol information

		Return value:  0 on success, 1 on error

Copyright:      (C) 2003 by Michael Baeuerle
License:        GPL V2 or any later version

Language:       C
Style rules:    -

Written for:    Platform:       all
                OS:             GNU/Linux 
Tested with:    Compiler:       gcc (Version: 2.91.66)
                Platform:       IA32 (Pentium)
                OS:             GNU/Linux (Kernel version: 2.2.10)
Tested with:    Compiler:       gcc (Version: 2.96.1)
                Platform:       IA32 (Pentium2)
                OS:             GNU/Linux (Kernel version: 2.4.21)
Tested with:    Compiler:       gcc (Version: 3.3.6)
                Platform:       IA32 (PentiumPro)
                OS:             GNU/Linux (Kernel version: 2.6.16.20)
Do not work:    Platform:       non GNU/Linux

Created:        2003-03-22 by Michael Baeuerle
Last mod.:      2007-09-04 by Michael Baeuerle

Changelog:

V0.0            Initial version (Inspired by sg-utils)
                LUN and blocksize must be specified at compile time

V0.1            Print manufacturer and model name of selected device and
                 ask if this is the desired device (using INQUIRY command)
                LUN selection removed: ID and LUN are implicit specified with
		 the sg device (The LUN value of V0.0 was ignored)
                Also print ID, LUN, Host and channel numbers
		Check for device type supports the FORMAT UNIT command

V0.2            The new blocksize can now be specified on command line
                The program now aborts if a SCSI command reports CHECK CONDITION
                The timeout can now be specified on the command line


To do:          -
********************************************************************************
*/


/*
********************************************************************************
* Include files
********************************************************************************
*/

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/param.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>
#include "sg_err.h" 


/*
********************************************************************************
* Global constants
********************************************************************************
*/

#define TIMEOUT  (48000 * HZ)            /* 800 minute FORMAT UNIT default timeout */
#define BS  512                         /* Default blocksize */
const char  NAME[] = "setblocksize";
const char  VER[] = "V0.2";


/*
********************************************************************************
* Main function
********************************************************************************
*/

int  main(int  argc, char**  argv)
{
   unsigned short int  bs = BS;
   int  timeout = TIMEOUT;
   int  sg_fd;
   int  i;
   int  ok;
   int  buf;
   char  sbuf[256];  
   char*  file_name = NULL;
   unsigned char  scsi_buf[65536];
   struct sg_header*  sghp = (struct sg_header*) scsi_buf;
   Sg_scsi_id  device;
   /* INQUIRY command */
   unsigned char  inquiry[6] = {0x12, 0x00, 0x00, 0x00, 0x20, 0x00};
   /* MODE SELECT command */
   unsigned char  mode_select[6] = {0x15, 0x11, 0x00, 0x00, 0x0C, 0x00};
   /* FORMAT UNIT command */
   unsigned char  format_unit[6] = {0x04, 0x00, 0x00, 0x00, 0x00, 0x00};
   /* Parameter list with block descriptor */
   unsigned char  para_list[12] = {0x00, 0x00, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
   int  inquiry_data_len = sizeof(struct sg_header) + 0x06;
   int  mode_select_data_len = sizeof(struct sg_header) + 0x06 + 0x0C;
   int  format_unit_data_len = sizeof(struct sg_header) + 0x06;

   /* Print info */
   sprintf(sbuf, "\n");
   strcat(sbuf, NAME);
   strcat(sbuf, " ");
   strcat(sbuf, VER);
   strcat(sbuf, "\n\n");
   printf(sbuf);

   /* Check parameters */
   printf("Checking parameters ...\n");
   for (i = 1; i < argc; i++)
   {
      if (*argv[i] == '-')
      {
         if (!strncmp(argv[i], "-b", 2))
         {
            /* Use specified blocksize */
            printf("   Blocksize specified.\n");
            ok = sscanf(argv[i], "-b%d", &buf);
            bs = (unsigned short int) buf;
            if (ok != 1)  break;
         }
         else if (!strncmp(argv[i], "-t", 2))
         {
            /* Use specified timeout */
            printf("   Timeout specified.\n");
            ok = sscanf(argv[i], "-t%d", &buf);
            if ((buf < 1) || (buf > 1800)) break;
            timeout =  buf * 60 * HZ;
            if (ok != 1)  break;
         }
         else
         {
            printf("   Unknown parameter: %s\n", argv[i]);
            file_name = 0;
            break;
         }
      }
      else
      { 
         if (file_name == NULL)
            file_name = argv[i];
         else
         {
           printf("   Parameter error\n");
           file_name = 0;
           break;
         }
      }
   }
   if (file_name == NULL)
   {
      /* Parameter error, print help message */
      fprintf(stderr, "   Parameter error!\n");
      sprintf(sbuf, "   Usage: '");
      strcat(sbuf, NAME);
      strcat(sbuf,
       " [-b<Blocksize in Byte>] [-t<Timeout in Minutes>] <sg_device>'\n\n");
      fprintf(stderr, sbuf);
      exit(1);
   }
   printf("   Done.\n");

   /* Set new block size */
   printf("New blocksize: %u Bytes\n", (unsigned int) bs);
   para_list[10] = (unsigned char) ((bs & 0xFF00) >> 8);
   para_list[11] = (unsigned char) (bs & 0x00FF);

   /* Print timeout */
   printf("Format timeout: %d minutes\n", (timeout / 60) / HZ);

   /* Open device file */
   printf("Open device file ...\n");
   fflush(stdout);
   if ((sg_fd = open(file_name, O_RDWR | O_EXCL)) < 0)
   {
      fprintf(stderr, "   File open error! (root permissions?)\n\n");
      exit(1);
   }
   /* Just to be safe, check we have a sg device by trying an ioctl */
   if (ioctl(sg_fd, SG_GET_TIMEOUT, NULL) < 0)
   {
      fprintf(stderr, "   File open error!\n");
      fprintf(stderr, "   '%s' doesn't seem to be a sg device\n\n", file_name);
      close(sg_fd);
      exit(1);
   }
   printf("   Done.\n");

   /* Send INQUIRY command */
   printf("Prepare command ...\n");
   fflush(stdout);
   sghp->reply_len = sizeof(struct sg_header) + 0x20;
   sghp->pack_id = 0;
   sghp->twelve_byte = 0;
   memcpy(scsi_buf + sizeof(struct sg_header), inquiry, 0x06);
   printf("   Done.\n");
   printf("Send INQUIRY command ...\n");
   fflush(stdout);
   if (write(sg_fd, scsi_buf, inquiry_data_len) < 0)
   {
      fprintf(stderr, "   Write error\n\n");
      close(sg_fd);
      exit(1);
   }
   /* Read status (sense_buffer) */
   if (read(sg_fd, scsi_buf, sizeof(struct sg_header) + 0x20) < 0)
   {
      fprintf(stderr, "   Read error\n\n");
      close(sg_fd);
      exit(1);
   }
   printf("   Done.\n");
   /* Error processing */
   printf("Check status ...\n");
   fflush(stdout);
   if (sghp->pack_id != 0)              /* This shouldn't happen */
      printf("   Inquiry pack_id mismatch: Wanted=%d, Got=%d\n!", 
       0, sghp->pack_id);
   ok = 0;
   switch (sg_err_category(sghp->target_status, sghp->host_status,
    sghp->driver_status, sghp->sense_buffer, SG_MAX_SENSE))
    {
      case SG_ERR_CAT_CLEAN:
         ok = 1;
         break;
      case SG_ERR_CAT_RECOVERED:
         printf("   Recovered error, continue\n");
         ok = 1;
         break;
      default:
         sg_chk_n_print("   Error", sghp->target_status, 
          sghp->host_status, sghp->driver_status, 
          sghp->sense_buffer, SG_MAX_SENSE);
         break;
   }
   if (ok)
      printf("   Command successful.\n");
   else
   {
      fprintf(stderr, "   Command NOT succesful!\n\n");
      close(sg_fd);
      exit(1);
   }

   /* Check for LUN to be valid */
   printf("Check for LUN ...\n");
   switch(scsi_buf[sizeof(struct sg_header)] >> 5)
   {
      case 0:
         printf("   LUN present.\n");
	 break;
      case 1:
         fprintf(stderr, "   Error: LUN supported but not present!\n\n");
         close(sg_fd);
         exit(1);
      case 3:
         fprintf(stderr, "   Error: LUN not supported by this device!\n\n");
         close(sg_fd);
         exit(1);
      default:
         fprintf(stderr, "   Error: Cannot determine status of LUN!\n\n");
         close(sg_fd);
         exit(1);
   }
   if (ioctl(sg_fd, SG_GET_SCSI_ID, &device) < 0)
   {
      fprintf(stderr, "   Cannot determine ID & LUN numbers!\n");
      close(sg_fd);
      exit(1);
   }

   /* Print device name and ask for OK */
   printf("\n=================================================================\
==============\n");
   printf("SCSI ID     : %d\n", device.scsi_id);
   printf("LUN         : %d\n", device.lun);
   printf("Connected to: Host%d / Channel%d\n", device.host_no, device.channel);
   strncpy(sbuf, scsi_buf + sizeof(struct sg_header) + 0x08, 0x08);
   sbuf[0x08] = 0x00;
   printf("Manufacturer: %s\n", sbuf);
   strncpy(sbuf, scsi_buf + sizeof(struct sg_header) + 0x10, 0x10);
   sbuf[0x10] = 0x00;
   printf("Model       : %s\n", sbuf);
   ok = 0;
   switch (scsi_buf[sizeof(struct sg_header)] & 0x1F)
   {
      case 0:
         sprintf(sbuf, "Disk");
	 ok = 1;
	 break;
      case 1:
         sprintf(sbuf, "Tape");
	 break;
      case 2:
         sprintf(sbuf, "Printer");
	 break;
      case 3:
         sprintf(sbuf, "Processor");
	 break;
      case 4:
         sprintf(sbuf, "WORM");
	 break;
      case 5:
         sprintf(sbuf, "CDROM");
	 break;
      case 6:
         sprintf(sbuf, "Scanner");
	 break;
      case 7:
         sprintf(sbuf, "Optical disk");
	 ok = 1;
	 break;
      case 8:
         sprintf(sbuf, "Media changer");
	 break;
      case 9:
         sprintf(sbuf, "Communication");
	 break;
      case 12:
         sprintf(sbuf, "Storage array controller");
	 break;
      default:
         sprintf(sbuf, "Unknown");
   }   
   printf("Device type : %s\n", sbuf);
   printf("=================================================================\
==============\n");
   if (!ok)
   {
      fprintf(stderr, "This type of device do not support the FORMAT UNIT \
command!\n");
      printf("Exiting ...\n\n");
      close(sg_fd);
      exit(1);
   }
   printf("Do you really want to reformat this device [y/n]? ");
   fflush(stdout);
   fscanf(stdin, "%c", &sbuf[0]);
   printf("\n");
   if (sbuf[0] != 'y')
   {
      printf("Aborted.\n\nExiting ...\n\n");
      close(sg_fd);
      exit(1);   
   }

   /* Send MODE SELECT command */
   printf("Prepare command ...\n");
   fflush(stdout);
   sghp->reply_len = sizeof(struct sg_header);
   sghp->pack_id = 0;
   sghp->twelve_byte = 0;
   memcpy(scsi_buf + sizeof(struct sg_header), mode_select, 0x06);
   memcpy(scsi_buf + sizeof(struct sg_header) + 6, para_list, 0x0C);
   printf("   Done.\n");
   printf("Send MODE SELECT command ...\n");
   fflush(stdout);
   if (write(sg_fd, scsi_buf, mode_select_data_len) < 0)
   {
      fprintf(stderr, "   Write error\n\n");
      close(sg_fd);
      exit(1);
   }
   /* Read status (sense_buffer) */
   if (read(sg_fd, scsi_buf, sizeof(struct sg_header)) < 0)
   {
      fprintf(stderr, "   Read error\n\n");
      close(sg_fd);
      exit(1);
   }
   printf("   Done.\n");
   /* Error processing */
   printf("Check status ...\n");
   fflush(stdout);
   if (sghp->pack_id != 0)              /* This shouldn't happen */
      printf("   Inquiry pack_id mismatch: Wanted=%d, Got=%d\n!", 
       0, sghp->pack_id);
   ok = 0;
   switch (sg_err_category(sghp->target_status, sghp->host_status,
    sghp->driver_status, sghp->sense_buffer, SG_MAX_SENSE))
   {
      case SG_ERR_CAT_CLEAN:
         ok = 1;
         break;
      case SG_ERR_CAT_RECOVERED:
         printf("   Recovered error, continue\n");
         ok = 1;
         break;
      default:
         sg_chk_n_print("   Error", sghp->target_status, 
          sghp->host_status, sghp->driver_status, 
          sghp->sense_buffer, SG_MAX_SENSE);
         break;
   }
   if (ok)
      printf("   Command successful.\n");
   else
   {
      fprintf(stderr, "   Command NOT succesful!\n\n");
      close(sg_fd);
      exit(1);
   }

   /* Send FORMAT UNIT command */
   printf("Prepare command ...\n");
   fflush(stdout);
   sghp->reply_len = sizeof(struct sg_header);
   sghp->pack_id = 0;
   sghp->twelve_byte = 0;
   memcpy(scsi_buf + sizeof(struct sg_header), format_unit, 0x06);
   buf = timeout;
   if (ioctl(sg_fd, SG_SET_TIMEOUT, &buf) < 0)
   {
      fprintf(stderr, "   Error!\n");
      fprintf(stderr, "   Cannot set timeout\n\n");
      close(sg_fd);
      exit(1);
   }
   printf("   Done.\n");
   printf("Send FORMAT UNIT command ...\n");
   fflush(stdout);
   if (write(sg_fd, scsi_buf, format_unit_data_len) < 0)
   {
      fprintf(stderr, "   Write error\n\n");
      close(sg_fd);
      exit(1);
   }
   /* Read status (sense_buffer) and data */
   printf("   *** Please wait - Do not manually interrupt or power down! ***\n");
   if (read(sg_fd, scsi_buf, sizeof(struct sg_header)) < 0)
   {
      fprintf(stderr, "   Read error\n\n");
      close(sg_fd);
      exit(1);
   }
   printf("   Done.\n");
   /* Error processing */
   printf("Check status ... \n");
   if (sghp->pack_id != 0)              /* This shouldn't happen */
      printf("   Inquiry pack_id mismatch: Wanted=%d, Got=%d\n!", 
         0, sghp->pack_id);
   ok = 0;
   switch (sg_err_category(sghp->target_status, sghp->host_status,
    sghp->driver_status, sghp->sense_buffer, SG_MAX_SENSE))
   {
      case SG_ERR_CAT_CLEAN:
         ok = 1;
         break;
      case SG_ERR_CAT_RECOVERED:
         printf("   Recovered error, continue\n");
         ok = 1;
         break;
      default:
         sg_chk_n_print("   Error", sghp->target_status, 
            sghp->host_status, sghp->driver_status, 
            sghp->sense_buffer, SG_MAX_SENSE);
         break;
   }
   if (ok)
      printf("   Command successful.\n");
   else
   {
      fprintf(stderr, "   Command NOT succesful!\n\n");
      close(sg_fd);
      exit(1);
   }

   /* Close device file */
   printf("Close device file ...\n");
   close(sg_fd);
   printf("   Done.\n");

   /* Exit */
   printf("\nExiting ...\n\n");
   exit(0);
}


/* EOF */
