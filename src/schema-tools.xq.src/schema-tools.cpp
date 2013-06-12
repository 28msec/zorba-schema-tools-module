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

#include <cstdlib>
#include <iostream>
#include <list>
#include <sstream>

#include <zorba/diagnostic_list.h>
#include <zorba/empty_sequence.h>
#include <zorba/external_module.h>
#include <zorba/function.h>
#include <zorba/item_factory.h>
#include <zorba/serializer.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/user_exception.h>
#include <zorba/util/base64_util.h>
#include <zorba/vector_item_sequence.h>
#include <zorba/zorba.h>

#include "JavaVMSingleton.h"

#define SCHEMATOOLS_MODULE_NAMESPACE "http://www.zorba-xquery.com/modules/schema-tools"
#define SCHEMATOOLS_OPTIONS_NAMESPACE "http://www.zorba-xquery.com/modules/schema-tools/schema-tools-options"

class JavaException {
};

#define CHECK_EXCEPTION(env)  if ((lException = env->ExceptionOccurred())) throw JavaException()

namespace zorba
{
namespace schematools
{

class SchemaToolsModule;
class Inst2xsdFunction;
class Xsd2instFunction;
class STOptions;


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
    {}

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
    {}

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


class STOptions
{
public:
  typedef enum
  {
    RUSSIAN_DOLL_DESIGN = 1,
    SALAMI_SLICE_DESIGN = 2,
    VENETIAN_BLIND_DESIGN = 3,
  } design_t;

  typedef enum
  {
    SMART_TYPES = 1,
    ALWAYS_STRING_TYPES = 2,
  } simple_content_types_t;


private:
  int theDesign;
  int theSimpleContentType;
  // useEnumeration NEVER = 1
  int theUseEnumeration;
  bool theVerbose;

  bool theNetworkDownloads;
  bool theNoPVR;
  bool theNoUPA;

public:
  STOptions() : theDesign(STOptions::VENETIAN_BLIND_DESIGN),
    theSimpleContentType(STOptions::SMART_TYPES),
    theUseEnumeration(10), theVerbose(false),
    theNetworkDownloads(false), theNoPVR(false), theNoUPA(false)
  {}

  void parseI(Item optionsNode, ItemFactory *itemFactory);
  void parseX(Item optionsNode, ItemFactory *itemFactory);

  int getDesign()
  {
    return theDesign;
  }

  int getSimpleContentType()
  {
    return theSimpleContentType;
  }

  int getUseEnumeration()
  {
    return theUseEnumeration;
  }

  bool isVerbose()
  {
    return theVerbose;
  }

  bool isNetworkDownloads()
  {
    return theNetworkDownloads;
  }

  bool isNoPVR()
  {
    return theNoPVR;
  }

