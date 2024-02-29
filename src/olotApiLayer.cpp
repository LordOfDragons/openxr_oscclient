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

#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <filesystem>
#include <chrono>
#include <ctime>

#include "olotApiLayer.h"
#include "olotOcsClient.h"
#include "exceptions/exceptions.h"



// xrCreateInstance is a special case that we can't hook. We get this amended call instead.
static XrResult XRAPI_PTR fxrCreateApiLayerInstance( const XrInstanceCreateInfo *info,
const XrApiLayerCreateInfo *apiLayerInfo, XrInstance *instance ){
	uint32_t i, enabledExtensionCount = 0;
	const char ** enabledExtensionNames = nullptr;
	
	try{
		{
		const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
		olotApiLayer::Get().log() << "Created api layer instance for app "
			<< info->applicationInfo.applicationName << std::endl;
		}
		
		// this layer adds two new extensions which the VR runtime probably does not know
		// about. if we let nextCreateApiLayerInstance see these extensions it can likely
		// fail us due to know nothing them. the workaround here is to create a modified
		// extension list with the extensions provided by this layer filtered out
		if( info->enabledExtensionCount > 0 ){
			enabledExtensionNames = new const char*[ info->enabledExtensionCount ];
			
			for( i=0; i<info->enabledExtensionCount; i++ ){
				if( strcmp( info->enabledExtensionNames[ i ], XR_EXT_EYE_GAZE_INTERACTION_EXTENSION_NAME ) == 0 ){
					continue;
				}
				if( strcmp( info->enabledExtensionNames[ i ], XR_HTC_FACIAL_TRACKING_EXTENSION_NAME ) == 0 ){
					continue;
				}
				enabledExtensionNames[ enabledExtensionCount++ ] = info->enabledExtensionNames[ i ];
			}
		}
		
		XrInstanceCreateInfo subinfo = {
			XR_TYPE_INSTANCE_CREATE_INFO,
			info->next,
			info->createFlags,
			info->applicationInfo,
			info->enabledApiLayerCount,
			info->enabledApiLayerNames,
			enabledExtensionCount,
			enabledExtensionNames
		};
		
		OLOTASSERT_SUCCESS( apiLayerInfo->nextInfo->nextCreateApiLayerInstance( &subinfo, apiLayerInfo, instance ) )
		
		if( enabledExtensionNames ){
			delete [] enabledExtensionNames;
			enabledExtensionNames = nullptr;
		}
		
		// add instance to api layer
		olotApiLayer::Get().AddInstance( std::make_shared<olotInstance>(
			apiLayerInfo->nextInfo->nextGetInstanceProcAddr, *info, *instance ) );
		
		return XR_SUCCESS;
		
	}catch( const olotException &e ){
		if( enabledExtensionNames ){
			delete [] enabledExtensionNames;
		}
		
		{
		const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
		olotApiLayer::Get().log() << "Failed to create api layer instance:" << std::endl;
		e.PrintError( olotApiLayer::Get().baseLogStream() );
		}
		return e.GetResult();
	}
}

static XRAPI_ATTR XrResult XRAPI_CALL fxrGetInstanceProcAddr(
XrInstance instance, const char *name, PFN_xrVoidFunction *function ){
	try{
		return olotApiLayer::Get().GetInstance( instance )->GetInstanceProcAddr( name, function );
		
	}catch( const olotException &e ){
		e.PrintError( olotApiLayer::Get().baseLogStream() );
		return e.GetResult();
	}
}



// class olotApiLayer
///////////////////////

static olotApiLayer vApiLayer;

olotApiLayer::olotApiLayer() :
pLayerName( "ocseyefacetracking" ),
pSupportsEyeGazeTracking( true ),
pSupportsFacialTracking( true )
{
	/*
	char buffer[ MAX_PATH ];
	size_t len = 0;
	memset( &buffer, 0, sizeof( buffer ) );
	( void )getenv_s( &len, &buffer[ 0 ], sizeof( buffer ), "LOCALAPPDATA" );
	
	auto dirLocalAppData = std::filesystem::path( buffer );
	
	auto dirLogDragonDreams = dirLocalAppData / "DragonDreams";
	std::filesystem::create_directory( dirLogDragonDreams );
	*/
	
	pLogFile.rdbuf()->pubsetbuf( nullptr, 0 );
	pLogFile.open( /*dirLogDragonDreams /*/ "XrApiLayer_ocseyefacetracking.log",
		std::ofstream::out | std::ofstream::trunc );
}

