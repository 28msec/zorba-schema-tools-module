package org.zorbaxquery.modules.schemaTools;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InterruptedIOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class CppInputStream
    extends InputStream
{
    List<Byte> data = new ArrayList<Byte>();
    int off = 0;
    private boolean endflag = false;

    public void push(byte[] d)
    {
        synchronized(data)
        {
            if(d == null)
            {
                this.endflag = true;
            }
            else
            {
                for(int i = 0; i < d.length;++i)
                {
                    data.add(d[i]);
                }
            }
        }
    }

    @Override
    public int read() throws IOException
    {
        synchronized(data)
        {
            while(data.isEmpty()&&!endflag)
            {
                try
                {
                    data.wait();
                }
                catch (InterruptedException e)
                {
                    throw new InterruptedIOException();
                }
            }
        }
        if(endflag)
            return -1;
        else
            return data.remove(0);
    }
}
