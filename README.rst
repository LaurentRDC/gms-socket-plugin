TCP Socket Plug-in for Gatan Microscopy Suite 3.x
=================================================

This small plug-in for the Gatan Microscopy Suite 3.x creates a
TCP/IP socket, which can then be used to communicate with the outside world.

Installation
------------

The plug-in :code:`.dll` must be placed in C:\\ProgramData\\Gatan\\Plugins.

API Reference
-------------

The plug-in exposes a few functions to Digital Micrograph scripts:

    * :code:`void TCPSocketConnect( string address, number port )` : 
        Connect the socket to a remote point, e.g. :code:`TCPSocketConnect( "127.0.0.1", 5555 )`

    * :code:`void TCPSocketBind( string address, number port )` :
        Bind the socket to a local endpoint, e.g. e.g. :code:`TCPSocketBind( "127.0.0.1", 5555 )`

    * :code:`void TCPSocketDisconnect( void )` : 
        Disconnect the socket from current connection. In case there were no connections, this function has no effect.

    * :code:`string TCPSocketRecv( void )` : 
        Returns the available input data. If nothing is available, returns an empty string. This function always returns
        immediately; use in conjunction with :code:`TCPSocketWaitIncoming()`:
    
    * :code:`void TCPSocketWaitIncoming( void )`:
        Blocks until data is available for reading. Use at your own risks.
    
    * :code:`number TCPSocketSend( string message )`:
        Send data over the socket. Returns the number of bytes written. If 0 is returned, an error has occured.

    * :code:`void TCPSocketToggleDebug( bool toggle )` : 
        Enable/disable messages written to the DigitalMicrograph results window. Enabled by default.

Example
-------

This is a simple example of sending a message::

    // Enable debug messages just in case
    // Boolean values in DM scripts are numbers (1 = True, 0 = False)
    TCPSocketToggleDebug( 1 ) 

    // The following line connects to a host
    // This function will always succeed, even when connection was unsuccessful.
    TCPSocketConnect("127.0.0.1", 5555)

    // Send the message
    // If the above connection attempt was unsuccessful, 
    // TCPSocketSend will always return 0 (since no bytes have been sent.)
    number bytes_sent = TCPSocketSend("Test message")

    // Let's pretend we are expecting a reply
    // Wait until the reply has arrived, then read it
    TCPSocketWaitIncoming()
    string reply = TCPSocketRecv()

Support / Report Issues
-----------------------

Please e-mail `laurent.renedecotret@mail.mcgill.ca <mailto:laurent.renedecotret@mail.mcgill.ca>`_ if you are experiencing
trouble, or if you would like a new feature.

License
-------

The code in this plug-in is made available under the GNU GPLv3 License. This means that any use/modification of
this software must also be open-source. For more details, see LICENSE.txt.
