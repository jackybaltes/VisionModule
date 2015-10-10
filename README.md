# VisionModule
Simple vision module for our robots.

## Dependencies

You may need to install the following dependencies using apt-get.

         + cmake
         + libbost-all-dev
         + libjpeg-dev
         + libv4l-dev

## Compilation

Clone the repository (or download the zip file).

    $ git clone https://github.com/jackybaltes/VisionModule

Run cmake to create the makefiles

    $ cd VisionModule/bin/debug
    $ cmake -DCMAKE_BUILD_TYPE=Debug ../..

Or if you want to build a release version
    $ cd VisionModule/bin/release
    $ cmake -DCMAKE_BUILD_TYPE=Release ../..

Run make to compile and link

    $ make

## Running

You can run vision_module with the --help option to get help on the arguments. 
All arguments can also be saved in a configuration file.

```
$ ./vision_module --help

Command Line Options:
  -v [ --version ]      print version string
  --help                produce help message
  -c [ --config ] arg   config file name


General Options:
  --subsample arg (=1)  sub sample
  --udp_port arg (=0)   udp port

Camera Options:
  -d [ --video_device ] arg (=/dev/video0)
                                        video device name
  -w [ --width ] arg (=320)             width
  -h [ --height ] arg (=240)            height
  --depth arg (=24)                     depth
  --brightness arg (=-1)                brightness
  --contrast arg (=-1)                  contrast
  --saturation arg (=-1)                saturation
  --sharpness arg (=-1)                 sharpness
  --gain arg (=-1)                      gain

Http Server Options:
  --http_port arg (=8080)    http port number
  --http_addr arg (=0.0.0.0) http address
  --docroot arg (=www/)      http document root
  --index arg (=index.html)  index.html file name

Colour Options:
  --colour arg          colour definition

Serial Port Options:
  --serial_device arg       serial device name or empty for no serial port 
                            output
  --baudrate arg (=B115200) baudrate
```

    $ ./vision_module -w 320 -h 240 --udp_port 2134 --index visionmodulecontroller.html

You can now use a web browser to connect to port 8080 on the running computer to see the interface.

    $ firefox http://localhost:8080

Create one or several colours that you want to track in the web interface

You can then switch to "segment colours" mode and run the visionclient to see what objects were found.

    $ ./visionclient 
    Results found Ok &shirt:6696:(245,87)[199,43-318,162]&shirt:3708:(51,215)[0,174-99,238]&shirt:256:(34,169)[29,152-39,185]&shirt:95:(109,227)[100,222-116,233]&shirt:44:(225,129)[220,127-231,132]&shirt:33:(302,116)[299,114-308,118]&shirt:25:(80,239)[68,239-92,239]&shirt:20:(30,239)[21,239-40,239]&shirt:13:(6,239)[0,239-12,239]
    Response Ok &shirt:6696:(245,87)[199,43-318,162]&shirt:3708:(51,215)[0,174-99,238]&shirt:256:(34,169)[29,152-39,185]&shirt:95:(109,227)[100,222-116,233]&shirt:44:(225,129)[220,127-231,132]&shirt:33:(302,116)[299,114-308,118]&shirt:25:(80,239)[68,239-92,239]&shirt:20:(30,239)[21,239-40,239]&shirt:13:(6,239)[0,239-12,239]
