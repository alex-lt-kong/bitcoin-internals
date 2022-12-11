# bitcoin-internals

* The repo is still a work in progress..

* Implementation of [examples](https://github.com/jimmysong/programmingbitcoin) 
in Jimmy Song's [Programming Bitcoin](https://www.oreilly.com/library/view/programming-bitcoin/9781492031482/) and more:
  * Jimmy's implementation is in Python, which hides a lot of interesting details. This repo implements
    everything in "C-style" C++--thus we can get down to the nitty-gritty of Bitcoin.
  * The book implements ECDSA only--all other cryptographic algorithms are from Python's libraries. Here,
    no cryptography libraries needed--all the necessary cryptographic algorithms are either implemented in this
    repo or in the [cryptographic-algorithms](https://github.com/alex-lt-kong/cryptographic-algorithms) 
repo

* The book's analogy on ECDSA makes the thing even more difficult to understand, may use this instead: https://cryptobook.nakov.com/digital-signatures/ecdsa-sign-verify-messages

* The project is developed on little-endian architectures, some bitwise operations will definitely **not** work on big-endian architectures.
  Fortunately, Intel, AMD and most ARM CPUs are little-endian.

## Dependancies:

* Boost for 256-bit integer support: `apt install libboost-all-dev` 
* cURL for network communication: `apt-get install libcurl4-gnutls-dev`
* Criterion for unit testing: `apt install libcriterion-dev libcriterion3`

## Repo structure

* `chapter-test`: driver functions that make sure libraries work correctly. There is one test driver source code file
for each chapter in Jimmy's book
* `src`: source code that implements a Bitcoin client
  * `ecc.h`/`ecc.cpp`: implementation of the ECDSA algorithm from scratch.
  * `script.cpp`/`script.h`: parser and serializer of Bitcoin's Script language.
  * `tx.h`/`tx.cpp`: transaction parser and serializer.
  * `op.h`: define operations of Bitcoin's Script virtual machine.
  * `utils.h`/`utils.cpp`: utility functions
