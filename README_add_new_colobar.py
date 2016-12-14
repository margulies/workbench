#!/usr/bin/python
'''
Following code is example of how to add new colorbar to wb_view.
After including the additional colorbar, the code needs to be recompiled using:

mkdir build
cd build
cmake ../src
make

'''
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as colors

def truncate_colormap(cmap, minval=0.0, maxval=1.0, n=100):
    new_cmap = colors.LinearSegmentedColormap.from_list(
        'trunc({n},{a:.2f},{b:.2f})'.format(n=cmap.name, a=minval, b=maxval),
        cmap(np.linspace(minval, maxval, n)))
    return new_cmap

cmap = plt.get_cmap('nipy_spectral')
new_cmap = truncate_colormap(cmap, 0.2, 0.95)

#colors1 = plt.cm.YlGnBu(np.linspace(0, 1, 128))
first = int((128*2)-np.round(255*(1.-0.90)))
second = (256-first)
#colors2 = new_cmap(np.linspace(0, 1, first))
colors2 = plt.cm.viridis(np.linspace(0.1, .98, first))
colors3 = plt.cm.YlOrBr(np.linspace(0.25, 1, second))
colors4 = plt.cm.PuBu(np.linspace(0., 0.5, second))
#colors4 = plt.cm.pink(np.linspace(0.9, 1., second))
# combine them and build a new colormap
cols = np.vstack((colors2,colors3))
mymap = colors.LinearSegmentedColormap.from_list('my_colormap', cols)
'''
num = 256
gradient = range(num)
for x in range(5):
    gradient = np.vstack((gradient, gradient))

fig, ax = plt.subplots(nrows=1)
ax.imshow(gradient, cmap=mymap, interpolation='nearest')
ax.set_axis_off()
fig.tight_layout()

plt.show()
'''

# Export colorbar to wb_view code: src/Files/PaletteFile.cxx

g = [f for f in cols[:,0:3] * 255]

m = []
for x in g:
    n = []
    for h in x:
        n.append(hex(int(h)))
    m.append(n)

for n, x in enumerate(m):
    print '    int mymap%s[3] = { %s, %s, %s };' % (n,x[0],x[1],x[2])
    print '    this->addColor("_mymap%s", mymap%s);' % (n,n)

print '    if (this->getPaletteByName("margulies") == NULL) {'
print '        Palette mymap;'
print '        mymap.setName("margulies");'

for n, x in enumerate(m):
    print '        mymap.addScalarAndColor( %ff, "_mymap%s");' % ((1-(n*1./255*2)), n)

print '        addPalette(mymap);'
print '    }'

# create inverse colormap:
g = [f for f in cols[:,0:3] * 255]
g = np.flipud(g)
m = []
for x in g:
    n = []
    for h in x:
        n.append(hex(int(h)))
    m.append(n)

for n, x in enumerate(m):
    print '    int mymapInv%s[3] = { %s, %s, %s };' % (n,x[0],x[1],x[2])
    print '    this->addColor("_mymapInv%s", mymapInv%s);' % (n,n)

print '    if (this->getPaletteByName("margulies_inv") == NULL) {'
print '        Palette mymapInv;'
print '        mymapInv.setName("margulies_inv");'

for n, x in enumerate(m):
    print '        mymapInv.addScalarAndColor( %ff, "_mymapInv%s");' % ((1-(n*1./255*2)), n)

print '        addPalette(mymapInv);'
print '    }'

# create inverse positive colormap:
g = [f for f in cols[:,0:3] * 255]
g = np.flipud(g)
m = []
for x in g:
    n = []
    for h in x:
        n.append(hex(int(h)))
    m.append(n)

for n, x in enumerate(m):
    print '    int mymapInvPos%s[3] = { %s, %s, %s };' % (n,x[0],x[1],x[2])
    print '    this->addColor("_mymapInvPos%s", mymapInvPos%s);' % (n,n)

print '    if (this->getPaletteByName("margulies_inv_pos") == NULL) {'
print '        Palette mymapInvPos;'
print '        mymapInvPos.setName("margulies_inv_pos");'

for n, x in enumerate(m):
    print '        mymapInvPos.addScalarAndColor( %ff, "_mymapInvPos%s");' % ((1-(n*1./255)), n)

print '        addPalette(mymapInvPos);'
print '    }'

# create positive colormap:
g = [f for f in cols[:,0:3] * 255]
m = []
for x in g:
    n = []
    for h in x:
        n.append(hex(int(h)))
    m.append(n)

for n, x in enumerate(m):
    print '    int mymapPos%s[3] = { %s, %s, %s };' % (n,x[0],x[1],x[2])
    print '    this->addColor("_mymapPos%s", mymapPos%s);' % (n,n)

print '    if (this->getPaletteByName("margulies_pos") == NULL) {'
print '        Palette mymapPos;'
print '        mymapPos.setName("margulies_pos");'

for n, x in enumerate(m):
    print '        mymapPos.addScalarAndColor( %ff, "_mymapPos%s");' % ((1-(n*1./255)), n)

print '        addPalette(mymapPos);'
print '    }'
