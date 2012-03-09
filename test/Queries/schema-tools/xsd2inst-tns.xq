import module namespace st = "http://www.zorba-xquery.com/modules/schema-tools";

declare namespace sto = "http://www.zorba-xquery.com/modules/schema-tools/schema-tools-options";



let $xsd  :=
  <xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema" 
      attributeFormDefault="unqualified" 
      elementFormDefault="qualified" 
      targetNamespace="zorba-xquery.com/test/modules/schema-tools"
      xmlns:sch="zorba-xquery.com/test/modules/schema-tools">
    <xs:element xmlns:sch="zorba-xquery.com/test/modules/schema-tools" name="a" type="sch:aType"/>
    <xs:element name="b" type="xs:byte"/>
    <xs:element name="c" type="xs:string"/>
    <xs:complexType name="aType">
      <xs:sequence>
        <xs:element type="xs:byte" name="b"/>
        <xs:element type="xs:string" name="c" maxOccurs="unbounded" minOccurs="0"/>
      </xs:sequence>
    </xs:complexType>
  </xs:schema>
let $opt  := <sto:xsd2inst-options/>
return
    st:xsd2inst(($xsd), "a", $opt)

