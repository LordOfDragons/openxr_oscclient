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

#include "olotInstance.h"
#include "olotApiLayer.h"
#include "exceptions/exceptions.h"



// Hooks
//////////

#define OXR_CHAIN_CALL(fn,c) \
	try{\
		return c;\
	}catch( const olotException &e ){\
		const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );\
		olotApiLayer::Get().log() << fn << " failed:" << std::endl;\
		e.PrintError( olotApiLayer::Get().baseLogStream() );\
		return e.GetResult();\
	}


static XrResult fxrGetSystemProperties( XrInstance instance, XrSystemId systemId,
XrSystemProperties *properties ){
	OXR_CHAIN_CALL( "xrGetSystemProperties", olotApiLayer::Get().
		GetInstance( instance )->GetSystemProperties( systemId, properties ) );
}

static XrResult fxrSuggestInteractionProfileBindings( XrInstance instance,
const XrInteractionProfileSuggestedBinding *suggestedBindings ){
	OXR_CHAIN_CALL( "xrSuggestInteractionProfileBindings", olotApiLayer::Get().
		GetInstance( instance )->SuggestInteractionProfileBindings( suggestedBindings ) );
}

static XrResult fxrDestroyInstance( XrInstance instance ){
	try{
		const XrResult result = olotApiLayer::Get().GetInstance( instance )->DestroyInstance();
		olotApiLayer::Get().RemoveInstance( instance );
		return result;
		
	}catch( const olotException &e ){
		e.PrintError( olotApiLayer::Get().baseLogStream() );
		
		if( olotApiLayer::Get().HasInstance( instance ) ){
			try{
				olotApiLayer::Get().RemoveInstance( instance );
				
			}catch( const olotException &e ){
				e.PrintError( olotApiLayer::Get().baseLogStream() );
			}
		}
		
		return e.GetResult();
	}
}

static XrResult fxrCreateSession( XrInstance instance,
const XrSessionCreateInfo *createInfo, XrSession *session ){
	try{
		return olotApiLayer::Get().GetInstance( instance )->CreateSession( createInfo, session );
		
	}catch( const olotException &e ){
		e.PrintError( olotApiLayer::Get().baseLogStream() );
		return e.GetResult();
	}
}

static XrResult fxrDestroySession( XrSession session ){
	OXR_CHAIN_CALL( "xrDestroySession", olotApiLayer::Get().
		GetSessions()[ session ]->DestroySession( session ) );
}

static XrResult fxrGetActionStatePose( XrSession session,
const XrActionStateGetInfo *getInfo, XrActionStatePose *state ){
	OXR_CHAIN_CALL( "xrGetActionStatePose", olotApiLayer::Get().
		GetSessions()[ session ]->GetActionStatePose( session, getInfo, state ) );
}

static XrResult fxrLocateSpace( XrSpace space, XrSpace baseSpace, XrTime time,
XrSpaceLocation *location ){
	try{
		const olotSpace &s = olotApiLayer::Get().GetSpaces()[ space ];
		return s.instance->LocateSpace( s, baseSpace, time, location );
	}catch( const olotException &e ){
		const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
		olotApiLayer::Get().log() << "xrLocateSpace failed:" << std::endl;
		e.PrintError( olotApiLayer::Get().baseLogStream() );
		return e.GetResult();
	}
}

static XrResult fxrCreateActionSpace( XrSession session,
const XrActionSpaceCreateInfo *createInfo, XrSpace *space ){
	try{
		return olotApiLayer::Get().GetSessions()[ session ]->
			CreateActionSpace( session, createInfo, space );
		
	}catch( const olotException &e ){
		e.PrintError( olotApiLayer::Get().baseLogStream() );
		return e.GetResult();
	}
}

static XrResult fxrCreateReferenceSpace( XrSession session,
const XrReferenceSpaceCreateInfo *createInfo, XrSpace *space ){
	try{
		return olotApiLayer::Get().GetSessions()[ session ]->
			CreateReferenceSpace( session, createInfo, space );
		
	}catch( const olotException &e ){
		e.PrintError( olotApiLayer::Get().baseLogStream() );
		return e.GetResult();
	}
}

