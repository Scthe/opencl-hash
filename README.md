# Hash calculation using GPU


## Introduction

There is an alphabet of following 16 characters: **_acdegilmnoprstuw_**. Hash of 9-letter word created based on this alphabet is provided and so is the hashing function. Would You kindly find the original word for us ?

Originally posted at [Fog Creek] careers page.


## Usage

Use make: **make run**. The only non standard thing in makefile is location of the MinGW - remove/change as fit. It is also recommended to use OpenCL.dll found in Your PATH, not one in [lib](libs/lib).

## Solution

We have 9 letters in word, each letter is one of 16 characters. This gives us 2^38 total permutations. Quite a lot. We are going to split the iterations between 3 groups:
* iterate over first 3 letters on cpu, invoking opencl kernel each time
* with each kernel invocation we are going to create 2^16 work items(*'threads'* loosely speaking), which is equal to iterating over 4 letters
* each work item is going to do 2^8 iterations

| letter: | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
| ------- |:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| on: | cpu | cpu | cpu | kernel id | kernel id | kernel id | kernel id | inside<br/>kernel | inside<br/>kernel



## Old version
You can find old version of this project [here](https://github.com/Scthe/hash-challenge--Fog-Creek--OpenCL). It contains both GPU & CPU implementation, but it is much, much slower.



[Fog Creek]: http://www.fogcreek.com/
