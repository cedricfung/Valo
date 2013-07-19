Valo
====

Full featured panoramic image and video viewer.


Dependencies
------------

* OpenGL 3.0+
* GLFW 3.0+ (http://glfw.org)
* FFmpeg 1.0+ (http://ffmpeg.org)
* 3dm (https://github.com/vecio/3dm)


Build and run
-------------

Install all dependencies above, except `3dm`. Then open your favorite terminal:

    git clone https://github.com/vecio/3dm.git 3dm
    git clone https://github.com/vecio/valo.git valo
    cd valo && make
    ./valo <sphere-precision> <image-or-video>

The `sphere-precision` should be a positive integer, don't make it too large, it will eat up your memory! By large, I mean **10**.


Key bindings
------------

* **ARROW KEYS**: turn perspective up, down, left or right.
* **SPACE**: pause video and reset the perspective.
* **B/F**: seek video backward or forward.


Changelog
---------

* Only 360 sphere panorama implemented.


LICENCE
-------

BSD 3-clause.
