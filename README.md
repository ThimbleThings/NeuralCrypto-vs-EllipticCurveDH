# Neural Cryptography vs. Elliptic Curve Diffie-Hellman

Implementation of two nodes communicating via UDP to negotiate a secret key.
Key negotiation can use either Elliptic Curve Diffie-Hellman or Neural Cryptography.

The Parameters for the Tree Parity Machine are fixed in code.
They currently are K=4, N=6, L=4, since this was the TPM which was fastest to synchronize on average, as well as still secure against a simple attack in 10.000 simulations.
If you want to change the parameters of the TPM, they are fixed in the init functions of the server and client in Server.cc, respective Client.cc.


## Results
The synchronization time is heavily dependent on the roundThreshold for the TPM.
For K=4, N=6, L=4 this was set to 600.
This results in around 2.5-3 seconds of update messages for a 30Mbps Network with a 10ms delay before the networks might by synchronized, while ECDH takes barely longer than 20ms.

The tested TPM parameters are also not secure against more sophisticated attacks on the Neural Cryptography scheme, thus any network suitable for real-world application will take significantly longer to synchronize. Even with the current parameters, synchronization of two TPMs on a network with a high delay can take several hours.

## Conclusion
Neural cryptography as replacement for any Diffie-Hellman key exchange cannot be recommended.
Even improved implementations in hardware and a more efficient message format will not get around the sequential updates causing the bottle neck in any network even with moderate delays.

# Running the Code

In order to run ns-3 simulations of Neural Cryptography vs. Elliptic Curve Diffie-Hellman Ubuntu 16.04.6 LTS and gcc version 5.4.0 are required, for additional dependencies see Dependencies.txt.
ns-3 simulations are meant to run within a built ns-3 installation. Therefore, a path to the build dir must be passed to the simulation's compiler. First build the provided version of ns-3 (I did not test newer versions of ns-3, but this one was good enough to provide the results I needed)

## Building ns-3:
	ns-3.19$ ./waf configure -d debug
	ns-3.19$ ./waf build

## Building NC-vs-ECDH:
	nc-vs-ecdh$ ./waf configure --with-ns3='../ns-3.19/build'
	nc-vs-ecdh$ ./waf build

## Example Calls
	nc-vs-ecdh$ ./build/main --keyProtocol=ecdh		nc-vs-ecdh$ ./build/main --keyProtocol=nc
	
	nc-vs-ecdh$ ./build/main --keyProtocol=nc --stopSim=700 --dataRate=3Mbps --delay=10ms		nc-vs-ecdh$ LD_LIBRARY_PATH=../ns-3.19/build build/main --help