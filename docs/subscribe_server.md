# Server subscriptions

A driver can subscribe to any number of topics. At least as long as the broker and the computer that runs the VSCP daemon can handle it.

Server subscriptions work in the same way as [driver subscriptions](./subscribe_driver.md) so read the information in that document as well.

Currently the server does not do anything with subscribed information. But in future versions [HLO (High Level Object)](https://grodansparadis.github.io/vscp-doc-spec/#/./class2.hlo) handling will be supported.