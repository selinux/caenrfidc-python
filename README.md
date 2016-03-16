caenrfidc-python
================

This package implements the client side of CAENRFID protocol in Python and allow
the user to supply some commands to the RFID UHF reader.

This code can be used to get access to the CAENRFID RFID UHF readers,
expecially for the Cosino Mega 2560 - UHF RFID KIT
(http://www.cosino.io/product/cosino-mega-2560-uhf-rfid-kit).

<em>Note that the following code is based on the unofficial version of the CAENRFID's code so
don't waste the CAENRFID's guys on questions about what you're reading here!</em>

<b>Note</b>: this program is still a beta version! So don't use it for production!!


Compile & Install
-----------------

First of all you need some software from your distribution. In case you're using Ubuntu or Debian just try using the following command to install it:

    $ sudo aptitude install libpython-dev

<b>Note</b>: you can use the <code>apt-get</code> command also.

Then you have to install the CAENRFID C libraries as described at http://www.cosino.io/caenrfid-c-libraries-rfid-uhf-readers-1159.

Now just execute the commands below:

    $ python setup.py build
    $ python setup.py install

<b>Note</b>: you may need the <code>sudo</code> command to succesfully run the last command.


Quick usage
-----------

TODO
