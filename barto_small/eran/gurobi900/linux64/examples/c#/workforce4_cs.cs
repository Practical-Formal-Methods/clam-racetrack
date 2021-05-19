/* Copyright 2019, Gurobi Optimization, LLC */

/* Assign workers to shifts; each worker may or may not be available on a
   particular day. We use Pareto optimization to solve the model:
   first, we minimize the linear sum of the slacks. Then, we constrain
   the sum of the slacks, and we minimize a quadratic objective that
   tries to balance the workload among the workers. */

using System;
using Gurobi;

class workforce4_cs
{
  static void Main()
  {
    try {

      // Sample data
      // Sets of days and workers
      string[] Shifts =
          new string[] { "Mon1", "Tue2", "Wed3", "Thu4", "Fri5", "Sat6",
              "Sun7", "Mon8", "Tue9", "Wed10", "Thu11", "Fri12", "Sat13",
              "Sun14" };
      string[] Workers =
          new string[] { "Amy", "Bob", "Cathy", "Dan", "Ed", "Fred", "Gu" };

      int nShifts = Shifts.Length;
      int nWorkers = Workers.Length;

      // Number of workers required for each shift
      double[] shiftRequirements =
          new double[] { 3, 2, 4, 4, 5, 6, 5, 2, 2, 3, 4, 6, 7, 5 };

      // Worker availability: 0 if the worker is unavailable for a shift
      double[,] availability =
          new double[,] { { 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1 },
              { 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0 },
              { 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
              { 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
              { 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1 },
              { 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1 },
              { 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } };

      // Model
      GRBEnv env = new GRBEnv();
      GRBModel model = new GRBModel(env);

      model.ModelName = "assignment";

      // Assignment variables: x[w][s] == 1 if worker w is assigned
      // to shift s. This is no longer a pure assignment model, so we must
      // use binary variables.
      GRBVar[,] x = new GRBVar[nWorkers, nShifts];
      for (int w = 0; w < nWorkers; ++w) {
        for (int s = 0; s < nShifts; ++s) {
          x[w,s] =
              model.AddVar(0, availability[w,s], 0, GRB.BINARY,
                           Workers[w] + "." + Shifts[s]);
        }
      }

      // Slack variables for each shift constraint so that the shifts can
      // be satisfied
      GRBVar[] slacks = new GRBVar[nShifts];
      for (int s = 0; s < nShifts; ++s) {
        slacks[s] =
          model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS,
                       Shifts[s] + "Slack");
      }

      // Variable to represent the total slack
      GRBVar totSlack = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS,
                                     "totSlack");

      // Variables to count the total shifts worked by each worker
      GRBVar[] totShifts = new GRBVar[nWorkers];
      for (int w = 0; w < nWorkers; ++w) {
        totShifts[w] = model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS,
                                    Workers[w] + "TotShifts");
      }

      GRBLinExpr lhs;

      // Constraint: assign exactly shiftRequirements[s] workers
      // to each shift s, plus the slack
      for (int s = 0; s < nShifts; ++s) {
        lhs = new GRBLinExpr();
        lhs.AddTerm(1.0, slacks[s]);
        for (int w = 0; w < nWorkers; ++w) {
          lhs.AddTerm(1.0, x[w, s]);
        }
        model.AddConstr(lhs == shiftRequirements[s], Shifts[s]);
      }

      // Constraint: set totSlack equal to the total slack
      lhs = new GRBLinExpr();
      for (int s = 0; s < nShifts; ++s) {
        lhs.AddTerm(1.0, slacks[s]);
      }
      model.AddConstr(lhs == totSlack, "totSlack");

      // Constraint: compute the total number of shifts for each worker
      for (int w = 0; w < nWorkers; ++w) {
        lhs = new GRBLinExpr();
        for (int s = 0; s < nShifts; ++s) {
          lhs.AddTerm(1.0, x[w, s]);
        }
        model.AddConstr(lhs == totShifts[w], "totShifts" + Workers[w]);
      }

      // Objective: minimize the total slack
      model.SetObjective(1.0*totSlack);

      // Optimize
      int status = solveAndPrint(model, totSlack, nWorkers, Workers, totShifts);
      if (status != GRB.Status.OPTIMAL) {
        return;
      }

      // Constrain the slack by setting its upper and lower bounds
      totSlack.UB = totSlack.X;
      totSlack.LB = totSlack.X;

      // Variable to count the average number of shifts worked
      GRBVar avgShifts =
        model.AddVar(0, GRB.INFINITY, 0, GRB.CONTINUOUS, "avgShifts");

      // Variables to count the difference from average for each worker;
      // note that these variables can take negative values.
      GRBVar[] diffShifts = new GRBVar[nWorkers];
      for (int w = 0; w < nWorkers; ++w) {
        diffShifts[w] = model.AddVar(-GRB.INFINITY, GRB.INFINITY, 0,
                                     GRB.CONTINUOUS, Workers[w] + "Diff");
      }

      // Constraint: compute the average number of shifts worked
      lhs = new GRBLinExpr();
      for (int w = 0; w < nWorkers; ++w) {
        lhs.AddTerm(1.0, totShifts[w]);
      }
      model.AddConstr(lhs == nWorkers * avgShifts, "avgShifts");

      // Constraint: compute the difference from the average number of shifts
      for (int w = 0; w < nWorkers; ++w) {
        model.AddConstr(totShifts[w] - avgShifts == diffShifts[w],
                        Workers[w] + "Diff");
      }

      // Objective: minimize the sum of the square of the difference from the
      // average number of shifts worked
      GRBQuadExpr qobj = new GRBQuadExpr();
      for (int w = 0; w < nWorkers; ++w) {
        qobj.AddTerm(1.0, diffShifts[w], diffShifts[w]);
      }
      model.SetObjective(qobj);

      // Optimize
      status = solveAndPrint(model, totSlack, nWorkers, Workers, totShifts);
      if (status != GRB.Status.OPTIMAL) {
        return;
      }

      // Dispose of model and env
      model.Dispose();
      env.Dispose();

    } catch (GRBException e) {
      Console.WriteLine("Error code: " + e.ErrorCode + ". " +
          e.Message);
    }
  }

  private static int solveAndPrint(GRBModel model, GRBVar totSlack,
                                   int nWorkers, String[] Workers,
                                   GRBVar[] totShifts)
  {
    model.Optimize();
    int status = model.Status;
    if ((status == GRB.Status.INF_OR_UNBD) ||
        (status == GRB.Status.INFEASIBLE) ||
        (status == GRB.Status.UNBOUNDED)) {
      Console.WriteLine("The model cannot be solved "
          + "because it is infeasible or unbounded");
      return status;
    }
    if (status != GRB.Status.OPTIMAL) {
      Console.WriteLine("Optimization was stopped with status " + status);
      return status;
    }

    // Print total slack and the number of shifts worked for each worker
    Console.WriteLine("\nTotal slack required: " + totSlack.X);
    for (int w = 0; w < nWorkers; ++w) {
      Console.WriteLine(Workers[w] + " worked " +
                        totShifts[w].X + " shifts");
    }
    Console.WriteLine("\n");
    return status;
  }
}
