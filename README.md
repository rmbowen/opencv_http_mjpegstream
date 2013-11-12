opencv_http_mjpstream
=====================

Creates an mjpeg stream from a web camera source provided by OpenCV

This is a rough version of code, works if you get all the dependencies working first.

Only tested on linux kernel = 3.5.0-17-generic

Using OpenCV 2.4.3 compiled from source with ffmpeg support

TBD:
1.) Specificy the actual dependencies 
2.) Provide OpenCV build instructions
3.) Create classes for better extendability
4.) Allow for command line arguments to change image properties and sources

Basic Overview:

Allows for multiple client connection to the mjpeg stream.

MJPEG stream is sent using sockets connects using SOCK_STREAM. default PORT is 8082

COMPILING (ASSUME OpenCV Configured correctly):

Just run make

USE:

./testWebCamServer

This will open the camera default = 0, and provide stream on 8082

./testWebCamClient

Once server is running this will connect and display the stream, default server is localhost and port 8082.

OTHER USES:

You can access the stream from http://computerip:8082, 
Currently some issues with direct access from browser or program such as VLC

You can also embed into an image on a webpage (you will obviously need a webserver then...)
SEE index.html as an example

MODIFY STREAM PROPERTIES:

--> check out the #defines in testWebCamServer.cpp 
--> Can change, port, image compression, image width/height, and if local display is executed.


Some known issues:

--> Need to ctr+c to exit and sometimes you have unplugg and plug back in your web camera.
--> Very slow when using browser directly and/or VLC. Fix coming soon.
