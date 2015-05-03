
mplaylist
=========

A simple `mpv <https://github.com/mpv-player/mpv>`_ playlist manager
written in Qt.

Instructions
============

You probably want to create a directory somewhere and check this out there. i.e.
open a console and type the following commands:

    mkdir -p ~/src/mplaylist
    
    cd ~/src/mplaylist

    git clone https://github.com/cmdrkotori/mplaylist.git
    
    cd mplaylist
    
    qtcreator mplaylist.pro

To compile, click 'configure project' using the default settings and press the
arrow button in the bottom left corner of the window.

After this the binary should be at ~/src/mplaylist/build.../mplaylist.  Pin that
to your taskbar, create a menu entry, place it on your desktop, or run from the
commandline.

The application calls mpv to play files, so Windows users please make sure the
binaries are in the same directory or mpv is in your path.  You may want to
install mpv with `chocolatey <https://chocolatey.org/>`_ for this purpose.
