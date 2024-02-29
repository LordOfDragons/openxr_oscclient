/**
 * MIT License
 * 
 * Copyright (c) 2024 DragonDreams (info@dragondreams.ch)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "olotApiLayer.h"
#include "olotOcsClient.h"
#include "olotOcsMessage.h"
#include "exceptions/exceptions.h"


// Callback
/////////////

static void fThreadRead( olotOcsClient *ocsclient, bool *exitThread ){
	{
	const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
	ocsclient->log() << "Enter read thread" << std::endl;
	}
	
	const int sock = ocsclient->OpenSocket();
	if( sock != -1 ){
		uint8_t buffer[ 4096 ];
		olotOcsMessage message;
		
		while( ! *exitThread ){
			sockaddr_in senderAddress = {};
			socklen_t senderAddressLen = sizeof( senderAddress );
			
			const ssize_t length = ( size_t )recvfrom( sock, buffer, sizeof( buffer ), 0,
				( sockaddr* )&senderAddress, &senderAddressLen );
			if( length != -1 ){
				if( message.Parse( buffer, length ) ){
					ocsclient->ProcessData( message );
				}
				
			}else if( length == 0 ){
				break;
			}
		}
		
		ocsclient->CloseSocket();
	}
	
	const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
	ocsclient->log() << "Exit read thread" << std::endl;
}


// class olotOcsClient
///////////////////////

olotOcsClient::olotOcsClient() :
pUsageCount( 1 ),
pExitThread( false ),
pSocket( -1 )
{
	{
	const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
	log() << "Create OCS Client" << std::endl;
	}
	
	try{
		pInitExpressions();
		pStartThread();
	}catch( const olotException & ){
		pCleanUp();
		throw;
	}
}

olotOcsClient::~olotOcsClient(){
	{
	const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
	log() << "Destroy OCS Client" << std::endl;
	}
	pCleanUp();
}



// Management
///////////////

void olotOcsClient::AddUsage(){
	pUsageCount++;
	const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
	log() << "AddUsage (" << pUsageCount << ")" << std::endl;
}

void olotOcsClient::RemoveUsage(){
	OLOTASSERT_TRUE( pUsageCount > 0, XR_ERROR_RUNTIME_FAILURE )
	
	pUsageCount--;
	{
	const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
	log() << "RemoveUsage (" << pUsageCount << ")" << std::endl;
	}
	
	if( pUsageCount == 0 ){
		olotApiLayer::Get().DropOcsClient();
	}
}


static inline float clamp( float value ){
	return std::max( std::min( value, 1.0f ), 0.0f );
}

void olotOcsClient::ProcessData( const olotOcsMessage &message ){
	if( message.GetParameterCount() == 0 ){
		return;
	}
	
	const olotOcsMessage::sParameter &parameter = message.GetParameterAt( 0 );
	if( parameter.type != olotOcsMessage::etFloat ){
		return;
	}
	
	const char * const target = message.GetTarget();
	int i;
	
	for( i=0; i<ExpressionCount; i++  ){
		if( strcmp( pExpressions[ i ].ocsTarget, target ) == 0 ){
			break;
		}
	}
	
	if( i == ExpressionCount ){
		return;
	}
	
	const std::lock_guard<std::mutex> guard( pMutexData );
	pExpressionValues[ i ] = clamp( parameter.valueFloat );
}

int olotOcsClient::OpenSocket(){
	CloseSocket();
	
	const std::lock_guard<std::mutex> guard( pMutexData );
	
	pSocket = socket( AF_INET, SOCK_DGRAM, 0 );
	if( pSocket == -1 ){
		const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
		log() << "Read thread: failed creating socket" << std::endl;
		return -1;
	}
	
	int opt = 1;
	if( setsockopt( pSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof( opt ) ) ){
		close( pSocket );
		pSocket = -1;
		
		const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
		log() << "Read thread: failed setting socket option" << std::endl;
		return -1;
	}
	
	sockaddr_in address = {};
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( 8888 );
	
	if( bind( pSocket, ( struct sockaddr* )&address, sizeof( address ) ) == -1 ){
		close( pSocket );
		pSocket = -1;
		
		const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
		log() << "Read thread: failed binding socket" << std::endl;
		return -1;
	}
	
	return pSocket;
}

void olotOcsClient::CloseSocket(){
	const std::lock_guard<std::mutex> guard( pMutexData );
	if( pSocket == -1 ){
		return;
	}
	
	shutdown( pSocket, SHUT_RDWR );
	close( pSocket );
	pSocket = -1;
}

void olotOcsClient::GetExpressionValues( float *values, int count ){
	OLOTASSERT_NOTNULL( values, XR_ERROR_RUNTIME_FAILURE )
	OLOTASSERT_TRUE( count >= 0, XR_ERROR_RUNTIME_FAILURE )
	OLOTASSERT_TRUE( count <= ExpressionCount, XR_ERROR_RUNTIME_FAILURE )
	
	const std::lock_guard<std::mutex> guard( pMutexData );
	memcpy( values, pExpressionValues, sizeof( float ) * count );
}

std::ostream &olotOcsClient::log(){
	return olotApiLayer::Get().baseLogStream()
		<< olotApiLayer::Get().GetLayerName() << ".OcsClient: ";
}



// Private Functions
//////////////////////

void olotOcsClient::pCleanUp(){
	pStopThread();
}

void olotOcsClient::pStartThread(){
	if( pThreadRead ){
		return;
	}
	
	{
	const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
	log() << "Start read thread" << std::endl;
	}
	
	pExitThread = false;
	pThreadRead = std::make_shared<std::thread>( std::thread( fThreadRead, this, &pExitThread ) );
	
	{
	const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
	log() << "Read thread started" << std::endl;
	}
}

void olotOcsClient::pStopThread(){
	if( ! pThreadRead ){
		return;
	}
	
	{
	const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
	log() << "Stop read thread" << std::endl;
	}
	
	pExitThread = true;
	CloseSocket();
	pThreadRead->join();
	pThreadRead.reset();
	pExitThread = false;
	
	{
	const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
	log() << "Read thread stopped" << std::endl;
	}
}

void olotOcsClient::pInitExpressions(){
	pExpressions[ 0 ] = { "/cheekPuffLeft", eeCheekPuffLeft };
	pExpressions[ 1 ] = { "/cheekPuffRight", eeCheekPuffRight };
	pExpressions[ 2 ] = { "/cheekSuckLeft", eeCheekSuckLeft };
	pExpressions[ 3 ] = { "/cheekSuckRight", eeCheekSuckRight };
	pExpressions[ 4 ] = { "/jawOpen", eeJawOpen };
	pExpressions[ 5 ] = { "/jawForward", eeJawForward };
	pExpressions[ 6 ] = { "/jawLeft", eeJawLeft };
	pExpressions[ 7 ] = { "/jawRight", eeJawRight };
	pExpressions[ 8 ] = { "/noseSneerLeft", eeNoseSneerLeft };
	pExpressions[ 9 ] = { "/noseSneerRight", eeNoseSneerRight };
	pExpressions[ 10 ] = { "/mouthFunnel", eeMouthFunnel };
	pExpressions[ 11 ] = { "/mouthPucker", eeMouthPucker };
	pExpressions[ 12 ] = { "/mouthLeft", eeMouthLeft };
	pExpressions[ 13 ] = { "/mouthRight", eeMouthRight };
	pExpressions[ 14 ] = { "/mouthRollUpper", eeMouthRollUpper };
	pExpressions[ 15 ] = { "/mouthRollLower", eeMouthRollLower };
	pExpressions[ 16 ] = { "/mouthShrugUpper", eeMouthShrugUpper };
	pExpressions[ 17 ] = { "/mouthShrugLower", eeMouthShrugLower };
	pExpressions[ 18 ] = { "/mouthClose", eeMouthClose };
	pExpressions[ 19 ] = { "/mouthSmileLeft", eeMouthSmileLeft };
	pExpressions[ 20 ] = { "/mouthSmileRight", eeMouthSmileRight };
	pExpressions[ 21 ] = { "/mouthFrownLeft", eeMouthFrownLeft };
	pExpressions[ 22 ] = { "/mouthFrownRight", eeMouthFrownRight };
	pExpressions[ 23 ] = { "/mouthDimpleLeft", eeMouthDimpleLeft };
	pExpressions[ 24 ] = { "/mouthDimpleRight", eeMouthDimpleRight };
	pExpressions[ 25 ] = { "/mouthUpperUpLeft", eeMouthUpperUpLeft };
	pExpressions[ 26 ] = { "/mouthUpperUpRight", eeMouthUpperUpRight };
	pExpressions[ 27 ] = { "/mouthLowerDownLeft", eeMouthLowerDownLeft };
	pExpressions[ 28 ] = { "/mouthLowerDownRight", eeMouthLowerDownRight };
	pExpressions[ 29 ] = { "/mouthPressLeft", eeMouthPressLeft };
	pExpressions[ 30 ] = { "/mouthPressRight", eeMouthPressRight };
	pExpressions[ 31 ] = { "/mouthStretchLeft", eeMouthStretchLeft };
	pExpressions[ 32 ] = { "/mouthStretchRight", eeMouthStretchRight };
	pExpressions[ 33 ] = { "/tongueOut", eeTongueOut };
	pExpressions[ 34 ] = { "/tongueUp", eeTongueUp };
	pExpressions[ 35 ] = { "/tongueDown", eeTongueDown };
	pExpressions[ 36 ] = { "/tongueLeft", eeTongueLeft };
	pExpressions[ 37 ] = { "/tongueRight", eeTongueRight };
	pExpressions[ 38 ] = { "/tongueRoll", eeTongueRoll };
	pExpressions[ 39 ] = { "/tongueBendDown", eeTongueBendDown };
	pExpressions[ 40 ] = { "/tongueCurlUp", eeTongueCurlUp };
	pExpressions[ 41 ] = { "/tongueSquish", eeTongueSquish };
	pExpressions[ 42 ] = { "/tongueFlat", eeTongueFlat };
	pExpressions[ 43 ] = { "/tongueTwistLeft", eeTongueTwistLeft };
	pExpressions[ 44 ] = { "/tongueTwistRight", eeTongueTwistRight };
	
	int i;
	for( i=0; i<ExpressionCount; i++ ){
		pExpressionValues[ i ] = 0.0f;
	}
}
