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

#ifndef _OLOTOCSCLIENT_H_
#define _OLOTOCSCLIENT_H_

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <sys/socket.h>

class olotOcsMessage;


/**
 * OCS Client.
 */
class olotOcsClient{
public:
	enum eExpression {
		eeCheekPuffLeft,
		eeCheekPuffRight,
		eeCheekSuckLeft,
		eeCheekSuckRight,
		eeJawOpen,
		eeJawForward,
		eeJawLeft,
		eeJawRight,
		eeNoseSneerLeft,
		eeNoseSneerRight,
		eeMouthFunnel,
		eeMouthPucker,
		eeMouthLeft,
		eeMouthRight,
		eeMouthRollUpper,
		eeMouthRollLower,
		eeMouthShrugUpper,
		eeMouthShrugLower,
		eeMouthClose,
		eeMouthSmileLeft,
		eeMouthSmileRight,
		eeMouthFrownLeft,
		eeMouthFrownRight,
		eeMouthDimpleLeft,
		eeMouthDimpleRight,
		eeMouthUpperUpLeft,
		eeMouthUpperUpRight,
		eeMouthLowerDownLeft,
		eeMouthLowerDownRight,
		eeMouthPressLeft,
		eeMouthPressRight,
		eeMouthStretchLeft,
		eeMouthStretchRight,
		eeTongueOut,
		eeTongueUp,
		eeTongueDown,
		eeTongueLeft,
		eeTongueRight,
		eeTongueRoll,
		eeTongueBendDown,
		eeTongueCurlUp,
		eeTongueSquish,
		eeTongueFlat,
		eeTongueTwistLeft,
		eeTongueTwistRight,
		eeLeftEyeLidExpandedSqueeze,
		eeRightEyeLidExpandedSqueeze
	};
	
	const static int ExpressionCount = eeRightEyeLidExpandedSqueeze + 1;
	
	enum eEyeState{
		eesLeftEyeX,
		eesRightEyeX,
		eesEyesY
	};
	
	const static int EyeStateCount = eesEyesY + 1;
	
private:
	struct sExpression {
		std::string ocsTarget;
		eExpression expression;
	};
	struct sEyeState {
		std::string ocsTarget;
		eEyeState state;
	};
	
	int pUsageCount;
	std::shared_ptr<std::thread> pThreadRead;
	std::mutex pMutexData;
	bool pExitThread;
	int pSocket;
	
	sExpression pExpressions[ ExpressionCount ];
	float pExpressionValues[ ExpressionCount ];
	
	sEyeState pEyeStates[ EyeStateCount ];
	float pEyeStateValues[ EyeStateCount ];
	
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** Create OCS Client. */
	olotOcsClient();
	
	/** Clean up OCS Client. */
	~olotOcsClient();
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	/** Add usage. */
	void AddUsage();
	
	/** Remove usage. */
	void RemoveUsage();
	
	/** Process query data. For internal use only. */
	void ProcessData( const olotOcsMessage &message );
	
	/** Open socket. For internal use only. */
	int OpenSocket();
	
	/** Close socket. For internal use only. */
	void CloseSocket();
	
	/** Copy expression values. */
	void GetExpressionValues( float *values, int count );
	
	/** Copy eye state values. */
	void GetEyeStateValues( float *values, int count );
	
	/** Log stream. */
	std::ostream &log();
	/*@}*/
	
	
	
private:
	void pCleanUp();
	void pStartThread();
	void pStopThread();
	void pInitExpressions();
	void pInitEyeStates();
};

#endif
