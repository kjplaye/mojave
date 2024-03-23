# Mojave
* spiritual successor to GGobi 
---
## Watch the video

https://youtu.be/nYdpSXOMqN8?si=69Oxh5sXusy4fk4K

## Get Mojave from source using pip 
We have to first install SDL headers and numpy, then we can try to install Mojave from source.

* sudo apt-get install libsdl2-image-dev libsdl2-ttf-dev
* pip install numpy
* pip install mojave-eda

## Get Mojave from source in github

Git clone this repo, run `make`, and add `mojave.py` path to `PYTHONPATH`.

## Gallery
Rotation of PCA results from 40 long strings picked out of "man bash" output. 

![alt text](https://github.com/kjplaye/mojave/blob/main/images/example_bash.gif?raw=true)

Simple synthetic data showing a 4d rotation.

![alt text](https://github.com/kjplaye/mojave/blob/main/images/example_toy.gif?raw=true)

Typical screen shot during brushing

![alt text](https://github.com/kjplaye/mojave/blob/main/images/example_mojave.png?raw=true)

## Usage
Please see the [python doc-string](https://github.com/kjplaye/mojave/blob/main/mojave.py#L21) for a listing of key mappings and usage examples to replicate the gallery.

## Backronym

__Multidimensional 
Orthographic 
Joint 
Analytic
Visual 
Explorer__
