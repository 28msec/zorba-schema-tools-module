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
			theModule(aModule), theFactory(Zorba::getInstance(0)->getItemFactory()),
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
		{ return "inst2xsd"; }

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
			theModule(aModule), theFactory(Zorba::getInstance(0)->getItemFactory()),
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
		{ return "xsd2inst"; }

		virtual ItemSequence_t
      evaluate(const ExternalFunction::Arguments_t& args,
               const zorba::StaticContext*,
               const zorba::DynamicContext*) const;
};

class FindXMLBeansFunction : public NonContextualExternalFunction
{
  private:
    const ExternalModule* theModule;
    ItemFactory* theFactory;

	private:
    void throwError(std::string aName) const;

	public:
		FindXMLBeansFunction(const ExternalModule* aModule) :
			theModule(aModule), theFactory(Zorba::getInstance(0)->getItemFactory())
		{}

		virtual String getURI() const
		{ return theModule->getURI(); }

		virtual String getLocalName() const
		{ return "find-xmlbeans"; }

    virtual ItemSequence_t 
			evaluate(const ExternalFunction::Arguments_t& args) const;
};

class SchemaToolsModule : public ExternalModule {
  private:
		ExternalFunction* inst2xsd;
		ExternalFunction* xsd2inst;
		ExternalFunction* findXMLBeans;

	public:
    SchemaToolsModule() :
			inst2xsd(new Inst2xsdFunction(this)),
			xsd2inst(new Xsd2instFunction(this)),
			findXMLBeans(new FindXMLBeansFunction(this))
		{}

