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
    """Mojave - Multidimensional Orthographic Joint Analytic Visual Explorer

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
       

    USAGE EXAMPLE (toy):
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

    USAGE EXAMPLE (man bash I):
    >>> import subprocess
    >>> import numpy as np
    >>> x = subprocess.check_output(["man", "bash"], text=True)
    >>> y = np.frombuffer(x.encode("utf-8"), dtype=np.uint8)
    >>> B = [y[i:i+40] for i in range(0,len(y) - 40,40)]
    >>> U,D,V = np.linalg.svd(B,0)
    >>> mojave(U)

    USAGE EXAMPLE (man bash II):
    >>> import subprocess
    >>> import numpy as np
    >>> from collections import Counter
    >>> from mojave import mojave
    >>> stop_word_cnt = 100
    >>> min_word_cnt = 30
    >>> word_step = 1
    >>> word_window = 10
    >>> min_word_occur = 3
    >>> man_bash = subprocess.check_output(["man", "bash"], text=True)
    >>> W = man_bash.replace('\n','').split(' ') 
    >>> C = Counter(W)
    >>> S = {e for e in C if C[e] < stop_word_cnt and C[e]>=min_word_cnt}
    >>> SL = sorted(list(S))
    >>> SI = {SL[i]:i for i in range(len(SL))}        
    >>> X = [[SI[x] for x in W[i:i+word_window] if x in SI] for i in range(0,len(W),word_step)]
    >>> def f(L):
    >>>     x = np.zeros(len(S))
    >>>     for e in L:
    >>>         x[e] += 1
    >>>     return x
    >>> X0 = [f(x) for x in X if len(x) >= min_word_occur]
    >>> [U,D,V] = np.linalg.svd(X0,0)
    >>> cl = mojave(U[:,:20])
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
