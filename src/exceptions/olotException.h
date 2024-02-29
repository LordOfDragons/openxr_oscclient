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

#ifndef _OLOTEXCEPTION_H_
#define _OLOTEXCEPTION_H_

#include <stddef.h>
#include <string>
#include <vector>

#include "../openxr/openxr.h"


/** Exception class. */
class olotException{
private:
	std::string pName;
	std::string pDescription;
	std::string pFile;
	int pLine;
	XrResult pResult;
	std::vector<std::string> pBacktrace;
	
	
	
public:
	/** \name Constructors, Destructors */
	/*@{*/
	/** Create a new exception object. */
	olotException( const std::string &name, const std::string &description,
		const std::string &file, int line, XrResult result );
	
	/** Clean up exception object. */
	virtual ~olotException();
	/*@}*/
	
	
	
	/** \name Accessors */
	/*@{*/
	/** Unique name to identify the exception. */
	inline const std::string &GetName() const{ return pName; }
	
	/** Description of the reason for the exception. */
	inline const std::string &GetDescription() const{ return pDescription; }
	
	/** Name of the source file where the exception occurred. */
	inline const std::string &GetFile() const{ return pFile; }
	
	/** Line number in file where the exception occurred. */
	inline int GetLine() const{ return pLine; }
	
	/** Result. */
	inline XrResult GetResult() const{ return pResult; }
	
	/** Backtrace. */
	inline const std::vector<std::string> &GetBacktrace() const{ return pBacktrace; }
	/*@}*/
	
	
	
	/** \name Tests */
	/*@{*/
	/** Test if the exception has the given name. */
	bool IsNamed( const std::string &name ) const;
	/*@}*/
	
	
	
	/** \name Display Functions */
	/*@{*/
	/**
	 * Subclass specific additional information.
	 * 
	 * Default implementation returns an empty list.
	 */
	virtual std::vector<std::string> AdditionalInformation() const;
	
	/**
	 * Format output into a list of strings.
	 * 
	 * Clears the list and adds the following entries:
	 * - "Exception:   {pName}"
	 * - "Description: {pDescription}"
	 * - "Source File: {pFile}"
	 * - "Source Line: {pLine}"
	 * - "BacktraceX: {pBacktrace[X]}"
	 * - AdditionalInformation
	 */
	std::vector<std::string> FormatOutput() const;
	
	/**
	 * Display a formated output of the stored information.
	 * 
	 * Print each line obtained by FormatOutput to stdout.
	 */
	void PrintError( std::ostream &stream ) const;
	/*@}*/
	
	
	
private:
	/** Build backtrace. */
	void pBuildBacktrace();
};

#endif

