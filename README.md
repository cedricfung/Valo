Valo
====

Full featured panoramic image and video viewer.


Dependencies
------------

* GCC 4.8+ or Clang 3.3+
* OpenGL 3.0+
* GLFW 3.0+ (http://glfw.org)
* FFmpeg 1.0+ (http://ffmpeg.org)
* 3DM (https://github.com/vecio/3DM)


Build and run
-------------

Install all dependencies above, except `3DM`. Then open your favorite terminal:

    git clone https://github.com/vecio/3DM.git 3dm
    git clone https://github.com/vecio/Valo.git valo
    cd valo && make
    ./valo <panorama-type> <precision> <image-or-video>

* `panorama-type`: **cylinder** or **sphere**.
* `precision`: should be a positive integer, don't make it too large, it will eat up your memory! By large, I mean **8**.


Key bindings
------------

* **ARROW KEYS**: turn perspective up, down, left or right.
* **SPACE**: pause video and reset the perspective.
* **B/F**: seek video backward or forward.
* **I/O**: zoom in/out of the scene.


Changelog
---------

* Support cylindric panorama. (Jul 23 2013)
* 360° spheric panorama implemented. (Jul 19 2013)


LICENCE
-------

BSD 3-clause.
