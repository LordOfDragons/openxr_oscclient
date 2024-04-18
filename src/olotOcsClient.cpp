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

#include <algorithm>
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
		pInitEyeStates();
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

inline const std::string strToLower( const char * const string ){
	std::string s( string );
	std::transform( s.begin(), s.end(), s.begin(), ::tolower );
	return s;
}

void olotOcsClient::ProcessData( const olotOcsMessage &message ){
	if( message.GetParameterCount() == 0 ){
		return;
	}
	
	const olotOcsMessage::sParameter &parameter = message.GetParameterAt( 0 );
	if( parameter.type != olotOcsMessage::etFloat ){
		return;
	}
	
	const std::string target( strToLower( message.GetTarget() ) );
	int i;
	
	// eye state
	for( i=0; i<EyeStateCount; i++  ){
		if( pEyeStates[ i ].ocsTarget == target ){
			const std::lock_guard<std::mutex> guard( pMutexData );
			pEyeStateValues[ i ] = clamp( parameter.valueFloat );
			return;
		}
	}
	
	// face expression
	for( i=0; i<ExpressionCount; i++  ){
		if( pExpressions[ i ].ocsTarget == target ){
			const std::lock_guard<std::mutex> guard( pMutexData );
			pExpressionValues[ i ] = clamp( parameter.valueFloat );
			return;
		}
	}
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

void olotOcsClient::GetEyeStateValues( float *values, int count ){
	OLOTASSERT_NOTNULL( values, XR_ERROR_RUNTIME_FAILURE )
	OLOTASSERT_TRUE( count >= 0, XR_ERROR_RUNTIME_FAILURE )
	OLOTASSERT_TRUE( count <= EyeStateCount, XR_ERROR_RUNTIME_FAILURE )
	
	const std::lock_guard<std::mutex> guard( pMutexData );
	memcpy( values, pEyeStates, sizeof( float ) * count );
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
	pExpressions[ 0 ] = { strToLower( "/cheekPuffLeft" ), eeCheekPuffLeft };
	pExpressions[ 1 ] = { strToLower( "/cheekPuffRight" ), eeCheekPuffRight };
	pExpressions[ 2 ] = { strToLower( "/cheekSuckLeft" ), eeCheekSuckLeft };
	pExpressions[ 3 ] = { strToLower( "/cheekSuckRight" ), eeCheekSuckRight };
	pExpressions[ 4 ] = { strToLower( "/jawOpen" ), eeJawOpen };
	pExpressions[ 5 ] = { strToLower( "/jawForward" ), eeJawForward };
	pExpressions[ 6 ] = { strToLower( "/jawLeft" ), eeJawLeft };
	pExpressions[ 7 ] = { strToLower( "/jawRight" ), eeJawRight };
	pExpressions[ 8 ] = { strToLower( "/noseSneerLeft" ), eeNoseSneerLeft };
	pExpressions[ 9 ] = { strToLower( "/noseSneerRight" ), eeNoseSneerRight };
	pExpressions[ 10 ] = { strToLower( "/mouthFunnel" ), eeMouthFunnel };
	pExpressions[ 11 ] = { strToLower( "/mouthPucker" ), eeMouthPucker };
	pExpressions[ 12 ] = { strToLower( "/mouthLeft" ), eeMouthLeft };
	pExpressions[ 13 ] = { strToLower( "/mouthRight" ), eeMouthRight };
	pExpressions[ 14 ] = { strToLower( "/mouthRollUpper" ), eeMouthRollUpper };
	pExpressions[ 15 ] = { strToLower( "/mouthRollLower" ), eeMouthRollLower };
	pExpressions[ 16 ] = { strToLower( "/mouthShrugUpper" ), eeMouthShrugUpper };
	pExpressions[ 17 ] = { strToLower( "/mouthShrugLower" ), eeMouthShrugLower };
	pExpressions[ 18 ] = { strToLower( "/mouthClose" ), eeMouthClose };
	pExpressions[ 19 ] = { strToLower( "/mouthSmileLeft" ), eeMouthSmileLeft };
	pExpressions[ 20 ] = { strToLower( "/mouthSmileRight" ), eeMouthSmileRight };
	pExpressions[ 21 ] = { strToLower( "/mouthFrownLeft" ), eeMouthFrownLeft };
	pExpressions[ 22 ] = { strToLower( "/mouthFrownRight" ), eeMouthFrownRight };
	pExpressions[ 23 ] = { strToLower( "/mouthDimpleLeft" ), eeMouthDimpleLeft };
	pExpressions[ 24 ] = { strToLower( "/mouthDimpleRight" ), eeMouthDimpleRight };
	pExpressions[ 25 ] = { strToLower( "/mouthUpperUpLeft" ), eeMouthUpperUpLeft };
	pExpressions[ 26 ] = { strToLower( "/mouthUpperUpRight" ), eeMouthUpperUpRight };
	pExpressions[ 27 ] = { strToLower( "/mouthLowerDownLeft" ), eeMouthLowerDownLeft };
	pExpressions[ 28 ] = { strToLower( "/mouthLowerDownRight" ), eeMouthLowerDownRight };
	pExpressions[ 29 ] = { strToLower( "/mouthPressLeft" ), eeMouthPressLeft };
	pExpressions[ 30 ] = { strToLower( "/mouthPressRight" ), eeMouthPressRight };
	pExpressions[ 31 ] = { strToLower( "/mouthStretchLeft" ), eeMouthStretchLeft };
	pExpressions[ 32 ] = { strToLower( "/mouthStretchRight" ), eeMouthStretchRight };
	pExpressions[ 33 ] = { strToLower( "/tongueOut" ), eeTongueOut };
	pExpressions[ 34 ] = { strToLower( "/tongueUp" ), eeTongueUp };
	pExpressions[ 35 ] = { strToLower( "/tongueDown" ), eeTongueDown };
	pExpressions[ 36 ] = { strToLower( "/tongueLeft" ), eeTongueLeft };
	pExpressions[ 37 ] = { strToLower( "/tongueRight" ), eeTongueRight };
	pExpressions[ 38 ] = { strToLower( "/tongueRoll" ), eeTongueRoll };
	pExpressions[ 39 ] = { strToLower( "/tongueBendDown" ), eeTongueBendDown };
	pExpressions[ 40 ] = { strToLower( "/tongueCurlUp" ), eeTongueCurlUp };
	pExpressions[ 41 ] = { strToLower( "/tongueSquish" ), eeTongueSquish };
	pExpressions[ 42 ] = { strToLower( "/tongueFlat" ), eeTongueFlat };
	pExpressions[ 43 ] = { strToLower( "/tongueTwistLeft" ), eeTongueTwistLeft };
	pExpressions[ 44 ] = { strToLower( "/tongueTwistRight" ), eeTongueTwistRight };
	pExpressions[ 45 ] = { strToLower( "/leftEyeLidExpandedSqueeze" ), eeLeftEyeLidExpandedSqueeze };
	pExpressions[ 46 ] = { strToLower( "/rightEyeLidExpandedSqueeze" ), eeRightEyeLidExpandedSqueeze };
	
	int i;
	for( i=0; i<ExpressionCount; i++ ){
		pExpressionValues[ i ] = 0.0f;
	}
}

void olotOcsClient::pInitEyeStates(){
	pEyeStates[ 0 ] = { strToLower( "/leftEyeX" ), eesLeftEyeX };
	pEyeStates[ 1 ] = { strToLower( "/rightEyeX" ), eesRightEyeX };
	pEyeStates[ 2 ] = { strToLower( "/eyesY" ), eesEyesY };
	
	int i;
	for( i=0; i<EyeStateCount; i++ ){
		pEyeStateValues[ i ] = 0.0f;
	}
}
