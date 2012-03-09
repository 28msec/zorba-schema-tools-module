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


import schema namespace st-options = "http://www.zorba-xquery.com/modules/schema-tools/schema-tools-options";

(:~
 : Import module for checking if options element is validated.
 :)
import module namespace schemaOptions = "http://www.zorba-xquery.com/modules/schema";


declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare namespace ver = "http://www.zorba-xquery.com/options/versioning";
declare option ver:module-version "1.0";


(:~
 : The inst2xsd function takes a set of sample instance elements as input and 
 : generates a schema element that reprezents one sample XML Schema that defines
 : the content of the given input.
 : <br />
 : Please consult the 
 : <a href="http://xmlbeans.apache.org/">official documentation for further
 : information</a>.
 : <br />
 : Example:<pre class="brush: xquery;">
 :
 :  import module namespace st = "http://www.zorba-xquery.com/modules/schema-tools";
 :  declare namespace sto =
 :      "http://www.zorba-xquery.com/modules/schema-tools/schema-tools-options";
 :  let $instances := (<a><b/><c/></a>, <b/>, <c>ccc</c>)
 :  let $options  :=
 :     <sto:options xmlns:sto=
 :       "http://www.zorba-xquery.com/modules/schema-tools/schema-tools-options">
 :       <sto:design>vbd</sto:design>
 :       <sto:simple-content-types>smart</sto:simple-content-types>
 :       <sto:use-enumeration>10</sto:use-enumeration>
 :     </sto:options>
 :  return
 :      st:inst2xsd($instances, $options)
 :
 : </pre>
 : <br />
 : @param $instances The XML instance elements that will define the schema.
 : @param $options The Inst2XSD options:<br />
 :    <ul>
 :      <li>design: Choose the generated schema design<br />
 :         - rdd: Russian Doll Design - local elements and local types<br />
 :         - ssd: Salami Slice Design - global elements and local types<br />
 :         - vbd (default): Venetian Blind Design - local elements and global
 :                          complex types</li>
 :      <li>simple-content-types: type of leaf nodes<br />
 :         - smart (default): try to find out the right simple shema type<br />
 :         - always-string: use xsd:string for all simple types</li>
 :      <li>use-enumeration: - when there are multiple valid values in a list<br />
 :         - 1: never use enumeration<br />
 :         - 2 or more (default 10): only if not more than the value - number option</li>
 :      <li>verbose: - stdout verbose info<br />
 :         - true: - output type holder information<br />
 :         - false (default): no output</li></ul>
 :
 :
 : @return The generated output document, representing a sample XMLSchema.
 : @error schema-tools:VM001 If zorba was unable to start the JVM.
 : @error schema-tools:JAVA-EXCEPTION If Apache XMLBeans throws an exception.
 : @example test/Queries/schema-tools/inst2xsd-opt1.xq
 : @example test/Queries/schema-tools/inst2xsd-opt2.xq
 : @example test/Queries/schema-tools/inst2xsd-opt3.xq
 : @example test/Queries/schema-tools/inst2xsd-simple.xq
 : @example test/Queries/schema-tools/inst2xsd-tns-default.xq
 : @example test/Queries/schema-tools/inst2xsd-tns.xq
 : @example test/Queries/schema-tools/inst2xsd-multiTns.xq
 : @example test/Queries/schema-tools/inst2xsd-err1-badOpt.xq
 :)
declare function
schema-tools:inst2xsd ($instances as element()+,
    $options as element(st-options:options)?)
  as document-node()*
{
  let $validated-options :=
    if(empty($options))
    then
      $options
    else if(schemaOptions:is-validated($options))
    then
      $options
    else
      validate{$options}
  return
    schema-tools:inst2xsd-internal($instances, $validated-options)
};


declare %private function
schema-tools:inst2xsd-internal( $instances as element()+,
    $options as element(st-options:options, st-options:optionsType)? )
  as document-node()* external;



(:~
 : The xsd2inst function takes a set of XML Schema file names as input and the
 : name of the root element and
 : generates an element that reprezents one sample XML instance of the given
 : input schema files.
 : <br />
 : Please consult the
 : <a href="http://xmlbeans.apache.org/">official documentation for further
 :   information</a>.
 : <br />
 : Example:<pre class="brush: xquery;">
 :
 :  import module namespace st = "http://www.zorba-xquery.com/modules/schema-tools";
 :  declare namespace sto =
 :      "http://www.zorba-xquery.com/modules/schema-tools/schema-tools-options";
 :  let $xsds  :=
 :     ( <xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"
 :           attributeFormDefault="unqualified"
 :           elementFormDefault="qualified">
 :         <xs:element name="a" type="aType"/>
 :         <xs:complexType name="aType">
 :           <xs:sequence>
 :             <xs:element type="xs:string" name="b"/>
 :             <xs:element type="xs:string" name="c"/>
 :           </xs:sequence>
 :         </xs:complexType>
 :       </xs:schema> )
 :  let $options :=
 :    <sto:options xmlns:sto=
 :      "http://www.zorba-xquery.com/modules/schema-tools/schema-tools-options">
 :      <sto:network-downloads>false</sto:network-downloads>
 :      <sto:no-pvr>false</sto:no-pvr>
 :      <sto:no-upa>false</sto:no-upa>
 :    </sto:options>
 :  return
 :      st:xsd2inst($xsds, "a", $options)
 : </pre><br />
 : @param $schema The XML Schema file names that define the schema type system.
 : @param $rootElementName The LocalName of the root element of the instance.
 :        If multiple target namespaces are used, first one found - using the
 :        sequence order - will be used.
 : @param $options Options:<br /><ul>
 :       <li>network-downloads: boolean (default false)<br />
 :             - true allowes XMLBeans to use network when resolving schema
 :               imports and includes</li>
 :       <li>no-pvr: boolean (default false)<br />
 :             - true to disable particle valid (restriction) rule,
 :               false othervise</li>
 :       <li>no-upa: boolean (default false)<br />
 :             - true to disable unique particle attribution rule,
 :               false othervise</li></ul>
 :
 : @return The generated output document, representing a sample XML instance.
 : @error schema-tools:VM001 If zorba was unable to start the JVM.
 : @error schema-tools:JAVA-EXCEPTION If Apache XMLBeans throws an exception.
 : @example test/Queries/schema-tools/xsd2inst-opt1.xq
 : @example test/Queries/schema-tools/xsd2inst-simple.xq
 : @example test/Queries/schema-tools/xsd2inst-tns.xq
 : @example test/Queries/schema-tools/xsd2inst-err1-badOpt.xq
 :)
declare function
schema-tools:xsd2inst ($schemas as element()+, $rootElementName as xs:string,
    $options as element(st-options:options)?)
  as document-node()
{
  let $validated-options :=
    if(empty($options))
    then
        $options
    else if(schemaOptions:is-validated($options))
    then
        $options
    else
        validate{$options}
  return
    schema-tools:xsd2inst-internal($schemas, $rootElementName, $validated-options)
};


declare %private function
schema-tools:xsd2inst-internal ($schemas as element()+,
    $rootElementName as xs:string,
    $options as element(st-options:options, st-options:optionsType)?)
  as document-node() external;