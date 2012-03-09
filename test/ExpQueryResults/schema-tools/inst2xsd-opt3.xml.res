<?xml version="1.0" encoding="UTF-8"?>
<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema" attributeFormDefault="unqualified" elementFormDefault="qualified">
  <xs:element name="b" type="xs:byte"/>
  <xs:element name="c" type="xs:string"/>
  <xs:element name="a">
    <xs:complexType>
      <xs:sequence>
        <xs:element ref="b"/>
        <xs:element ref="c" maxOccurs="unbounded" minOccurs="0"/>
      </xs:sequence>
    </xs:complexType>
  </xs:element>
</xs:schema>


