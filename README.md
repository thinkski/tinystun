# 👉 tinystun

A simple STUN client

Implements the Session Traversal Utilities for NAT (STUN) protocol, defined by
RFC 5389. The STUN protocol enables a host behind a Network Address Translator
(NAT) to discover its server reflexive address.

## Quickstart

To build:
```
make
```

To run, specify a STUN server. For instance, to use Google's:
```
$ tinystun stun.l.google.com:19302
87.120.84.233:36449
```

If only a host is specified, destination port 3478 is assumed.
```
$ tinystun liburtc.org
87.120.84.233:35069
```

