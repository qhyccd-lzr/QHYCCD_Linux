examples
======

qhy5liiviewer
----------------
Build and install libqhyccd.so first.

Install libsdl2-image-dev and libcfitsio3-dev

Build qhy5liiviewer using

    g++ -fpermissive -o qhy5liiviewer *c -I../src -lSDL -lSDL_image -lpthread -lusb -lcfitsio -lqhyccd `pkg-config --libs opencv` -DQHY5L_DEBUG

Usage :

<pre><code>
fphg@mint:examples$ ./qhy5liiviewer --help
USAGE: qhy5lviewer [options]

OPTIONS:
  -g | --gain <gain>
        Sensor Gain<0 - 1000> (default: 100)

  -t | --exposure <exposure>
        Exposure time in msec (default: 100)

  -f | --file <filename>
        Output file name to write images (default: image)

  -m | --format <fmt>
        File type to write (default: FITS, else ppm file will be created.)

  -h | --help
        Show this message

IN-PROGRAM SHORTCUTS:
  S -> start/stop frame grabbing
  P -> gain +10
  O -> gain -10
  L -> exposure time +100
  K -> exposure time -100
  Q -> exit program
</code></pre>