static XrResult fxrDestroySpace( XrSpace space ){
	OXR_CHAIN_CALL( "xrDestroySpace", olotApiLayer::Get().GetSpaces()[ space ].
		instance->DestroySpace( space ) );
}

static XrResult fxrCreateActionSet( XrInstance instance,
const XrActionSetCreateInfo *createInfo, XrActionSet *actionSet ){
	OXR_CHAIN_CALL( "xrCreateActionSet", olotApiLayer::Get().
		GetInstance( instance )->CreateActionSet( createInfo, actionSet ) );
}

static XrResult fxrDestroyActionSet( XrActionSet actionSet ){
	OXR_CHAIN_CALL( "xrDestroyActionSet", olotApiLayer::Get().
		GetActionSets()[ actionSet ]->DestroyActionSet( actionSet ) );
}

static XrResult fxrCreateAction( XrActionSet actionSet,
const XrActionCreateInfo *createInfo, XrAction *action ){
	OXR_CHAIN_CALL( "xrCreateActionSet", olotApiLayer::Get().
		GetActionSets()[ actionSet ]->CreateAction( actionSet, createInfo, action ) );
}

static XrResult fxrDestroyAction( XrAction action ){
	OXR_CHAIN_CALL( "xrDestroyAction", olotApiLayer::Get().
		GetActions()[ action ]->DestroyAction( action ) );
}

static XrResult fxrCreateFacialTrackerHTC( XrSession session,
const XrFacialTrackerCreateInfoHTC *createInfo, XrFacialTrackerHTC *facialTracker ){
	OXR_CHAIN_CALL( "xrCreateFacialTrackerHTC", olotApiLayer::Get().
		GetSessions()[ session ]->CreateFacialTracker( session, createInfo, facialTracker ) );
}

static XrResult fxrDestroyFacialTrackerHTC( XrFacialTrackerHTC facialTracker ){
	OXR_CHAIN_CALL( "xrDestroyFacialTrackerHTC", ( ( ( olotFacialTracker* )facialTracker ) )->
		DestroyFacialTracker() );
}

static XrResult fxrGetFacialExpressionsHTC( XrFacialTrackerHTC facialTracker,
XrFacialExpressionsHTC *facialExpressions ){
	OXR_CHAIN_CALL( "xrGetFacialExpressionsHTC", ( ( olotFacialTracker* )facialTracker )->
		GetFacialExpressionsHTC( facialExpressions ) );
}



// class olotInstance
///////////////////////

int olotInstance::pNextId = 0;

#define OLOT_GET_NEXT_FUNC(fn,next) \
	OLOTASSERT_SUCCESS( nextXrGetInstanceProcAddr( instance, fn, ( PFN_xrVoidFunction* )&next ) )

#define OLOT_GET_NEXT_FUNC_OPT(fn,next) \
	( void )nextXrGetInstanceProcAddr( instance, fn, ( PFN_xrVoidFunction* )&next )

olotInstance::olotInstance( PFN_xrGetInstanceProcAddr nextXrGetInstanceProcAddr,
	const XrInstanceCreateInfo &info, XrInstance instance ) :
