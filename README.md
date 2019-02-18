# Paxos

Simple paxos implementation. Consensus algorithm for a single value.

Note: The work is still in progress, to start the algorithm all nodes must be running
      and successfully connect to each other before timeout expiration.

# Dependencies

Poco library: https://pocoproject.org/

Protobuf: https://developers.google.com/protocol-buffers/docs/overview

Google test/mock: configured as external project, downloadable on configuration

# Usage:
Starting single node instantion:
./Paxos json_network_config_file instantion_node_name

Example:

./Paxos ../config/network_topology.json node1

./Paxos ../config/network_topology.json node2

./Paxos ../config/network_topology.json node3
...

# Config
Configuration of network must be described in a json file.
Required fields:
- num_of_nodes
- seed_name 
- node, as list of nodes

Node structure requires:
- name
- ip
- port


Example:
```json
{
    "num_of_nodes" : 3,
    "seed_name" : "node1",
    "node" :
    [
        {
            "name" : "node1",
            "ip" : "127.0.0.1",
            "port" : 6170
        },
        {
            "name" : "node2",
            "ip" : "127.0.0.1",
            "port" : 6180
        },
        {
            "name" : "node3",
            "ip" : "127.0.0.1",
            "port" : 6190
        }
    ]
}
```
# Example output

```console
./Paxos ../config/network_topology.json node1
Running node: [node1, 127.0.0.1:6170]
Peer node: [node1, 127.0.0.1:6170]
Peer node: [node2, 127.0.0.1:6180]
Peer node: [node3, 127.0.0.1:6190]
Seed node: [node1, 127.0.0.1:6170]
Connected to node1
Got join: node1; Seen 1 of 3 voters
Got join: node2; Seen 2 of 3 voters
Got join: node3; Seen 3 of 3 voters
Sending welcome to: node1
Sending welcome to: node2
Sending welcome to: node3
Connected to node2
Connected to node3
Must re-propose ballot, because of preemption
Must re-propose ballot, because of preemption
Going to second stage with my ballot
Going to send accept for my data
Got accepted.
number: 640

input: 61
node_name: "node1"

./Paxos ../config/network_topology.json node2
Running node: [node2, 127.0.0.1:6180]
Peer node: [node1, 127.0.0.1:6170]
Peer node: [node2, 127.0.0.1:6180]
Peer node: [node3, 127.0.0.1:6190]
Seed node: [node1, 127.0.0.1:6170]
Connected to node1
Connected to node2
Connected to node3
Must re-propose ballot, because of preemption
Must re-propose ballot, because of preemption
Must re-propose ballot, because of preemption
Going to second stage with my ballot
Got accepted.
number: 650

input: 61
node_name: "node1"

./Paxos ../config/network_topology.json node3
Running node: [node3, 127.0.0.1:6190]
Peer node: [node1, 127.0.0.1:6170]
Peer node: [node2, 127.0.0.1:6180]
Peer node: [node3, 127.0.0.1:6190]
Seed node: [node1, 127.0.0.1:6170]
Connected to node1
Connected to node2
Connected to node3
Going to second stage with my ballot
Going to send accept for my data
Haven't got majority of acceptances
Re-proposing
Must re-propose ballot, because of preemption
Must re-propose ballot, because of preemption
Must re-propose ballot, because of preemption
Going to second stage with my ballot
Got accepted.
number: 660

input: 61
node_name: "node1"
```