olotApiLayer::~olotApiLayer(){
	pLogFile.flush();
	pLogFile.close();
}



// Management
///////////////

olotApiLayer &olotApiLayer::Get(){
	return vApiLayer;
}



XrResult olotApiLayer::Negotiate( const XrNegotiateLoaderInfo *loaderInfo,
const char *layerName, XrNegotiateApiLayerRequest *apiLayerRequest ){
	pLayerName = layerName;
	
	{
	std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
	log() << "Using API layer: " << layerName << std::endl;
	
	log() << "loader API version min: "
		<< XR_VERSION_MAJOR( loaderInfo->minApiVersion ) << "."
		<< XR_VERSION_MINOR( loaderInfo->minApiVersion ) << "."
		<< XR_VERSION_PATCH( loaderInfo->minApiVersion ) << "."
		<< " max: "
		<< XR_VERSION_MAJOR( loaderInfo->maxApiVersion ) << "."
		<< XR_VERSION_MINOR( loaderInfo->maxApiVersion ) << "."
		<< XR_VERSION_PATCH( loaderInfo->maxApiVersion ) << "."
		<< std::endl;
	
	log() << "loader interface version min: "
		<< XR_VERSION_MAJOR( loaderInfo->minInterfaceVersion ) << "."
		<< XR_VERSION_MINOR( loaderInfo->minInterfaceVersion ) << "."
		<< XR_VERSION_PATCH( loaderInfo->minInterfaceVersion ) << "."
		<< " max: "
		<< XR_VERSION_MAJOR( loaderInfo->maxInterfaceVersion ) << "."
		<< XR_VERSION_MINOR( loaderInfo->maxInterfaceVersion ) << "."
		<< XR_VERSION_PATCH( loaderInfo->maxInterfaceVersion ) << "."
		<< std::endl;
	}
	
	// TODO: proper version check
	// On error return XR_ERROR_INITIALIZATION_FAILED
	apiLayerRequest->layerInterfaceVersion = loaderInfo->maxInterfaceVersion;
	apiLayerRequest->layerApiVersion = loaderInfo->maxApiVersion;
	apiLayerRequest->getInstanceProcAddr = fxrGetInstanceProcAddr;
	apiLayerRequest->createApiLayerInstance = fxrCreateApiLayerInstance;
	
	return XR_SUCCESS;
}



bool olotApiLayer::HasInstance( XrInstance instance ) const{
	return pInstances.find( instance ) != pInstances.cend();
}

olotInstance::Ref olotApiLayer::GetInstance( XrInstance instance ) const{
	MapInstances::const_iterator iter( pInstances.find( instance ) );
	OLOTASSERT_FALSE( iter == pInstances.cend(), XR_ERROR_HANDLE_INVALID )
	return iter->second;
}

void olotApiLayer::AddInstance( const olotInstance::Ref &instance ){
	OLOTASSERT_NOTNULL( instance, XR_ERROR_HANDLE_INVALID )
	pInstances[ instance->GetInstance() ] = instance;
}

void olotApiLayer::RemoveInstance( XrInstance instance ){
	MapInstances::iterator iter( pInstances.find( instance ) );
	OLOTASSERT_FALSE( iter == pInstances.cend(), XR_ERROR_HANDLE_INVALID )
	pInstances.erase( iter );
}



olotOcsClient *olotApiLayer::AcquireOcsClient(){
	if( pOcsClient ){
		pOcsClient->AddUsage();
		
	}else{
		pOcsClient = std::make_shared<olotOcsClient>();
	}
	
	return pOcsClient.get();
}

void olotApiLayer::DropOcsClient(){
	pOcsClient.reset();
}



std::ostream& olotApiLayer::baseLogStream()
{
	const auto now = std::time( nullptr );
	const auto tm_info = std::localtime( &now );
	
	char buffer[ 100 ];
	std::strftime( buffer, 100, "%Y-%m-%d %H:%M:%S", tm_info );
	
	return pLogFile << "[" << buffer << "] ";
}

std::ostream &olotApiLayer::log(){
	return baseLogStream() << pLayerName << ": ";
}