pId( pNextId++ ),
pInstance( instance ),
pEnableEyeGaze( false ),
pEnableFacial( false ),
pNextXrGetInstanceProcAddr( nextXrGetInstanceProcAddr ),
pXrStringToPath( nullptr ),
pNextXrGetSystemProperties( nullptr ),
pNextXrSuggestInteractionProfileBindings( nullptr ),
pNextXrDestroyInstance( nullptr ),
pNextXrCreateSession( nullptr ),
pNextXrDestroySession( nullptr ),
pNextXrGetActionStatePose( nullptr ),
pNextXrLocateSpace( nullptr ),
pNextXrCreateActionSpace( nullptr ),
pNextXrCreateReferenceSpace( nullptr ),
pNextXrDestroySpace( nullptr ),
pNextXrCreateActionSet( nullptr ),
pNextXrDestroyActionSet( nullptr ),
pNextXrCreateAction( nullptr ),
pNextXrDestroyAction( nullptr ),
pPathProfileEyeGaze( XR_NULL_PATH )
{
	try{
		OLOT_GET_NEXT_FUNC( "xrStringToPath", pXrStringToPath );
		
		OLOT_GET_NEXT_FUNC( "xrGetSystemProperties", pNextXrGetSystemProperties );
		OLOT_GET_NEXT_FUNC( "xrSuggestInteractionProfileBindings", pNextXrSuggestInteractionProfileBindings );
		OLOT_GET_NEXT_FUNC( "xrDestroyInstance", pNextXrDestroyInstance );
		OLOT_GET_NEXT_FUNC( "xrCreateSession", pNextXrCreateSession );
		OLOT_GET_NEXT_FUNC( "xrDestroySession", pNextXrDestroySession );
		OLOT_GET_NEXT_FUNC( "xrGetActionStatePose", pNextXrGetActionStatePose );
		OLOT_GET_NEXT_FUNC( "xrLocateSpace", pNextXrLocateSpace );
		OLOT_GET_NEXT_FUNC( "xrCreateActionSpace", pNextXrCreateActionSpace );
		OLOT_GET_NEXT_FUNC( "xrCreateReferenceSpace", pNextXrCreateReferenceSpace );
		OLOT_GET_NEXT_FUNC( "xrDestroySpace", pNextXrDestroySpace );
		OLOT_GET_NEXT_FUNC( "xrCreateActionSet", pNextXrCreateActionSet );
		OLOT_GET_NEXT_FUNC( "xrDestroyActionSet", pNextXrDestroyActionSet );
		OLOT_GET_NEXT_FUNC( "xrCreateAction", pNextXrCreateAction );
		OLOT_GET_NEXT_FUNC( "xrDestroyAction", pNextXrDestroyAction );
		
		pPathProfileEyeGaze = GetXrPathFor( "/interaction_profiles/ext/eye_gaze_interaction" );
		
		olotApiLayer &apiLayer = olotApiLayer::Get();
		uint32_t i;
		
		for( i=0; i<info.enabledExtensionCount; i++ ){
			if( strcmp( info.enabledExtensionNames[ i ], XR_EXT_EYE_GAZE_INTERACTION_EXTENSION_NAME ) == 0 ){
				if( ! apiLayer.GetSupportsEyeGazeTracking() ){
					{
					const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
					log() << "Enable eye gaze interaction requested but not supported" << std::endl;
					}
					OLOTASSERT_SUCCESS( XR_ERROR_EXTENSION_NOT_PRESENT )
				}
				pEnableEyeGaze = true;
				
			}else if( strcmp( info.enabledExtensionNames[ i ], XR_HTC_FACIAL_TRACKING_EXTENSION_NAME ) == 0 ){
				if( ! apiLayer.GetSupportsEyeGazeTracking() ){
					{
					const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
					log() << "Enable facial tracking requested but not supported" << std::endl;
					}
					OLOTASSERT_SUCCESS( XR_ERROR_EXTENSION_NOT_PRESENT )
				}
				pEnableFacial = true;
			}
		}
		
		{
		const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
		log() << "Enable eye gaze interaction: " << ( pEnableEyeGaze ? "yes" : "no" ) << std::endl;
		log() << "Enable facial tracking: " << ( pEnableFacial ? "yes" : "no" ) << std::endl;
		}
		
		if( pEnableEyeGaze ){
			{
			const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
			log() << "Create eye gaze tracker" << std::endl;
			}
			pEyeGazeTracker = std::make_shared<olotEyeGazeTracker>( *this );
		}
		
	}catch( const olotException & ){
		pCleanUp();
		throw;
	}
}

#undef OLOT_GET_NEXT_FUNC_OPT
#undef OLOT_GET_NEXT_FUNC

