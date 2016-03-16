#!/usr/bin/env python

import os, sys
import caenrfidc

NAME = os.path.basename(sys.argv[0])
if len(sys.argv) < 2 :
    sys.exit('usage: %s server' % NAME)

h = caenrfidc.open(sys.argv[1])

print "--- system status ---"
print "firmware release: %s" % caenrfidc.fw_release(h)
print "Ant0 is Source_0: %s" % caenrfidc.check_readpoint(h, "Source_0", "Ant0")
print "current power: %dmw" % caenrfidc.get_power(h)

print

# ###
# readpoint test
#
# This test simply tries to move an antenna from a readpoint to another
# ###

sys.stdout.write("readpoint test... ")

# Force Ant0 in Source_0
if not caenrfidc.check_readpoint(h, "Source_0", "Ant0") :
	caenrfidc.add_readpoint(h, "Source_0", "Ant0")

caenrfidc.remove_readpoint(h, "Source_0", "Ant0")
caenrfidc.add_readpoint(h, "Source_1", "Ant0")
if not caenrfidc.check_readpoint(h, "Source_1", "Ant0") :
	print "ko! (Ant0 is not in Source_1)"
	sys.exit(-1)

caenrfidc.remove_readpoint(h, "Source_1", "Ant0")
caenrfidc.add_readpoint(h, "Source_0", "Ant0")
if not caenrfidc.check_readpoint(h, "Source_0", "Ant0") :
	print "ko! (Ant0 is not in Source_0)"
	sys.exit(-1)

print "ok"

# ###
# Power test
#
# This test simply reads current power and then try to change it, then
# verifies that the power is changed and, in the end, restores initial value
# and checks it again.
# ###

sys.stdout.write("power test... ")

pow = caenrfidc.get_power(h)
caenrfidc.set_power(h, 1000)
new_pow = caenrfidc.get_power(h)
if new_pow not in range(900, 1100) :
	print "ko! (%d not near to 1000)" % new_pow
	sys.exit(-1)

caenrfidc.set_power(h, pow)
new_pow = caenrfidc.get_power(h)
if new_pow not in range(int(pow * .9), int(pow * 1.1)) :
	print "ko! (%d not near to %d)" % (new_pow, pow)
	sys.exit(-1)

print "ok"

caenrfidc.close(h)
