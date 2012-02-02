/*
 * Copyright 2006-2008 The FLWOR Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <list>

#include <zorba/base64.h>
#include <zorba/empty_sequence.h>
#include <zorba/diagnostic_list.h>
#include <zorba/function.h>
#include <zorba/external_module.h>
#include <zorba/user_exception.h>
#include <zorba/file.h>
#include <zorba/item_factory.h>
#include <zorba/serializer.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/vector_item_sequence.h>
#include <zorba/zorba.h>

#include "JavaVMSingelton.h"

#define SCHEMATOOLS_MODULE_NAMESPACE "http://www.zorba-xquery.com/modules/schema-tools"

class JavaException {
};

#define CHECK_EXCEPTION(env)  if ((lException = env->ExceptionOccurred())) throw JavaException()

namespace zorba
{
namespace schematools
{
 
class Inst2xsdFunction : public ContextualExternalFunction
{
  private:
    const ExternalModule* theModule;
    ItemFactory* theFactory;
    XmlDataManager* theDataManager;

  public:
    Inst2xsdFunction(const ExternalModule* aModule) :
      theModule(aModule),
      theFactory(Zorba::getInstance(0)->getItemFactory()),
      theDataManager(Zorba::getInstance(0)->getXmlDataManager())
    {}

		~Inst2xsdFunction()
		{
      JavaVMSingelton::destroyInstance();
    }

  public:
		virtual String getURI() const
		{ return theModule->getURI(); }

		virtual String getLocalName() const
    { return "inst2xsd-internal"; }

    virtual ItemSequence_t 
      evaluate(const ExternalFunction::Arguments_t& args,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
};

class Xsd2instFunction : public ContextualExternalFunction
{
	private:
		const ExternalModule* theModule;
		ItemFactory* theFactory;
		XmlDataManager* theDataManager;

	public:
		Xsd2instFunction(const ExternalModule* aModule) :
      theModule(aModule),
      theFactory(Zorba::getInstance(0)->getItemFactory()),
			theDataManager(Zorba::getInstance(0)->getXmlDataManager())
		{}

		~Xsd2instFunction()
		{
			JavaVMSingelton::destroyInstance();
		}

	public:
		virtual String getURI() const
		{ return theModule->getURI(); }

		virtual String getLocalName() const
    { return "xsd2inst-internal"; }

		virtual ItemSequence_t
      evaluate(const ExternalFunction::Arguments_t& args,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
};


class SchemaToolsModule : public ExternalModule {
  private:
		ExternalFunction* inst2xsd;
		ExternalFunction* xsd2inst;

	public:
    SchemaToolsModule() :
			inst2xsd(new Inst2xsdFunction(this)),
      xsd2inst(new Xsd2instFunction(this))
		{}

		~SchemaToolsModule()
		{
			delete inst2xsd;
			delete xsd2inst;
    }

		virtual String getURI() const
		{ return SCHEMATOOLS_MODULE_NAMESPACE; }

    virtual ExternalFunction* getExternalFunction(const String& localName);

		virtual void destroy()
		{
      delete this;
    }
};

ExternalFunction* SchemaToolsModule::getExternalFunction(const String& localName)
{
  if (localName == "inst2xsd-internal")
	{
		return inst2xsd;
	}
  else if (localName == "xsd2inst-internal")
	{
		return xsd2inst;
	}

	return 0;
}


ItemSequence_t
Inst2xsdFunction::evaluate(const ExternalFunction::Arguments_t& args,
                           const zorba::StaticContext* aStaticContext,
                           const zorba::DynamicContext* aDynamincContext) const
{
	jthrowable lException = 0;
	static JNIEnv* env;

	try
  {
    String cp = aStaticContext->getFullJVMClassPath();
    //std::cout << "Inst2xsdFunction::evaluate: '" << cp << "'" << std::endl; std::cout.flush();
    env = JavaVMSingelton::getInstance(cp.c_str())->getEnv();

		// Local variables
		Zorba_SerializerOptions_t lOptions;
		lOptions.omit_xml_declaration = ZORBA_OMIT_XML_DECLARATION_YES;
		Serializer_t lSerializer = Serializer::createSerializer(lOptions);
		jclass myClass;
		jmethodID myMethod;

		// read input param 0
    Iterator_t lIter = args[0]->getIterator();
		lIter->open();

		Item item;
		std::vector<jstring> xmlUtfVec;

		while( lIter->next(item) )
		{
			// Searialize Item
			std::ostringstream os;
			SingletonItemSequence lSequence(item);
			lSerializer->serialize(&lSequence, os);
			std::string xmlString = os.str();
			const char* xml = xmlString.c_str();
			//std::cout << "  xmlString: '" << xml << "'" << std::endl; std::cout.flush();
			xmlUtfVec.push_back( env->NewStringUTF(xml) );
			CHECK_EXCEPTION(env);
		}

		lIter->close();

		// Create String[]
		jclass strCls = env->FindClass("Ljava/lang/String;");
		CHECK_EXCEPTION(env);
		jobjectArray jXmlStrArray = env->NewObjectArray(xmlUtfVec.size(), strCls, NULL);
		//std::cout << "  NewObjectArray: '" << jXmlStrArray << "'" << std::endl; std::cout.flush();
		CHECK_EXCEPTION(env);

		for ( jsize i = 0; i<(jsize)xmlUtfVec.size(); i++)
		{
			env->SetObjectArrayElement(jXmlStrArray, i, xmlUtfVec[i]);
			CHECK_EXCEPTION(env);
			env->DeleteLocalRef((jstring)xmlUtfVec[i]);
			CHECK_EXCEPTION(env);
		}

		// Create a Inst2XsdHelper class
		myClass = env->FindClass("Inst2XsdHelper");
		CHECK_EXCEPTION(env);
		myMethod = env->GetStaticMethodID(myClass, "inst2xsd", "([Ljava/lang/String;)[Ljava/lang/String;");
		CHECK_EXCEPTION(env);
		jobjectArray resStrArray = (jobjectArray) env->CallStaticObjectMethod(myClass, myMethod, jXmlStrArray);
		CHECK_EXCEPTION(env);
		//std::cout << "  CallStaticObjectMethod: '" << jXmlStrArray << "'" << std::endl; std::cout.flush();

		jsize resStrArraySize = env->GetArrayLength(resStrArray);
		CHECK_EXCEPTION(env);
		//std::cout << "  GetArrayLength: '" << resStrArraySize << "'" << std::endl; std::cout.flush();
		std::vector<Item> vec;


		for( jsize i=0; i<resStrArraySize; i++)
		{
			jobject resStr = env->GetObjectArrayElement(resStrArray, i);

			const char *str;
			str = env->GetStringUTFChars( (jstring)resStr, NULL);
			if ( str == NULL ) return NULL;

			std::string lBinaryString(str);

			env->ReleaseStringUTFChars( (jstring)resStr, str);
			//std::cout << "  lBinaryString '" << lBinaryString << "'" << std::endl; std::cout.flush();

			std::stringstream lStream(lBinaryString);
			Item lRes = theDataManager->parseXML(lStream);

			vec.push_back(lRes);
		}

		return ItemSequence_t(new VectorItemSequence(vec));
	}
	catch (VMOpenException&)
	{
		Item lQName = theFactory->createQName(SCHEMATOOLS_MODULE_NAMESPACE,
				"VM001");
		throw USER_EXCEPTION(lQName, "Could not start the Java VM (is the classpath set?)");
	}
	catch (JavaException&)
	{
		jclass stringWriterClass = env->FindClass("java/io/StringWriter");
		jclass printWriterClass = env->FindClass("java/io/PrintWriter");
		jclass throwableClass = env->FindClass("java/lang/Throwable");
		jobject stringWriter = env->NewObject(
				stringWriterClass,
				env->GetMethodID(stringWriterClass, "<init>", "()V"));

		jobject printWriter = env->NewObject(
				printWriterClass,
				env->GetMethodID(printWriterClass, "<init>", "(Ljava/io/Writer;)V"),
				stringWriter);

		env->CallObjectMethod(lException,
				env->GetMethodID(throwableClass, "printStackTrace",
						"(Ljava/io/PrintWriter;)V"),
				printWriter);

		//env->CallObjectMethod(printWriter, env->GetMethodID(printWriterClass, "flush", "()V"));
		jmethodID toStringMethod =
			env->GetMethodID(stringWriterClass, "toString", "()Ljava/lang/String;");
		jobject errorMessageObj = env->CallObjectMethod(
				stringWriter, toStringMethod);
		jstring errorMessage = (jstring) errorMessageObj;
		const char *errMsg = env->GetStringUTFChars(errorMessage, 0);
		std::stringstream s;
		s << "A Java Exception was thrown:" << std::endl << errMsg;
		env->ReleaseStringUTFChars(errorMessage, errMsg);
		std::string err("");
		err += s.str();
		env->ExceptionClear();
		Item lQName = theFactory->createQName(SCHEMATOOLS_MODULE_NAMESPACE,
				"JAVA-EXCEPTION");
		throw USER_EXCEPTION(lQName, err);
	}

	return ItemSequence_t(new EmptySequence());
}



ItemSequence_t
Xsd2instFunction::evaluate(const ExternalFunction::Arguments_t& args,
                           const zorba::StaticContext* aStaticContext,
                           const zorba::DynamicContext* aDynamicContext) const
{
	Item classPathItem;
	// assemble classpath (list of path concatenated with ":" or ";")
	Iterator_t lIter = args[2]->getIterator();
	lIter->open();
	std::ostringstream lClassPath;
	while (lIter->next(classPathItem))
	{
		lClassPath << classPathItem.getStringValue() << File::getPathSeparator();
	}

	lIter->close();

	jthrowable lException = 0;
	static JNIEnv* env;

	try
	{
    String cp = aStaticContext->getFullJVMClassPath();
    //std::cout << "Xsd2instFunction::evaluate: '" << cp << "'" << std::endl; std::cout.flush();
    env = JavaVMSingelton::getInstance(cp.c_str())->getEnv();

		//jstring outFotmatString = env->NewStringUTF(outputFormat.getStringValue().c_str());

		// Local variables
		std::ostringstream os;
		Zorba_SerializerOptions_t lOptions;
		lOptions.omit_xml_declaration = ZORBA_OMIT_XML_DECLARATION_YES;
		Serializer_t lSerializer = Serializer::createSerializer(lOptions);

		const char* xml;
		std::string xmlString;

		// read input param 0
		lIter = args[0]->getIterator();
		lIter->open();

		Item item;
		std::vector<jstring> xmlUtfVec;

		while( lIter->next(item) )
		{
			// Searialize Item
			std::ostringstream os;
			SingletonItemSequence lSequence(item);
			lSerializer->serialize(&lSequence, os);
			std::string xmlString = os.str();
			const char* xml = xmlString.c_str();
			//std::cout << "  xmlString: '" << xml << "'" << std::endl; std::cout.flush();
			xmlUtfVec.push_back( env->NewStringUTF(xml) );
			CHECK_EXCEPTION(env);
		}

		lIter->close();


		// Create String[]
		jclass strCls = env->FindClass("Ljava/lang/String;");
		CHECK_EXCEPTION(env);
		jobjectArray jXmlStrArray = env->NewObjectArray(xmlUtfVec.size(), strCls, NULL);
		//std::cout << "  NewObjectArray: '" << jXmlStrArray << "'" << std::endl; std::cout.flush();
		CHECK_EXCEPTION(env);

		for ( jsize i = 0; i<(jsize)xmlUtfVec.size(); i++)
		{
			env->SetObjectArrayElement(jXmlStrArray, i, xmlUtfVec[i]);
			CHECK_EXCEPTION(env);
			env->DeleteLocalRef((jstring)xmlUtfVec[i]);
			CHECK_EXCEPTION(env);
		}

		// Get and create 2nd param rootName string in jStrParam2
		lIter = args[1]->getIterator();
		lIter->open();
		lIter->next(item);
		lIter->close();
		//   Searialize Item
		SingletonItemSequence lSequence(item);
		lSerializer->serialize(&lSequence, os);
		xmlString = os.str();
		xml = xmlString.c_str();
		jstring jStrParam2 = env->NewStringUTF(xml);

		// Create a Inst2XsdHelper class
		jclass myClass = env->FindClass("Xsd2InstHelper");
		CHECK_EXCEPTION(env);
		jmethodID myMethod = env->GetStaticMethodID(myClass, "xsd2inst", "([Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
		CHECK_EXCEPTION(env);
		jobject resStr = (jobjectArray) env->CallStaticObjectMethod(myClass, myMethod, jXmlStrArray, jStrParam2);
		CHECK_EXCEPTION(env);
		//std::cout << "  CallStaticObjectMethod: '" << resStr << "'" << std::endl; std::cout.flush();

		const char *str;
		str = env->GetStringUTFChars( (jstring)resStr, NULL);
		CHECK_EXCEPTION(env);

		std::string lBinaryString(str);

		env->ReleaseStringUTFChars( (jstring)resStr, str);
		//std::cout << "  lBinaryString '" << lBinaryString << "'" << std::endl; std::cout.flush();

		std::stringstream lStream(lBinaryString);
		Item lRes = theDataManager->parseXML(lStream);

		return ItemSequence_t(new SingletonItemSequence(lRes));
	}
	catch (VMOpenException&)
	{
		Item lQName = theFactory->createQName(SCHEMATOOLS_MODULE_NAMESPACE,
				"VM001");
		throw USER_EXCEPTION(lQName, "Could not start the Java VM (is the classpath set?)");
	}
	catch (JavaException&)
	{
		jclass stringWriterClass = env->FindClass("java/io/StringWriter");
		jclass printWriterClass = env->FindClass("java/io/PrintWriter");
		jclass throwableClass = env->FindClass("java/lang/Throwable");
		jobject stringWriter = env->NewObject(
				stringWriterClass,
				env->GetMethodID(stringWriterClass, "<init>", "()V"));

		jobject printWriter = env->NewObject(
				printWriterClass,
				env->GetMethodID(printWriterClass, "<init>", "(Ljava/io/Writer;)V"),
				stringWriter);

		env->CallObjectMethod(lException,
				env->GetMethodID(throwableClass, "printStackTrace",
						"(Ljava/io/PrintWriter;)V"),
				printWriter);

		//env->CallObjectMethod(printWriter, env->GetMethodID(printWriterClass, "flush", "()V"));
		jmethodID toStringMethod =
			env->GetMethodID(stringWriterClass, "toString", "()Ljava/lang/String;");
		jobject errorMessageObj = env->CallObjectMethod(
				stringWriter, toStringMethod);
		jstring errorMessage = (jstring) errorMessageObj;
		const char *errMsg = env->GetStringUTFChars(errorMessage, 0);
		std::stringstream s;
		s << "A Java Exception was thrown:" << std::endl << errMsg;
		env->ReleaseStringUTFChars(errorMessage, errMsg);
		std::string err("");
		err += s.str();
		env->ExceptionClear();
		Item lQName = theFactory->createQName(SCHEMATOOLS_MODULE_NAMESPACE,
				"JAVA-EXCEPTION");
		throw USER_EXCEPTION(lQName, err);
	}

	return ItemSequence_t(new EmptySequence());
}

}}; // namespace zorba, schematools

#ifdef WIN32
#  define DLL_EXPORT __declspec(dllexport)
#else
#  define DLL_EXPORT __attribute__ ((visibility("default")))
#endif

extern "C" DLL_EXPORT zorba::ExternalModule* createModule()
{
  return new zorba::schematools::SchemaToolsModule();
}
