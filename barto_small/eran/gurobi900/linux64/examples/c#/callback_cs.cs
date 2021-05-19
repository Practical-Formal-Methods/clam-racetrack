/* Copyright 2019, Gurobi Optimization, LLC */

/*
   This example reads a model from a file, sets up a callback that
   monitors optimization progress and implements a custom
   termination strategy, and outputs progress information to the
   screen and to a log file.

   The termination strategy implemented in this callback stops the
   optimization of a MIP model once at least one of the following two
   conditions have been satisfied:
     1) The optimality gap is less than 10%
     2) At least 10000 nodes have been explored, and an integer feasible
        solution has been found.
   Note that termination is normally handled through Gurobi parameters
   (MIPGap, NodeLimit, etc.).  You should only use a callback for
   termination if the available parameters don't capture your desired
   termination criterion.
*/

using System;
using System.IO;
using Gurobi;

class callback_cs : GRBCallback
{
  private double       lastiter;
  private double       lastnode;
  private GRBVar[]     vars;
  private StreamWriter logfile;

  public callback_cs(GRBVar[] xvars, StreamWriter xlogfile)
  {
    lastiter = lastnode = -GRB.INFINITY;
    vars = xvars;
    logfile = xlogfile;
  }

  protected override void Callback()
  {
    try {
      if (where == GRB.Callback.POLLING) {
        // Ignore polling callback
      } else if (where == GRB.Callback.PRESOLVE) {
        // Presolve callback
        int cdels = GetIntInfo(GRB.Callback.PRE_COLDEL);
        int rdels = GetIntInfo(GRB.Callback.PRE_ROWDEL);
        if (cdels != 0 || rdels != 0) {
          Console.WriteLine(cdels + " columns and " + rdels
              + " rows are removed");
        }
      } else if (where == GRB.Callback.SIMPLEX) {
        // Simplex callback
        double itcnt = GetDoubleInfo(GRB.Callback.SPX_ITRCNT);
        if (itcnt - lastiter >= 100) {
          lastiter = itcnt;
          double obj    = GetDoubleInfo(GRB.Callback.SPX_OBJVAL);
          int    ispert = GetIntInfo(GRB.Callback.SPX_ISPERT);
          double pinf   = GetDoubleInfo(GRB.Callback.SPX_PRIMINF);
          double dinf   = GetDoubleInfo(GRB.Callback.SPX_DUALINF);
          char ch;
          if (ispert == 0)      ch = ' ';
          else if (ispert == 1) ch = 'S';
          else                  ch = 'P';
          Console.WriteLine(itcnt + " " + obj + ch + " "
              + pinf + " " + dinf);
        }
      } else if (where == GRB.Callback.MIP) {
        // General MIP callback
        double nodecnt = GetDoubleInfo(GRB.Callback.MIP_NODCNT);
        double objbst  = GetDoubleInfo(GRB.Callback.MIP_OBJBST);
        double objbnd  = GetDoubleInfo(GRB.Callback.MIP_OBJBND);
        int    solcnt  = GetIntInfo(GRB.Callback.MIP_SOLCNT);
        if (nodecnt - lastnode >= 100) {
          lastnode = nodecnt;
          int actnodes = (int) GetDoubleInfo(GRB.Callback.MIP_NODLFT);
          int itcnt    = (int) GetDoubleInfo(GRB.Callback.MIP_ITRCNT);
          int cutcnt   = GetIntInfo(GRB.Callback.MIP_CUTCNT);
          Console.WriteLine(nodecnt + " " + actnodes + " "
              + itcnt + " " + objbst + " " + objbnd + " "
              + solcnt + " " + cutcnt);
        }
        if (Math.Abs(objbst - objbnd) < 0.1 * (1.0 + Math.Abs(objbst))) {
          Console.WriteLine("Stop early - 10% gap achieved");
          Abort();
        }
        if (nodecnt >= 10000 && solcnt > 0) {
          Console.WriteLine("Stop early - 10000 nodes explored");
          Abort();
        }
      } else if (where == GRB.Callback.MIPSOL) {
        // MIP solution callback
        int      nodecnt = (int) GetDoubleInfo(GRB.Callback.MIPSOL_NODCNT);
        double   obj     = GetDoubleInfo(GRB.Callback.MIPSOL_OBJ);
        int      solcnt  = GetIntInfo(GRB.Callback.MIPSOL_SOLCNT);
        double[] x       = GetSolution(vars);
        Console.WriteLine("**** New solution at node " + nodecnt
            + ", obj " + obj + ", sol " + solcnt
            + ", x[0] = " + x[0] + " ****");
      } else if (where == GRB.Callback.MIPNODE) {
        // MIP node callback
        Console.WriteLine("**** New node ****");
        if (GetIntInfo(GRB.Callback.MIPNODE_STATUS) == GRB.Status.OPTIMAL) {
          double[] x = GetNodeRel(vars);
          SetSolution(vars, x);
        }
      } else if (where == GRB.Callback.BARRIER) {
        // Barrier callback
        int    itcnt   = GetIntInfo(GRB.Callback.BARRIER_ITRCNT);
        double primobj = GetDoubleInfo(GRB.Callback.BARRIER_PRIMOBJ);
        double dualobj = GetDoubleInfo(GRB.Callback.BARRIER_DUALOBJ);
        double priminf = GetDoubleInfo(GRB.Callback.BARRIER_PRIMINF);
        double dualinf = GetDoubleInfo(GRB.Callback.BARRIER_DUALINF);
        double cmpl    = GetDoubleInfo(GRB.Callback.BARRIER_COMPL);
        Console.WriteLine(itcnt + " " + primobj + " " + dualobj + " "
            + priminf + " " + dualinf + " " + cmpl);
      } else if (where == GRB.Callback.MESSAGE) {
        // Message callback
        string msg = GetStringInfo(GRB.Callback.MSG_STRING);
        if (msg != null) logfile.Write(msg);
      }
    } catch (GRBException e) {
      Console.WriteLine("Error code: " + e.ErrorCode);
      Console.WriteLine(e.Message);
      Console.WriteLine(e.StackTrace);
    } catch (Exception e) {
      Console.WriteLine("Error during callback");
      Console.WriteLine(e.StackTrace);
    }
  }

