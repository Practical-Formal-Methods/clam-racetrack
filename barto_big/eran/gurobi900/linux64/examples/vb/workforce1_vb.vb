' Copyright 2019, Gurobi Optimization, LLC
'
' Assign workers to shifts; each worker may or may not be available on a
' particular day. If the problem cannot be solved, use IIS to find a set of
' conflicting constraints. Note that there may be additional conflicts
' besides what is reported via IIS.

Imports System
Imports Gurobi

Class workforce1_vb
    Shared Sub Main()
        Try

            ' Sample data
            ' Sets of days and workers
            Dim Shifts As String() = New String() {"Mon1", "Tue2", "Wed3", "Thu4", _
                                                   "Fri5", "Sat6", "Sun7", "Mon8", _
                                                   "Tue9", "Wed10", "Thu11", _
                                                   "Fri12", "Sat13", "Sun14"}
            Dim Workers As String() = New String() {"Amy", "Bob", "Cathy", "Dan", _
                                                    "Ed", "Fred", "Gu"}

            Dim nShifts As Integer = Shifts.Length
            Dim nWorkers As Integer = Workers.Length

            ' Number of workers required for each shift
            Dim shiftRequirements As Double() = New Double() {3, 2, 4, 4, 5, 6, _
                                                              5, 2, 2, 3, 4, 6, _
                                                              7, 5}

            ' Amount each worker is paid to work one shift
            Dim pay As Double() = New Double() {10, 12, 10, 8, 8, 9, 11}

            ' Worker availability: 0 if the worker is unavailable for a shift
            Dim availability As Double(,) = New Double(,) { _
                       {0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1}, _
                       {1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0}, _
                       {0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1}, _
                       {0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1}, _
                       {1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1}, _
                       {1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1}, _
                       {1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}

            ' Model
            Dim env As New GRBEnv()
            Dim model As New GRBModel(env)

            model.ModelName = "assignment"

            ' Assignment variables: x(w)(s) == 1 if worker w is assigned
            ' to shift s. Since an assignment model always produces integer
            ' solutions, we use continuous variables and solve as an LP.
            Dim x As GRBVar(,) = New GRBVar(nWorkers - 1, nShifts - 1) {}
            For w As Integer = 0 To nWorkers - 1
                For s As Integer = 0 To nShifts - 1
                    x(w, s) = model.AddVar(0, availability(w, s), pay(w), _
                                           GRB.CONTINUOUS, _
                                           Workers(w) & "." & Shifts(s))
                Next
            Next

            ' The objective is to minimize the total pay costs
            model.ModelSense = GRB.MINIMIZE

            ' Constraint: assign exactly shiftRequirements(s) workers
            ' to each shift s
            For s As Integer = 0 To nShifts - 1
                Dim lhs As GRBLinExpr = 0
                For w As Integer = 0 To nWorkers - 1
                    lhs.AddTerm(1.0, x(w, s))
                Next
                model.AddConstr(lhs = shiftRequirements(s), Shifts(s))
            Next

            ' Optimize
            model.Optimize()
            Dim status As Integer = model.Status
            If status = GRB.Status.UNBOUNDED Then
                Console.WriteLine("The model cannot be solved " & _
                                  "because it is unbounded")
                Exit Sub
            End If
            If status = GRB.Status.OPTIMAL Then
                Console.WriteLine("The optimal objective is " & model.ObjVal)
                Exit Sub
            End If
            If (status <> GRB.Status.INF_OR_UNBD) AndAlso _
               (status <> GRB.Status.INFEASIBLE) Then
                Console.WriteLine("Optimization was stopped with status " & status)
                Exit Sub
            End If

            ' Do IIS
            Console.WriteLine("The model is infeasible; computing IIS")
            model.ComputeIIS()
            Console.WriteLine(vbLf & "The following constraint(s) " & _
                              "cannot be satisfied:")
            For Each c As GRBConstr In model.GetConstrs()
                If c.IISConstr = 1 Then
                    Console.WriteLine(c.ConstrName)
                End If

            Next

            ' Dispose of model and env
            model.Dispose()
            env.Dispose()

        Catch e As GRBException
            Console.WriteLine("Error code: " & e.ErrorCode & ". " & e.Message)
        End Try
    End Sub
End Class
