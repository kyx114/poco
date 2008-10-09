//
// Form.cpp
//
// $Id: //poco/Main/WebWidgets/src/Form.cpp#6 $
//
// Library: WebWidgets
// Package: Views
// Module:  Form
//
// Copyright (c) 2008, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-execuForm object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/WebWidgets/Form.h"
#include "Poco/WebWidgets/RequestHandler.h"
#include "Poco/NumberParser.h"
#include "Poco/Net/HTMLForm.h"


namespace Poco {
namespace WebWidgets {


const std::string Form::FORM_ID("__form__");
const std::string Form::EV_RELOAD("reload");
const std::string Form::METHOD_GET("GET");
const std::string Form::METHOD_POST("POST");
const std::string Form::ENCODING_URL("application/x-www-form-urlencoded");
const std::string Form::ENCODING_MULTIPART("multipart/form-data");


Form::Form(const Poco::URI& uri):
	ContainerView(typeid(Form)),
	_method(METHOD_POST),
	_encoding(ENCODING_MULTIPART),
	_uri(uri),
	_namedChildren()
{
}

	
Form::Form(const std::string& name, const Poco::URI& uri):
	ContainerView(name, typeid(Form)),
	_method(METHOD_POST),
	_encoding(ENCODING_MULTIPART),
	_uri(uri),
	_namedChildren()
{
}


Form::Form(const std::string& name, const std::type_info& type, const Poco::URI& uri):
	ContainerView(name, type),
	_method(METHOD_POST),
	_encoding(ENCODING_MULTIPART),
	_uri(uri),
	_namedChildren()
{
}

	
Form::Form(const std::type_info& type, const Poco::URI& uri):
	ContainerView(type),
	_method(METHOD_POST),
	_encoding(ENCODING_MULTIPART),
	_uri(uri),
	_namedChildren()
{
}


Form::~Form()
{
}


void Form::setMethod(const std::string& method)
{
	_method = method;
}


void Form::setEncoding(const std::string& encoding)
{
	_encoding = encoding;
}


void Form::handleForm(const std::string& field, const std::string& value)
{
}


void Form::handleForm(const Poco::Net::HTMLForm& form)
{
	Renderable::ID formID = Poco::NumberParser::parse(form.get(Form::FORM_ID));
	poco_assert (formID == id());
		
	Poco::Net::NameValueCollection::ConstIterator it = form.begin();	
	RequestProcessorMap processors = _namedChildren; // copy so that a second submit works too!
	for (;it != form.end(); ++it)
	{
		const std::string& key = it->first;
		RequestProcessorMap::iterator itR = processors.find(key);
		if (itR != processors.end())
		{
			itR->second->handleForm(key, it->second);
			processors.erase(itR);
		}
	}
	//those that are not included are either deselected or empty
	RequestProcessorMap::iterator itR = processors.begin();
	std::string empty;
	for (; itR != processors.end(); ++itR)
	{
		itR->second->handleForm(itR->first, empty);
	}

}

	
void Form::handleAjaxRequest(const Poco::Net::NameValueCollection& args, Poco::Net::HTTPServerResponse& response)
{
	//a form only supports the refresh event
	const std::string& ev = args[RequestHandler::KEY_EVID];
	if (ev == EV_RELOAD)
	{
		Form* pThis = this;
		beforeReload.notify(this, pThis);
		/// send the JS presentation of the page
		response.setContentType("text/javascript");
		response.setChunkedTransferEncoding(true);
		serializeJSONImpl(response.send());
	}
	else
		response.send();
}


void Form::serializeJSONImpl(std::ostream& out)
{
	// FIXME: ExtJs specific
	out << "{";
		out << "success: true,";
		out << "data: { ";
			// serialize children
			RequestProcessorMap::iterator it = _namedChildren.begin();
			bool writeComma = false;
			for (; it != _namedChildren.end(); ++it)
			{
				if (writeComma)
					out << ",";
				writeComma = it->second->serializeJSON(out, it->first);
			}
		out << "}";
	out << "}";
}


void Form::registerFormProcessor(const std::string& fieldName, RequestProcessor* pProc)
{
	// per default we register everything that has a name as form processor
	std::pair<RequestProcessorMap::iterator, bool> res = _namedChildren.insert(std::make_pair(fieldName, pProc));
	if (!res.second)
		res.first->second = pProc;
}


RequestProcessor* Form::getFormProcessor(const std::string& fieldName)
{
	RequestProcessorMap::iterator it = _namedChildren.find(fieldName);
	if (it == _namedChildren.end())
		return 0;
	return it->second;
}


} } // namespace Poco::WebWidgets