  static void Main(string[] args)
  {
    if (args.Length < 1) {
      Console.Out.WriteLine("Usage: callback_cs filename");
      return;
    }

    StreamWriter logfile = null;

    try {
      // Create environment
      GRBEnv env = new GRBEnv();

      // Read model from file
      GRBModel model = new GRBModel(env, args[0]);

      // Turn off display and heuristics
      model.Parameters.OutputFlag = 0;
      model.Parameters.Heuristics = 0.0;

      // Open log file
      logfile = new StreamWriter("cb.log");

      // Create a callback object and associate it with the model
      GRBVar[]    vars = model.GetVars();
      callback_cs cb   = new callback_cs(vars, logfile);

      model.SetCallback(cb);

      // Solve model and capture solution information
      model.Optimize();

      Console.WriteLine("");
      Console.WriteLine("Optimization complete");
      if (model.SolCount == 0) {
        Console.WriteLine("No solution found, optimization status = "
            + model.Status);
      } else {
        Console.WriteLine("Solution found, objective = " + model.ObjVal);

        string[] vnames = model.Get(GRB.StringAttr.VarName, vars);
        double[] x      = model.Get(GRB.DoubleAttr.X, vars);

        for (int j = 0; j < vars.Length; j++) {
          if (x[j] != 0.0) Console.WriteLine(vnames[j] + " " + x[j]);
        }
      }

      // Dispose of model and environment
      model.Dispose();
      env.Dispose();

    } catch (GRBException e) {
      Console.WriteLine("Error code: " + e.ErrorCode);
      Console.WriteLine(e.Message);
      Console.WriteLine(e.StackTrace);
    } catch (Exception e) {
      Console.WriteLine("Error during optimization");
      Console.WriteLine(e.Message);
      Console.WriteLine(e.StackTrace);
    } finally {
      // Close log file
      if (logfile != null) logfile.Close();
    }
  }
}
