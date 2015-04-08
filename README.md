Woodycxx
=======
> Woodycxx is a C++ library for XMPP Core.
>
> in developing ...
>
> Build & Run Unit Test in |  Linux |
> -------------------------|--------|
> Status |  [![Build Status](https://travis-ci.org/nasacj/woodycxx.svg?branch=master)](https://travis-ci.org/nasacj/woodycxx) |

Woodycxx also contains some Libraries:

    woodycxx/io             I/O classes for base and network, designed as Java usage.
    woodycxx/net            For socket encapsulation.
    woodycxx/smart_ptr      Smart Pointer like boost, but much simpler include to use.


***

LICENSE
-------
Under BSD Liense

***
Dependent Library
-------
Boost

***

INSTALL
=======
> Woodycxx is written in C++11
>
> autotool is needed to configure and compile the code
Prerequisites
-------
-   **[Required]** GCC or Clang or VC support ISO C++ 11.
-   **[Required]** Autotool - autoreconf
-   **[Required]** Boost - libboost-dev

Build
-------
Run command:
    autoreconf --install
    ./configure
    make
