#define STRICT

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN												 // Exclude rarely-used stuff from Windows headers
#endif

#include <iostream>
#include <string>
#include <vector>

#include "GMS_TargetVersion.h"


#ifndef _BIND_TO_CURRENT_VCLIBS_VERSION								 // Force the CRT/MFC version to be put into the manifest
#define _BIND_TO_CURRENT_VCLIBS_VERSION 1
#endif

// BOOST ASIO for sockets
#define BOOST_ASIO_NO_WIN32_LEAN_AND_MEAN
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>

// never let windows.h define their min/max macros
#include <Windows.h>

#define _GATANPLUGIN_USES_LIBRARY_VERSION 2
#define _GATAN_USE_STL_STRING

// GMS Foundation
#include "GMSFoundation.h"

#include "DMPlugInBasic.h"
#include "DMPlugInMain.h"

using namespace Gatan;
using boost::asio::ip::tcp;

/*
	Definition of functions exposed to Digital Micrograph

	Functions exposed so far:
	
	TCPSocketBind
	TCPSocketConnect
	TCPSocketDisconnect
	TCPSocketWaitIncoming
	TCPSocketRecv
	TCPSocketSend
	TCPSocketToggleDebug

*/

// The object `gms_tcp_socket` is made
// global so that it can be used by the
// plug-in functions without
// passing it as a parameter
boost::asio::io_service IOSERVICE;
tcp::socket gms_tcp_socket(IOSERVICE);
enum { MAX_MESSAGE_LEN = 1024 };

bool debugFlag = true;

void TCPSocketToggleDebug( bool enable )
/*
** Enable/disable debug mode, which only
** includes printing to the DigitalMicrograph
** console at this time.
**
** Parameters
** ----------
** enable : bool
**     If `true`, debug mode is turned on.
*/
{	
	debugFlag = enable;
}

void debugMessage( std::string message )
/*
** Print a debug message to the DigitalMicrograph console depending 
** on the state of the debug flag.
**
** Parameters
** ----------
** message : std::string
**     Debug message. It will be prepended with "TCPSocket: " and appended 
**     with a newline "\n".
*/
{
	if (debugFlag) {
		DM::Result( "TCPSocket: " + message + "\n");
	}
}

void handlePlugInError( boost::system::error_code ec )
/*
** Handle an error. Depending on the state of the
** debug flag, this might print the result to DigitalMicrograph'
** result console or do nothing.
**
** Parameters
** ----------
** ec : boost::system::error_code
**     Error code. Error code can be `no error` as well.
*/
{	
	if (ec) {
		debugMessage("Error occured:");
		debugMessage("    " + ec.message());
	}
}

void TCPSocketBind( DM_StringToken addr, uint32 port )
/* 
** Bind the socket to an address. The socket is automatically
** opened as well.
**
** Parameters
** -----------
** addr : str
**     IP address, e.g. "127.0.0.1"
** port : int
**     Port number (e.g. 42056)
*/ 
{
	boost::system::error_code ec;

	PLUG_IN_ENTRY

	if ( !gms_tcp_socket.is_open() ){
		gms_tcp_socket.open(tcp::v4(), ec);
		if (ec) { 
			handlePlugInError(ec);
			return; 
		}
	}

	// Conversion of addr to a std::string
	std::string addr_str = DM::String(addr).get_string();
	tcp::endpoint full_address(
		boost::asio::ip::address::from_string(addr_str), port);
	
	gms_tcp_socket.bind(full_address, ec);

	if (ec) {
		handlePlugInError(ec);
	} else {
		debugMessage("Binding successful. ");
	}

	PLUG_IN_EXIT
}

void TCPSocketConnect( DM_StringToken addr, uint32 port )
/* 
** Connect the socket to an address. 
** If the socket was already open, the connection is 
** closed first, and then a  connection is initiated.
**
** Parameters
** -----------
** addr : str
**     IP address, e.g. "127.0.0.1"
** port : int
**     Port number (e.g. 42056)
*/ 
{	
	// Potential error code from connection
	boost::system::error_code ec;

	// PLUG_IN_ENTRY and PLUG_IN_EXIT are required in any script function to handle
	// C++ exceptions properly.
	PLUG_IN_ENTRY
	
	if ( gms_tcp_socket.is_open() ){
		gms_tcp_socket.close();
	}

	// Conversion of addr to a std::string
	std::string addr_str = DM::String(addr).get_string();
	tcp::endpoint full_address(
		boost::asio::ip::address::from_string(addr_str), port);
	
	gms_tcp_socket.connect(full_address, ec);

	if (ec) {
		handlePlugInError(ec);
	} else {
		debugMessage("Connection successful.");
	}

	PLUG_IN_EXIT

}

void TCPSocketDisconnect( void )
/* 
** Disconnect the socket.
** It is safe to call this function even
** if the socket was not connected.
*/ 
{
	// PLUG_IN_ENTRY and PLUG_IN_EXIT are required in any script function to handle
	// C++ exceptions properly.
	PLUG_IN_ENTRY

	if ( gms_tcp_socket.is_open() ) {
		gms_tcp_socket.close();
	}

	debugMessage("Socket disconnected. ");

	PLUG_IN_EXIT
}

