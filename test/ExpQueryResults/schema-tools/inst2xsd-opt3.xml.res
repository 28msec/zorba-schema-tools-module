<?xml version="1.0" encoding="UTF-8"?>
<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema" attributeFormDefault="unqualified" elementFormDefault="qualified">
  <xs:element name="b">
    <xs:simpleType>
      <xs:restriction base="xs:byte">
        <xs:enumeration>
          <value>1</value>
        </xs:enumeration>
        <xs:enumeration>
          <value>2</value>
        </xs:enumeration>
      </xs:restriction>
    </xs:simpleType>
  </xs:element>
  <xs:element name="c">
    <xs:simpleType>
      <xs:restriction base="xs:string">
        <xs:enumeration>
          <value>c</value>
        </xs:enumeration>
        <xs:enumeration>
          <value>cc</value>
        </xs:enumeration>
        <xs:enumeration>
          <value>ccc</value>
        </xs:enumeration>
      </xs:restriction>
    </xs:simpleType>
  </xs:element>
  <xs:element name="a">
    <xs:complexType>
      <xs:sequence>
        <xs:element ref="b"/>
        <xs:element ref="c" maxOccurs="unbounded" minOccurs="0"/>
      </xs:sequence>
    </xs:complexType>
  </xs:element>
</xs:schema>


