# Mindset

Back propagation C++ network code for education and fun

## Description
I started this project in 1993 after reading a September 1992 article in Scientific American explaining back-propagation; "Neurons for Computers".

https://www.scientificamerican.com/magazine/sa/1992/09-01/

There wasn't any code available at the time, so I worked it out myself from the mathematics formulas in the article.  Over the next 10
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

## Documentation

Once you fetch the repo locally, you click on mindset/nnengine/doxygen/html/index.html
and view the C++ class documentation.
