# Assignment 0

I implemented Garland and Heckbert's algorithm on mesh simplification.

## Running Instructions

```
make
./a0 < garg.obj
```

## Key inputs

* Key `s` starts simplification
* Key `r` rotates the camera about the y-axis
* Key `c` toggles between preset colors
* Arrow keys control where light is being shined from

## Timing results

|             | Verices | Time (s) |
| ----------- | ------- | -------- |
| sphere.obj  | 382     | 0.025    |
| torus.obj   | 800     | 0.054    |
| garg.obj    | 21278   | 1.642    |

## What more can be done

* Disallow face flipping
* Virtual edges between nearby vertices
* Interpolating vertex normals for the new vertex based on its distance from the contracted vertices