olotInstance::~olotInstance(){
	pCleanUp();
}



// Management
///////////////

#define OLOT_GET_INST_PROC_ADDR(fn, f)\
	if( name == fn ){\
		*function = ( PFN_xrVoidFunction )f;\
		return XR_SUCCESS;\
	}

XrResult olotInstance::GetInstanceProcAddr( const std::string &name, PFN_xrVoidFunction *function ) const{
	OLOT_GET_INST_PROC_ADDR( "xrGetSystemProperties", fxrGetSystemProperties )
	OLOT_GET_INST_PROC_ADDR( "xrSuggestInteractionProfileBindings", fxrSuggestInteractionProfileBindings );
	OLOT_GET_INST_PROC_ADDR( "xrDestroyInstance", fxrDestroyInstance );
	OLOT_GET_INST_PROC_ADDR( "xrCreateSession", fxrCreateSession );
	OLOT_GET_INST_PROC_ADDR( "xrDestroySession", fxrDestroySession );
	OLOT_GET_INST_PROC_ADDR( "xrGetActionStatePose", fxrGetActionStatePose );
	OLOT_GET_INST_PROC_ADDR( "xrLocateSpace", fxrLocateSpace );
	OLOT_GET_INST_PROC_ADDR( "xrCreateActionSpace", fxrCreateActionSpace );
	OLOT_GET_INST_PROC_ADDR( "xrCreateReferenceSpace", fxrCreateReferenceSpace );
	OLOT_GET_INST_PROC_ADDR( "xrDestroySpace", fxrDestroySpace );
	OLOT_GET_INST_PROC_ADDR( "xrCreateActionSet", fxrCreateActionSet );
	OLOT_GET_INST_PROC_ADDR( "xrDestroyActionSet", fxrDestroyActionSet );
	OLOT_GET_INST_PROC_ADDR( "xrCreateAction", fxrCreateAction );
	OLOT_GET_INST_PROC_ADDR( "xrDestroyAction", fxrDestroyAction );
	
	OLOT_GET_INST_PROC_ADDR( "xrCreateFacialTrackerHTC", fxrCreateFacialTrackerHTC );
	OLOT_GET_INST_PROC_ADDR( "xrDestroyFacialTrackerHTC", fxrDestroyFacialTrackerHTC );
	OLOT_GET_INST_PROC_ADDR( "xrGetFacialExpressionsHTC", fxrGetFacialExpressionsHTC );
	
	return pNextXrGetInstanceProcAddr( pInstance, name.c_str(), function );
}

#undef OLOT_GET_INST_PROC_ADDR

XrResult olotInstance::GetSystemProperties( XrSystemId systemId, XrSystemProperties *properties ){
	OLOTASSERT_NOTNULL( properties, XR_ERROR_VALIDATION_FAILURE )
	
	olotApiLayer &apiLayer = olotApiLayer::Get();
	void *next = properties->next;
	
	while( next ){
		const XrBaseOutStructure &bos = *( ( XrBaseOutStructure* )next );
		
		switch( bos.type ){
		case XR_TYPE_SYSTEM_EYE_GAZE_INTERACTION_PROPERTIES_EXT:{
			XrSystemEyeGazeInteractionPropertiesEXT &p = *( ( XrSystemEyeGazeInteractionPropertiesEXT* )next );
			p.supportsEyeGazeInteraction = apiLayer.GetSupportsEyeGazeTracking() ? XR_TRUE : XR_FALSE;
			}break;
			
		case XR_TYPE_SYSTEM_FACIAL_TRACKING_PROPERTIES_HTC:{
			XrSystemFacialTrackingPropertiesHTC &p = *( ( XrSystemFacialTrackingPropertiesHTC* )next );
			p.supportEyeFacialTracking = apiLayer.GetSupportsFacialTracking() ? XR_TRUE : XR_FALSE;
			p.supportLipFacialTracking = apiLayer.GetSupportsFacialTracking() ? XR_TRUE : XR_FALSE;
			}break;
			
		default:
			break;
		}
		
		next = bos.next;
	}
	
	return pNextXrGetSystemProperties( pInstance, systemId, properties );
}

