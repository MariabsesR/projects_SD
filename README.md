Project Overview

The project involves developing a key-value storage service similar to the one used by Amazon for its web services. It consists of four phases:

    Phase 1: Defines data structures and implements functions for handling data stored in a hash table. Also includes a data serialization module to familiarize with the need for data serialization in distributed applications.

    Phase 2: Implements a simple client-server system where the server manages a hash table, and the client communicates with the server to perform operations on the table. Google Protocol Buffers are used for automated serialization and deserialization of data based on a .proto file.

    Phase 3: Creates a concurrent system that handles requests from multiple clients simultaneously using threads. This phase includes managing concurrency in data access and providing additional functionality for server statistics.

    Phase 4: Adds fault tolerance through chain replication using ZooKeeper for coordination. This phase involves:
        Implementing server coordination in ZooKeeper to support chain replication.
        Modifying the server to handle replication chain operations.
        Modifying the client to interact with ZooKeeper to find and communicate with servers at the head and tail of the replication chain.

Detailed Description
Chain Replication Model

The system uses chain replication to ensure data consistency and fault tolerance. Servers form a chain where each server communicates with its successor. Clients connect to the head of the chain for write operations and to the tail of the chain for read operations.
ZooKeeper

ZooKeeper is used to coordinate the chain of servers and monitor changes. Each server registers with ZooKeeper, and the chain is organized to allow automatic detection of failures and changes in the chain topology.
Changes in Servers and Clients

    Servers: Must connect to ZooKeeper, manage their position in the chain, and replicate write operations to the successor server.
    Clients: Must connect to ZooKeeper to obtain the head and tail servers of the chain and perform read and write operations accordingly.

How to Run

To run the system, follow these steps:

    Compilation: Compile the project using the Makefile. In the terminal, execute:

    bash

make

Start ZooKeeper: Ensure the ZooKeeper service is running and note its IP and port.

Start Servers:

    Run the server with the ZooKeeper IP and port as an argument:

    bash

    ./table_server <zookeeper_ip>:<zookeeper_port> <server_port> <num_lists>

Start Clients:

    Run the client with the ZooKeeper IP and port as an argument:

    bash

    ./table_client <zookeeper_ip>:<zookeeper_port>

Operations: Use the client to perform read and write operations on the storage system.
