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

#ifndef _OLOTFACIALTRACKER_H_
#define _OLOTFACIALTRACKER_H_

#include <memory>
#include <vector>

#include "openxr/openxr.h"
#include "olotStructs.h"
#include "olotOcsClient.h"

class olotInstance;


/**
 * Facial tracker class.
 */
class olotFacialTracker{
public:
	/** Reference. */
	typedef std::shared_ptr<olotFacialTracker> Ref;
	
	/** Type. */
	enum eType{
		etEye,
		etLip
	};
	
	
	
private:
	olotInstance &pInstance;
	eType pType;
	
	float pOcsValues[ olotOcsClient::ExpressionCount ];
	
	float *pWeights;
	uint32_t pWeightCount;
	
	bool pActive;
	
	olotOcsClient *pOcsClient;
	bool pDestroyed;
	
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** Create eye gaze tracker. */
	olotFacialTracker( olotInstance &instance, const XrFacialTrackerCreateInfoHTC &createInfo );
	
	/** Clean up eye gaze tracker. */
	~olotFacialTracker();
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	/** Instance. */
	inline olotInstance &GetInstance() const{ return pInstance; }
	
	/** xrDestroyFacialTrackerHTC. */
	XrResult DestroyFacialTracker();
	
	/** xrGetFacialExpressionsHTC. */
	XrResult GetFacialExpressionsHTC( XrFacialExpressionsHTC *facialExpressions );
	
	/** Log stream. */
	std::ostream &log();
	/*@}*/
	
	
	
private:
	void pCleanUp();
	void pCreateEyeTracker();
	void pCreateLipTracker();
};

#endif
