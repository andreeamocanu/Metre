Metre :: An XMPP Server (Sort Of)
=================================

Latest CI builds:

* CircleCI: [![CircleCI](https://circleci.com/gh/surevine/Metre.svg?style=svg)](https://circleci.com/gh/surevine/Metre)
* Docker: ![Docker Cloud Build Status](https://img.shields.io/docker/cloud/build/surevinecom/metre.svg)
* Linux: <a href="http://217.155.137.59:8111/viewType.html?buildTypeId=Metre_Build&guest=1"><img src="https://img.shields.io/teamcity/https/teamcity.cridland.io/e/Metre_Build.svg?label=TeamCity%20Build&logo=linux&logoColor=%23ffdd44&style=popout"/></a>
* OS X: <a href="http://217.155.137.59:8111/viewType.html?buildTypeId=Metre_MacBuild&guest=1"><img src="https://img.shields.io/teamcity/https/teamcity.cridland.io/e/Metre_MacBuild.svg?label=TeamCity%20Build&logo=apple&logoColor=%23999999&style=popout"/></a>
* Windows: <a href="http://217.155.137.59:8111/viewType.html?buildTypeId=Metre_WindowsBuild&guest=1"><img src="https://img.shields.io/teamcity/https/teamcity.cridland.io/e/Metre_WindowsBuild.svg?label=TeamCity%20Build&logo=windows&logoColor=%230099ff&style=popout"/></a>

Metre (check that spelling) is an XMPP Server, of sorts. Unlike traditional XMPP servers
which host services internally, Metre is specifically designed to connect between servers,
mediating connections and traffic.

It's written in C++23 (ie, modern C++), and aims to provide a semantically-aware filter
between domains, to avoid exposing your internal XMPP fully to the world.

You'll probably want to read the [FAQ](FAQ.md), and you may wish to find some
[BUILD](BUILD.md) instructions. There is also a documentation file on [FILTERS](FILTERS.md)

Finally, there's a detailed guide to all the configuration parameters [CONFIGURING](CONFIGURING.md).

The [LICENCE](LICENSE) is MIT, and the copyright
rests (mostly - see the [Base 64 code](src/base64.cc)) jointly with Surevine Ltd and
Dave Cridland.

Metre has been deployed in several cases in production, and has even worked.

Currently working:
* Component hosting [XEP-0114](https://xmpp.org/extensions/xep-0114.html)
* TLS
* X.509 auth (PKIX)
* Dialback and S2S [XEP-0220](https://xmpp.org/extensions/xep-0220.html)
* Dialback without Dialback  [XEP-0344](https://xmpp.org/extensions/xep-0344.html)
* Basic forwarding/routing
* DNSSEC (Including [RFC 6125](https://tools.ietf.org/html/rfc6125) additional reference identifiers)
* S2S <-> S2S proxying
* DNS overrides (SRV, A, and TLSA per-domain)
* DANE (including via TLSA overrides as above)
* Basic Filtering
* Daemonizing
* IPv6
* Direct TLS [XEP-0368](https://xmpp.org/extensions/xep-0368.html)

Currently poorly tested:
* [XEP-0361](https://xmpp.org/extensions/xep-0361.html)

Currently unimplemented but planned:
* Semantic filtering
