import module namespace st = "http://www.zorba-xquery.com/modules/schema-tools";

declare namespace sto = "http://www.zorba-xquery.com/modules/schema-tools/schema-tools-options";


let $inst := (<a><b>1</b><c>c</c><c>cc</c></a>, <b>2</b>, <c>ccc</c>)
let $opt  := <sto:options>
                <sto:verbose>true</sto:verbose>
                <sto:design>vbd</sto:design>
                <sto:simple-content-types>smart</sto:simple-content-types>
                <sto:use-enumeration>2</sto:use-enumeration>
             </sto:options>
return
    st:inst2xsd($inst, $opt)
