package ace;
import java.util.*;
import java.io.*;

/**
 * An example of using the Ace evaluator API.  The files simple.net.lmap and
 * simple.net.ac must have been compiled using the SOP kind and must be in the
 * directory from which this program is executed.
 * <code>
 * Usage java edu.ucla.belief.Test
 * </code>
 * 
 * @author Mark Chavira
 */

public class UpperBound {

  /**
   * The main program.
   * 
   * @param args command line parameters - ignored.
   * @throws Exception if execution fails.
   */
  
  public static void main(String[] args) throws Exception {

	BufferedReader br = new BufferedReader(new FileReader(args[0]));
	String acFile = br.readLine();
	String lmapFile = br.readLine();
	OnlineEngineSop g = new OnlineEngineSop(
		        lmapFile,
		    	acFile,
		        true);
	Evidence e = new Evidence(g);
	boolean[] srcVarFree = new boolean[g.numVariables()];
	
	
	String numEviStr = br.readLine(); int numEvi = Integer.parseInt(numEviStr);
	
	for (int i = 0; i < numEvi; i++) {
		String line = br.readLine();
		String[] tokens = line.split("\\s+");
		String varName = tokens[0];
		if (tokens.length > 2) {
			int[] vals = new int[tokens.length - 1];
			for (int j = 0; j < tokens.length - 1; j++) {
				vals[j] = Integer.parseInt(tokens[j + 1]);
			}
			e.varCommitMult(g.varForName(varName), vals);
		}
		else {
			int val = Integer.parseInt(tokens[1]);
			e.varCommit(g.varForName(varName), val);
		}
	}
	
	String numIntervStr = br.readLine(); int numInterv = Integer.parseInt(numIntervStr);
	for (int i = 0; i < numInterv; i++) {
		String varName = br.readLine();
		srcVarFree[g.varForName(varName)] = true;
	}
	
	
	g.bubbleUp(srcVarFree, e);
    
    double introbUB = g.bubbleUpResults();
    
    System.out.println("INTROB UB: " + introbUB);
    
  }
}
