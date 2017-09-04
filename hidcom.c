//$Id: hidcom.c 567 2014-12-08 00:16:48Z gaul1 $
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
/*
  * 2016 Enhanced by gaul1@lifesim.de, see also: https://github.com/rundekugel/HidCom
  * 2017 added: strings as input; serial number as filter 
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/hiddev.h>
#include <linux/input.h>
#include <time.h>
#include <ctype.h>
#include <getopt.h>
#include <string.h>


#if 0	//use defaults
 #define FREE_VID		0x16c0
 #define HID_PID		0x27d9
#else
 #define FREE_VID		0
 #define HID_PID		0
#endif

//prototypes
//void getStringByIdx(int fd, struct hiddev_string_descriptor HString, int index);
//void showDesc(int fd);

#define SHOW_DESC 0
#if SHOW_DESC	>0 //prepared for future use
void showDesc(int fd){
  int i,j;
  int ret;
  struct hiddev_report_info rinfo;
  struct hiddev_field_info finfo;
  struct hiddev_usage_ref uref;
  
  rinfo.report_type = HID_REPORT_TYPE_INPUT;
  rinfo.report_id = HID_REPORT_ID_FIRST;
  ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
   
  while (ret >= 0) {
    for (i = 0; i < rinfo.num_fields; i++) {
      finfo.report_type = rinfo.report_type;
      finfo.report_id = rinfo.report_id;
      finfo.field_index = i;
      ioctl(fd, HIDIOCGFIELDINFO, &finfo);
      for (j = 0; j < finfo.maxusage; j++) {
        uref.report_type = rinfo.report_type;
        uref.report_id = rinfo.report_id;
        uref.field_index = i;
        uref.usage_index = j;
        ioctl(fd, HIDIOCGUCODE, &uref);
        ioctl(fd, HIDIOCGUSAGE, &uref);
        printf("i,j:%d,%d;typ,id:%d,%d; code[$%x]/index[%d]:%x.\r\n",
          i,j, uref.report_type, uref.report_id,
          uref.usage_code, uref.usage_index, uref.value
          
        );
      }
    }
    rinfo.report_id |= HID_REPORT_ID_NEXT;
    ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
   }
}
#endif

void getStringByIdx(int fd,  struct hiddev_string_descriptor* HString, int index){
    HString->index = index;
    HString->value[0]=0;
    ioctl(fd, HIDIOCGSTRING, HString);
}

int main (int argc, char **argv) {
  int n=64,d=0,info=0,increment=0,vid=FREE_VID,pid=HID_PID,q=0,f=0,ctrl=0,r=1,c,i,j;
  int stringNum = -1;
  char serial[256]= {0,0};
  char path[256],buf[256];
  path[0]=0;
  for(i=0;i<256;i++) buf[i]=0;
  opterr = 0;
  int option_index = 0;
  struct option long_options[] =
  {
    {"control", no_argument,      &ctrl,1},
    {"c",       no_argument,      &ctrl,1},
    {"delay",   required_argument, 0, 'd'},
    {"d",       required_argument, 0, 'd'},
    {"feature", no_argument,         &f,1},
    {"f",       no_argument,         &f,1},
    {"help",    no_argument,       0, 'h'},
    {"i",       no_argument,      &info,1},
    {"info",    no_argument,      &info,1},
    {"increment",no_argument,&increment,1},
    {"I",       no_argument, &increment,1},
    {"serial",  required_argument, 0, 'n'},
    {"path",    required_argument, 0, 'P'},
    {"pid",     required_argument, 0, 'p'},
    {"quiet",   no_argument,        &q, 1},
    {"q",       no_argument,        &q, 1},
    {"repeat",  required_argument, 0, 'r'},
    {"size",    required_argument, 0, 's'},
    {"string",  required_argument, 0, 'S'},
    {"vid",     required_argument, 0, 'v'},
    {0, 0, 0, 0}
  };
  while ((c = getopt_long (argc, argv, "cd:fhiIn:p:qv:r:s:S:",long_options,&option_index)) != -1)
     switch (c)
    {
    case '?':
    case 'h':
    printf("hidcom $Rev: 567 $ usage:\
    \nhidCom [options] [data]\
    \noptions:\
    \n-h, --help       help\
    \n-c, --control    use control transfer [no]\
    \n-d, --delay      read delay (ms) [0]\
    \n-f, --feature    use feature report [no]\
    \n-i, --info       device info [no]\
    \n-I, --increment  increment byte 6 every transfer [no]\
    \n-n, --serial     serial number\
    \n--path           device path [/dev/usb/hiddev0]\
    \n-p, --pid        Product ID [0]\
    \n-q, --quiet      print response only [no]\
    \n-r, --repeat     repeat N times [1]\
    \n-s, --size       report size [64]\
    \n-S, --String     get descriptor string with given offset 0..255\
    \n-v, --vid        Vendor ID [0]\
    \n example:  HidCom -i 1 2 3 4\n");
    exit(1);
    break;
    case 'c':  //control endpoint
       ctrl=1;
             break;
    case 'd':  //delay
      d = atoi(optarg);
      break;
    case 'f':  //feature report
      f=1;
      break;
    case 'i':  //info
      info=1;
      break;
    case 'I':  //increment
      increment=1;
      break;
    case 'p':  //pid
      sscanf(optarg, "%x", &pid);
      break;
    case 'n':  //serial num
      strncpy(serial,optarg,256);
      //printf("given serial:%s\r\n", serial);
      break;
    case 'P':  //path
      strncpy(path,optarg,256);
      break;
    case 'q':  //quiet
      q=1;
      break;
    case 'r':  //repeat
      r = atoi(optarg);
      break;
    case 's':  //report size
      n = atoi(optarg);  //No always add report index
      break;
    case 'S':  //string num to read
      stringNum = atoi(optarg);  
      break;
    case 'v':  //vid
      sscanf(optarg, "%x", &vid);
      break;
    //case '?':
      //fprintf (stderr, "Unknown option character 0x%02x (%c)\n",optopt,optopt);
      //return 1;
    default:
      fprintf (stderr, "Unknown option character 0x%02x (%c)\n",optopt,optopt);
      return 1;    
     //abort ();     
    break;
    }

  for (j=0,i = optind; i < argc&&i<128; i++,j++) sscanf(argv[i], "%x", (unsigned int*)&buf[j] );
  int fd = -1;
  struct hiddev_devinfo device_info;    
  if(pid && vid){
    for(i=0;i<20;i++){
      sprintf(path,"/dev/usb/hiddev%d",i);
      if ((fd = open(path, O_RDONLY )) >= 0){
        ioctl(fd, HIDIOCGDEVINFO, &device_info);
        printf("hid.vid=%04x;pid=%04x;ver=%04x;\r\n",
          device_info.vendor,
          device_info.product,
          device_info.version);
        if(device_info.vendor==vid && device_info.product==pid) i=29;
        
      }
    }
    if(i==20){
      printf("Can't find device");
      exit(1);
    }
  }
  else{ 
    if(!path[0]) sprintf(path,"/dev/usb/hiddev0");
    if ((fd = open(path, O_RDONLY )) < 0) {  //O_RDWR
      perror("hiddev open");
      exit(1);
    }    
  }  
  if(info){
    int i;
    struct hiddev_string_descriptor Hstring1,Hstring2;
    Hstring1.index=1;
    Hstring1.value[0]=0;
    Hstring2.index=2;
    Hstring2.value[0]=0;
    ioctl(fd, HIDIOCGDEVINFO, &device_info);
    printf("VID: 0x%04hX  PID: 0x%04hX  version: 0x%04hX\n",
      device_info.vendor, device_info.product, device_info.version);
    printf("Applications: %i\n", device_info.num_applications);
    printf("Bus: %d Devnum: %d Ifnum: %d\n",
      device_info.busnum, device_info.devnum, device_info.ifnum);

    printf("try to read descriptor strings...\n");
    if(device_info.vendor == 0x22c9){ //dirty hack for StepOver Devices
      Hstring1.index=4;
      Hstring2.index=32;
    }
    ioctl(fd, HIDIOCGSTRING, &Hstring1);
    ioctl(fd, HIDIOCGSTRING, &Hstring2);
    printf("Path: %s\nManufacturer: %s\nProduct: %s\n",path,Hstring1.value,Hstring2.value); 
    if(device_info.vendor == 0x22c9){ //StepOver Device
      if(device_info.product < 4) { //bootloader
        Hstring1.index = 54; }
      getStringByIdx(fd, &Hstring1, Hstring1.index);
      if(Hstring1.value[0]){
        printf("Serial number: %s\r\n", Hstring1.value);    
      }
    }
    #if SHOW_DESC	>0     
      showDesc(fd);
    #else
    int n=0;
    printf("read all strings...\n");
    for( i=0; i<0xff;i++){
      getStringByIdx(fd, &Hstring1, i);
      if(Hstring1.value[0]){
        printf("more[idx=%d]: %s\r\n", i, Hstring1.value);
        n++;
        if(n>2) i=0xffff;
      }
    }
    #endif
  }
  
  if(stringNum >0){
    struct hiddev_string_descriptor HstringSN;
    
    HstringSN.index = stringNum;
    HstringSN.value[0]=0;
    ioctl(fd, HIDIOCGSTRING, &HstringSN);
    if(HstringSN.value[0]){
      printf("%s\r\n", HstringSN.value);
    }  
    close(fd);  
    exit(0);
  }
  
  struct hiddev_report_info rep_info_i,rep_info_u;  
  struct hiddev_usage_ref_multi ref_multi_i,ref_multi_u;
  rep_info_u.report_type=HID_REPORT_TYPE_OUTPUT;
  rep_info_i.report_type=HID_REPORT_TYPE_INPUT;
  rep_info_u.report_id=rep_info_i.report_id=HID_REPORT_ID_FIRST;
  rep_info_u.num_fields=rep_info_i.num_fields=1;
  ref_multi_u.uref.report_type=HID_REPORT_TYPE_OUTPUT;
  ref_multi_i.uref.report_type=HID_REPORT_TYPE_INPUT;
  ref_multi_u.uref.report_id=ref_multi_i.uref.report_id=HID_REPORT_ID_FIRST;
  ref_multi_u.uref.field_index=ref_multi_i.uref.field_index=0;
  ref_multi_u.uref.usage_index=ref_multi_i.uref.usage_index=0;
  ref_multi_u.num_values=ref_multi_i.num_values=n;
  for(i=0;i<n;i++) ref_multi_u.values[i]=buf[i];
  int res=0;
  if(f){        //use feature report
      rep_info_u.report_type=rep_info_i.report_type=HID_REPORT_TYPE_FEATURE;
      ref_multi_u.uref.report_type=ref_multi_i.uref.report_type=HID_REPORT_TYPE_FEATURE;
  }
  for(j=0;j<r;j++){
    if(!q){ 
      printf("-> ");
      for(i=0;i<n;i++) printf("%02X ",(unsigned char)ref_multi_u.values[i]);
      printf("\n");
    }
    if(f){        //use feature report
      ioctl(fd,HIDIOCSUSAGES, &ref_multi_u);
      ioctl(fd,HIDIOCSREPORT, &rep_info_u);
      usleep(d*1000);
      ioctl(fd,HIDIOCGREPORT, &rep_info_i);
      ioctl(fd,HIDIOCGUSAGES, &ref_multi_i);
    }
    else if(ctrl){    //use control endpoint, at least for reading
      ioctl(fd,HIDIOCSUSAGES, &ref_multi_u);
      ioctl(fd,HIDIOCSREPORT, &rep_info_u);
      usleep(d*1000);
      ioctl(fd,HIDIOCGREPORT, &rep_info_i);
      ioctl(fd,HIDIOCGUSAGES, &ref_multi_i);
    }
    else{        //use interrupt endpoint
      ioctl(fd,HIDIOCSUSAGES, &ref_multi_u); 
      ioctl(fd,HIDIOCSREPORT, &rep_info_u);
      usleep(d*1000);
      ioctl(fd,HIDIOCGUSAGES, &ref_multi_i); 
      ioctl(fd,HIDIOCGREPORT, &rep_info_i);
    }
    if(!q) printf("<- ");
    for(i=0;i<n;i++) printf("%02X ",ref_multi_i.values[i]);
    printf("\n");
    if(increment) ref_multi_u.values[5]++; 
  }
  close(fd);
  exit(res);
}
