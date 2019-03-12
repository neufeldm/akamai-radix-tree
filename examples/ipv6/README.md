Copyright (C) 2019 Akamai Technologies, Inc

https://www.akamai.com

# IPv4/6 Examples
This directory contains examples that utilize radix trees in order
to perform "useful" tasks. The code in this directory has *not* been
subjected to much in the way of testing, use at your own risk. When
the examples require input, the expected format is a simple text
file with an IP address/subnet followed by whatever the associated
value is. For example, the following text would associate some
numbers with IPv4 and IPv6 subnets:

    1.0.0.0/24 13335
    1.1.8.0/24 4134
    1.1.8.1/32 4134
    2c0f:fe38:a::/48 33771
    2c0f:fe38:a::1/128 33771

IPv4 and IPv6 subnets may be mixed and aren't required to be in
a particular order. One reasonable source of data for experimenting
is available from CAIDA:

http://data.caida.org/datasets/routing

The files located there are mappings from IP subnets to AS number(s)
derived from the RouteViews project:

http://www.routeviews.org

The examples directory contains a simple shell script:

print_routeviews.sh

that will decompress and convert the snapshot files from the CAIDA
format to match what the sample applications expect. In order for
the script to work you must have zcat, sed, and awk installed.

## IP Address Lookup
The IP address lookup samples use the radix tree library to perform
longest prefix lookups using a few different techniques. The base
utility classes used by all of the examples are the following:

* *IPAddressUtils.h*
* *IPAddressUtils.cc*
* *IPAddressBlock.h*
* *IPAddressBlock.cc*
* *IPLookupUtils.h*

These implement parsing/printing of IP addresses and define an abstract
interface that the different sample implementations provide. Each IP
address lookup example creates an executable that requires a file containing
subnets and values and expects at least a single argument containing a path
to a text file with lines containing IP subnets and associated values as
outlined earlier in this document. If no further arguments are provided
then the application will expect to read IP address subnets on stdin, and
will write the matched IP subnet and value on stdout. Two further optional arguments
may be provided: a path to an input file containing subnets to lookup (instead of
stdin) and a path to an output file for writing the results (instead of stdout).
The examples will also track the time required for each lookup and print simple
summary statistics.

The lookup examples all use the simple node/pointer tree implementations storing
string values. They use the faster "lookup" cursors when possible. That said,
better performance would likely be had by using the binary word or read-only WORM
trees and storing integers instead of strings.

### Simple Tree Lookup
The "simple tree" lookup example is implemented in these files:

* *BinaryTreeLookup.h*
* *IPLookup.cc*

IPv4 address/value pairs are stored in a binary radix tree, 32 bits deep, and IPv6
address/value pairs are stored in a binary radix tree, 128 bits deep. The lookup
process is a simple navigation of either the IPv4 or IPV6 tree.

### "Hop" Lookup
The "hop" lookup example is implemented in these files:

* *BinaryHopLookup.h*
* *IPLookupHop.cc*

Instead of using a single binary radix tree, the hop lookup uses a higher degree
radix tree (4,8, or 16 in these examples) to "hop" down multiple bits of the IP address
at a time, leading to a regular binary radix tree to fill in the bits between. For example,
looking up a /24 in the simple tree lookup would require walking 24 steps down the tree. The
"hop" could perform the same lookup in 6 steps if degree 4, 3 steps if degree 8, or 9 steps
(a single 16 bit step followed by 8 one bit steps) if degree 16.

### "Leap" Lookup
The "Leap" lookup example is implemented in these files:

* *BinaryLeapLookup.h*
* *IPLookupLeap.cc*

As another variation on the "hop" lookup, the "leap" lookup starts by using a
std::unordered_map to track a either 16 or 32 of the first path bits, followed
by either a "hop" or "simple" lookup object to cover the rest of the bits.
Within the bits covered by the "leap" another "hop" or "simple" lookup object
is used.