XrResult olotInstance::SuggestInteractionProfileBindings(
const XrInteractionProfileSuggestedBinding *suggestedBindings ){
	if( suggestedBindings->interactionProfile != pPathProfileEyeGaze ){
		return pNextXrSuggestInteractionProfileBindings( pInstance, suggestedBindings );
	}
	
	if( pEyeGazeTracker ){
		return pEyeGazeTracker->SuggestInteractionProfileBindings( *suggestedBindings );
		
	}else{
		return XR_ERROR_FEATURE_UNSUPPORTED;
	}
}

XrResult olotInstance::DestroyInstance(){
	{
	const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
	log() << "Destroy instance" << std::endl;
	}
	
	olotApiLayer &apiLayer = olotApiLayer::Get();
	
	olotApiLayer::MapSessions &sessions = apiLayer.GetSessions();
	for( olotApiLayer::MapSessions::iterator iter = sessions.begin(); iter != sessions.end(); ){
		if( this == iter->second ){
			iter = sessions.erase( iter );
			
		}else{
			iter++;
		}
	}
	
	olotApiLayer::MapActionSets &actionSets = apiLayer.GetActionSets();
	for( olotApiLayer::MapActionSets::iterator iter = actionSets.begin(); iter != actionSets.end(); ){
		if( this == iter->second ){
			iter = actionSets.erase( iter );
			
		}else{
			iter++;
		}
	}
	
	olotApiLayer::MapActions &actions = apiLayer.GetActions();
	for( olotApiLayer::MapActions::iterator iter = actions.begin(); iter != actions.end(); ){
		if( this == iter->second ){
			iter = actions.erase( iter );
			
		}else{
			iter++;
		}
	}
	
	return pNextXrDestroyInstance( pInstance );
}

XrResult olotInstance::CreateSession( const XrSessionCreateInfo *createInfo, XrSession *session ){
	const XrResult result = pNextXrCreateSession( pInstance, createInfo, session );
	if( XR_SUCCEEDED( result ) ){
		olotApiLayer::Get().GetSessions()[ *session ] = this;
	}
	return result;
}

XrResult olotInstance::DestroySession( XrSession session ){
	olotApiLayer::MapSessions::iterator iter( olotApiLayer::Get().GetSessions().find( session ) );
	if( iter != olotApiLayer::Get().GetSessions().end() ){
		olotApiLayer::Get().GetSessions().erase( iter );
	}
	return pNextXrDestroySession( session );
}

XrResult olotInstance::CreateActionSet( const XrActionSetCreateInfo *createInfo, XrActionSet *actionSet ){
	const XrResult result = pNextXrCreateActionSet( pInstance, createInfo, actionSet );
	if( XR_SUCCEEDED( result ) ){
		olotApiLayer::Get().GetActionSets()[ *actionSet ] = this;
	}
	return result;
}

XrResult olotInstance::DestroyActionSet( XrActionSet actionSet ){
	olotApiLayer::MapActionSets::iterator iter( olotApiLayer::Get().GetActionSets().find( actionSet ) );
	if( iter != olotApiLayer::Get().GetActionSets().end() ){
		olotApiLayer::Get().GetActionSets().erase( iter );
	}
	return pNextXrDestroyActionSet( actionSet );
}

XrResult olotInstance::CreateAction( XrActionSet actionSet,
const XrActionCreateInfo *createInfo, XrAction *action ){
	const XrResult result = pNextXrCreateAction( actionSet, createInfo, action );
	if( XR_SUCCEEDED( result ) ){
		olotApiLayer::Get().GetActions()[ *action ] = this;
	}
	return result;
}

