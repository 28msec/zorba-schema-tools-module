import module namespace st = "http://www.zorba-xquery.com/modules/schema-tools";

declare namespace sto = "http://www.zorba-xquery.com/modules/schema-tools/schema-tools-options";
declare namespace myNS1 = "zorba-xquery.com/test/modules/schema-tools.1";
declare namespace myNS2 = "zorba-xquery.com/test/modules/schema-tools.2";
declare namespace myNS3 = "zorba-xquery.com/test/modules/schema-tools.3";

let $inst := (<myNS1:a><myNS2:b>1</myNS2:b><myNS3:c>c</myNS3:c><myNS3:c>cc</myNS3:c></myNS1:a>, 
              <myNS2:b>2</myNS2:b>, 
              <myNS3:c>ccc</myNS3:c>)
let $opt  := <sto:options></sto:options>              
return
    <res>{st:inst2xsd($inst, $opt)}</res>

