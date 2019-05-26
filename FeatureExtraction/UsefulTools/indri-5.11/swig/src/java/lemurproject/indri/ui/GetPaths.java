package lemurproject.indri.ui;
import java.util.*;
import java.io.*;
/** Utility class to get the paths to support applications from the
    Windows registry.
 */
public class GetPaths {
    private static final String REGQUERY_UTIL = "reg query ";
    private static final String REGSTR_TOKEN = "REG_SZ";
    private static final String REGDWORD_TOKEN = "REG_DWORD";
    private static final String PATH_CMD = REGQUERY_UTIL +
	"\"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
    /**
       Look up an applicaton in the registry and retrun the full path
       to the executable.
     */
    public static String getPath(String prog) {
	try {
	    String key = PATH_CMD + prog + "\"";
	    Process process = Runtime.getRuntime().exec(key);
	    StreamReader reader = new StreamReader(process.getInputStream());
	    reader.start();
	    process.waitFor();
	    reader.join();
	    String result = reader.getResult();
	    int p1 = result.indexOf("<NO NAME>");
	    result = result.substring(p1 + 4).trim();
	    int p2 = result.indexOf(".exe");
	    if (p2 == -1) p2 = result.indexOf(".EXE");
	    result = result.substring(0, p2 + 4).trim();
	    int p = result.indexOf(REGSTR_TOKEN);
	    if (p == -1)
		return null;
	    return result.substring(p + REGSTR_TOKEN.length()).trim();
	} catch (Exception e) {
	    return null;
	}
    }
    /**
       Read a stream into a String.
     */
    static class StreamReader extends Thread {
	private InputStream is;
	private StringWriter sw;

	StreamReader(InputStream is) {
	    this.is = is;
	    sw = new StringWriter();
	}
	public void run() {
	    try {
		int c;
		while ((c = is.read()) != -1)
		    sw.write(c);
	    } catch (IOException e) {
		;
	    }
	}
	String getResult() {
	    return sw.toString();
	}
    }
	
}
