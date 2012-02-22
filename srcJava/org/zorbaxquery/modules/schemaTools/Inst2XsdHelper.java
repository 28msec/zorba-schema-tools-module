package org.zorbaxquery.modules.schemaTools;

import org.apache.xmlbeans.XmlException;
import org.apache.xmlbeans.XmlOptions;
import org.apache.xmlbeans.impl.inst2xsd.Inst2Xsd;
import org.apache.xmlbeans.impl.inst2xsd.Inst2XsdOptions;
import org.apache.xmlbeans.impl.xb.xsdschema.SchemaDocument;

import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;

public class Inst2XsdHelper
{
    public static String[] inst2xsd(String[] insts, Inst2XsdOptions opt)
        throws XmlException, IOException
    {
        //System.out.println("inst2Xsd inst: '" + insts + "'"); System.out.flush();

        Reader[] instReaders = new Reader[insts.length];
        for (int i=0; i< insts.length; i++)
        {
            instReaders[i] = new StringReader(insts[i]);
        }

        SchemaDocument[] xsds = Inst2Xsd.inst2xsd(instReaders, opt);

        //System.out.println("inst2Xsd end result '" + xsds[0].toString() + "'");

        String[] res = new String[xsds.length];

        XmlOptions options = new XmlOptions();
        options.put( XmlOptions.SAVE_INNER );
        options.put( XmlOptions.SAVE_PRETTY_PRINT );
        options.put( XmlOptions.SAVE_AGGRESSIVE_NAMESPACES );
        //options.put( XmlOptions.SAVE_USE_DEFAULT_NAMESPACE ); don't use this can generate buggy schema
        options.setSaveNamespacesFirst();

        for (int i = 0; i < xsds.length; i++)
        {
            res[i] = xsds[i].xmlText(options);
        }

        return res;
    }
}
