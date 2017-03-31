# Unscented Kalman Filter Project Starter Code
Self-Driving Car Engineer Nanodegree Program

---

## Implementation of UKF
It was straight forward implementation after Udacity classes
More complex was to tune parameters for process noise.
While it was easy to tune parameters for each separate input,
it was more complex to find values that would satisfy both
at the same time.

Previously I studied AI for Robotics class and it included
Twiddle function in one of the lessons for parameter tuning.

Using twiddle it was very easy to find parameters for each of the inputs.
Finding parameters for both functions required some tweaking
of the twiddle parameters (initial values and step size).



## Dependencies

* cmake >= v3.5
* make >= v4.1
* gcc/g++ >= v5.4

## Basic Build Instructions

1. Clone this repo.
2. Make a build directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./UnscentedKF path/to/input.txt path/to/output.txt`. You can find
   some sample inputs in 'data/'.
    - eg. `./UnscentedKF ../data/sample-laser-radar-measurement-data-1.txt output.txt`
