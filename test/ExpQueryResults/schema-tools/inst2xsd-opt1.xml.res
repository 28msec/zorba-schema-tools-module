<?xml version="1.0" encoding="UTF-8"?>
<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema" attributeFormDefault="unqualified" elementFormDefault="qualified">
   <xs:element name="a" type="aType"/>
   <xs:element name="b" type="xs:byte"/>
   <xs:element name="c" type="xs:string"/>
   <xs:complexType name="aType">
     <xs:sequence>
       <xs:element type="xs:byte" name="b"/>
       <xs:element name="c" maxOccurs="unbounded" minOccurs="0">
         <xs:simpleType>
           <xs:restriction base="xs:string">
             <xs:enumeration>
               <value>c</value>
             </xs:enumeration>
             <xs:enumeration>
               <value>cc</value>
             </xs:enumeration>
           </xs:restriction>
         </xs:simpleType>
       </xs:element>
     </xs:sequence>
   </xs:complexType>
</xs:schema>

