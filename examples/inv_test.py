#!/usr/bin/env python

import os, sys
import caenrfidc

NAME = os.path.basename(sys.argv[0])
if len(sys.argv) < 2 :
    sys.exit('usage: %s server' % NAME)

h = caenrfidc.open(sys.argv[1])

print "inventory data:"
print caenrfidc.inventory(h, "Source_0")

caenrfidc.close(h)
