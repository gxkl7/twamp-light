# TWAMP Light
This project is a C++ implementation of the TWAMP Light protocol defined in Appendix I of [RFC 5357](https://datatracker.ietf.org/doc/html/rfc5357) for precise two-way network latency measurement using stateless UDP packet reflection.
## Supported Modes
* TWAMP Light Controller/Sender
* TWAMP Light Responder/Reflector
## Installation
```
git clone https://github.com/gxkl7/twamp-light.git
mkdir build
cd build
cmake ..
make
```
## Usage
```
$ ./twamp_light --help

usage:

        IPv4 Address (Required)

        Port Number (Required)

        -f controller || reflector (Required)

        -c packet_count (controller mode only)

        -i interval_ms (controller mode only)

        -t timeout_ms (controller mode only)

-h, --help Help string

defaults: packet_count=3, interval=10ms, timeout=1000ms
```
## Example Output
TBD
