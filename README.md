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

MJPEG stream is sent using sockets connects using SOCK_STREAM. PORT is 8082

USE:

1.) Install OpenCV
2.) make
3.) ./testWebCam

--> You can access the stream from http://computerip:8082
    (Recommend initially viewing stream from VLC then working with it elsewise)

--> You can also embed into an image on a webpage (you will obviously need a webserver then...)

MODIFY STREAM PROPERTIES:

--> check out the #defines in testWebCam.cpp 
--> Can change, port, image compression, image width/height, and if local display is executed.


Some known issues:

--> Need to ctr+c to exit and sometimes you have unplugg and plug back in your web camera.