XrResult olotInstance::DestroyAction( XrAction action ){
	olotApiLayer::MapActions::iterator iter( olotApiLayer::Get().GetActions().find( action ) );
	if( iter != olotApiLayer::Get().GetActions().end() ){
		olotApiLayer::Get().GetActions().erase( iter );
	}
	return pNextXrDestroyAction( action );
}

XrResult olotInstance::CreateActionSpace( XrSession session,
const XrActionSpaceCreateInfo *createInfo, XrSpace *space ){
	const XrResult result = pNextXrCreateActionSpace( session, createInfo, space );
	if( XR_SUCCEEDED( result ) ){
		olotApiLayer::Get().GetSpaces()[ *space ] =
			{ *space, this, createInfo->action, createInfo->subactionPath };
	}
	return result;
}

XrResult olotInstance::CreateReferenceSpace( XrSession session,
const XrReferenceSpaceCreateInfo *createInfo, XrSpace *space ){
	const XrResult result = pNextXrCreateReferenceSpace( session, createInfo, space );
	if( XR_SUCCEEDED( result ) ){
		olotApiLayer::Get().GetSpaces()[ *space ] =
			{ *space, this, XR_NULL_HANDLE, XR_NULL_PATH };
	}
	return result;
}

XrResult olotInstance::DestroySpace( XrSpace space ){
	olotApiLayer::MapSpaces::iterator iter( olotApiLayer::Get().GetSpaces().find( space ) );
	if( iter != olotApiLayer::Get().GetSpaces().end() ){
		olotApiLayer::Get().GetSpaces().erase( iter );
	}
	return pNextXrDestroySpace( space );
}

XrResult olotInstance::GetActionStatePose( XrSession session,
const XrActionStateGetInfo *getInfo, XrActionStatePose *state ){
	if( pEyeGazeTracker && pEyeGazeTracker->Matches( getInfo->action, getInfo->subactionPath ) ){
		return pEyeGazeTracker->GetActionStatePose( *state );
	}
	
	return pNextXrGetActionStatePose( session, getInfo, state );
}

XrResult olotInstance::LocateSpace( const olotSpace &space, XrSpace baseSpace,
XrTime time, XrSpaceLocation* location ){
	if( pEyeGazeTracker && pEyeGazeTracker->Matches( space.action, space.subactionPath ) ){
		return pEyeGazeTracker->LocateSpace( space, baseSpace, time, location );
	}
	
	return pNextXrLocateSpace( space.space, baseSpace, time, location );
}

XrResult olotInstance::CreateFacialTracker( XrSession /*session*/,
const XrFacialTrackerCreateInfoHTC *createInfo, XrFacialTrackerHTC *facialTracker ){
	OLOTASSERT_TRUE( pEnableFacial, XR_ERROR_FEATURE_UNSUPPORTED )
	
	const olotFacialTracker::Ref ft( std::make_shared<olotFacialTracker>( *this, *createInfo ) );
	pFacialTrackers.push_back( ft );
	*facialTracker = ( XrFacialTrackerHTC )ft.get();
	return XR_SUCCESS;
}

XrResult olotInstance::DestroyFacialTracker( olotFacialTracker *facialTracker ){
	OLOTASSERT_NOTNULL( facialTracker, XR_ERROR_RUNTIME_FAILURE )
	
	ListFacialTrackers::iterator iter;
	for( iter = pFacialTrackers.begin(); iter != pFacialTrackers.end(); iter++ ){
		if( iter->get() == facialTracker ){
			pFacialTrackers.erase( iter );
			break;
		}
	}
	
	return XR_SUCCESS;
}



XrPath olotInstance::GetXrPathFor( const std::string &path ) const{
	XrPath xrp = XR_NULL_PATH;
	OLOTASSERT_SUCCESS( pXrStringToPath( pInstance, path.c_str(), &xrp ) )
	return xrp;
}

std::ostream &olotInstance::log(){
	return olotApiLayer::Get().baseLogStream() << olotApiLayer::Get().GetLayerName()
		<< ".Instance[" << pId << "]: ";
}



// Private Functions
//////////////////////

void olotInstance::pCleanUp(){
}
