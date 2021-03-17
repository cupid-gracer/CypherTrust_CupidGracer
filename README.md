# cyphertrust_connector
Code repository for the CypherTrust's Connector component. These Connectors are built to 

* rapidly ingest crypto-exchange level 2 orderbook data (either through REST API or WebSockets)
* ensure the sequence in which data is received is in the correct order
* transform the incoming data into the internal Cyphertrust Orderbook Format
* send a heartbeat for monitoring practices
* stream the transformed orderbook data onto the Redis channel
