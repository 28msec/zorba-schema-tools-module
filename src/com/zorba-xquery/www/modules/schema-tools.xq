xquery version "3.0";

(:
 : Copyright 2006-2009 The FLWOR Foundation.
 :
 : Licensed under the Apache License, Version 2.0 (the "License");
 : you may not use this file except in compliance with the License.
 : You may obtain a copy of the License at
 :
 : http://www.apache.org/licenses/LICENSE-2.0
 :
 : Unless required by applicable law or agreed to in writing, software
 : distributed under the License is distributed on an "AS IS" BASIS,
 : WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 : See the License for the specific language governing permissions and
 : limitations under the License.
:)
(:~
 : Schema related tools
 : <br /> 
 : This module uses Apache XMLBeans for inst2xsd and xsd2inst functions.
 : See <a href="http://xmlbeans.apache.org/">Apache XMLBeans documentation</a> for further information.
 : <br />
 : <br />
 : <b>Note for Windows users</b>: On Windows, this module won't work out of the box, since
 : this module uses Java. But the Java VM dll is not in the system path by default. To make
 : this module work, you need to add the directory where the jvm.dll is located to the
 : system path. This dll is located at JRE_DIR\bin\client. On a standard installation, this would
 : be something a path like "C:\Program Files\Java\jre6\bin\client".
 :
 : @author Cezar Andrei
 : @see http://xmlbeans.apache.org/
 : @library <a href="http://www.oracle.com/technetwork/java/javase/downloads/index.html">JDK - Java Development Kit</a>
 : @project data processing/metadata
 :)
module namespace schema-tools = "http://www.zorba-xquery.com/modules/schema-tools";


declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare namespace ver = "http://www.zorba-xquery.com/options/versioning";
declare option ver:module-version "0.1";






(:~
 : The inst2xsd function takes a set of sample instance elements as input and 
 : generates a schema element that reprezents one sample XML Schema that defines
 : the content of the given input.
 :
 : Please consult the 
 : <a href="http://xmlbeans.apache.org/">official documentation for further information</a>.
 :
 : @param $instances The XML instance elements that will define the schema.
 : @param $options The Inst2XSD options:
 :      * design: Choose the generated schema design
 :         - rdd: Russian Doll Design - local elements and local types
 :         - ssd: Salami Slice Design - global elements and local types
 :         - vbd (default): Venetian Blind Design - local elements and global complex types
 :      * simple-content-types: type of leaf nodes
 :         - smart (default): try to find out the right simple shema type
 :         - always-string: use xsd:string for all simple types
 :      * use-enumeration: - when there are multiple valid values in a list
 :         - 0 or less: never use enumeration
 :         - 1 or more (default 10): only if not more than the value - number option
 :
 :  <options xmlns='http://www.zorba-xquery.com/modules/schema-tools/schemma-tools-options'>
 :    <design>vbd</design>
 :    <simple-content-types>smart</simple-content-types>
 :    <use-enumeration>10</use-enumeration>
 :  </options>
 :
 : @return The generated output document, representing a sample XMLSchema.
 : @error schema-tools:VM001 If zorba was unable to start the JVM.
 : @error schema-tools:JAVA-EXCEPTION If Apache XMLBeans throws an exception.
 :)
declare function
schema-tools:inst2xsd ($instances as element()+, $options as element())
    as document-node()* external;


(:~
 : The xsd2inst function takes a set of XML Schema file names as input and the 
 : name of the root element and
 : generates an element that reprezents one sample XML instance of the given input schema files.
 :
 : Please consult the 
 : <a href="http://xmlbeans.apache.org/">official documentation for further information</a>.
 : Example:
 :
 :  let $inst := (<a><b/><c/></a>, <b/>, <c>ccc</c>)
 :  let $opt  := <xb:options>
 :                   <xb:design>rdd</xb:design>\
 :                   <xb:design>rdd</xb:design>\
 :                   <xb:design>rdd</xb:design>\
 :               </xb:options>
 :  return
 :      xb:inst2xsd($inst, $opt)
 :
 : @param $schema The XML Schema file names that define the schema type system.
 : @param $rootElementName The QName of the root element of the instance
 : @param $options Options:
 :       * network-downloads: boolean - if allowed to use network when resolving
 :                                      schema imports and includes
 :       * no-pvr: boolean - true to disable particle valid (restriction) rule,
 :                           false othervise
 :       * no-upa: boolean - true to disable unique particle attribution rule,
 :                           false othervise
 :
 : @return The generated output document, representing a sample XML instance.
 : @error schema-tools:VM001 If zorba was unable to start the JVM.
 : @error schema-tools:JAVA-EXCEPTION If Apache XMLBeans throws an exception.
 :)
declare function
schema-tools:xsd2inst ($schema as element()+, $rootElementName as xs:string, $options)
   as document-node() external;