void TCPSocketWaitIncoming( void )
/*
** Wait until data is available to read.
*/
{
	boost::system::error_code ec;

	PLUG_IN_ENTRY

	gms_tcp_socket.wait(tcp::socket::wait_read, ec);
	handlePlugInError(ec);

	PLUG_IN_EXIT
}

DM_StringToken_1Ref TCPSocketRecv( void )
/* 
** Read message waiting in the socket.
** In case there are no pending data, this function returns an empty string.
** 
** Returns
** -------
** message : str
**     Available message. If no message is available,
**     `message` is an empty string.
*/
{
	// Potential error code from connection
	boost::system::error_code ec;
	
	PLUG_IN_ENTRY
	
	// In case the socket is closed or nothing is available 
	// to receive, return empty message
	if (!gms_tcp_socket.is_open() || (gms_tcp_socket.available() == 0)){
		debugMessage("No incoming data available.");
		return DM::String("").release();
	}
	else {
		// Put message in array called `message_received`
		char message_received[MAX_MESSAGE_LEN];
		std::size_t message_len = gms_tcp_socket.receive(
			boost::asio::buffer(message_received, MAX_MESSAGE_LEN), 0, ec);

		if (ec) {
			handlePlugInError(ec);
		} else {
			debugMessage("Following message received:");
			debugMessage("    " + std::string(message_received));
		}
		return DM::String( message_received ).release();
	}

	PLUG_IN_EXIT
	
	// To get rid of 'Not all control paths return a value' warning
	return DM::String("").release();
}

long TCPSocketSend(DM_StringToken message)
/* 
** Send a message through the socket. This operation
** only completes once all data has been written.
** 
** Parameters
** ----------
** message : str
**     Message to be sent.
**
** Returns
** -------
** bytes_written : int
**     Number of bytes written. In case an error
**     occurs, `bytes_written` will be zero.
*/
{	
	// Potential error code
	boost::system::error_code ec;

	PLUG_IN_ENTRY
	// In case the socket is not open,
	// we cannot write data;
	// hence, no bytes have been sent
	if (!gms_tcp_socket.is_open()){
		return 0;
	} 

	// If we are here, ready to send.
	std::string message_str = DM::String(message).get_string();
	std::size_t bytes_written = boost::asio::write(
		gms_tcp_socket, boost::asio::buffer(message_str, message_str.size()), ec);

	if (ec) {
		handlePlugInError(ec);
	} else {
		debugMessage("Following message sent:");
		debugMessage("    " + std::string(message_str));
	}

	return (long) bytes_written;

	PLUG_IN_EXIT
}

/*
	Plug-in Definition
*/

class TCPSocketPlugIn : public Gatan::PlugIn::PlugInMain
{
public:
	virtual void Start();
	virtual void Run();
	virtual void Cleanup();
	virtual void End();
};

///
/// This is called when the plugin is loaded.  Whenever DM is
/// launched, it calls 'Start' for each installed plug-in.
/// When it is called, there is no guarantee that any given
/// plugin has already been loaded, so the code should not
/// rely on scripts installed from other plugins.  The primary
/// use is to install script functions.
///
void TCPSocketPlugIn::Start()
{
	AddFunction("void TCPSocketBind( dm_string, uint32 )",	  (void *) TCPSocketBind);	
	AddFunction("void TCPSocketConnect( dm_string, uint32 )", (void *) TCPSocketConnect);
	AddFunction("void TCPSocketDisconnect( void )",			  (void *) TCPSocketDisconnect);
	AddFunction("void TCPSocketWaitIncoming( void )",         (void *) TCPSocketWaitIncoming);
	AddFunction("dm_string TCPSocketRecv( void )",	          (void *) TCPSocketRecv);
	AddFunction("long TCPSocketSend( dm_string )",	          (void *) TCPSocketSend);
	AddFunction("void TCPSocketToggleDebug( bool )",          (void *) TCPSocketToggleDebug);
}

///
/// This is called when the plugin is loaded, after the 'Start' method.
/// Whenever DM is launched, it calls the 'Run' method for
/// each installed plugin after the 'Start' method has been called
/// for all such plugins and all script packages have been installed.
/// Thus it is ok to use script functions provided by other plugins.
///
void TCPSocketPlugIn::Run()
{
	debugMessage("Plug-in successfully loaded.");
}

///
/// This is called when the plugin is unloaded.  Whenever DM is
/// shut down, the 'Cleanup' method is called for all installed plugins
/// before script packages are uninstalled and before the 'End'
/// method is called for any plugin.  Thus, script functions provided
/// by other plugins are still available.  This method should release
/// resources allocated by 'Run'.
///
void TCPSocketPlugIn::Cleanup()
{
	debugMessage("Plug-in successfully unloaded.");
}

///
/// This is called when the plugin is unloaded.  Whenever DM is shut
/// down, the 'End' method is called for all installed plugins after
/// all script packages have been unloaded, and other installed plugins
/// may have already been completely unloaded, so the code should not
/// rely on scripts installed from other plugins.  This method should
/// release resources allocated by 'Start', and in particular should
/// uninstall all installed script functions.
///
void TCPSocketPlugIn::End()
{	
	TCPSocketDisconnect();
	RemoveAllFunctions();

	debugMessage("Plug-in successfully terminated.");
}

TCPSocketPlugIn gTCPSocketPlugIn;