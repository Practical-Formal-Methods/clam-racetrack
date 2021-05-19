/* Copyright 2019, Gurobi Optimization, LLC */

/* This example reads an LP model from a file and solves it.
   If the model can be solved, then it finds the smallest positive variable,
   sets its upper bound to zero, and resolves the model two ways:
   first with an advanced start, then without an advanced start
   (i.e. 'from scratch'). */

using System;
using Gurobi;

class lpmod_cs
{
  static void Main(string[] args)
  {
    if (args.Length < 1) {
      Console.Out.WriteLine("Usage: lpmod_cs filename");
      return;
    }

    try {
      // Read model and determine whether it is an LP
      GRBEnv env = new GRBEnv();
      GRBModel model = new GRBModel(env, args[0]);
      if (model.IsMIP != 0) {
        Console.WriteLine("The model is not a linear program");
        Environment.Exit(1);
      }

      model.Optimize();

      int status = model.Status;

      if ((status == GRB.Status.INF_OR_UNBD) ||
          (status == GRB.Status.INFEASIBLE) ||
          (status == GRB.Status.UNBOUNDED)) {
        Console.WriteLine("The model cannot be solved because it is "
            + "infeasible or unbounded");
        Environment.Exit(1);
      }

      if (status != GRB.Status.OPTIMAL) {
        Console.WriteLine("Optimization was stopped with status " + status);
        Environment.Exit(0);
      }

      // Find the smallest variable value
      double minVal = GRB.INFINITY;
      GRBVar minVar = null;
      foreach (GRBVar v in model.GetVars()) {
        double sol = v.X;
        if ((sol > 0.0001) && (sol < minVal) && (v.LB == 0.0)) {
          minVal = sol;
          minVar = v;
        }
      }

      Console.WriteLine("\n*** Setting " +
          minVar.VarName + " from " + minVal +
          " to zero ***\n");
      minVar.UB = 0.0;

      // Solve from this starting point
      model.Optimize();

      // Save iteration & time info
      double warmCount = model.IterCount;
      double warmTime = model.Runtime;

      // Reset the model and resolve
      Console.WriteLine("\n*** Resetting and solving "
          + "without an advanced start ***\n");
      model.Reset();
      model.Optimize();

      double coldCount = model.IterCount;
      double coldTime = model.Runtime;

      Console.WriteLine("\n*** Warm start: " + warmCount + " iterations, " +
          warmTime + " seconds");
      Console.WriteLine("*** Cold start: " + coldCount + " iterations, " +
          coldTime + " seconds");

      // Dispose of model and env
      model.Dispose();
      env.Dispose();

    } catch (GRBException e) {
      Console.WriteLine("Error code: " + e.ErrorCode + ". " +
          e.Message);
    }
  }
}
