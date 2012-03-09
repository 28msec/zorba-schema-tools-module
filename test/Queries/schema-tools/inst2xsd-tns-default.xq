import module namespace st = "http://www.zorba-xquery.com/modules/schema-tools";

declare namespace sto = "http://www.zorba-xquery.com/modules/schema-tools/schema-tools-options";
declare default element namespace "my.own.simple/tns";

let $inst := (<a><b>1</b><c>c</c><c>cc</c></a>, 
              <b>2</b>, 
              <c>ccc</c>)
let $opt  := <sto:options>
               <sto:use-enumeration>1</sto:use-enumeration>
             </sto:options>              
return
    st:inst2xsd($inst, $opt)

