import module namespace st = "http://www.zorba-xquery.com/modules/schema-tools";

declare namespace sto = "http://www.zorba-xquery.com/modules/schema-tools/schema-tools-options";
declare namespace myNS = "zorba-xquery.com/test/modules/schema-tools";

let $inst := (<myNS:a><myNS:b>1</myNS:b><myNS:c>c</myNS:c><myNS:c>cc</myNS:c></myNS:a>, 
              <myNS:b>2</myNS:b>, 
              <myNS:c>ccc</myNS:c>)
let $opt  := <sto:options></sto:options>              
return
    st:inst2xsd($inst, $opt)

