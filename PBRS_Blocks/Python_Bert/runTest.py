#!/usr/bin/env python

import sys
# add module path to python module search path
sys.path.append("./modules/")
import RxBert
import TxBert

import time

myTxBert = TxBert.TxBert(4)
myRxBert = RxBert.RxBert(4)

buffer_size = 2048

print "PN BERT Example"
print "BERT GEN and Receiver using PN15"
print "Buffer Size = "+str(buffer_size)

# make 128 byte buffer
buffer = ""
buffer = buffer.zfill(buffer_size)

print "running...",

start = time.time()

for x in range(0, 10):
    #fill buffer with Bert Data
    myTxBert.fill( buffer )
    #check buffer
    myRxBert.check( buffer )

stop = time.time()

print "Done"
print

duration = stop - start

print "Run Time: "+str( duration )+" seconds"
print "bitrate: "+str( ((buffer_size/duration)*80)/1e6 )+" Mbps"

# Print Results
print "-------------------------------"
print "Bert Results:"
print "Bits Input         : "+str( myRxBert.getBitsRX() )
print "synced bits RX     : "+str( myRxBert.getBitsRXinSync() )
print "sycned bit errors  : "+str( myRxBert.getErrors() )
print "Bert Synced?       : "+str( myRxBert.synced() )
print "Bert Sync Loss Cnt : "+str( myRxBert.getSyncLossCount() )

