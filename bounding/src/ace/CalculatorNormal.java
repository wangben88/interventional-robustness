package ace;
import java.util.*;
import java.io.*;

/**
 * A calculator that works in normal space.
 * <p>
 * This is an internal class: you cannot use it directly.
 * 
 * @author Mark Chavira
 */

class CalculatorNormal extends Calculator {

  //============================================================================
  // Private
  //============================================================================

  // If evaluation results were computed, then a map from node to raw value,
  // which might need to be adjusted by the force to zero value; otherwise, a
  // map from node to garbage.ß
  private double[] fNodeToValue;
  
  // If differentiation is not enabled, then null; otherwise if differentiation
  // results were computed, then a map from node to derivative; otherwise, a
  // map from node to garbage.
  private double[] fNodeToDerivative;

  // The zero and one values.
  private static final double ZERO = 0.0;
  private static final double ONE = 1.0;
  
  /**
   * The constructor.
   * 
   * @param numNodes the number of nodes.
   * @param enableDifferentiation whether to allocate data structures for
   *   differentiation.
   */
  protected CalculatorNormal(int numNodes, boolean enableDifferentiation) {
    super(numNodes);
    fNodeToValue = new double[numNodes];
    if (enableDifferentiation) {fNodeToDerivative = new double[numNodes];}
  }

