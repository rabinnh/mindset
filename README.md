# Mindset

Back propagation C++ network code for education and fun

## Description
I started this project in 1994 after reading the Sept 1993 issue cover story in Scientific American explaining back-propagation
neural networks.  I just ran across it in my local repo.

There wasn't any code available at the time, so I worked it out myself from the mathematics in the article.  OVer the next 10
years I occasionally revisited and update the code - I learned about scaling inputs, quick prop, bias, etc.  All old hat today
but not back then.  This code is written in C++ and gives a nice view into the internals of backpropagation networks.

It's single threaded and CPU based.  I thought of upgrading it, but with the excellent libraries released to open source these
days, like Pytorch, Tensorflow, MXNet, it's wouldn't really be that satisfying.

### Purpose

It was very useful back in the day.  Today, it's for educational purposes.  There are some small test data samples in the
Examples directory.

### What does it do?
It can learn by example and generate predictions.

## Building

Include the network code in your project.  See the NNTest and NNSolvetest sub-projects to see how to include the class code into a C++ project.

Intended for Linux but works on Windows.

This project requires the expat library
