import module namespace st = "http://www.zorba-xquery.com/modules/schema-tools";

declare namespace sto = "http://www.zorba-xquery.com/modules/schema-tools/schema-tools-options";


let $inst := (<a><b>1</b><c>c</c><c>cc</c></a>, <b>2</b>, <c>ccc</c>)
let $opt  := <sto:inst2xsd-options>
                <sto:design>rdd</sto:design>
                <sto:simple-content-types>always-string</sto:simple-content-types>
                <sto:use-enumeration>1</sto:use-enumeration>
             </sto:inst2xsd-options>
return
    st:inst2xsd($inst, $opt)