		~SchemaToolsModule()
		{
			delete inst2xsd;
			delete xsd2inst;
			delete findXMLBeans;
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
	if (localName == "inst2xsd")
	{
		return inst2xsd;
	}
	else if (localName == "xsd2inst")
	{
		return xsd2inst;
	}
	else if (localName == "find-xmlbeans")
	{
		return findXMLBeans;
  }

	return 0;
}

void FindXMLBeansFunction::throwError(std::string aName) const
{
  Item lQName = theFactory->createQName(SCHEMATOOLS_MODULE_NAMESPACE,
			"XMLBEANS-JAR-NOT-FOUND");

  throw USER_EXCEPTION(lQName, aName);
}

ItemSequence_t FindXMLBeansFunction::evaluate(
  const ExternalFunction::Arguments_t& args) const
{
  std::string lDirectorySeparator(File::getDirectorySeparator());
	std::string lXMLBeansHome;
  {
		char* lXMLBeansHomeEnv = getenv("XMLBEANS_HOME");
		if (lXMLBeansHomeEnv != 0)
		{
			lXMLBeansHome = lXMLBeansHomeEnv;
    }
#ifdef APPLE
		else
		{
			// If Apache XMLBeans is installed with Mac Ports, XMLBeans
			// is typicaly installed in /opt/local/share/java/xmlbeans,
      // so we check here, if the installation directory can
      // be found in this directory.
			std::string lXMLBeansPath("/opt/local/share/java/xmlbeans/");
			File_t lRootDir = File::createFile(lXMLBeansPath);
			if (lRootDir->exists() && lRootDir->isDirectory())
			{
        DirectoryIterator_t lFiles = lRootDir->files();
        std::string lFileName;
				// The XMLBeans directory is in a subdirectory with the version
        // number - so we check all subdirectories to get the final
        // path.
				while (lFiles->next(lFileName))
				{
					File_t lFile = File::createFile(lXMLBeansPath + lFileName);
					if (lFile->isDirectory())
					{
            std::stringstream lStr(lFileName);
            double lDirDouble = 0.0;
						if (lStr >> lDirDouble)
						{
							if (lDirDouble != 0.0)
							{
								lXMLBeansHome = lXMLBeansPath + lFileName;
                break;
              }
            }
          }
        }
      }
    }
#endif
  }
	std::string lXMLBeansLibDir;
  {
		char* lEnv = getenv("XMLBEANS_LIB_DIR");
		if (lEnv != 0)
		{
			lXMLBeansLibDir = lEnv;
    }
#ifdef LINUX
    // on a Ubuntu installation, all required
    // jar files should be in /usr/share/java
		// if Apache XMLBeans is installed.
		else
		{
			lXMLBeansLibDir = "/usr/share/java";
    }
#endif
  }
	// If neither a path to the XMLBeans install dir, nor a path
  // to the jar files was found so far, we throw an exception.
	if (lXMLBeansHome == "" && lXMLBeansLibDir == "")
	{
		throwError("None of the environment variables XMLBEANS_HOME and XMLBEANS_LIB_DIR have been set.");
  }

	std::string lXMLBeansJarFile;

	{
		// Here we look for the xmlbeans.jar file, which should be either in $XMLBEANS_HOME/build or
    // in the directory, where all jar files are.
		lXMLBeansJarFile = lXMLBeansHome + lDirectorySeparator + "build" +
				lDirectorySeparator + "xmlbeans.jar";
		std::string lXMLBeansJarFile1 = lXMLBeansJarFile;
		File_t lJarFile = File::createFile(lXMLBeansJarFile);
		if (!lJarFile->exists())
		{
			lXMLBeansJarFile = lXMLBeansLibDir + lDirectorySeparator + "xmlbeans.jar";
			lJarFile = File::createFile(lXMLBeansJarFile);
			if (!lJarFile->exists())
			{
				std::string errmsg = "Could not find xmlbeans.jar. If you are using ";
				errmsg += "Ubuntu or Mac OS X, please make sure, ";
				errmsg += "that you have installed it, else make sure, that you have ";
				errmsg += "set the envroinment variable ";
				errmsg += "XMLBEANS_HOME or XMLBEANS_LIB_DIR correctly. Tried '";
				errmsg +=  lXMLBeansJarFile1;
        errmsg += "' and '";
				errmsg += lXMLBeansJarFile;
        errmsg += "'.";
        throwError(errmsg);
      }
    }
  }

  std::vector<Item> lClassPath;
	lClassPath.push_back(theFactory->createString(lXMLBeansJarFile));

	{
		std::string lJarDir = lXMLBeansLibDir;

		if (lXMLBeansHome != "")
			lJarDir = lXMLBeansHome + lDirectorySeparator + "lib";

		// This is a list of all jar files, Apache XMLBeans depends on.
    std::list<std::string> lDeps;
		/*lDeps.push_back("avalon-framework");
    lDeps.push_back("batik-all");
    lDeps.push_back("commons-io");
    lDeps.push_back("commons-logging");
    lDeps.push_back("serializer");
    lDeps.push_back("xalan");
		lDeps.push_back("xmlgraphics-commons");*/

    File_t lJarDirF = File::createFile(lJarDir);
    DirectoryIterator_t lFiles = lJarDirF->files();
		std::string lFile;
		size_t count = 0;

    // We check for all files, if it is a potential dependency and add it to
    // the result
		while (lFiles->next(lFile))
		{
      // If the file is not a jar file, we don't do anything
      if (lFile.substr(lFile.size() - 4, std::string::npos) != ".jar")
        continue;

			for (std::list<std::string>::iterator i = lDeps.begin(); i != lDeps.end(); ++i)
			{
        std::string lSub = lFile.substr(0, i->size());
				if (lSub == *i)
				{
          std::string lFull = lJarDir + lDirectorySeparator + lFile;
          File_t f = File::createFile(lFull);

					if (f->exists() && !f->isDirectory())
					{
            lClassPath.push_back(theFactory->createString(lFull));
            // We count all jar files we add to the dependencies.
            ++count;
            break;
          }
        }
      }
    }
    // Last, we check if all dependencies are found
		if (count < lDeps.size())
		{
      std::string errmsg = "Could not find ";
      errmsg += lDeps.front();
      throwError(errmsg);
    }
  }

	return ItemSequence_t(new VectorItemSequence(lClassPath));
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