  bool isNoUPA()
  {
    return theNoUPA;
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
    env = zorba::jvm::JavaVMSingleton::getInstance(aStaticContext)->getEnv();

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

    // read input parm 1: $options
    Item optionsItem;
    Iterator_t arg1Iter = args[1]->getIterator();
    arg1Iter->open();
    bool hasOptions = arg1Iter->next(optionsItem);
    arg1Iter->close();

    STOptions options;
    if (hasOptions)
      options.parseI(optionsItem, theFactory);


    // make options object
    jclass optClass = env->FindClass("org/apache/xmlbeans/impl/inst2xsd/Inst2XsdOptions");
    CHECK_EXCEPTION(env);
    jmethodID optConstrId = env->GetMethodID(optClass, "<init>", "()V" );
    CHECK_EXCEPTION(env);
    jobject optObj = env->NewObject(optClass, optConstrId);
    CHECK_EXCEPTION(env);
    jmethodID optSetDegignId = env->GetMethodID(optClass, "setDesign", "(I)V" );
    CHECK_EXCEPTION(env);
    env->CallVoidMethod(optObj, optSetDegignId, options.getDesign());
    CHECK_EXCEPTION(env);
    jmethodID optSetEnumId = env->GetMethodID(optClass, "setUseEnumerations", "(I)V" );
    CHECK_EXCEPTION(env);
    env->CallVoidMethod(optObj, optSetEnumId, options.getUseEnumeration());
    CHECK_EXCEPTION(env);
    jmethodID optSetSCId = env->GetMethodID(optClass, "setSimpleContentTypes", "(I)V" );
    CHECK_EXCEPTION(env);
    env->CallVoidMethod(optObj, optSetSCId, options.getSimpleContentType());
    CHECK_EXCEPTION(env);
    jmethodID optSetVerboseId = env->GetMethodID(optClass, "setVerbose", "(Z)V" );
    CHECK_EXCEPTION(env);
    env->CallVoidMethod(optObj, optSetVerboseId, options.isVerbose());
    CHECK_EXCEPTION(env);

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
    myClass = env->FindClass("org/zorbaxquery/modules/schemaTools/Inst2XsdHelper");
    CHECK_EXCEPTION(env);
    myMethod = env->GetStaticMethodID(myClass, "inst2xsd",
        "([Ljava/lang/String;Lorg/apache/xmlbeans/impl/inst2xsd/Inst2XsdOptions;)[Ljava/lang/String;");
    CHECK_EXCEPTION(env);
    jobjectArray resStrArray = (jobjectArray) env->CallStaticObjectMethod(myClass,
        myMethod, jXmlStrArray, optObj);
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
  catch (zorba::jvm::VMOpenException&)
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
  Iterator_t lIter;

  jthrowable lException = 0;
  static JNIEnv* env;

  try
  {
    env = zorba::jvm::JavaVMSingleton::getInstance(aStaticContext)->getEnv();

    // Local variables
    std::ostringstream os;
    Zorba_SerializerOptions_t lOptions;
    lOptions.omit_xml_declaration = ZORBA_OMIT_XML_DECLARATION_YES;
    Serializer_t lSerializer = Serializer::createSerializer(lOptions);

    const char* xml;
    std::string xmlString;

    // read input param 0: schemas
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

    // Get and create param 1: rootName string in jStrParam2
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

    // read input param 2: $options
    Item optionsItem;
    STOptions options;
    lIter = args[2]->getIterator();
    lIter->open();
    bool isOpen = lIter->isOpen();
    if ( isOpen )
    {
      bool hasOptions = lIter->next(optionsItem);
      lIter->close();

      if (hasOptions)
        options.parseX(optionsItem, theFactory);
    }

    // make options object
    jclass optClass = env->FindClass("org/zorbaxquery/modules/schemaTools/Xsd2InstHelper$Xsd2InstOptions");
    CHECK_EXCEPTION(env);
    jmethodID optConstrId = env->GetMethodID(optClass, "<init>", "()V" );
    CHECK_EXCEPTION(env);
    jobject optObj = env->NewObject(optClass, optConstrId);
    CHECK_EXCEPTION(env);
    jmethodID optSetNetId = env->GetMethodID(optClass, "setNetworkDownloads", "(Z)V" );
    CHECK_EXCEPTION(env);
    env->CallVoidMethod(optObj, optSetNetId, options.isNetworkDownloads());
    CHECK_EXCEPTION(env);
    jmethodID optSetNoPVRId = env->GetMethodID(optClass, "setNopvr", "(Z)V" );
    CHECK_EXCEPTION(env);
    env->CallVoidMethod(optObj, optSetNoPVRId, options.isNoPVR());
    CHECK_EXCEPTION(env);
    jmethodID optSetNoUPAId = env->GetMethodID(optClass, "setNoupa", "(Z)V" );
    CHECK_EXCEPTION(env);
    env->CallVoidMethod(optObj, optSetNoUPAId, options.isNoUPA());
    CHECK_EXCEPTION(env);

    // Create a Inst2XsdHelper class
    jclass myClass = env->FindClass("org/zorbaxquery/modules/schemaTools/Xsd2InstHelper");
    CHECK_EXCEPTION(env);
    jmethodID myMethod = env->GetStaticMethodID(myClass, "xsd2inst",
        "([Ljava/lang/String;Ljava/lang/String;Lorg/zorbaxquery/modules/schemaTools/Xsd2InstHelper$Xsd2InstOptions;)Ljava/lang/String;");
    CHECK_EXCEPTION(env);
    jobject resStr = (jobjectArray) env->CallStaticObjectMethod(myClass, myMethod, jXmlStrArray, jStrParam2, optObj);
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
  catch (zorba::jvm::VMOpenException&)
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



bool compareItemQName(Item item, const char *localname, const char *ns)
{
  int node_kind = item.getNodeKind();
  if(node_kind != store::StoreConsts::elementNode)
    return false;
  Item node_name;
  item.getNodeName(node_name);
  String  item_namespace = node_name.getNamespace();
  if(ns && ns[0] && item_namespace != ns )
  {
    return false;
  }
  String  item_name = node_name.getLocalName();
  if(item_name != localname)
  {
    return false;
  }
  return true;
}


bool getChild(zorba::Iterator_t children, const char *localname, const char *ns,
                           zorba::Item &child_item);
bool getChild(zorba::Item &lItem, const char *localname, const char *ns,
                           zorba::Item &child_item)
{
  Iterator_t    children;
  children = lItem.getChildren();
  children->open();
  bool retval = getChild(children, localname, ns, child_item);
  children->close();
  return retval;
}


bool getChild(zorba::Iterator_t children, const char *localname, const char *ns,
              zorba::Item &child_item)
{
  while(children->next(child_item))
  {
    if(child_item.getNodeKind() != store::StoreConsts::elementNode)
      continue;
    Item    child_name;
    child_item.getNodeName(child_name);
    String  item_namespace = child_name.getNamespace();
    if(item_namespace != ns)
    {
      continue;//next child
    }
    String  item_name = child_name.getLocalName();
    if(item_name != localname)
    {
      continue;//next child
    }
    return true;
  }
  return false;
}

void STOptions::parseI(Item optionsNode, ItemFactory *itemFactory)
{
  if(optionsNode.isNull())
    return;

  if(!compareItemQName(optionsNode, "inst2xsd-options", SCHEMATOOLS_OPTIONS_NAMESPACE) )
  {
    std::stringstream lErrorMessage;
    Item options_qname;
    optionsNode.getNodeName(options_qname);
    lErrorMessage << "Options field must be of element 'inst2xsd-options' " <<
                     "instead of '" <<
                     options_qname.getStringValue() << "'. ";
    Item errWrongParamQName;
    String errName("WrongParam");
    errWrongParamQName = itemFactory->createQName(SCHEMATOOLS_OPTIONS_NAMESPACE, errName);
    String errDescription(lErrorMessage.str());
    throw USER_EXCEPTION(errWrongParamQName, errDescription);
  }

  zorba::Item child_item;

  if(getChild(optionsNode, "design", SCHEMATOOLS_OPTIONS_NAMESPACE, child_item))
  {
    String design_text = child_item.getStringValue();
    if ( design_text == "rdd" )
      theDesign = RUSSIAN_DOLL_DESIGN;
    else if ( design_text == "ssd" )
      theDesign = SALAMI_SLICE_DESIGN;
    else if ( design_text == "vbd" )
      theDesign = VENETIAN_BLIND_DESIGN;
  }

  if(getChild(optionsNode, "simple-content-types", SCHEMATOOLS_OPTIONS_NAMESPACE, child_item))
  {
    String sct_text = child_item.getStringValue();
    if ( sct_text == "always-string" )
      theSimpleContentType = ALWAYS_STRING_TYPES;
    else if ( sct_text == "smart" )
      theSimpleContentType = SMART_TYPES;
  }

  if(getChild(optionsNode, "verbose", SCHEMATOOLS_OPTIONS_NAMESPACE, child_item))
  {
    String sct_text = child_item.getStringValue();
    if ( sct_text == "true" || sct_text == "1" )
      theVerbose = true;
    else
      theVerbose = false;
  }

  if(getChild(optionsNode, "use-enumeration", SCHEMATOOLS_OPTIONS_NAMESPACE, child_item))
  {
    String sct_text = child_item.getStringValue();
    int ival = atoi(sct_text.c_str());
    if (ival>1)
      theUseEnumeration = ival;
    else
      theUseEnumeration = 1;
  }
}

void STOptions::parseX(Item optionsNode, ItemFactory *itemFactory)
{
  if(optionsNode.isNull())
    return;

  if(!compareItemQName(optionsNode, "xsd2inst-options", SCHEMATOOLS_OPTIONS_NAMESPACE))
  {
    std::stringstream lErrorMessage;
    Item options_qname;
    optionsNode.getNodeName(options_qname);
    lErrorMessage << "Options field must be of element 'xsd2inst-options' instead of '" <<
                     options_qname.getStringValue() << "'";
    Item errWrongParamQName;
    String errName("WrongParam");
    errWrongParamQName = itemFactory->createQName(SCHEMATOOLS_OPTIONS_NAMESPACE, errName);
    String errDescription(lErrorMessage.str());
    throw USER_EXCEPTION(errWrongParamQName, errDescription);
  }

  zorba::Item child_item;

  if(getChild(optionsNode, "network-downloads", SCHEMATOOLS_OPTIONS_NAMESPACE, child_item))
  {
    //theNetworkDownloads = child_item.getBooleanValue();
    String sct_text = child_item.getStringValue();
    if ( sct_text == "true" || sct_text == "1" )
      theNetworkDownloads = true;
    else
      theNetworkDownloads = false;
  }

  if(getChild(optionsNode, "no-pvr", SCHEMATOOLS_OPTIONS_NAMESPACE, child_item))
  {
    //theNoPVR = child_item.getBooleanValue();
    String sct_text = child_item.getStringValue();
    if ( sct_text == "true" || sct_text == "1" )
      theNetworkDownloads = true;
    else
      theNetworkDownloads = false;
  }

  if(getChild(optionsNode, "no-pvr", SCHEMATOOLS_OPTIONS_NAMESPACE, child_item))
  {
    //theNoUPA = child_item.getBooleanValue();
    String sct_text = child_item.getStringValue();
    if ( sct_text == "true" || sct_text == "1" )
      theNetworkDownloads = true;
    else
      theNetworkDownloads = false;
  }
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
/* vim:set et sw=2 ts=2: */