  //============================================================================
  // Evaluation
  //============================================================================

  
  @Override
  protected void evaluate(
      int numNodes,
      byte[] nodeToType,
      int[] nodeToLit,
      int[] nodeToLastEdge,
      int[] edgeToTailNode,
      Evidence ev)
      throws Exception {
    double[] negValues = ev.fVarToCurrentNegWeight;
    double[] posValues = ev.fVarToCurrentPosWeight;
    for (int n = 0; n < numNodes; n++) {
      switch (nodeToType[n]) {
        case OnlineEngine.MULT:
          int mLast = nodeToLastEdge[n];
          double mt = ONE;
          int numZeros = 0;
          for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < mLast; e++) {
            int ch = edgeToTailNode[e];
            double chVal = nodeValue(ch);
            if (chVal == ZERO) {
              if (++numZeros > 1) {mt = ZERO; break;}
            } else {
              mt *= chVal;
              if (mt == ZERO) {throw new UnderflowException();}
            }
          }
          fNodeToForceValueZero[n] = numZeros == 1;
          fNodeToValue[n] = mt;
          break;
        case OnlineEngine.ADD:
          int aLast = nodeToLastEdge[n];
          double at = ZERO;
          for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < aLast; e++) {
            int ch = edgeToTailNode[e];
            double chVal = nodeValue(ch);
            at += chVal;
          }
          fNodeToValue[n] = at;
          break;
        case OnlineEngine.MAX:
          int xLast = nodeToLastEdge[n];
          double xt = ZERO;
          for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < xLast; e++) {
            int ch = edgeToTailNode[e];
            double chVal = nodeValue(ch);
            if (chVal > xt) {xt = chVal;}
          }
          fNodeToValue[n] = xt;
          break;
        case OnlineEngine.LITERAL:
          int l = nodeToLit[n];
          fNodeToValue[n] = l < 0 ? negValues[-l] : posValues[l];
          break;
        case OnlineEngine.CONSTANT:
          break;
        default:
          throw new Exception("Unexpected node type!");
      }
    }
  }
  
  @Override
  protected double nodeValue(int n) {
    return fNodeToForceValueZero[n] ? ZERO : fNodeToValue[n];
  }
  
  //============================================================================
  // Differentiation
  //============================================================================

  @Override
  protected void differentiate(
      boolean mop,
      int numNodes,
      byte[] nodeToType,
      int[] nodeToLastEdge,
      int[] edgeToTailNode)
      throws Exception {
    Arrays.fill(fNodeToDerivative, ZERO);
    fNodeToDerivative[numNodes - 1] = ONE;
    for (int n = numNodes - 1; n >= 0; n--) {
      switch (nodeToType[n]) {
        case OnlineEngine.MULT:
          int mLast = nodeToLastEdge[n];
          double mt = fNodeToDerivative[n];
          if (mt == ZERO) {continue;}
          double mv = fNodeToValue[n];
          if (mv == ZERO) {continue;}
          mt *= mv;
          if (mt == ZERO) {throw new UnderflowException();}
          if (fNodeToForceValueZero[n]) { // exactly one zero
            for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < mLast; e++) {
              int ch = edgeToTailNode[e];
              double chVal = nodeValue(ch);
              if (chVal == ZERO) {
                if (mop) {
                  if (mt > fNodeToDerivative[ch]) {fNodeToDerivative[ch] = mt;}
                } else {
                  fNodeToDerivative[ch] += mt;
                }
                break;
              }
            }
          } else { // no zeros
            for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < mLast; e++) {
              int ch = edgeToTailNode[e];
              double chVal = nodeValue(ch);
              double contribution = mt / chVal;
              if (mop) {
                if (contribution > fNodeToDerivative[ch]) {
                  fNodeToDerivative[ch] = contribution;
                }
              } else {
                fNodeToDerivative[ch] += contribution;
              }
            }
          }
          break;
        case OnlineEngine.ADD:
          if (mop) {throw new Exception("Did not expect an addition node!");}
          int aLast = nodeToLastEdge[n];
          double at = fNodeToDerivative[n];
          for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < aLast; e++) {
            int ch = edgeToTailNode[e];
            fNodeToDerivative[ch] += at;
          }
          break;
        case OnlineEngine.MAX:
          if (!mop) {throw new Exception("Did not expect a max node!");}
          int xLast = nodeToLastEdge[n];
          double xt = fNodeToDerivative[n];
          for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < xLast; e++) {
            int ch = edgeToTailNode[e];
            if (xt > fNodeToDerivative[ch]) {fNodeToDerivative[ch] = xt;}
          }
          break;
        case OnlineEngine.LITERAL:
        case OnlineEngine.CONSTANT:
          continue;
      }
    }
  }
  
  @Override
  protected double nodePartial(int n) { return fNodeToDerivative[n]; }
  
  @Override
  protected double nodeMarginal(int n) {
    return fNodeToDerivative[n] * nodeValue(n);
  }
  
  @Override
  protected double nodePosterior(int n, int numNodes) {
    double overallValue = nodeValue(numNodes - 1);
    return (overallValue == ZERO) ?
           ZERO :
           nodeValue(n) * fNodeToDerivative[n] / overallValue;
  }
  
  //============================================================================
  // Support for counting
  //============================================================================

  @Override
  protected boolean valuesAreClose(int n1, int n2) {
    return close(nodeValue(n1), nodeValue(n2));
  }
  
  @Override
  protected boolean valuesAreEqual(int n1, int n2) {
    return nodeValue(n1) == nodeValue(n2);
  }

  //============================================================================
  // Support for other
  //============================================================================

  @Override
  protected double zero() { return ZERO; }
  
  //============================================================================
  // My Methods
  //=========================================================================
  
  // Note that, in contrast to standard evaluation, we don't keep track of 
  // zeros separately: we just do the multiplication (for simplicity)
  // NOTE: 10/01 fixed a serious bug where we had nodeValue in place of 
  // fNodeToValue, meaning that this could calculate incorrectly if evaluation
  // had been called prior to bubbleup.
  protected double computeNode(
	  int n,
	  byte[] nodeToType,
	  int[] nodeToLit,
	  int[] nodeToLastEdge,
	  int[] edgeToTailNode,
	  double[] negValues,
	  double[] posValues) 
      throws Exception {
	switch (nodeToType[n]) {
      case OnlineEngine.MULT:
        int mLast = nodeToLastEdge[n];
        double mt = ONE;
        //System.out.print(n);
        //System.out.println(" MULT");
        for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < mLast; e++) {
          int ch = edgeToTailNode[e];
          double chVal = fNodeToValue[ch];
          //System.out.print(chVal); System.out.print(" ");
          mt *= chVal;
        }
        //System.out.println();
        // fNodeToForceValueZero[n] = numZeros == 1;
        return mt;
      case OnlineEngine.ADD:
        int aLast = nodeToLastEdge[n];
        double at = ZERO;
        //System.out.print(n);
        //System.out.println(" ADD");
        for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < aLast; e++) {
          int ch = edgeToTailNode[e];
          double chVal = fNodeToValue[ch];
          //System.out.print(chVal); System.out.print(" ");
          at += chVal;
        }
        //System.out.println();
        return at;
      case OnlineEngine.MAX:
        int xLast = nodeToLastEdge[n];
        double xt = ZERO;
        //System.out.print(n);
        //System.out.println(" MAX");
        for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < xLast; e++) {
          int ch = edgeToTailNode[e];
          double chVal = fNodeToValue[ch];
          //System.out.print(chVal); System.out.print(" ");
          if (chVal > xt) {xt = chVal;}
        }
        //System.out.println();
        return xt;
      case OnlineEngine.LITERAL:
        int l = nodeToLit[n];
        return (l < 0 ? negValues[-l] : posValues[l]);
      // Not sure what constant represents...?
      case OnlineEngine.CONSTANT:
    	  throw new Exception("Found CONSTANT node....");
      default:
        throw new Exception("Unexpected node type!");
    }
  }
  
   
  
  
  // NOTE: I avoid making a call to evaluate() so that we can extend to do more
  // flexible things with the AC in future, and also so that we can distinguish
  // safely between evaluation results and bubbleup results (through the bools
  // EvaluationResultsAvailable, ... etc)
  @Override
  protected void bubbleUp(
	  int numNodes,
	  byte[] nodeToType,
	  int[] nodeToLit,
	  int[] nodeToLastEdge,
	  int[] edgeToTailNode,
	  boolean[] srcVarFree,
	  int[] nodeToSplitVar,
	  int[] acVarToSrcVar,
	  boolean[] acVarIsIndicator,
	  int[] acVarParameterToSrcVar,
	  Evidence ev)
	  throws Exception {
	 //System.out.println(acVarParameterToSrcVar[24]);  
	  
    double[] negValues = ev.fVarToCurrentNegWeight.clone();
    double[] posValues = ev.fVarToCurrentPosWeight.clone();
    byte[] modNodeToType = nodeToType.clone();
    for (int n = 0; n < numNodes; n++) {
      // Set intervenable parameters to 1
      if (modNodeToType[n] == OnlineEngine.LITERAL) {
    	int acVar = Math.abs(nodeToLit[n]);
    	if ((!acVarIsIndicator[acVar]) && (srcVarFree[acVarParameterToSrcVar[acVar]])){
      		// set parameter to 1
    		// NOTE: in all networks I've tried, the negative literals
    		// are never used. If they are used in future, will need to check
    		// these assignments.
      		negValues[acVar] = 1;
      		posValues[acVar] = 1;
      		//System.out.println(acVar);
      	}
      }
      
      // If + node, set to MAX if associated split src variable is intervenable
      if (modNodeToType[n] == OnlineEngine.ADD) {
      	int splitVar = nodeToSplitVar[n];
      	int splitSrcVar = acVarToSrcVar[splitVar];
      	// Note, if not in top order, some + nodes can be from intermediates in NNF
      	// Thus if this is the case, we ignore them
      	if ((splitSrcVar != -1) && (srcVarFree[splitSrcVar])) {
      		modNodeToType[n] = OnlineEngine.MAX;
        }
      }
      double val = computeNode(n, 
			  modNodeToType, 
			  nodeToLit, 
			  nodeToLastEdge,
			  edgeToTailNode,
			  negValues,
			  posValues);
      fNodeToValue[n] = val;
    }
  }
  

  @Override
  protected void countTerms(
	      int numNodes,
	      byte[] nodeToType,
	      int[] nodeToLit,
	      int[] nodeToLastEdge,
	      int[] edgeToTailNode,
	      int[] nodeToSplitVar,
	      Evidence ev)
	      throws Exception {
	    int[] numTerms = new int[numNodes];
	    for (int n = 0; n < numNodes; n++) {
	      switch (nodeToType[n]) {
	        case OnlineEngine.MULT:
	          int mLast = nodeToLastEdge[n];
	          int terms = 1;
	          for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < mLast; e++) {
	            int ch = edgeToTailNode[e];
	            terms *= numTerms[ch];
	          }
	          numTerms[n] = terms;
	          break;
	        case OnlineEngine.ADD:
	          int aLast = nodeToLastEdge[n];
	          int terms2 = 0;
	          for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < aLast; e++) {
	            int ch = edgeToTailNode[e];
	            terms2 += numTerms[ch];
	          }
	          numTerms[n] = terms2;
	          break;
	        case OnlineEngine.MAX:
	          break;
	        case OnlineEngine.LITERAL:
	          numTerms[n] = 1;
	          break;
	        case OnlineEngine.CONSTANT:
	          break;
	        default:
	          throw new Exception("Unexpected node type!");
	      }
	    }
	    System.out.println("Num terms: " + String.valueOf(numTerms[numNodes - 1]));
	    /*
	    int node = numNodes - 1;
	    Queue<Integer> q = new LinkedList<>();
	    q.add(node);
	    while (!q.isEmpty()) {
	    	node = q.remove();
	    	if (nodeToType[node] == OnlineEngine.ADD) {
	    		//System.out.print(String.valueOf(node) + " ");
	    		System.out.print(String.valueOf(nodeToSplitVar[node]) + " ");
	    		System.out.println(String.valueOf(numTerms[node]));
	    		int mLast = nodeToLastEdge[node];
	    		int e = node == 0 ? 0 : (mLast - 1);
		    	int ch = edgeToTailNode[e];
		    	q.add(ch);
	    	}
	    	else if (nodeToType[node] == OnlineEngine.MULT) {
	    		System.out.println(String.valueOf(numTerms[node]));
	    		int mLast = nodeToLastEdge[node];
	    		for (int e = node == 0 ? 0 : nodeToLastEdge[node - 1]; e < mLast; e++) {
		            int ch = edgeToTailNode[e];
		            q.add(ch);
		          }
	    	}

	    	
	    }
	    System.out.println();
	    */
  }
  
  /////////////////
  // OLD STUFF
  ////////////////
  /*
  // Recursively evaluates all nodes that are descendants of n. If it finds a
  // node which represents a parameter for splitSrcVar, it sets that parameter
  // to 1.
  protected double recEvalInt(
	  int n,
	  int splitSrcVar,
	  byte[] nodeToType,
	  int[] nodeToLit,
	  int[] nodeToLastEdge,
	  int[] edgeToTailNode,
	  boolean[] srcVarFree,
	  int[] nodeToSplitVar,
	  int[] acVarToSrcVar,
	  boolean[] acVarIsIndicator,
	  int[] acVarParameterToSrcVar,
	  double[] negValues,
	  double[] posValues)
  	  throws Exception {
	switch (nodeToType[n]) {
      case OnlineEngine.MULT:
    	int mLast = nodeToLastEdge[n];
    	double mt = ONE;
        for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < mLast; e++) {
          int ch = edgeToTailNode[e];
          double chVal = recEvalInt(ch,
        		   splitSrcVar,
				   nodeToType,
				   nodeToLit,
				   nodeToLastEdge,
				   edgeToTailNode,
				   srcVarFree,
				   nodeToSplitVar,
				   acVarToSrcVar,
				   acVarIsIndicator,
				   acVarParameterToSrcVar,
				   negValues,
				   posValues);
          mt *= chVal;
        }
        return mt;
        
      case OnlineEngine.ADD:
    	  int aLast = nodeToLastEdge[n];
      	  double at = ZERO;
          for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < aLast; e++) {
            int ch = edgeToTailNode[e];
            double chVal = recEvalInt(ch,
            		splitSrcVar,
  				   nodeToType,
  				   nodeToLit,
  				   nodeToLastEdge,
  				   edgeToTailNode,
  				   srcVarFree,
  				   nodeToSplitVar,
  				   acVarToSrcVar,
  				   acVarIsIndicator,
  				   acVarParameterToSrcVar,
  				   negValues,
  				   posValues);
            at += chVal;
          }
          return at;
        
      case OnlineEngine.MAX:
    	throw new Exception("Should not be MAX node");
    	
      case OnlineEngine.LITERAL:
    	  int l = nodeToLit[n];
    	if ((acVarIsIndicator[Math.abs(nodeToLit[n])])) {
            return (l < 0 ? negValues[-l] : posValues[l]);
    	}
    	else if (acVarParameterToSrcVar[l] == splitSrcVar){
    		// i.e. set parameter to 1
    		//System.out.println(posValues[l]);
    		return 1;
    	}
    	else {
    		return (l < 0 ? negValues[-l] : posValues[l]);
    	}
        
      // Not sure what constant represents...?
      case OnlineEngine.CONSTANT:
        throw new Exception("Found CONSTANT node....");
      default:
        throw new Exception("Unexpected node type!");
    }
	  
  }
  
  
  // Old "bubbleup" algorithm.
  // Proceeds evaluating each node as usual, but when it encounters an 
  // intervenable node, it redos the computation, applying recEvalInt above.
  // NOTE: This doesn't work
  
  @Override
  protected void oldBubbleUp(
	  int numNodes,
	  byte[] nodeToType,
	  int[] nodeToLit,
	  int[] nodeToLastEdge,
	  int[] edgeToTailNode,
	  boolean[] srcVarFree,
	  int[] nodeToSplitVar,
	  int[] acVarToSrcVar,
	  boolean[] acVarIsIndicator,
	  int[] acVarParameterToSrcVar,
	  Evidence ev)
	  throws Exception {
	 //System.out.println(acVarParameterToSrcVar[24]);
    double[] negValues = ev.fVarToCurrentNegWeight;
    double[] posValues = ev.fVarToCurrentPosWeight;
    for (int n = 0; n < numNodes; n++) {
      int splitSrcVar = acVarToSrcVar[nodeToSplitVar[n]];
      if ((nodeToType[n] == OnlineEngine.ADD)
    		  && (splitSrcVar != -1)
    		  && (srcVarFree[splitSrcVar]) ) {
    	  int xLast = nodeToLastEdge[n];
          double xt = ZERO;
          for (int e = n == 0 ? 0 : nodeToLastEdge[n - 1]; e < xLast; e++) {
            int ch = edgeToTailNode[e];
            double chVal = recEvalInt(ch,
            	   splitSrcVar,
 				   nodeToType,
 				   nodeToLit,
 				   nodeToLastEdge,
 				   edgeToTailNode,
 				   srcVarFree,
 				   nodeToSplitVar,
 				   acVarToSrcVar,
 				   acVarIsIndicator,
 				   acVarParameterToSrcVar,
 				   negValues,
 				   posValues);
            System.out.println(chVal);
            if (chVal > xt) {xt = chVal;}
          }
          fNodeToValue[n] = xt;
          double val = computeNode(n, 
				  nodeToType, 
				  nodeToLit, 
				  nodeToLastEdge,
				  edgeToTailNode,
				  negValues,
				  posValues);
          System.out.println("ORIG:");
          System.out.println(val);
		
		  //fNodeToValue[n] = val;
      }
      else {
    	
		double val = computeNode(n, 
				  nodeToType, 
				  nodeToLit, 
				  nodeToLastEdge,
				  edgeToTailNode,
				  negValues,
				  posValues);
		
		fNodeToValue[n] = val;
      }
    }
  }
  */
}


