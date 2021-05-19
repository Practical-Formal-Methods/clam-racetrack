/* Copyright 2019, Gurobi Optimization, LLC

 This example formulates and solves the following simple model
 with PWL constraints:

  maximize
        sum c[j] * x[j]
  subject to
        sum A[i,j] * x[j] <= 0,  for i = 0, ..., m-1
        sum y[j] <= 3
        y[j] = pwl(x[j]),        for j = 0, ..., n-1
        x[j] free, y[j] >= 0,    for j = 0, ..., n-1
  where pwl(x) = 0,     if x  = 0
               = 1+|x|, if x != 0

  Note
   1. sum pwl(x[j]) <= b is to bound x vector and also to favor sparse x vector.
      Here b = 3 means that at most two x[j] can be nonzero and if two, then
      sum x[j] <= 1
   2. pwl(x) jumps from 1 to 0 and from 0 to 1, if x moves from negative 0 to 0,
      then to positive 0, so we need three points at x = 0. x has infinite bounds
      on both sides, the piece defined with two points (-1, 2) and (0, 1) can
      extend x to -infinite. Overall we can use five points (-1, 2), (0, 1),
      (0, 0), (0, 1) and (1, 2) to define y = pwl(x)
*/

import gurobi.*;
import java.util.*;

public class GCPWL {

  public static void main(String[] args) {
    try {
      int n = 5;
      int m = 5;
      double c[] = { 0.5, 0.8, 0.5, 0.1, -1 };
      double A[][] = { {0, 0, 0, 1, -1},
                       {0, 0, 1, 1, -1},
                       {1, 1, 0, 0, -1},
                       {1, 0, 1, 0, -1},
                       {1, 0, 0, 1, -1} };
      double xpts[] = {-1, 0, 0, 0, 1};
      double ypts[] = {2, 1, 0, 1, 2};

      // Env and model
      GRBEnv env = new GRBEnv();
      GRBModel model = new GRBModel(env);
      model.set(GRB.StringAttr.ModelName, "GCPWL");

      // Add variables, set bounds and obj coefficients
      GRBVar[] x = model.addVars(n, GRB.CONTINUOUS);
      for (int i = 0; i < n; i++) {
        x[i].set(GRB.DoubleAttr.LB, -GRB.INFINITY);
        x[i].set(GRB.DoubleAttr.Obj, c[i]);
      }

      GRBVar[] y = model.addVars(n, GRB.CONTINUOUS);

      // Set objective to maximize
      model.set(GRB.IntAttr.ModelSense, GRB.MAXIMIZE);

      // Add linear constraints
      for (int i = 0; i < m; i++) {
        GRBLinExpr le = new GRBLinExpr();
        for (int j = 0; j < n; j++) {
          le.addTerm(A[i][j], x[j]);
        }
        model.addConstr(le, GRB.LESS_EQUAL, 0, "cx" + i);
      }

      GRBLinExpr le1 = new GRBLinExpr();
      for (int j = 0; j < n; j++) {
        le1.addTerm(1.0, y[j]);
      }
      model.addConstr(le1, GRB.LESS_EQUAL, 3, "cy");

      // Add piecewise constraints
      for (int j = 0; j < n; j++) {
        model.addGenConstrPWL(x[j], y[j], xpts, ypts, "pwl" + j);
      }

      // Optimize model
      model.optimize();

      for (int j = 0; j < n; j++) {
        System.out.println("x[" + j + "] = " + x[j].get(GRB.DoubleAttr.X));
      }
      System.out.println("Obj: " + model.get(GRB.DoubleAttr.ObjVal));

      // Dispose of model and environment
      model.dispose();
      env.dispose();
    } catch (GRBException e) {
      System.out.println("Error code: " + e.getErrorCode() + ". " +
          e.getMessage());
    }
  }
}
