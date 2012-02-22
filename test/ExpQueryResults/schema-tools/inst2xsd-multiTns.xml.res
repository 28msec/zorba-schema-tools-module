<?xml version="1.0" encoding="UTF-8"?>
<res><xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema" attributeFormDefault="unqualified" elementFormDefault="qualified" targetNamespace="zorba-xquery.com/test/modules/schema-tools.2">
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
</xs:schema><xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema" attributeFormDefault="unqualified" elementFormDefault="qualified" targetNamespace="zorba-xquery.com/test/modules/schema-tools.3">
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
</xs:schema><xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema" attributeFormDefault="unqualified" elementFormDefault="qualified" targetNamespace="zorba-xquery.com/test/modules/schema-tools.1">
  <xs:element xmlns:sch="zorba-xquery.com/test/modules/schema-tools.1" name="a" type="sch:aType"/>
  <xs:complexType name="aType">
    <xs:sequence>
      <xs:element xmlns:sch="zorba-xquery.com/test/modules/schema-tools.2" ref="sch:b"/>
      <xs:element xmlns:sch="zorba-xquery.com/test/modules/schema-tools.3" ref="sch:c" maxOccurs="unbounded" minOccurs="0"/>
    </xs:sequence>
  </xs:complexType>
</xs:schema></res>

