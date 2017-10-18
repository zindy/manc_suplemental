import libatrous
import numpy as np
import time
import tifffile

if 1:
    #a = np.zeros((10,4000,4000),np.float32)
    a = np.zeros((16000,16000),np.float32)
    #a = np.zeros(2000,np.float32)
    print "zeros will do...",a.shape
else:
    #tif = tifffile.TiffFile('../../../../Desktop/2012.03.23 d7 Pb NK65 GFP CX3CR1 x DsRed mouse 1 movie 2.lsm')
    #tif = tifffile.TiffFile('../../../../Desktop/2012.03.24 d7 Pb NK65 GFP DsRed mouse 2 movie 2.lsm')
    print "reading..."
    tif = tifffile.TiffFile(r"C:\Users\zindy\Desktop\2012.02.29 d7 pba gfp cx3cr1 x dsred mouse 2 movie 3.lsm")

    #tp0
    stacks_time = tif.asarray()[0,:,2,:,:]
    print stacks_time.shape
    n_stacks = stacks_time.shape[0]

    print tif[0].tags
    exit()
    

#setting grid
if 1:
    print "2 dimensions..."
    libatrous.set_grid(1,1)
    print "3 dimensions..."
    libatrous.set_grid(1,1,1)

#kernels
if 1:
    kernel = libatrous.choose_kernel(libatrous.SPL5)
    print kernel

n_test = 10
if 0:
    print "testing convolve"
    t = time.time()
    for i in range(n_test):
        b = libatrous.convolve(a,0,0)
    print "took",(time.time()-t)/n_test
    print b.shape == a.shape
    print np.sum(b) == 0

if 0:
    print "testing scale"
    kernel_index = 1
    scale = 8
    t = time.time()
    b = libatrous.scale(a,kernel,scale)
    print "took",time.time()-t
    print a.shape
    print b.shape
    print a.ndim
    print b.ndim
    print b.shape == a.shape
    #print np.sum(b) == 0

if 0:
    print "testing stack"
    kernel_index = 0
    n_scales = 12
    t = time.time()
    scales = libatrous.stack(a,kernel,n_scales)
    print scales.shape
    print "took",time.time()-t
    print scales.shape[1::] == a.shape
    print scales.shape[0] == n_scales + 1
    print np.sum(scales) == 0
    del scales
    print
    time.sleep(2)

if 1:
    print "testing iterscale"
    kernel_index = 0
    n_scales = 12
    t = time.time()

    c = a.copy()
    scales = [None]*(n_scales+1)
    for i in range(n_scales):
        #print "doing %d..."%i,c.shape,
        b,c = libatrous.iterscale(c,kernel,i)
        #print b.shape,c.shape
        scales[i] = b
    scales[n_scales] = c
    #print np.sum(scales) == 0
    print "took",time.time()-t

