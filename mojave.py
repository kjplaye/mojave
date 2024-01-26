from ctypes import *
from subprocess import Popen, PIPE
import multiprocessing as mp
import numpy as np
import os


my_path = os.path.dirname(os.path.abspath(__file__))
_mojave = cdll.LoadLibrary(my_path + '/_mojave.so')

def _do_mojave(queue, X, cl, window_name, my_path):
    Xa = np.require(X, dtype = 'float64', requirements = 'C')
    Xp = Xa.ctypes._as_parameter_
    cl_a = np.require(np.array(cl).copy(), dtype = 'int32', requirements = 'C')
    cl_p = cl_a.ctypes._as_parameter_
    _mojave.mojave(Xp, cl_p, Xa.shape[0], Xa.shape[1],
                   window_name.encode(), my_path.encode())
    queue.put(cl_a)
    
def mojave(X, cl = None, window_name = 'Mojave'):
    """
    Parameters
    ----------
    X : array_like
        2-d array shape (data_size,dimension) usually data_size >> dimension.
    cl : array_like, optional
        Cluster labels (or colors), we make up colors and glyphs.

    KEYS:
       C              : Change color mode
       D              : Decimate
       E              : Eraser mode
       H              : Hide current color
       I              : Info
       N              : Next color
       O              : Color picker      
       Q              : Quit
       R              : Rotation mode
       S              : Zoom Standard
       X              : x/y plots
       Y              : Redo
       Z              : Undo
       Comma/Period   : Change point size
       Dowm/Up        : Scroll control down/up
       Equals/Minus   : Change zoom setting
       Left/Right     : Change brush color
       LR Bracket     : Change rotation speed
       PgDn/PgUp      : Scroll control down/up (faster)
       Colon/Quote    : Change intensity
       Space          : Change rotation angles
       

    USAGE EXAMPLE:
    >>> import numpy as np
    >>> cl_in = np.random.randint(4,size = (2000))
    >>> bit = np.random.randint(2,size = (2000))
    >>> V = np.array([[1,0,0,0],[0,1,0,0],[0,0,1,0],[1,1,1,0]])
    >>> ANGLE = np.random.random(2000) * 2 * np.pi
    >>> X = np.cos(ANGLE)*0.2
    >>> Y = np.sin(ANGLE)*0.2
    >>> N = np.random.normal(size = (2000,4))*0.03
    >>> D = V[cl_in] + np.array([X,Y,np.zeros(2000),bit]).T + N
    >>> cl_out = mojave(D,cl_in)
    """
    X0 = np.array(X)
    if X0.shape[0] < X0.shape[1]:
        raise ValueError("Matrix should be taller than wide")
    delta = np.max(X0,0) - np.min(X0,0)
    delta[delta == 0] = 1
    X1 = 2.0 * ((X0 - np.min(X0,0)) / delta) - 1.0
    X1[:,delta == 0] = 0
    if cl is None:
        cl = np.zeros(len(X))
    queue = mp.Queue()
    args = [queue, X1, cl, window_name, my_path]
    p = mp.Process(target = _do_mojave, args = args)
    p.start()
    p.join()
    return queue.get()
