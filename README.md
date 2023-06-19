# HidCom
Easy Linux command line tool to communicate with USB/HID devices

I use it often for debugging with USB/HID. You can also use it to speak with USB/HID via bash scripts.
## Build
build with

    gcc hidcom.c -o hidcom
## Use
hidCom [options] [path of USB-Device] [data]    
options:    
-h, --help       help    
-c, --control    use control transfer [no]    
-d, --delay      read delay (ms) [0]    
-f, --feature    use feature report [no]    
-i, --info       device info [no]    
-I, --increment  increment byte 6 every transfer [no]    
-n, --serial     serial number    
--path           device path [/dev/usb/hiddev0]    
-p, --pid        Product ID (HEX) [0]    
-q, --quiet      print response only [no]    
-r, --repeat     repeat N times [1]    
-s, --size       report size [64]    
-S, --String     get String N    
-v, --vid        Vendor ID (HEX) [0]    

Select device by path or vid and pid. If not given, the /dev/usb/hiddev0 is used as default

## Example

    HidCom -i 1 2 3 4
  

